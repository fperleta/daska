
/*
 * daska/main.cc
 * copyright (c) 2015 Frano Perleta
 */

#include <argp.h>
#include <iostream>
#include <stdarg.h>
#include <ev++.h>

#include "common.h"
#include "config.h"
#include "view.h"
#include "ui.h"
#include "mod/clock.h"
#include "mod/xmonad.h"
#include "mod/power.h"

// arguments {{{

const char* argp_program_version = "daska " DASKA_VERSION;

static struct argp_option opts[] = {
    {"quiet", 'q', nullptr, 0, "Suppress all output.", 0},
    {"verbose", 'v', nullptr, 0, "Enable more verbose output.", 0},
    { nullptr, 0, nullptr, 0, nullptr, 0 }
};

static error_t
parse_opt (int key, char* arg UNUSED, struct argp_state* state)
{
    config* cfg = static_cast<config*> (state->input);

    switch (key)
    {
        case 'q':
            cfg->quiet = true;
            break;
        case 'v':
            cfg->verbose = true;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = {
    .options = opts,
    .parser = parse_opt,
    .args_doc = "",
    .doc = nullptr,
    .children = nullptr,
    .help_filter = nullptr,
    .argp_domain = nullptr
};

// }}}

// stuff {{{

void
panic__ (const char* fn, int ln, const char* fmt, ...)
{
    char buf[1024];

    va_list ap;
    va_start (ap, fmt);
    vsnprintf (buf, 1024, fmt, ap);
    va_end (ap);

    fprintf (stderr, "panic:%s:%d: %s\n", fn, ln, buf);

    exit (EXIT_FAILURE);
}

void
config::say (const char* fmt, ...)
{
    if (quiet) return;

    char buf[1024];
    va_list ap;
    va_start (ap, fmt);
    vsnprintf (buf, 1024, fmt, ap);
    va_end (ap);

    fprintf (stderr, "%s\n", buf);
}

void
config::vsay (const char* fmt, ...)
{
    if (quiet || !verbose) return;

    char buf[1024];
    va_list ap;
    va_start (ap, fmt);
    vsnprintf (buf, 1024, fmt, ap);
    va_end (ap);

    fprintf (stderr, "%s\n", buf);
}

// }}}

// watchers {{{

static void
sigint_cb (struct ev_loop* loop, ev_signal* w UNUSED, int revents UNUSED)
{
    cfg.say ("sigint");
    ev_break (loop, EVBREAK_ALL);
}

// }}}

config cfg;

// event loop {{{

template<typename Fn>
void eventloop (Fn fn)
{
    struct ev_loop* loop = ev_default_loop (0);

    ev_signal sigint_w;
    ev_signal_init (&sigint_w, sigint_cb, SIGINT);
    ev_signal_start (loop, &sigint_w);

    fn ([&] {
        ev_loop (loop, 0);
    });
}

// }}}

int
main (int argc, char** argv)
{
    argp_parse (&argp, argc, argv, 0, 0, &cfg);

    eventloop ([] (auto run) {
        ui::conn conn (EV_DEFAULT);
        ui::rect r {0, 0, conn.xscrn->width_in_pixels, 14};

        using namespace std;
        using namespace draw;

        auto default_font = pango::make_font ("Terminus 8");
        auto title_font = pango::make_font ("Droid Sans 10");

        auto bar = make_shared<view::bar> ();

        auto pre_ws = make_shared<view::text_cell> ();
        pre_ws->font = default_font;
        bar->left.push_back (pre_ws);

        auto cur_ws = make_shared<view::text_cell> ();
        cur_ws->font = default_font;
        cur_ws->bgcolor = rgba {color::base1, 0.75};
        cur_ws->fgcolor = color::base02;
        bar->left.push_back (cur_ws);

        auto pst_ws = make_shared<view::text_cell> ();
        pst_ws->font = default_font;
        bar->left.push_back (pst_ws);

        auto urg_ws = make_shared<view::text_cell> ();
        urg_ws->font = default_font;
        urg_ws->bgcolor = rgba {color::red, 0.9};
        urg_ws->fgcolor = color::base03;
        bar->left.push_back (urg_ws);

        auto state = make_shared<view::text_cell> ();
        state->font = default_font;
        state->bgcolor = rgba {color::base02, 0.9};
        bar->left.push_back (state);

        auto title = make_shared<view::text_cell> ();
        title->font = title_font;
        bar->middle = title;

        auto clock = make_shared<view::text_cell> ();
        clock->font = default_font;
        bar->right.push_back (clock);

        auto power = make_shared<view::gauge_cell> ();
        power->width = 40;
        power->height = 6;
        bar->right.push_back (power);

        mod::xmonad xm (conn, cur_ws, pre_ws, pst_ws, urg_ws, state, title);
        mod::clock clk (clock);
        mod::power pwr (power);

        ui::daska_window w (conn, bar);
        w.map ();

        run ();
    });

    exit (EXIT_SUCCESS);
}

// vim:fdm=marker:
