
/*
 * daska/mod/clock.cc
 * copyright (c) 2015 Frano Perleta
 */

#include <ctime>
#include "mod/clock.h"

using namespace std;
using namespace mod;
using namespace view;

// constructor {{{

clock::clock (view::text_cell_ptr cell_)
    : cell {cell_}
{
    timer.set<clock, &clock::timer_cb> (this);
    timer.set (0, 1);
    timer_cb (timer, 0);
}

// }}}

// destructor {{{

clock::~clock ()
{
    timer.stop ();
}

// }}}

// timer callback {{{

void
clock::timer_cb (ev::timer&, int)
{
    char buf[256];
    time_t t = time (nullptr);
    strftime (buf, 255, format.c_str (), localtime (&t));
    cell->text = buf;
    cell->dirty = true;
    timer.again ();
}

// }}}

// vim:fdm=marker:
