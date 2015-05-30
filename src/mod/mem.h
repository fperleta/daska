
/*
 * daska/mod/mem.h
 * copyright (c) 2015 Frano Perleta
 */

#ifndef DASKA_MOD_MEM_H
#define DASKA_MOD_MEM_H

#include <string>
#include <ev++.h>
#include "common.h"
#include "view.h"

namespace mod {

    struct mem {
        view::text_cell_ptr cell;
        double usage = 0;
        double tight_mark = 0.8;

        mem (view::text_cell_ptr, unsigned = 2);
        ~mem ();

      private:
        ev::timer timer;
        void timer_cb (ev::timer&, int);
        int fd;
    };

};

#endif /* daska/mod/mem.h */

// vim:fdm=marker:
