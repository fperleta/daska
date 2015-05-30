
/*
 * daska/view.h
 * copyright (c) 2015 Frano Perleta
 */

#ifndef DASKA_VIEW_H
#define DASKA_VIEW_H

#include <memory>
#include <vector>
#include "common.h"
#include "draw.h"

namespace view {

    // cell {{{

    struct cell {
        bool dirty = true;
        bool visible = true;
        int width = 12, height = 12;
        int padding = 8;
        draw::rgba bgcolor = draw::rgba (draw::color::base03, 0.8);

        virtual void predraw (draw::pango&) = 0;
        virtual void draw (draw::cairo&, draw::pango&) = 0;
    };

    typedef std::shared_ptr<cell> cell_ptr;

    // }}}

    // text cell {{{

    struct text_cell : public cell {
        std::string text {};
        draw::pango::font_ptr font;
        draw::rgba fgcolor = draw::color::base1;

        void predraw (draw::pango&) override;
        void draw (draw::cairo&, draw::pango&) override;
    };

    typedef std::shared_ptr<text_cell> text_cell_ptr;

    // }}}

    // gauge cell {{{

    struct gauge_cell : public cell {
        double value;
        draw::rgba fgcolor = draw::rgba {draw::color::base2, 0.9};

        void predraw (draw::pango&) override;
        void draw (draw::cairo&, draw::pango&) override;
    };

    typedef std::shared_ptr<gauge_cell> gauge_cell_ptr;

    // }}}

    // bar {{{

    struct bar {
        bool visible = true;
        int width = 1024, height = 16;

        std::vector<cell_ptr> left;
        std::vector<cell_ptr> right;
        cell_ptr middle {};

        bar ();

        bool dirty () {
            if (middle.get () && middle->dirty) return true;
            for (auto& cell : left)
                if (cell->dirty) return true;
            for (auto& cell : right)
                if (cell->dirty) return true;
            return false;
        }
    };

    typedef std::shared_ptr<bar> bar_ptr;

    // }}}

};

#endif /* daska/view.h */

// vim:fdm=marker:
