
/*
 * daska/ui.cc
 * copyright (c) 2015 Frano Perleta
 */

#include "common.h"
#include "config.h"
#include "draw.h"
#include "view.h"
#include "ui.h"

using namespace std;

// conn {{{

// constructor {{{

static void
xfd_cb (struct ev_loop* loop UNUSED, ev_io* io, int revents UNUSED)
{
    ui::conn* self = static_cast<ui::conn*> (static_cast<void*> (io));
    self->drain ();
}

ui::conn::conn
    ( struct ev_loop* loop_
    , const char* display
    , int screen
    )
    : loop (loop_)
{
    xconn = xcb_connect (display, &screen);
    if (has_error ())
        panic ("couldn't connect to X!");

    xscrn = (xcb_setup_roots_iterator (xcb_get_setup (xconn))).data;

    prefetch_atoms ({"WM_PROTOCOLS", "WM_DELETE_WINDOW"});

    int xfd = xcb_get_file_descriptor (xconn);
    ev_io_init (&xfd_w, xfd_cb, xfd, EV_READ);
    ev_io_start (loop, &xfd_w);

    uint32_t mask = 0;
    uint32_t values[1];

    mask |= XCB_CW_EVENT_MASK;
    values[0] = XCB_EVENT_MASK_PROPERTY_CHANGE;

    xcb_change_window_attributes
        ( xconn, xscrn->root, mask, values );
}

// }}}

// destructor {{{

ui::conn::~conn ()
{
    ev_io_stop (loop, &xfd_w);

    xcb_flush (xconn);
    xcb_disconnect (xconn);
}

// }}}

// atoms {{{

void
ui::conn::prefetch_atoms (vector<string> names)
{
    vector<xcb_intern_atom_cookie_t> cookies;
    cookies.reserve (names.size ());

    for (auto name : names) {
        auto x = xcb_intern_atom (xconn, 0, name.length (), name.c_str ());
        cookies.push_back (x);
    }

    xcb_flush (xconn);

    for (size_t i = 0; i < names.size (); i++) {
        auto reply = xcb_intern_atom_reply (xconn, cookies[i], nullptr);
        if (!reply)
            panic ("xcb_intern_atom (\"%s\") failed.", names[i].c_str ());
        atoms[names[i]] = reply->atom;
        free (reply);
    }
}

xcb_atom_t
ui::conn::atom (const std::string& name)
{
    auto x = atoms.find (name);
    if (x != atoms.end ())
        return x->second;
    prefetch_atoms ({name});
    return atoms[name];
}

// }}}

// drain events {{{

void
ui::conn::drain ()
{
    xcb_generic_event_t* ev = nullptr;

    if (xcb_connection_has_error (xconn))
        panic ("X connection has errors.");

    while ((ev = xcb_poll_for_event (xconn))) {
        switch (ev->response_type & ~0x80)
        {
            case XCB_CONFIGURE_NOTIFY:
                {
                    auto* cn_ev = (xcb_configure_notify_event_t*) ev;
                    window* w = windows[cn_ev->window];
                    w->r = ui::rect
                        { cn_ev->x
                        , cn_ev->y
                        , cn_ev->width
                        , cn_ev->height
                        };
                    w->reposition ();
                }
                break;

            case XCB_EXPOSE:
                {
                    auto* e_ev = (xcb_expose_event_t*) ev;
                    window* w = windows[e_ev->window];
                    rect r {(int16_t) e_ev->x, (int16_t) e_ev->y, e_ev->width, e_ev->height};
                    w->expose (r);
                }
                break;

            case XCB_PROPERTY_NOTIFY:
                {
                    auto* pn_ev = (xcb_property_notify_event_t*) ev;
                    string buf;
                    bool got = false;
                    for (auto* w : watchers) {
                        if ((w->xw != pn_ev->window) || (w->xprop != pn_ev->atom))
                            continue;
                        if (!got) {
                            got = true;
                            buf = get_prop (w->xw, w->xprop, w->xtype);
                        }
                        w->update (buf);
                    }
                }
                break;

            default:
                break;
        }
        free (ev);
    }
}

// }}}

// properties {{{

string
ui::conn::get_prop (xcb_window_t xw, xcb_atom_t xprop, xcb_atom_t xtype)
{
    auto cookie = xcb_get_property (xconn, 0, xw, xprop, xtype, 0, 1024);
    auto reply = xcb_get_property_reply (xconn, cookie, nullptr);
    auto buf = string
        ( (const char*) xcb_get_property_value (reply)
          , xcb_get_property_value_length (reply) );
    free (reply);
    return buf;
}

// }}}

// }}}

// window {{{

// constructor {{{

ui::window::window (ui::conn& c_, ui::rect r_)
    : c (c_)
    , r (r_)
{
    xw = xcb_generate_id (c.xconn);

    c.windows[xw] = this;

    /* create the window */ {
        uint32_t mask = 0;
        uint32_t values[3];

        mask |= XCB_CW_BACK_PIXMAP;
        values[0] = XCB_BACK_PIXMAP_PARENT_RELATIVE;

        mask |= XCB_CW_OVERRIDE_REDIRECT;
        values[1] = 1;

        mask |= XCB_CW_EVENT_MASK;
        values[2]
            = XCB_EVENT_MASK_EXPOSURE
            | XCB_EVENT_MASK_VISIBILITY_CHANGE;

        xcb_create_window (c.xconn, XCB_COPY_FROM_PARENT, xw, c.xscrn->root,
                r.x, r.y, r.w, r.h, 0,
                XCB_WINDOW_CLASS_INPUT_OUTPUT,
                c.xscrn->root_visual,
                mask, values);
    }

    xcb_flush (c.xconn);
}

// }}}

// destructor {{{

ui::window::~window ()
{
    c.windows.erase (xw);
    xcb_destroy_window (c.xconn, xw);
    xcb_flush (c.xconn);
}

// }}}

// mapping {{{

void
ui::window::map ()
{
    xcb_map_window (c.xconn, xw);
    xcb_flush (c.xconn);
    mapped = true;
    reposition ();
}

// }}}

// }}}

// cairo_window {{{

// constructor {{{

ui::cairo_window::cairo_window (conn& c_, rect r_)
    : window (c_, r_)
{
    xgc = xcb_generate_id (c.xconn);
    xpix = xcb_generate_id (c.xconn);

    /* create the gc */ {
        uint32_t mask
            = XCB_GC_FOREGROUND
            | XCB_GC_BACKGROUND;
        uint32_t values[2] {
            c.xscrn->black_pixel,
            c.xscrn->white_pixel
        };

        xcb_create_gc (c.xconn, xgc, xw, mask, values);
    }

    xcb_visualtype_t* vt = nullptr;

    /* get visual type */ {
        xcb_depth_iterator_t diter = xcb_screen_allowed_depths_iterator (c.xscrn);
        for (; diter.rem; xcb_depth_next (&diter))
        {
            xcb_visualtype_iterator_t viter = xcb_depth_visuals_iterator (diter.data);
            for (; viter.rem; xcb_visualtype_next (&viter))
                if (c.xscrn->root_visual == viter.data->visual_id)
                    vt = viter.data;
        }
    }

    if (vt == nullptr)
        panic ("couldn't get visual type");

    xcb_create_pixmap
        ( c.xconn
        , c.xscrn->root_depth
        , xpix, xw
        , r.w, r.h
        );
    xcb_rectangle_t rect {r.x, r.y, r.w, r.h};
    xcb_poly_fill_rectangle (c.xconn, xpix, xgc, 1, &rect);

    surf = cairo_xcb_surface_create (c.xconn, xpix, vt, r.w, r.h);
}

// }}}

// destructor {{{

ui::cairo_window::~cairo_window ()
{
    cairo_surface_destroy (surf);
    xcb_free_pixmap (c.xconn, xpix);
    xcb_free_gc (c.xconn, xgc);
}

// }}}

// expose {{{

void
ui::cairo_window::expose (const rect& r_)
{
    //cfg.say ("expose (%d,%d) (%u,%u)", r_.x, r_.y, r_.w, r_.h);

    cairo_surface_flush (surf);

    /* pseutransparency */ {
        auto cookie = xcb_get_property
            ( c.xconn
            , 0, c.xscrn->root
            , c.atom (std::string {"ESETROOT_PMAP_ID"})
            , XCB_ATOM_PIXMAP
            , 0, 1
            );

        auto reply = xcb_get_property_reply (c.xconn, cookie, nullptr);
        xcb_pixmap_t xroot = *(xcb_pixmap_t*) xcb_get_property_value (reply);
        free (reply);

        xcb_copy_area
            ( c.xconn
            , xroot, xpix, xgc
            , r.x, r.x
            , 0, 0
            , r.w, r.h
            );
        xcb_flush (c.xconn);
    }

    cairo_surface_mark_dirty (surf);
    redraw ();
    cairo_surface_flush (surf);

    xcb_copy_area
        ( c.xconn
        , xpix, xw, xgc
        , r_.x, r_.x
        , r_.x, r_.x
        , r_.w, r_.h
        );

    xcb_flush (c.xconn);
}

// }}}

// }}}

// daska_window {{{

void
ui::daska_window::idle_cb (ev::idle&, int)
{
    if (bar->dirty ())
        expose (r);
    ev_sleep (0.05);
}

ui::daska_window::daska_window
    ( conn& c_
    , view::bar_ptr bar_ )
    : cairo_window {c_, {0, 0, c_.xscrn->width_in_pixels, 14}}
    , bar (bar_)
{
    idle.set<ui::daska_window, &ui::daska_window::idle_cb> (this);
    idle.start ();
}

ui::daska_window::~daska_window ()
{
    idle.stop ();
}

void
ui::daska_window::reposition ()
{
    bar->width = r.w;
    bar->height = r.h;
}

void
ui::daska_window::redraw ()
{
    draw::cairo xr {surf};
    draw::pango pg {xr};

    int loffs = 0, roffs = bar->width;
    int h = bar->height;

    for (auto cell : bar->left) {
        if (!cell->visible) continue;
        if (roffs - loffs < 2 * cell->padding)
            break;

        cell->predraw (pg);

        if (!loffs) {
            xr.move_to (0, 0);
            xr.line_to (0, h);
        } else {
            xr.move_to (loffs + 4, 0);
            xr.curve_to (loffs, 0, loffs, h, loffs - 4, h);
        }

        int loffs_ = loffs + cell->width + 2*cell->padding;
        xr.line_to (loffs_ - 4, h);
        xr.curve_to (loffs_, h, loffs_, 0, loffs_ + 4, 0);
        xr.close_path ();

        xr.clip ();
        xr.save ();
        xr.translate (loffs + cell->padding, h/2.0);
        cell->draw (xr, pg);
        cell->dirty = false;
        xr.restore ();
        xr.reset_clip ();

        loffs = loffs_ - 1;
    }

    for (auto cell : bar->right) {
        if (!cell->visible) continue;
        if (roffs - loffs < 2 * cell->padding)
            break;

        cell->predraw (pg);

        //cfg.vsay ("roffs = %d", roffs);
        if (roffs == bar->width) {
            xr.move_to (roffs, 0);
            xr.line_to (roffs, h);
        } else {
            xr.move_to (roffs - 4, 0);
            xr.curve_to (roffs, 0, roffs, h, roffs + 4, h);
        }

        int roffs_ = roffs - cell->width - 2*cell->padding;
        xr.line_to (roffs_ + 4, h);
        xr.curve_to (roffs_, h, roffs_, 0, roffs_ - 4, 0);
        xr.close_path ();

        xr.clip ();
        xr.save ();
        xr.translate (roffs_ + cell->padding, h/2.0);
        cell->draw (xr, pg);
        cell->dirty = false;
        xr.restore ();
        xr.reset_clip ();

        roffs = roffs_ + 1;
    }

    auto cell = bar->middle;

    if (cell.get () == nullptr)
        return;
    if (!cell->visible)
        return;
    if (roffs - loffs < 2 * cell->padding)
        return;

    cell->predraw (pg);

    if (!loffs) {
        xr.move_to (loffs, 0);
        xr.line_to (loffs, h);
    } else {
        xr.move_to (loffs + 4, 0);
        xr.curve_to (loffs, 0, loffs, h, loffs - 4, h);
    }
    if (roffs == bar->width) {
        xr.line_to (roffs, h);
        xr.line_to (roffs, 0);
    } else {
        xr.line_to (roffs + 4, h);
        xr.curve_to (roffs, h, roffs, 0, roffs - 4, 0);
    }
    xr.close_path ();

    xr.clip ();
    xr.save ();
    xr.translate (loffs + cell->padding, h/2.0);
    cell->draw (xr, pg);
    cell->dirty = false;
    xr.restore ();
    xr.reset_clip ();
}

// }}}

// vim:fdm=marker:
