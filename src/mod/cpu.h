
/*
 * daska/mod/cpu.h
 * copyright (c) 2015 Frano Perleta
 */

#ifndef DASKA_MOD_CPU_H
#define DASKA_MOD_CPU_H

#include <string>
#include <ev++.h>
#include "common.h"
#include "view.h"

namespace mod {

    struct cpu {
        view::text_cell_ptr cell;
        std::vector<std::pair<uint64_t, uint64_t>> cpus;
        double cold_mark = 0.5;
        double hot_mark = 0.75;

        cpu (view::text_cell_ptr, unsigned = 2);
        ~cpu ();

      private:
        ev::timer timer;
        void timer_cb (ev::timer&, int);
        int fd;
    };

};

#endif /* daska/mod/cpu.h */

// vim:fdm=marker:
