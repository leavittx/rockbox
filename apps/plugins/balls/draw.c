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

/* Bitmaps */
#include "pluginbitmaps/balls_board.h"
#include "pluginbitmaps/balls_balls.h"
#include "pluginbitmaps/balls_background.h"
#include "pluginbitmaps/balls_chooser.h"

#include "game.h"
#include "draw.h"

void Draw(BallsGame *Game)
{
	int i, j;
	
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
			if (BUSY(i, j, Game))
			{
				rb->lcd_bitmap_transparent_part(balls_balls, 0,
					CELLSIZE * (Game->Board[i][j] - 1),
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
			CELLSIZE * (Game->Next[i] - 1),
			STRIDE(SCREEN_MAIN, BALLS_W, BALLS_H),
			LCD_WIDTH - (LCD_WIDTH - (BRDXOFFSET + BRDLEN - BALLSIZE)) / 2,
			(LCD_HEIGHT - N_BALLS_ADD * BALLSIZE) / 2 + BALLSIZE * i,
			BALLSIZE,
			BALLSIZE);
	}

	/* Picked ball */
	if (Game->ispicked)
	{
		rb->lcd_bitmap(balls_chooser,
			BRDXOFFSET + (Game->xpos * CELLSIZE) + ((CELLSIZE - BMPWIDTH_balls_chooser) / 2),
			BRDYOFFSET + (Game->ypos * CELLSIZE) + ((CELLSIZE - BMPHEIGHT_balls_chooser) / 2),
			BMPWIDTH_balls_chooser,
			BMPHEIGHT_balls_chooser);
		
		rb->lcd_bitmap_transparent_part(balls_balls, 0,
			CELLSIZE * (Game->curtype - 1),
			STRIDE(SCREEN_MAIN, BALLS_W, BALLS_H),
			BRDXOFFSET + (Game->xpos * CELLSIZE) + BALL_OFFSET,
			BRDYOFFSET + (Game->ypos * CELLSIZE) + BALL_OFFSET,
			BALLSIZE,
			BALLSIZE);
	}
	
	rb->lcd_update();
	Game->need_redraw = false;
}
