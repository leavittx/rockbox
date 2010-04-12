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

#define GETBALL() (rb->rand() % N_BALL_TYPES + 1)
#define ONBOARD(x, y) \
	((x) >= 0 && (x) < NCELLS && \
	 (y) >= 0 && (y) < NCELLS)
#define FREE(x, y) (Board[(x)][(y)] == CELL_FREE)
#define BUSY(x, y) (Board[(x)][(y)] != CELL_FREE)
#define CORRECTMOVE(x, y, oldx, oldy, iscached)	\
	(ONBOARD((x), (y)) && \
	 FREE((x), (y)) && \
	 IsWalkable((oldx), (oldy), (x), (y), (iscached)))
/* State of game */
enum {
	ADDBALLS,
	TURN,
	GAMEOVER
} State = ADDBALLS;

/* Cell type */
typedef enum {
	CELL_FREE = 0,
	CELL_T1 = 1,
	CELL_T2 = 2,
	CELL_T3 = 3,
	CELL_T4 = 4,
	CELL_T5 = 5,
	CELL_T6 = 6,
	CELL_T7 = 7
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
	
	/* Vertical */
	for (i = 0; i < NCELLS; i++)
	{
		prev = Board[i][0];
		len = 1;
		
		for (j = 1; j < NCELLS; j++)
		{
			if (Board[i][j] == prev && BUSY(i, j))
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
			if (Board[j][i] == prev && BUSY(j, i))
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
			if (Board[j][j - i] == prev && BUSY(j, j - i))
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
			if (Board[j - i][j] == prev && BUSY(j - i, j))
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
			if (Board[j][i - j] == prev && BUSY(j, i - j))
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
			if (Board[NCELLS - 1 - (j - i)][j] == prev && BUSY(NCELLS - 1 - (j - i), j))
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
				Board[i][j] = CELL_FREE;
				/* TODO: count score in proper way */
				newscore += 5;
			}
			
	Score += newscore;
	
	if (newscore)
		return true;
	return false;
}

/* Counts number of free cells on the board */
int FreeCellsNum(void)
{
	int i, j;
	int nfree = NCELLS * NCELLS;

	for (i = 0; i < NCELLS; i++)
		for (j = 0; j < NCELLS; j++)
			if (BUSY(i, j))
				nfree--;

	return nfree;
}

/* Marks all cells in Walk to which exists path form cell (x, y) */
void FillWalkableCells(short x, short y, bool (*Walk)[NCELLS])
{
	int i, j;
	bool isanynew;
	
	Walk[x][y] = true;
	
	while (1)
	{
		isanynew = false;
		
		for (i = 0; i < NCELLS; i++)
			for (j = 0; j < NCELLS; j++)
				if (Walk[i][j])
				{
					if (i < NCELLS - 1 && !Walk[i + 1][j] && FREE(i + 1, j))
						Walk[i + 1][j] = true, isanynew = true;
						
					if (i > 0 && !Walk[i - 1][j] && FREE(i - 1, j))
						Walk[i - 1][j] = true, isanynew = true;
						
					if (j < NCELLS - 1 && !Walk[i][j + 1] && FREE(i, j + 1))
						Walk[i][j + 1] = true, isanynew = true;
						
					if (j > 0 && !Walk[i][j - 1] && FREE(i, j - 1))
						Walk[i][j - 1] = true, isanynew = true;
				}
		
		if (!isanynew) /* No allowed cells were added during the latest iteration */
			return;
	}
}

/* Checks if there is a path from cell (x1, y1) to cell (x2, y2).
 * The algo (including FillWalkableCells() function) is really really stupid,
 * maybe we should use a smarter one, like A*.
 * But who cares? Not me. This is fast enough.
 * Caching adds some speed-up too */
bool IsWalkable(short x1, short y1, short x2, short y2, bool cached)
{
	bool Walk[NCELLS][NCELLS] = {{false}, {false}};
	static bool WalkCache[NCELLS][NCELLS];
	
	if (cached)
		return WalkCache[x2][y2];
	
	FillWalkableCells(x1, y1, Walk);
	
	/* Make the cache */
	memcpy(WalkCache, Walk, sizeof(Walk));

	if (Walk[x2][y2]) /* Path exists */
		return true;
	return false; /* No path */
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
	bool iscached = false;
	short oldxpos = 0, oldypos = 0;
	Celltype curtype = CELL_FREE;
	
	/* Set the touchscreen to pointer mode */
	rb->touchscreen_set_mode(TOUCHSCREEN_POINT);
	/* We wanna new balls every time */
	rb->srand(*rb->current_tick);
	
	/* Prepare first balls */
	for (i = 0; i < N_BALLS_ADD; i++)
		Next[i] = GETBALL();
	
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
					if (BUSY(i, j))
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
				if (FREE(xpos, ypos))
					Board[xpos][ypos] = Next[nadded++];
			}
			
			for (i = 0; i < N_BALLS_ADD; i++)
				Next[i] = GETBALL();
			
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
					/* For testing */
				/*	Board[xpos][ypos] = GETBALL();
					DoKill();
					need_redraw = true;
				*/
					if (ONBOARD(xpos, ypos) && BUSY(xpos, ypos))
					{
						/* Pick the ball */
						ispicked = true;
						oldxpos = xpos;
						oldypos = ypos;
						curtype = Board[xpos][ypos];
						Board[xpos][ypos] = CELL_FREE;
						/* Create the walk cache at first touch */
						IsWalkable(xpos, ypos, xpos, ypos, false);
						iscached = true;
					}
				}
				else if (button == BUTTON_REPEAT)
				{
					if (ispicked)
						if (CORRECTMOVE(xpos, ypos, oldxpos, oldypos, iscached))
							need_redraw = true;
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
						/* Cache benchmark */
					/*	unsigned long tick;
						
						tick = *rb->current_tick;
						for (i = 0; i < 5000; i++)
							IsWalkable(xpos, ypos, oldxpos, oldypos, true);
						rb->splashf(HZ*2, "with cache: %lu", (unsigned long)((*rb->current_tick - tick) / (double)HZ * 1000));
						
						tick = *rb->current_tick;
						for (i = 0; i < 5000; i++)
							IsWalkable(xpos, ypos, oldxpos, oldypos, false);
						rb->splashf(HZ*2, "without cache: %lu", (unsigned long)((*rb->current_tick - tick) / (double)HZ * 1000));
					*/	
						if (CORRECTMOVE(xpos, ypos, oldxpos, oldypos, iscached))
						{
							Board[xpos][ypos] = curtype;
							
							if (oldxpos != xpos || oldypos != ypos)
								if (!DoKill())
									State = ADDBALLS;
						}
						else
							Board[oldxpos][oldypos] = curtype;
					
						ispicked = false;
						iscached = false;
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
