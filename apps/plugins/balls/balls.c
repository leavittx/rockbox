/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2010 Lev Panov
 * 
 * Balls (lines) plugin
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

#ifdef HAVE_LCD_BITMAP
#include "lib/pluginlib_actions.h"
#include "lib/helper.h"

/* This macros must always be included. Should be placed at the top by
   convention, although the actual position doesn't matter */
PLUGIN_HEADER

#include "game.h"
#include "user.h"
#include "draw.h"

#define SLEEP_TIME 3

void cleanup(void *parameter)
{
	(void)parameter;

	backlight_use_settings();
#ifdef HAVE_REMOTE_LCD
	remote_backlight_use_settings();
#endif
}

/* 
 * Main code 
 */

int plugin_main(void)
{
	BallsGame Game;
    int action; /* Key action */

	/* Set the touchscreen to pointer mode */
	rb->touchscreen_set_mode(TOUCHSCREEN_POINT);
	/* We wanna new balls every time */
	rb->srand(*rb->current_tick);

	Init(&Game);
		
	/* Main loop */
    while (true)
    {
		/* Draw everything (if needed) */
		if (Game.need_redraw)
			Draw(&Game);
		
		rb->sleep(SLEEP_TIME);
		
		if (Game.State == ADDBALLS)
			AddBalls(&Game);
		
		action = rb->get_action(CONTEXT_STD, TIMEOUT_NOBLOCK);
		
		if (action == ACTION_TOUCHSCREEN)
            HandleTouchscreen(&Game);
        else if (action == BUTTON_POWER)
        {
			/* TODO: figure out why %i causes strange behavior on real hardware
			 * (prints 'i' letter instead of score) */
			/* TODO: use haighscore lib */
			rb->splashf(HZ*2, "Score: %u", Game.Score);
			
            cleanup(NULL);
            return PLUGIN_OK;
        }
    }
}

/* this is the plugin entry point */
enum plugin_status plugin_start(const void* parameter)
{
	int ret;
	
	/* avoid the compiler warning about unused parameter */
	(void)parameter;
    
#if LCD_DEPTH > 1
	rb->lcd_set_backdrop(NULL);
#endif
	backlight_force_on(); /* backlight control in lib/helper.c */
#ifdef HAVE_REMOTE_LCD
	remote_backlight_force_on(); /* remote backlight control in lib/helper.c */
#endif

	ret = plugin_main();

	return ret;
}

#endif /* #ifdef HAVE_LCD_BITMAP */
