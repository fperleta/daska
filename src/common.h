
/*
 * daska/common.h
 * copyright (c) 2015 Frano Perleta
 */

#ifndef DASKA_COMMON_H
#define DASKA_COMMON_H

#include <stdint.h>
#include <stdlib.h>

#define DASKA_VERSION DASKA_HEAD

#define UNUSED __attribute__((unused))

extern void panic__ (const char*, int, const char*, ...);
#define panic(fmt,...) panic__(__FILE__,__LINE__,fmt, ## __VA_ARGS__)

#endif /* daska/common.h */
