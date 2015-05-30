
/*
 * daska/mod/volume.cc
 * copyright (c) 2015 Frano Perleta
 * see the file LICENCE.txt for copyright information.
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "mod/volume.h"

using namespace std;
using namespace mod;
using namespace draw;

// constructor {{{

volume::volume (view::gauge_cell_ptr cell_, double seconds)
    : cell {cell_}
{
    snd_hctl_open (&hctl, "hw:0", 0);
    snd_hctl_load (hctl);

    timer.set<volume, &volume::timer_cb> (this);
    timer.set (0, seconds);
    timer_cb (timer, 0);
}

// }}}

// destructor {{{

volume::~volume ()
{
    timer.stop ();
    snd_hctl_close (hctl);
}

// }}}

// timer callback {{{

void
volume::timer_cb (ev::timer&, int)
{
    snd_ctl_elem_id_t* mv_id;
    snd_ctl_elem_id_alloca (&mv_id);
    snd_ctl_elem_id_set_interface (mv_id, SND_CTL_ELEM_IFACE_MIXER);
    snd_ctl_elem_id_set_name (mv_id, "Master Playback Volume");
    snd_hctl_elem_t* mv_elem = snd_hctl_find_elem (hctl, mv_id);

    snd_ctl_elem_info_t* mv_info;
    snd_ctl_elem_info_alloca (&mv_info);
    snd_hctl_elem_info (mv_elem, mv_info);
    long mv_max = snd_ctl_elem_info_get_max (mv_info);

    snd_ctl_elem_value_t* mv_val;
    snd_ctl_elem_value_alloca (&mv_val);
    snd_ctl_elem_value_set_id (mv_val, mv_id);
    snd_hctl_elem_read (mv_elem, mv_val);
    long mv_vol = snd_ctl_elem_value_get_integer (mv_val, 0);

    snd_ctl_elem_id_t* ms_id;
    snd_ctl_elem_id_alloca (&ms_id);
    snd_ctl_elem_id_set_interface (ms_id, SND_CTL_ELEM_IFACE_MIXER);
    snd_ctl_elem_id_set_name (ms_id, "Master Playback Switch");
    snd_hctl_elem_t* ms_elem = snd_hctl_find_elem (hctl, ms_id);

    snd_ctl_elem_value_t* ms_val;
    snd_ctl_elem_value_alloca (&ms_val);
    snd_ctl_elem_value_set_id (ms_val, ms_id);
    snd_hctl_elem_read (ms_elem, ms_val);
    bool ms_mute = !snd_ctl_elem_value_get_boolean (ms_val, 0);

    cell->value = mv_vol / (double) mv_max;
    cell->fgcolor = ms_mute? color::violet : color::base2;
    cell->dirty = mv_vol;

    timer.again ();
}

// }}}

// vim:fdm=marker:
