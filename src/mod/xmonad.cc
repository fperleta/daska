
/*
 * daska/mod/xmonad.cc
 * copyright (c) 2015 Frano Perleta
 * see the file LICENCE.txt for copyright information.
 */

#include <algorithm>
#include "ui.h"
#include "mod/xmonad.h"

using namespace std;
using namespace mod;
using namespace view;

// constructor {{{

xmonad::xmonad
    ( ui::conn& c_
    , tcell cur_ws_
    , tcell pre_ws_
    , tcell pst_ws_
    , tcell urg_ws_
    , tcell state_
    , tcell title_ )
    : c (c_)
    , cur_ws (cur_ws_), pre_ws (pre_ws_), pst_ws (pst_ws_), urg_ws (urg_ws_)
    , state (state_), title (title_)
{
    xw = c.xscrn->root;
    xprop = c.atom ("_XMONAD_LOGX");
    xtype = c.atom ("UTF8_STRING");
    c.watchers.push_back (this);
    update (c.get_prop (xw, xprop, xtype));
}

// }}}

// destructor {{{

xmonad::~xmonad ()
{
    remove_if
        ( begin (c.watchers)
        , end (c.watchers)
        , [this] (auto* w) { return w == this; }
        );
}

// }}}

// update {{{

void
xmonad::update (const string& data)
{
    vector<string> fs;

    /* split fields */ {
        size_t offs = 0;
        while (offs < data.length ()) {
            auto delim = data.find ('\t', offs);
            auto len = (delim == data.npos? data.length () : delim) - offs;
            fs.push_back (data.substr (offs, len));
            offs += len + 1;
        }
    }

    auto nfs = fs.size ();

    cur_ws->text = (nfs > 0)? move (fs[0]) : "error";
    cur_ws->dirty = true;

    pre_ws->text = (nfs > 1)? move (fs[1]) : "";
    pre_ws->visible = pre_ws->text.length () > 0;

    pst_ws->text = (nfs > 2)? move (fs[2]) : "";
    pst_ws->visible = pst_ws->text.length () > 0;

    urg_ws->text = (nfs > 3)? move (fs[3]) : "";
    urg_ws->visible = urg_ws->text.length () > 0;

    state->text = (nfs > 4)? move (fs[4]) : "";
    state->visible = state->text.length () > 0;

    title->text = (nfs > 5)? move (fs[5]) : "";
    title->visible = title->text.length () > 0;
}

// }}}

// vim:fdm=marker:
