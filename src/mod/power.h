
/*
 * daska/mod/power.h
 * copyright (c) 2015 Frano Perleta
 */

#ifndef DASKA_MOD_POWER_H
#define DASKA_MOD_POWER_H

#include <ev++.h>
#include "common.h"
#include "view.h"

namespace mod {

    struct power {
        ev::timer timer;
        view::gauge_cell_ptr cell;
        bool ac_online = false;
        double bat_energy = 0;
        double low_mark = 0.25;
        double high_mark = 0.75;

        power (view::gauge_cell_ptr, unsigned = 2);
        ~power ();

      private:
        void timer_cb (ev::timer&, int);
        int ac_fd;
        int bat_fd;
        unsigned bat_full;
    };

};

#endif /* daska/mod/power.h */

// vim:fdm=marker:
