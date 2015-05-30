
/*
 * daska/config.h
 * copyright (c) 2015 Frano Perleta
 */

#ifndef DASKA_CONFIG_H
#define DASKA_CONFIG_H

#include "common.h"
#include "draw.h"

struct config {
    bool verbose = false;
    bool quiet = false;

    void say (const char*, ...);
    void vsay (const char*, ...);
};

extern config cfg;

#endif /* daska/config */
