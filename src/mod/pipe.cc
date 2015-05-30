
/*
 * daska/mod/pipe.cc
 * copyright (c) 2015 Frano Perleta
 * see the file LICENCE.txt for copyright information.
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "mod/pipe.h"

using namespace std;
using namespace mod;

// constructor {{{

pipe::pipe (int fd_, view::text_cell_ptr cell_)
    : fd {fd_}
    , fd_r {}
    , buf {}
    , cell {cell_}
{
    fd_r.set<pipe, &pipe::fd_cb> (this);
    fd_r.set (fd, ev::READ);
    fd_r.start ();
}

// }}}

// destructor {{{

pipe::~pipe ()
{
    fd_r.stop ();
}

// }}}

// read callback {{{

void
pipe::fd_cb (ev::io&, int)
{
    char bytes[1024];
    ssize_t len = read (fd, bytes, 1024);
    if (len == -1)
        panic ("read pipe: %s", strerror (errno));
    buf.append (bytes, len);

    auto last = buf.rfind ('\n');
    if (last == buf.npos)
        return;

    auto prev = buf.rfind ('\n', last - 1);
    auto start = (prev == buf.npos)? 0 : prev + 1;
    cell->text = buf.substr (start, last - start);
    cell->dirty = true;
    buf.erase (0, last);
    //cfg.vsay ("pipe: %s", cell->text.c_str ());
}

// }}}

// vim:fdm=marker:
