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
#include "pluginbitmaps/balls_chooser.h"

/* Board length in cells */
#define NCELLS 9
/* Same in pixels. Board always is a square, only one dimention then */
#define BRDLEN BMPWIDTH_balls_board

#define BRDYOFFSET ((LCD_HEIGHT - BRDLEN) / 2)

#define BRDEXTRAXOFFSET 10

#define BRDXOFFSET (BRDYOFFSET + BRDEXTRAXOFFSET)
/* Length of cell in pixels */
#define CELLSIZE (BMPWIDTH_balls_board / NCELLS)

#define N_BALL_TYPES 7

#define BALLS_W BMPWIDTH_balls_balls
#define BALLS_H BMPHEIGHT_balls_balls

#define BALLSIZE BALLS_W

#define BALL_OFFSET ((CELLSIZE - BALLSIZE) / 2)

#define N_BALLS_KILL 5

#define N_BALLS_ADD 3

#define getball() (rb->rand() % N_BALL_TYPES + 1)

/* State of game */
enum {
	ADDBALLS,
	TURN,
	GAMEOVER
} State = ADDBALLS;

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

/* Game board */
Celltype Board[NCELLS][NCELLS];
/* Next balls */
Celltype Next[N_BALLS_ADD];
/* Player's score */
int Score;

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

/* Does some magic */
bool DoKill(void)
{
	int i, j;
	bool Kill[NCELLS][NCELLS] = {{false}, {false}};
	int len;
	Celltype prev;
	int newscore = 0;
	
	/* TODO: count the score,
	 * if different possibilities to kill - kill them all, more points */
	
	/* Vertical */
	for (i = 0; i < NCELLS; i++)
	{
		prev = Board[i][0];
		len = 1;
		
		for (j = 1; j < NCELLS; j++)
		{
			if (Board[i][j] == prev && Board[i][j] != FREE)
				len++;
			else
			{
				if (len >= N_BALLS_KILL)
					for (; len; len--)
						Kill[i][j - len] = true;
				
				prev = Board[i][j];
				len = 1;
			}
		}
		
		if (len >= N_BALLS_KILL)
			for (; len; len--)
				Kill[i][j - len] = true;
	}
	/* Horizontal */
	for (i = 0; i < NCELLS; i++)
	{
		prev = Board[0][i];
		len = 1;
		
		for (j = 1; j < NCELLS; j++)
		{
			if (Board[j][i] == prev && Board[j][i] != FREE)
				len++;
			else
			{
				if (len >= N_BALLS_KILL)
					for (; len; len--)
						Kill[j - len][i] = true;
				
				prev = Board[j][i];
				len = 1;
			}
		}
		
		if (len >= N_BALLS_KILL)
			for (; len; len--)
				Kill[j - len][i] = true;
	}
	/* Diagonals(\), upper the main one */
	for (i = 0; i < NCELLS; i++)
	{
		prev = Board[i][0];
		len = 1;
		
		for (j = i + 1; j < NCELLS; j++)
		{
			if (Board[j][j - i] == prev && Board[j][j - i] != FREE)
				len++;
			else
			{
				if (len >= N_BALLS_KILL)
					for (; len; len--)
						Kill[j - len][j - i - len] = true;
				
				prev = Board[j][j - i];
				len = 1;
			}
		}
		
		if (len >= N_BALLS_KILL)
			for (; len; len--)
				Kill[j - len][j - i - len] = true;
	}
	/* Diagonals (\), lower the main one */
	for (i = 1; i < NCELLS; i++)
	{
		prev = Board[0][i];
		len = 1;
		
		for (j = i + 1; j < NCELLS; j++)
		{
			if (Board[j - i][j] == prev && Board[j - i][j] != FREE)
				len++;
			else
			{
				if (len >= N_BALLS_KILL)
					for (; len; len--)
						Kill[j - i - len][j - len] = true;
				
				prev = Board[j - i][j];
				len = 1;
			}
		}
		
		if (len >= N_BALLS_KILL)
			for (; len; len--)
				Kill[j - i - len][j - len] = true;
	}
	/* Diagonals(/), upper the main one */
	for (i = 0; i < NCELLS; i++)
	{
		prev = Board[i][0];
		len = 1;
		
		for (j = i - 1; j >= 0; j--)
		{
			if (Board[j][i - j] == prev && Board[j][i - j] != FREE)
				len++;
			else
			{
				if (len >= N_BALLS_KILL)
					for (; len; len--)
						Kill[j + len][i - j - len] = true;
				
				prev = Board[j][i - j];
				len = 1;
			}
		}
		
		if (len >= N_BALLS_KILL)
			for (; len; len--)
				Kill[j + len][i - j - len] = true;
	}
	/* Diagonals (/), lower the main one */
	for (i = 1; i < NCELLS; i++)
	{
		prev = Board[NCELLS - 1][i];
		len = 1;
		
		for (j = i + 1; j < NCELLS; j++)
		{
			if (Board[NCELLS - 1 - (j - i)][j] == prev && Board[NCELLS - 1 - (j - i)][j] != FREE)
				len++;
			else
			{
				if (len >= N_BALLS_KILL)
					for (; len; len--)
						Kill[NCELLS - 1 - (j - i) + len][j - len] = true;
				
				prev = Board[NCELLS - 1 - (j - i)][j];
				len = 1;
			}
		}
		
		if (len >= N_BALLS_KILL)
			for (; len; len--)
				Kill[NCELLS - 1 - (j - i) + len][j - len] = true;
	}
	
	for (i = 0; i < NCELLS; i++)
		for (j = 0; j < NCELLS; j++)
			if (Kill[i][j])
			{
				Board[i][j] = FREE;
				/* TODO: count score in proper way */
				newscore += 5;
			}
			
	Score += newscore;
	
	if (newscore)
		return true;
	return false;
}

/* Returns number of free cells on the board */
int FreeCellsNum(void)
{
	int i, j;
	int nfree = NCELLS * NCELLS;
	
	/* Count number of free cells */
	for (i = 0; i < NCELLS; i++)
		for (j = 0; j < NCELLS; j++)
			if (Board[i][j] != FREE)
				nfree--;
		
	return nfree;	
}

/* Checks if there is a path from cell (x1, y1) to cell (x2, y2).
 * The algo is really really stupid, maybe we should use a smarter one, like A*.
 * But who cares? Not me. This is fast enough. */
bool IsWalkable(short x1, short y1, short x2, short y2)
{
	int i, j;
	bool Walk[NCELLS][NCELLS] = {{false}, {false}};
	bool isanynew;
	
	Walk[x1][y1] = true;
	
	while (1)
	{
		isanynew = false;
		
		for (i = 0; i < NCELLS; i++)
			for (j = 0; j < NCELLS; j++)
			{
				if (Walk[i][j])
				{
					if (i == x2 && j == y2)
						return true;
					
					if (i < NCELLS - 1 && !Walk[i + 1][j] && Board[i + 1][j] == FREE)
						Walk[i + 1][j] = true, isanynew = true;
						
					if (i > 0 && !Walk[i - 1][j] && Board[i - 1][j] == FREE)
						Walk[i - 1][j] = true, isanynew = true;
						
					if (j < NCELLS - 1 && !Walk[i][j + 1] && Board[i][j + 1] == FREE)
						Walk[i][j + 1] = true, isanynew = true;
						
					if (j > 0 && !Walk[i][j - 1] && Board[i][j - 1] == FREE)
						Walk[i][j - 1] = true, isanynew = true;
				}
			}
		
		if (!isanynew)
			return false;
	}
}

/* TODO: place all draw-related stuff in separate function */
/* void draw() */

int plugin_main(void)
{
    int action, button; /* Key/touchscreen action */
	int i, j;
	bool need_redraw = true;
	short x, y;
	short xpos = 0, ypos = 0;
	bool ispicked = false;
	short oldxpos = 0, oldypos = 0;
	Celltype curtype = FREE;
	
	/* Set the touchscreen to pointer mode */
	rb->touchscreen_set_mode(TOUCHSCREEN_POINT);
	
	/* Prepare first balls */
	for (i = 0; i < N_BALLS_ADD; i++)
		Next[i] = getball();
	
	/* Main loop */
    while (true)
    {
		/* Draw everything (if needed) */
		if (need_redraw)
		{
			/* This isn't really needed (we already have background image), huh? */
			//rb->lcd_clear_display();
			
			/* Background image */
			rb->lcd_bitmap(balls_background,
				0,
				0,
				BMPWIDTH_balls_background,
				BMPHEIGHT_balls_background);
	
			/* Game board */
			rb->lcd_bitmap(balls_board,
				BRDXOFFSET,
				BRDYOFFSET,
				BRDLEN,
				BRDLEN);
			
			/* Balls */
			for (i = 0; i < NCELLS; i++)
				for (j = 0; j < NCELLS; j++)
					if (Board[i][j] != FREE)
					{
						rb->lcd_bitmap_transparent_part(balls_balls, 0,
							CELLSIZE * (Board[i][j] - 1),
							STRIDE(SCREEN_MAIN, BALLS_W, BALLS_H),
							BRDXOFFSET + (i * CELLSIZE) + BALL_OFFSET,
							BRDYOFFSET + (j * CELLSIZE) + BALL_OFFSET,
							BALLSIZE,
							BALLSIZE);
					}
				
			/* Next balls */
			for (i = 0; i < N_BALLS_ADD; i++)
			{
				rb->lcd_bitmap_transparent_part(balls_balls, 0,
							CELLSIZE * (Next[i] - 1),
							STRIDE(SCREEN_MAIN, BALLS_W, BALLS_H),
							LCD_WIDTH - (LCD_WIDTH - (BRDXOFFSET + BRDLEN - BALLSIZE)) / 2,
							(LCD_HEIGHT - N_BALLS_ADD * BALLSIZE) / 2 + BALLSIZE * i,
							BALLSIZE,
							BALLSIZE);
			}
			
			/* Picked ball */
			if (ispicked)
			{
				rb->lcd_bitmap(balls_chooser,
					BRDXOFFSET + (xpos * CELLSIZE) + ((CELLSIZE - BMPWIDTH_balls_chooser) / 2),
					BRDYOFFSET + (ypos * CELLSIZE) + ((CELLSIZE - BMPHEIGHT_balls_chooser) / 2),
					BMPWIDTH_balls_chooser,
					BMPHEIGHT_balls_chooser);
				
				rb->lcd_bitmap_transparent_part(balls_balls, 0,
							CELLSIZE * (curtype - 1),
							STRIDE(SCREEN_MAIN, BALLS_W, BALLS_H),
							BRDXOFFSET + (xpos * CELLSIZE) + BALL_OFFSET,
							BRDYOFFSET + (ypos * CELLSIZE) + BALL_OFFSET,
							BALLSIZE,
							BALLSIZE);
			}
			
			rb->lcd_update();
			need_redraw = false;
		}
				
		rb->sleep(SLEEP_TIME);
		//rb->yield();
		
		if (State == ADDBALLS)
		{
			int nadd, nadded = 0;
			
			if ((nadd = FreeCellsNum()) > N_BALLS_ADD)
				nadd = N_BALLS_ADD;

			while (nadded != nadd)
			{
				xpos = rb->rand() % NCELLS;
				ypos = rb->rand() % NCELLS;
				if (Board[xpos][ypos] == FREE)
					Board[xpos][ypos] = Next[nadded++];
			}
			
			for (i = 0; i < N_BALLS_ADD; i++)
				Next[i] = getball();
			
			if (!DoKill() && nadd < N_BALLS_ADD)
				State = GAMEOVER;
			else
				State = TURN;
			
			need_redraw = true;
		}
		
		action = rb->get_action(CONTEXT_STD, TIMEOUT_NOBLOCK);
		if (action == ACTION_TOUCHSCREEN)
        {
            button = rb->action_get_touchscreen_press(&x, &y);
            
            if (State == TURN)
            {
				xpos = (x - BRDXOFFSET) / CELLSIZE;
				ypos = (y - BRDYOFFSET) / CELLSIZE;

				if (button == BUTTON_TOUCHSCREEN)
				{
					if (xpos >= 0 && xpos < NCELLS && ypos >= 0 && ypos < NCELLS &&
						Board[xpos][ypos] != FREE)
					{
						ispicked = true;
						oldxpos = xpos;
						oldypos = ypos;
						curtype = Board[xpos][ypos];
						Board[xpos][ypos] = FREE;
					}
				}
				else if (button == BUTTON_REPEAT)
				{
					if (ispicked)
					{
						if (xpos >= 0 && xpos < NCELLS && ypos >= 0 && ypos < NCELLS &&
							Board[xpos][ypos] == FREE && IsWalkable(oldxpos, oldypos, xpos, ypos))
						{
							need_redraw = true;
						}
					}
				}
				else if (button == BUTTON_REL)
				{
					Board[oldxpos][oldypos] = curtype;
					ispicked = false;
				}
				else if (button == (BUTTON_REPEAT | BUTTON_REL))
				{
					if (ispicked)
					{
						if (xpos >= 0 && xpos < NCELLS && ypos >= 0 && ypos < NCELLS &&
							Board[xpos][ypos] == FREE && IsWalkable(oldxpos, oldypos, xpos, ypos))
						{
							Board[xpos][ypos] = curtype;
							
							if (oldxpos != xpos || oldypos != ypos)
								if (!DoKill())
									State = ADDBALLS;
						}
						else
							Board[oldxpos][oldypos] = curtype;
					
						ispicked = false;
						need_redraw = true;
					}				
				}
			}            
        }
        else if (action == BUTTON_POWER)
        {
			/* TODO: figure out why %i causes strange behavior on real hardware
			 * (prints 'i' letter instead of score) */
			/* TODO: use haighscore lib */
			rb->splashf(HZ*2, "Score: %u", Score);
			
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
