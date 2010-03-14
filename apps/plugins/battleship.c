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
 * Battleship plugin
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
#include "pluginbitmaps/battleship_huy.h"
#include "pluginbitmaps/battleship_map.h"
#include "pluginbitmaps/battleship_placeships.h"
#include "pluginbitmaps/battleship_player1.h"
#include "pluginbitmaps/battleship_player2.h"
#include "pluginbitmaps/battleship_n0.h"
#include "pluginbitmaps/battleship_n1.h"
#include "pluginbitmaps/battleship_n2.h"
#include "pluginbitmaps/battleship_n3.h"
#include "pluginbitmaps/battleship_n4.h"
#include "pluginbitmaps/battleship_n5.h"
#include "pluginbitmaps/battleship_n6.h"
#include "pluginbitmaps/battleship_n7.h"
#include "pluginbitmaps/battleship_n8.h"
#include "pluginbitmaps/battleship_n9.h"
#include "pluginbitmaps/battleship_nsep.h"
#include "pluginbitmaps/battleship_aim.h"
#include "pluginbitmaps/battleship_missed.h"
#include "pluginbitmaps/battleship_shipone.h"
#include "pluginbitmaps/battleship_shiponedead.h"
#include "pluginbitmaps/battleship_noships.h"

unsigned short const *numbitmaps[] = {
	battleship_n0, battleship_n1,
	battleship_n2, battleship_n3,
	battleship_n4, battleship_n5,
	battleship_n6, battleship_n7,
	battleship_n8, battleship_n9
};

enum { 
	PLACE_SHIPS_PLAYER1,
	PLACE_SHIPS_PLAYER2,
	PLAYER1,
	PLAYER2 
} State;

typedef enum {
	VERT,
	HORIZ
} Orientation;

typedef enum { 
	FREE, 		/* Nothing */
	SHIP, 		/* A bit of ship */
	SHOTSHIP, 	/* Partly shot or entirely dead */
	SHOTWATER,	/* Missed! */
	NOSHIPS 	/* Being a ship in this square isn't possible */
} Sqtype;

#define X 	0
#define Y 	1

#define D1	0
#define D2	1
#define D3	2
#define D4	3

#define N1	0
#define N2	1
#define N3	2
#define N4	3

#define NUM_OF_S1 4
#define NUM_OF_S2 3
#define NUM_OF_S3 2
#define NUM_OF_S4 1

typedef struct {
	int nS1, nS2, nS3, nS4;
	int S1[NUM_OF_S1][1][2], S2[NUM_OF_S2][2][2], S3[NUM_OF_S3][3][2], S4[NUM_OF_S4][4][2];
	bool issunkS1[NUM_OF_S1], issunkS2[NUM_OF_S2], issunkS3[NUM_OF_S3], issunkS4[NUM_OF_S4];
} Ships;

#define FLD_LEN 10
#define XOFFSET 20
#define YOFFSET 20
#define SQSIZE 	22

Sqtype battlefield_p1[FLD_LEN][FLD_LEN];
Sqtype battlefield_p2[FLD_LEN][FLD_LEN];

Ships ships_p1;
Ships ships_p2;

const struct button_mapping *plugin_contexts[]
= {generic_directions, generic_actions,
#if defined(HAVE_REMOTE_LCD)
    remote_directions
#endif
};
#define NB_ACTION_CONTEXTS \
    sizeof(plugin_contexts)/sizeof(struct button_mapping*)

void cleanup(void *parameter)
{
	(void)parameter;

	backlight_use_settings();
#ifdef HAVE_REMOTE_LCD
	remote_backlight_use_settings();
#endif
}

/* Main code */

unsigned long get_sec(void) {
	return *rb->current_tick / HZ;
}

void InitFields(void)
{
	int i, j;
	
	for (i = 0; i < FLD_LEN; i++)
		for (j = 0; j < FLD_LEN; j++)
		{
			battlefield_p1[i][j] = FREE;
			battlefield_p2[i][j] = FREE;
		}
	/*		
	battlefield_p1[2][2] = SHIP;
	battlefield_p1[2][3] = SHIP;
	battlefield_p1[2][4] = SHIP;
	battlefield_p1[2][5] = SHIP;
	battlefield_p1[7][2] = SHIP;
	*/
	
	for (i = 0; i < NUM_OF_S1; i ++)
		ships_p1.issunkS1[i] = true;
	for (i = 0; i < NUM_OF_S2; i ++)
		ships_p1.issunkS2[i] = true;
	for (i = 0; i < NUM_OF_S3; i ++)
		ships_p1.issunkS3[i] = true;
	for (i = 0; i < NUM_OF_S4; i ++)
		ships_p1.issunkS4[i] = true;
	
	ships_p1.S4[N1][D1][X] = 0;
	ships_p1.S4[N1][D1][Y] = 0;
	ships_p1.S4[N1][D2][X] = 1;
	ships_p1.S4[N1][D2][Y] = 0;
	ships_p1.S4[N1][D3][X] = 2;
	ships_p1.S4[N1][D3][Y] = 0;
	ships_p1.S4[N1][D4][X] = 3;
	ships_p1.S4[N1][D4][Y] = 0;
	ships_p1.issunkS4[N1] = false;
	
	ships_p1.S3[N1][D1][X] = 4;
	ships_p1.S3[N1][D1][Y] = 4;
	ships_p1.S3[N1][D2][X] = 4;
	ships_p1.S3[N1][D2][Y] = 5;
	ships_p1.S3[N1][D3][X] = 4;
	ships_p1.S3[N1][D3][Y] = 6;
	ships_p1.issunkS3[N1] = false;
	
	ships_p1.S1[N1][D1][X] = 6;
	ships_p1.S1[N1][D1][Y] = 6;
	ships_p1.issunkS1[N1] = false;
	
	battlefield_p1[0][0] =
		battlefield_p1[1][0] = 
			battlefield_p1[2][0] = 
				battlefield_p1[3][0] = 
					battlefield_p1[4][4] = 
						battlefield_p1[4][5] = 
							battlefield_p1[4][6] = 
								battlefield_p1[6][6] = SHIP;
}

int plugin_main(void)
{
    int action;
	int i, j, n;
	bool need_redraw = true; /* Redraw field or not? */
	unsigned long startsec, prevsec = 0, sec, min = 0; /* Time stuff */
	int xpos = 0, ypos = 0;
	bool aim = true;
	Orientation orientation = HORIZ;
	
    FOR_NB_SCREENS(n)
    {
        struct screen *display = rb->screens[n];

        if (display->is_color)
            display->set_background(LCD_BLACK);
    }

	/* Set all squares free */
	InitFields();
	
	State = PLACE_SHIPS_PLAYER1;
	
	/* Get start second */
	startsec = get_sec();

    while (true)
    {
		/* Measure time */
		sec = get_sec() - startsec;
		/* Update display in this case */
		if (sec != prevsec)
		{
			min = sec / 60;
			sec -= min * 60;
			/* > 60 min */
			min %= 60;
			need_redraw = true;
		}
		
		if (need_redraw)
		{
			FOR_NB_SCREENS(n)
			{
				struct screen *display = rb->screens[n];
				
				rb->lcd_clear_display();
				
				/* Battle map */
				display->bitmap(battleship_map,
								0,
								0,
								BMPWIDTH_battleship_map,
								BMPHEIGHT_battleship_map);
				
				/* Player's name */
				display->bitmap(battleship_player1,
								BMPWIDTH_battleship_map,
								0,
								BMPWIDTH_battleship_player1,
								BMPHEIGHT_battleship_player1);
				
				/* Place your ships please */
				display->bitmap(battleship_placeships,
								BMPWIDTH_battleship_map,
								BMPHEIGHT_battleship_player1,
								BMPWIDTH_battleship_placeships,
								BMPHEIGHT_battleship_placeships);
								
				/* Objects on field */
				for (i = 0; i < FLD_LEN; i++)
					for (j = 0; j < FLD_LEN; j++)
					{
						if (battlefield_p1[i][j] == SHOTWATER)
							display->bitmap(battleship_missed,
											XOFFSET + i * SQSIZE + (SQSIZE - BMPWIDTH_battleship_missed) / 2,
											YOFFSET + j * SQSIZE + (SQSIZE - BMPWIDTH_battleship_missed) / 2,
											BMPWIDTH_battleship_missed,
											BMPHEIGHT_battleship_missed);
						else if (battlefield_p1[i][j] == SHIP)
							display->bitmap(battleship_shipone,
											XOFFSET + i * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shipone) / 2,
											YOFFSET + j * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shipone) / 2,
											BMPWIDTH_battleship_shipone,
											BMPHEIGHT_battleship_shipone);
						else if (battlefield_p1[i][j] == SHOTSHIP)
							display->bitmap(battleship_shiponedead,
											XOFFSET + i * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shiponedead) / 2,
											YOFFSET + j * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shiponedead) / 2,
											BMPWIDTH_battleship_shiponedead,
											BMPHEIGHT_battleship_shiponedead);
						else if (battlefield_p1[i][j] == NOSHIPS)
							display->bitmap(battleship_noships,
											XOFFSET + i * SQSIZE + (SQSIZE - BMPWIDTH_battleship_noships) / 2,
											YOFFSET + j * SQSIZE + (SQSIZE - BMPWIDTH_battleship_noships) / 2,
											BMPWIDTH_battleship_noships,
											BMPHEIGHT_battleship_noships);
					}
					
				/* Aim */
				if (aim)
					rb->lcd_bitmap_transparent(battleship_aim,
								XOFFSET + xpos * SQSIZE + (SQSIZE - BMPWIDTH_battleship_aim) / 2,
								YOFFSET + ypos * SQSIZE + (SQSIZE - BMPHEIGHT_battleship_aim) / 2,
								BMPWIDTH_battleship_aim,
								BMPHEIGHT_battleship_aim);
				
				/* Timer */
				display->bitmap(numbitmaps[min / 10],
								BMPWIDTH_battleship_map,
								LCD_HEIGHT - BMPHEIGHT_battleship_n0 - 5,
								BMPWIDTH_battleship_n0,
								BMPHEIGHT_battleship_n0);
								
				display->bitmap(numbitmaps[min % 10],
								BMPWIDTH_battleship_map + BMPWIDTH_battleship_n0 - 2,
								LCD_HEIGHT - BMPHEIGHT_battleship_n0 - 5,
								BMPWIDTH_battleship_n0,
								BMPHEIGHT_battleship_n0);
				
				display->bitmap(battleship_nsep,
								BMPWIDTH_battleship_map + BMPWIDTH_battleship_n0 * 2 - 4,
								LCD_HEIGHT - BMPHEIGHT_battleship_n0 - 5,
								BMPWIDTH_battleship_nsep,
								BMPHEIGHT_battleship_nsep);
				
				display->bitmap(numbitmaps[sec / 10],
								BMPWIDTH_battleship_map + BMPWIDTH_battleship_n0 * 2 - 4 + BMPWIDTH_battleship_nsep,
								LCD_HEIGHT - BMPHEIGHT_battleship_n0 - 5,
								BMPWIDTH_battleship_n0,
								BMPHEIGHT_battleship_n0);
								
				display->bitmap(numbitmaps[sec % 10],
								BMPWIDTH_battleship_map + BMPWIDTH_battleship_n0 * 3 - 6 + BMPWIDTH_battleship_nsep,
								LCD_HEIGHT - BMPHEIGHT_battleship_n0 - 5,
								BMPWIDTH_battleship_n0,
								BMPHEIGHT_battleship_n0);
				
				display->update();
			}
			need_redraw = false;
		}
		
		rb->sleep(SLEEP_TIME);
		//rb->yield();

		action = pluginlib_getaction(TIMEOUT_NOBLOCK,
									 plugin_contexts,
									 NB_ACTION_CONTEXTS);
		switch (action)
		{
			case PLA_QUIT:
				cleanup(NULL);
				return PLUGIN_OK;
				
			case PLA_UP:
				if (ypos == 0)
					ypos = FLD_LEN - 1;
				else
					ypos--;
				aim = true;
				break;
				
			case PLA_DOWN:
				if (ypos == FLD_LEN - 1)
					ypos = 0;
				else
					ypos++;
				aim = true;
				break;
			
			case PLA_RIGHT:
				if (xpos == FLD_LEN - 1)
					xpos = 0;
				else
					xpos++;
				aim = true;
				break;
			
			case PLA_LEFT:
				if (xpos == 0)
					xpos = FLD_LEN - 1;
				else
					xpos--;
				aim = true;
				break;
			
			case PLA_MENU: /* Change ship orientation */
				if (State == PLACE_SHIPS_PLAYER1 || State == PLACE_SHIPS_PLAYER2)
					orientation = orientation == VERT ? HORIZ : VERT;
				break;
			
			case PLA_FIRE: /* Shoot / place a ship */
				if (battlefield_p1[xpos][ypos] == FREE)
				{
					aim = false;
					battlefield_p1[xpos][ypos] = SHOTWATER;
				}
				else if (battlefield_p1[xpos][ypos] == SHIP)
				{
					aim = false;
					battlefield_p1[xpos][ypos] = SHOTSHIP;
					
					for (i = 0; i < NUM_OF_S1; i++)
					{
						if (!ships_p1.issunkS1[i] && ships_p1.S1[i][D1][X] == xpos && ships_p1.S1[i][D1][Y] == ypos)
						{
							ships_p1.issunkS1[i] = true;
							if (xpos != 0)
							{
								if (battlefield_p1[xpos - 1][ypos] == FREE)
									battlefield_p1[xpos - 1][ypos] = NOSHIPS;
								if (ypos != 0)
									if (battlefield_p1[xpos - 1][ypos - 1] == FREE)
										battlefield_p1[xpos - 1][ypos - 1] = NOSHIPS;
								if (ypos != FLD_LEN - 1)
									if (battlefield_p1[xpos - 1][ypos + 1] == FREE)
										battlefield_p1[xpos - 1][ypos + 1] = NOSHIPS;
							}
							if (xpos != FLD_LEN - 1)
							{
								if (battlefield_p1[xpos + 1][ypos] == FREE)
									battlefield_p1[xpos + 1][ypos] = NOSHIPS;
								if (ypos != 0)
									if (battlefield_p1[xpos + 1][ypos - 1] == FREE)
										battlefield_p1[xpos + 1][ypos - 1] = NOSHIPS;
								if (ypos != FLD_LEN - 1)
									if (battlefield_p1[xpos + 1][ypos + 1] == FREE)
										battlefield_p1[xpos + 1][ypos + 1] = NOSHIPS;
							}
							if (ypos != 0)
								if (battlefield_p1[xpos][ypos - 1] == FREE)
									battlefield_p1[xpos][ypos - 1] = NOSHIPS;
							if (ypos != FLD_LEN - 1)
								if (battlefield_p1[xpos][ypos + 1] == FREE)
									battlefield_p1[xpos][ypos + 1] = NOSHIPS;
						}
					}
						
				}
				break;
				
			case PLA_START: /* Shoot / place a ship */
				if (battlefield_p1[xpos][ypos] == FREE)
					battlefield_p1[xpos][ypos] = SHIP;
				break;
				
			default:
				if (rb->default_event_handler_ex(action, cleanup, NULL)
					== SYS_USB_CONNECTED)
					return PLUGIN_USB_CONNECTED;
				break;
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
