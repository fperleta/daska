
/*
 * daska/mod/net.cc
 * copyright (c) 2015 Frano Perleta
 */

#include <fcntl.h>
#include <unistd.h>
#include "ui.h"
#include "draw.h"
#include "mod/net.h"

using namespace std;
using namespace mod;
using namespace view;
using namespace draw;

// utilities {{{

inline uint32_t
readull (int fd)
{
    char buf[64] {0};
    pread (fd, buf, 63, 0);
    uint64_t x = stoull (buf);
    return x;
}

// }}}

// constructor {{{

net::net
    ( text_cell_ptr cell_
    , const string& iface_
    , unsigned seconds )
    : cell (cell_), iface (iface_)
{
    const auto pfx = "/sys/class/net/"s;

    /* rx bytes */ {
        string path {pfx};
        path.append (iface);
        path.append ("/statistics/rx_bytes");
        rx_fd = open (path.c_str (), O_RDONLY);
        rx_bytes = readull (rx_fd);
    }

    /* tx bytes */ {
        string path {pfx};
        path.append (iface);
        path.append ("/statistics/tx_bytes");
        tx_fd = open (path.c_str (), O_RDONLY);
        tx_bytes = readull (tx_fd);
    }

    /* operstate */ {
        string path {pfx};
        path.append (iface);
        path.append ("/operstate");
        os_fd = open (path.c_str (), O_RDONLY);
    }

    last = ev::now (EV_DEFAULT);

    timer.set<net, &net::timer_cb> (this);
    timer.set (0, seconds);
    timer_cb (timer, 0);
}

// }}}

// destructor {{{

net::~net ()
{
    close (rx_fd);
    close (tx_fd);
    close (os_fd);
    timer.stop ();
}

// }}}

// timer callback {{{

void
net::timer_cb (ev::timer&, int)
{
    /* operstate */ {
        char buf[16];
        pread (os_fd, buf, 15, 0);
        up = (buf[0] == 'u') && (buf[1] == 'p');
    }

    uint64_t rx_now = readull (rx_fd);
    uint64_t tx_now = readull (tx_fd);

    double here = ev::now (EV_DEFAULT);
    double dt = here - last;

    double rx_speed = (rx_now - rx_bytes) / (1024 * dt);
    double tx_speed = (tx_now - tx_bytes) / (1024 * dt);

    rx_bytes = rx_now;
    tx_bytes = tx_now;
    last = here;

    string buf {iface};
    buf += " ↓"; buf += to_string ((unsigned) rx_speed);
    buf += " ↑"; buf += to_string ((unsigned) tx_speed);
    cell->text = buf;
    cell->visible = up;
    cell->dirty = true;

    timer.again ();
}

// }}}

// vim:fdm=marker:
