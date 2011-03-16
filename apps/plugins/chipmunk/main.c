/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2011 Lev Panov
 *
 * Chipmunk plugin
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

#include "plugin.h"
#include "lib/pluginlib_actions.h"
#include "lib/pluginlib_exit.h"

#include "lib/helper.h"
#include "lib/display_text.h"
#include "lib/highscore.h"
#include "lib/playback_control.h"

#include "chipmunk.h"

unsigned long CYCLETIME = 35;

const struct button_mapping *plugin_contexts[] = {
    pla_main_ctx,
    #if defined(HAVE_REMOTE_LCD)
    pla_remote_ctx,
    #endif
};

#define NB_ACTION_CONTEXTS \
    (sizeof(plugin_contexts) / sizeof(struct button_mapping*))

int main(void)
{
    return PLUGIN_OK;
}

static void cleanup(void)
{
    /* This is handled by plugin api. Remove in future */
    /* rb->lcd_setfont(FONT_UI); */

    /* Turn on backlight timeout (revert to settings) */
    backlight_use_settings(); /* Backlight control in lib/helper.c */
#ifdef HAVE_REMOTE_LCD
    remote_backlight_use_settings();
#endif
}

/* This is the plugin entry point */
enum plugin_status plugin_start(const void* parameter)
{
    int ret;

    atexit(cleanup);

    rb->lcd_setfont(FONT_SYSFIXED);
#if LCD_DEPTH > 1
    rb->lcd_set_backdrop(NULL);
#endif
    /* Turn off backlight timeout */
    backlight_force_on(); /* Backlight control in lib/helper.c */
#ifdef HAVE_REMOTE_LCD
    remote_backlight_force_on(); /* Remote backlight control in lib/helper.c */
#endif

    ret = main();

    return ret;
}
