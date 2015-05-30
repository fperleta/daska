
/*
 * daska/mod/xmonad.h
 * copyright (c) 2015 Frano Perleta
 */

#ifndef DASKA_MOD_XMONAD_H
#define DASKA_MOD_XMONAD_H

#include <string>
#include <ev++.h>
#include "common.h"
#include "ui.h"
#include "view.h"

namespace mod {

    struct xmonad : ui::prop_watcher {
        ui::conn& c;
        typedef view::text_cell_ptr tcell;
        tcell cur_ws;
        tcell pre_ws;
        tcell pst_ws;
        tcell urg_ws;
        tcell state;
        tcell title;

        xmonad (ui::conn&, tcell, tcell, tcell, tcell, tcell, tcell);
        ~xmonad ();

        void update (const std::string&) override;
    };

};

#endif /* daska/mod/xmonad.h */

// vim:fdm=marker:
