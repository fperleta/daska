
/*
 * daska/mod/cpu.cc
 * copyright (c) 2015 Frano Perleta
 * see the file LICENCE.txt for copyright information.
 */

#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include "ui.h"
#include "draw.h"
#include "mod/cpu.h"

using namespace std;
using namespace mod;
using namespace view;
using namespace draw;

// constructor {{{

cpu::cpu
    ( text_cell_ptr cell_
    , unsigned seconds )
    : cell (cell_)
{
    fd = open ("/proc/stat", O_RDONLY);

    timer.set<cpu, &cpu::timer_cb> (this);
    timer.set (0, seconds);
    timer_cb (timer, 0);
}

// }}}

// destructor {{{

cpu::~cpu ()
{
    timer.stop ();
    close (fd);
}

// }}}

// timer callback {{{

void
cpu::timer_cb (ev::timer&, int)
{
    char buf[4096];
    string::size_type len = pread (fd, buf, 4096, 0);
    if (len == (unsigned) -1)
        return;

    vector<pair<uint64_t, uint64_t>> data;

    {
        string str {buf, len};
        size_t offs = 0;
        do {
            if (str.substr (offs, 3) != "cpu")
                break;
            offs += 4;

            uint64_t sum = 0, idle = 0;
            for (size_t i = 0; i < 10; i++) {
                offs = str.find (' ', offs);
                size_t d;
                uint64_t x = stoull (str.substr (offs, len - offs), &d);
                sum += x;
                idle += (i == 3)? x : 0;
                offs += d;
            }
            offs = str.find ('\n', offs) + 1;
            data.push_back ({sum, idle});
        } while (offs < len);
    }

    vector<double> usage;

    for (size_t i = 0; i < data.size (); i++) {
        auto& d1 = data[i];
        if (i >= cpus.size ())
            cpus.push_back (d1);
        auto& d0 = cpus[i];

        auto u = 1.0 - (d1.second - (double) d0.second)
                     / (d1.first - (double) d0.first);
        usage.push_back (u);
    }

    cpus = data;

    string out;
    size_t col = 0;
    for (auto u : usage) {
        if (col > 1)
            out.push_back (' ');
        if (col > 0)
            out += to_string ((unsigned) ceil (100 * u));
        col++;
    }
    cell->text = out;
    bool cold = usage[0] <= cold_mark;
    bool hot = usage[0] >= hot_mark;
    cell->bgcolor
        = hot
        ? rgba {color::red, 0.9}
        : cold
        ? rgba {color::base03, 0.8}
        : rgba {color::orange, 0.9};
    cell->fgcolor
        = cold
        ? color::base1
        : color::base03;
    cell->dirty = true;

    timer.again ();
}

// }}}

// vim:fdm=marker:
