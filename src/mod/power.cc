
/*
 * daska/mod/power.cc
 * copyright (c) 2015 Frano Perleta
 * see the file LICENCE.txt for copyright information.
 */

#include <string>
#include <fcntl.h>
#include <unistd.h>
#include "ui.h"
#include "draw.h"
#include "mod/power.h"

using namespace std;
using namespace mod;
using namespace view;
using namespace draw;

// constructor {{{

power::power
    ( gauge_cell_ptr cell_
    , unsigned seconds )
    : cell (cell_)
{
    /* ac online */ {
        ac_fd = open ("/sys/class/power_supply/ADP1/online", O_RDONLY);
    }

    /* bat energy */ {
        bat_fd = open ("/sys/class/power_supply/BAT0/energy_now", O_RDONLY);
    }

    /* bat full */ {
        int fd = open ("/sys/class/power_supply/BAT0/energy_full", O_RDONLY);
        char buf[16] {0};
        read (fd, buf, 15);
        close (fd);
        bat_full = stoul (buf);
    }

    timer.set<power, &power::timer_cb> (this);
    timer.set (0, seconds);
    timer_cb (timer, 0);
}

// }}}

// destructor {{{

power::~power ()
{
    close (ac_fd);
    close (bat_fd);
    timer.stop ();
}

// }}}

// timer callback {{{

void
power::timer_cb (ev::timer&, int)
{
    /* ac online */ {
        char buf[4] {0};
        pread (ac_fd, buf, 3, 0);
        ac_online = stoul (buf);
    }

    /* bat energy */ {
        char buf[16] {0};
        pread (bat_fd, buf, 15, 0);
        unsigned bat_now = stoul (buf);
        bat_energy = bat_now / (double) bat_full;
    }

    cell->value = bat_energy;
    cell->fgcolor
        = ac_online
        ? color::base1
        : (bat_energy <= low_mark)
        ? color::red
        : (bat_energy <= high_mark)
        ? color::orange
        : color::green;
    cell->dirty =  true;

    timer.again ();
}

// }}}

// vim:fdm=marker:
