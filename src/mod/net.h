
/*
 * daska/mod/net.h
 * copyright (c) 2015 Frano Perleta
 */

#ifndef DASKA_MOD_NET_H
#define DASKA_MOD_NET_H

#include <string>
#include <ev++.h>
#include "common.h"
#include "view.h"

namespace mod {

    struct net {
        view::text_cell_ptr cell;
        std::string iface;
        uint64_t rx_bytes;
        uint64_t tx_bytes;
        ev::tstamp last;
        bool up = false;

        net (view::text_cell_ptr, const std::string&, unsigned = 2);
        ~net ();

      private:
        ev::timer timer;
        void timer_cb (ev::timer&, int);
        int rx_fd;
        int tx_fd;
        int os_fd;
    };

};

#endif /* daska/mod/net.h */

// vim:fdm=marker:
