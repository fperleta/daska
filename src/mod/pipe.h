
/*
 * daska/mod/pipe.h
 * copyright (c) 2015 Frano Perleta
 * see the file LICENCE.txt for copyright information.
 */

#ifndef DASKA_MOD_PIPE_H
#define DASKA_MOD_PIPE_H

#include <string>
#include <ev++.h>
#include "common.h"
#include "view.h"

namespace mod {

    struct pipe {
        int fd;
        ev::io fd_r;
        std::string buf;
        view::text_cell_ptr cell;

        pipe (int, view::text_cell_ptr);
        ~pipe ();

      private:
        void fd_cb (ev::io&, int);
    };

};

#endif /* daska/mod/pipe.h */

// vim:fdm=marker:
