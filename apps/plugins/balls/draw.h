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

#ifndef _DRAW_H
#define _DRAW_H

//#include "game.h"

/* Board length in pixels. Board always is a square, only one dimention then */
#define BRDLEN BMPWIDTH_balls_board
#define BRDYOFFSET ((LCD_HEIGHT - BRDLEN) / 2)
#define BRDEXTRAXOFFSET 10
#define BRDXOFFSET (BRDYOFFSET + BRDEXTRAXOFFSET)
/* Length of cell in pixels */
#define CELLSIZE (BMPWIDTH_balls_board / NCELLS)
#define BALLS_W BMPWIDTH_balls_balls
#define BALLS_H BMPHEIGHT_balls_balls
#define BALLSIZE BALLS_W
#define BALL_OFFSET ((CELLSIZE - BALLSIZE) / 2)

void Draw(BallsGame *Game);

#endif /* _DRAW_H */
