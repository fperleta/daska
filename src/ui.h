
/*
 * daska/ui.h
 * copyright (c) 2015 Frano Perleta
 */

#ifndef DASKA_UI_H
#define DASKA_UI_H

#include <string>
#include <map>
#include <vector>
#include <ev++.h>
#include <xcb/xcb.h>
#include <cairo-xcb.h>
#include "common.h"
#include "view.h"

namespace ui {
    struct window;
    struct prop_watcher;

    // conn {{{

    struct conn {
        struct ev_io xfd_w;
        struct ev_loop* loop;
        xcb_connection_t* xconn = nullptr;
        xcb_screen_t* xscrn = nullptr;
        std::map<xcb_window_t, window*> windows;
        std::vector<prop_watcher*> watchers;

        conn ( struct ev_loop* loop_
             , const char* display = nullptr
             , int screen = 0
             );
        ~conn ();

        inline bool has_error () {
            return xcb_connection_has_error (xconn);
        }

        xcb_atom_t atom (const std::string& name);
        void prefetch_atoms (std::vector<std::string> names);

        void drain ();

        std::string get_prop (xcb_window_t, xcb_atom_t, xcb_atom_t);

      private:
        std::map<std::string, xcb_atom_t> atoms;
    };

    // }}}

    // prop_watcher {{{

    struct prop_watcher {
        xcb_window_t xw;
        xcb_atom_t xprop;
        xcb_atom_t xtype;
        virtual void update (const std::string&) = 0;
    };

    // }}}

    // window {{{

    struct rect {
        int16_t x, y;
        uint16_t w, h;

        rect (int16_t x_, int16_t y_, uint16_t w_, uint16_t h_)
            : x (x_), y (y_), w (w_), h (h_) {}
    };

    struct window {
        conn& c;
        xcb_window_t xw;
        rect r;
        bool mapped;

        window (conn& c_, rect r_);
        ~window ();

        void map ();

        virtual void reposition () = 0;
        virtual void expose (const rect&) = 0;
    };

    // }}}

    // cairo_window {{{

    struct cairo_window : public window {
        xcb_gcontext_t xgc;
        xcb_pixmap_t xpix;
        cairo_surface_t* surf;

        cairo_window (conn& c_, rect r_);
        ~cairo_window ();

        void expose (const rect&) override;

        virtual void redraw () = 0;
    };

    // }}}

    // daska_window {{{

    struct daska_window : public cairo_window {
        view::bar_ptr bar;
        ev::idle idle;

        daska_window (conn&, view::bar_ptr);
        ~daska_window ();

        void reposition () override;
        void redraw () override;

        void idle_cb (ev::idle&, int);
    };

    // }}}
};

#endif /* daska/ui.h */

// vim:fdm=marker:
