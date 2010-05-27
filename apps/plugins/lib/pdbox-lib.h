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

typedef long long t_time;

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

#ifndef SIMULATOR
typedef struct
{
    int quot;
    int rem;
}
div_t;
div_t div(int x, int y);
#endif /* #ifndef SIMULATOR */

union f2i
{
    float f;
    int32_t i;
};

#endif /* _PDBOX_LIB_H_ */
