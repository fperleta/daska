
/*
 * daska/mod/volume.h
 * copyright (c) 2015 Frano Perleta
 * see the file LICENCE.txt for copyright information.
 */

#ifndef DASKA_MOD_VOLUME_H
#define DASKA_MOD_VOLUME_H

#include <string>
#include <ev++.h>
#include <alsa/asoundlib.h>
#include <alsa/control.h>
#include "common.h"
#include "view.h"

namespace mod {

    struct volume {
        view::gauge_cell_ptr cell;
        double value = 0;
        bool mute = 0;

        volume (view::gauge_cell_ptr, double = 0.25);
        ~volume ();

      private:
        ev::timer timer;
        void timer_cb (ev::timer&, int);

        snd_hctl_t* hctl;
    };

};

#endif /* daska/mod/volume.h */

// vim:fdm=marker:
