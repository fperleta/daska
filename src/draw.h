
/*
 * daska/draw.h
 * copyright (c) 2015 Frano Perleta
 * see the file LICENCE.txt for copyright information.
 */

#ifndef DASKA_DRAW_H
#define DASKA_DRAW_H

#include <string>
#include <tuple>
#include <memory>
#include <cairo.h>
#include <pango/pangocairo.h>
#include "config.h"
#include "common.h"

namespace draw {

    // rgb & rgba {{{

    struct rgb {
        double r, g, b;
        rgb (double r_, double g_, double b_)
            : r (r_), g (g_), b (b_)
            {}
    };

    struct rgba {
        double r, g, b, a;
        rgba (double r_, double g_, double b_, double a_)
            : r (r_), g (g_), b (b_), a (a_)
            {}
        rgba (const rgb& c, double a_ = 1)
            : r (c.r), g (c.g), b (c.b), a (a_)
            {}
    };

    inline rgb rgb8 (uint8_t r_, uint8_t g_, uint8_t b_)
    { return rgb {r_/255.0, g_/255.0, b_/255.0}; }

    inline rgba rgba8 (uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_)
    { return rgba {r_/255.0, g_/255.0, b_/255.0, a_/255.0}; }

    // }}}

    // color scheme {{{

    namespace color {
        const rgb yellow  = rgb8 (0xb5, 0x89, 0x00);
        const rgb orange  = rgb8 (0xcb, 0x4b, 0x16);
        const rgb red     = rgb8 (0xdc, 0x32, 0x2f);
        const rgb magenta = rgb8 (0xd3, 0x36, 0x82);
        const rgb violet  = rgb8 (0x6c, 0x71, 0xc4);
        const rgb blue    = rgb8 (0x26, 0x8b, 0xd2);
        const rgb cyan    = rgb8 (0x2a, 0xa1, 0x98);
        const rgb green   = rgb8 (0x85, 0x99, 0x00);

        const rgb base03  = rgb8 (0x00, 0x2b, 0x36); // dark
        const rgb base02  = rgb8 (0x07, 0x36, 0x42);
        const rgb base01  = rgb8 (0x58, 0x6e, 0x75);
        const rgb base00  = rgb8 (0x65, 0x7b, 0x83);
        const rgb base0   = rgb8 (0x83, 0x94, 0x96);
        const rgb base1   = rgb8 (0x93, 0xa1, 0xa1);
        const rgb base2   = rgb8 (0xee, 0xe8, 0xd5);
        const rgb base3   = rgb8 (0xfd, 0xf6, 0xe3); // light
    };

    // }}}

    // cairo {{{

    struct cairo {
        cairo_t* cr;

        cairo (cairo_surface_t* surf) {
            cr = cairo_create (surf);
        }

        ~cairo () {
            cairo_destroy (cr);
        }

        inline void save () { cairo_save (cr); }
        inline void restore () { cairo_restore (cr); }

        inline void translate (double x, double y)
        { cairo_translate (cr, x, y); }

        inline void clip () { cairo_clip (cr); }
        inline void reset_clip () { cairo_reset_clip (cr); }
        inline void fill () { cairo_fill (cr); }
        inline void paint () { cairo_paint (cr); }
        inline void stroke () { cairo_stroke (cr); }
        inline void close_path () { cairo_close_path (cr); }

        inline void move_to (double x, double y)
        { cairo_move_to (cr, x, y); }
        inline void line_to (double x, double y)
        { cairo_line_to (cr, x, y); }
        inline void curve_to (double x1, double y1, double x2, double y2, double x3, double y3)
        { cairo_curve_to (cr, x1, y1, x2, y2, x3, y3); }
        inline void rectangle (double x, double y, double w, double h)
        { cairo_rectangle (cr, x, y, w, h); }

        inline void source (const rgb& c)
        { cairo_set_source_rgb (cr, c.r, c.g, c.b); }
        inline void source (const rgba& c)
        { cairo_set_source_rgba (cr, c.r, c.g, c.b, c.a); }

        inline void line_width (double w)
        { cairo_set_line_width (cr, w); }
        inline void line_cap (cairo_line_cap_t cap)
        { cairo_set_line_cap (cr, cap); }
        inline void line_join (cairo_line_join_t join)
        { cairo_set_line_join (cr, join); }
    };

    // }}}

    // pango {{{

    struct pango {
        PangoContext* ctx;

        struct font {
            PangoFontDescription* desc;
            font (font&& font_)
                : desc (font_.desc)
                { font_.desc = nullptr; }
            font (const char* spec)
                : desc (pango_font_description_from_string (spec))
                {}
            ~font () {
                if (desc)
                    pango_font_description_free (desc);
            }
        };

        typedef std::shared_ptr<font> font_ptr;
        static inline font_ptr make_font (const char* spec)
        { return std::make_shared<font> (spec); }

        pango (cairo& xr) {
            ctx = pango_cairo_create_context (xr.cr);
        }

        ~pango () {
            g_object_unref (ctx);
        }

        void draw (cairo& xr, font& font, std::string str) {
            PangoLayout* layout = pango_layout_new (ctx);

            pango_layout_set_font_description (layout, font.desc);
            pango_layout_set_text (layout, str.c_str (), -1);
            pango_cairo_show_layout (xr.cr, layout);

            g_object_unref (layout);
        }

        std::pair<int,int> pixel_size (font& font, std::string str) {
            PangoLayout* layout = pango_layout_new (ctx);

            pango_layout_set_font_description (layout, font.desc);
            pango_layout_set_text (layout, str.c_str (), -1);
            int w, h;
            pango_layout_get_pixel_size (layout, &w, &h);

            g_object_unref (layout);
            return std::pair<int,int> {w, h};
        }
    };

    // }}}

};

#endif /* daska/draw.h */

// vim:fdm=marker:
