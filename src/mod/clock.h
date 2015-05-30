
/*
 * daska/mod/clock.h
 * copyright (c) 2015 Frano Perleta
 * see the file LICENCE.txt for copyright information.
 */

#ifndef DASKA_MOD_CLOCK_H
#define DASKA_MOD_CLOCK_H

#include <string>
#include <ev++.h>
#include "common.h"
#include "view.h"

namespace mod {

    struct clock {
        ev::timer timer;
        view::text_cell_ptr cell;
        std::string format = "%F %a %H:%M:%S";

        clock (view::text_cell_ptr);
        ~clock ();

      private:
        void timer_cb (ev::timer&, int);
    };

};

#endif /* daska/mod/clock.h */

// vim:fdm=marker:
