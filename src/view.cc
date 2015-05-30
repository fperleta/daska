
/*
 * daska/view.cc
 * copyright (c) 2015 Frano Perleta
 * see the file LICENCE.txt for copyright information.
 */

#include <pango/pangocairo.h>
#include "config.h"
#include "view.h"

using namespace std;
using namespace draw;
using namespace view;

// text cell {{{

void
text_cell::predraw (pango& pg)
{
    auto size = pg.pixel_size (*font, text);
    width = size.first;
    height = size.second;
}

void
text_cell::draw (cairo& xr, pango& pg)
{
    xr.source (bgcolor);
    xr.paint ();

    xr.source (fgcolor);
    xr.move_to (0, -height/2.0);
    pg.draw (xr, *font, text);
}

// }}}

// gauge cell {{{

void
gauge_cell::predraw (pango&)
{
}

void
gauge_cell::draw (cairo& xr, pango&)
{
    xr.source (bgcolor);
    xr.paint ();

    xr.line_width (1);
    xr.source (fgcolor);
    xr.rectangle (0.5, -height/2.0 + 0.5, width, height);
    xr.stroke ();
    xr.rectangle (0, -height/2.0, width * value, height);
    xr.fill ();
}

// }}}

// bar {{{

bar::bar ()
{
}

// }}}

// vim:fdm=marker:
