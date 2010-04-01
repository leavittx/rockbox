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

#define SLEEP_TIME 3

/* Bitmaps */
#include "pluginbitmaps/balls_board.h"
#include "pluginbitmaps/balls_balls.h"
#include "pluginbitmaps/balls_background.h"

/* State of game */
enum {
	GAMEOVER
} State;

/* Cell type */
typedef enum {
	FREE = 0,
	T1 = 1,
	T2 = 2,
	T3 = 3,
	T4 = 4,
	T5 = 5,
	T6 = 6,
	T7 = 7
} Celltype;

/* Number of cells on the board */
#define NCELLS 9
/* Board is always a square, so only one dimention */
#define BRDLEN BMPWIDTH_balls_board
#define BRDOFFSET ((LCD_HEIGHT - BRDLEN) / 2)
/* Length of square in pixels */
#define CELLSIZE (BMPWIDTH_balls_board / NCELLS)

#define N_BALL_TYPES 7
#define BALLS_W BMPWIDTH_balls_balls
#define BALLS_H BMPHEIGHT_balls_balls
#define BALLSIZE BALLS_W
#define BALL_OFFSET ((CELLSIZE - BALLSIZE) / 2)

Celltype Board[NCELLS][NCELLS];

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
    int action, button; /* Key/touchscreen action */
	int i, j;
	bool need_redraw = true;
	short x, y;
	short xpos, ypos;
	
	rb->touchscreen_set_mode(TOUCHSCREEN_POINT);
	
	/* Main loop */
    while (true)
    {
		/* Draw everything (if needed) */
		if (need_redraw)
		{
			rb->lcd_clear_display();
			
			rb->lcd_bitmap(balls_board,
				BRDOFFSET,
				BRDOFFSET,
				BRDLEN,
				BRDLEN);
				
			for (i = 0; i < NCELLS; i++)
				for (j = 0; j < NCELLS; j++)
				{
					if (Board[i][j] != FREE)
					{
						rb->lcd_bitmap_transparent_part(balls_balls, 0,
							CELLSIZE * (Board[i][j] - 1),
							STRIDE(SCREEN_MAIN, BALLS_W, BALLS_H),
							BRDOFFSET + (i * CELLSIZE) + BALL_OFFSET,
							BRDOFFSET + (j * CELLSIZE) + BALL_OFFSET,
							BALLSIZE,
							BALLSIZE);
					}
				}
			
			rb->lcd_update();
			need_redraw = false;
		}
				
		//rb->sleep(SLEEP_TIME);
		rb->yield();
		
		action = rb->get_action(CONTEXT_STD, TIMEOUT_NOBLOCK);
		if (action == ACTION_TOUCHSCREEN)
        {
            button = rb->action_get_touchscreen_press(&x, &y);
            
            if (button == BUTTON_REPEAT/*BUTTON_TOUCHSCREEN*/)
            {
				xpos = (x - BRDOFFSET) / CELLSIZE;
				ypos = (y - BRDOFFSET) / CELLSIZE;
				
				if (xpos >= 0 && xpos < NCELLS &&
					ypos >= 0 && ypos < NCELLS)
				{
					Board[xpos][ypos] = rb->rand() % N_BALL_TYPES + 1;
					
					need_redraw = true;
				}
            }
            
        }
        else if (action == BUTTON_POWER)
        {
            cleanup(NULL);
            return PLUGIN_OK;
        }
    }
}

/* this is the plugin entry point */
enum plugin_status plugin_start(const void* parameter)
{
	int ret;
	
    /* if you don't use the parameter, you can do like
       this to avoid the compiler warning about it */
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
