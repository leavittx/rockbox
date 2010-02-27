/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2009 Wincent Balin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#ifndef _PDBOX_LIB_H_
#define _PDBOX_LIB_H_


#if 0 /* 1 */
/* NOT Use TLSF. */
#include "codecs/lib/tlsf/src/tlsf.h"
#endif

/* Pure Data */
/* This is lib, so not needed
#include "PDa/src/m_pd.h"
*/

typedef long long t_time;

/* Minimal memory size. */
/* Not needed
#define MIN_MEM_SIZE (4 * 1024 * 1024)
*/

/* Memory prototypes. */
/* Not needed */
#if 0 /* 1 */
/* Direct memory allocator functions to TLSF. */
#define malloc(size) tlsf_malloc(size)
#define free(ptr) tlsf_free(ptr)
#define realloc(ptr, size) tlsf_realloc(ptr, size)
#define calloc(elements, elem_size) tlsf_calloc(elements, elem_size)
#endif

/* Additional functions. */
char *rb_strncat(char *s, const char *t, size_t n);
double rb_strtod(const char*, char**);
double rb_atof(const char*);
void rb_ftoan(float, char*, int);
float rb_floor(float);
long rb_atol(const char* s);
float rb_sin(float rad);
float rb_cos(float rad);
int rb_fscanf_f(int fd, float* f);
int rb_fprintf_f(int fd, float f);
float rb_log10(float);
float rb_log(float);
float rb_exp(float);
float rb_pow(float, float);
float rb_sqrt(float);
float rb_fabs(float);
float rb_atan(float);
float rb_atan2(float, float);
float rb_sinh(float);
float rb_tan(float);
typedef struct
{
    int quot;
    int rem;
}
div_t;
div_t div(int x, int y);
union f2i
{
    float f;
    int32_t i;
};

/* Everything else not needed,
 * otherwise will not compile */

/* Redefinitons of ANSI C functions. */
/*
#include "lib/wrappers.h"

#define strncmp rb->strncmp
#define atoi rb->atoi
#define write rb->write

#define strncat rb_strncat
#define floor rb_floor
#define atof rb_atof
#define atol rb_atol
#define ftoan rb_ftoan
#define sin rb_sin
#define cos rb_cos
#define log10 rb_log10
#define log rb_log
#define exp rb_exp
#define pow rb_pow
#define sqrt rb_sqrt
#define fabs rb_fabs
#define atan rb_atan
#define atan2 rb_atan2
#define sinh rb_sinh
#define tan rb_tan

#define strtok_r rb->strtok_r
#define strstr rb->strcasestr
*/

#endif /* _PDBOX_LIB_H_ */
