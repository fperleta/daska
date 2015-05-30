
/*
 * daska/mod/mem.cc
 * copyright (c) 2015 Frano Perleta
 * see the file LICENCE.txt for copyright information.
 */

#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include "ui.h"
#include "draw.h"
#include "mod/mem.h"

using namespace std;
using namespace mod;
using namespace view;
using namespace draw;

// constructor {{{

mem::mem
    ( text_cell_ptr cell_
    , unsigned seconds )
    : cell (cell_)
{
    fd = open ("/proc/meminfo", O_RDONLY);

    timer.set<mem, &mem::timer_cb> (this);
    timer.set (0, seconds);
    timer_cb (timer, 0);
}

// }}}

// destructor {{{

mem::~mem ()
{
    timer.stop ();
    close (fd);
}

// }}}

// timer callback {{{

void
mem::timer_cb (ev::timer&, int)
{
    char buf[1024];
    string::size_type len = pread (fd, buf, 1024, 0);
    if (len == (unsigned) -1)
        return;

    uint64_t total_kb;
    uint64_t avail_kb;
    /* parse */ {
        string str {buf, len};
        size_t offs = 1 + str.find (':');
        total_kb = stoull (str.substr (offs, 20));
        offs = 1 + str.find (':', offs);
        offs = 1 + str.find (':', offs);
        avail_kb = stoull (str.substr (offs, 20));
    }

    usage = 1.0 - avail_kb / (double) total_kb;

    cell->text = to_string ((unsigned) ceil (100 * usage));
    bool tight = usage >= tight_mark;
    cell->bgcolor
        = tight
        ? rgba {color::red, 0.9}
        : rgba {color::base03, 0.8};
    cell->fgcolor
        = tight
        ? color::base03
        : color::base1;
    cell->dirty = true;

    timer.again ();
}

// }}}

// vim:fdm=marker:
