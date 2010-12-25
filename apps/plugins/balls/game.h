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

#ifndef _GAME_H
#define _GAME_H

/* Board length in cells */
#define NCELLS 9

#define N_BALL_TYPES 7
#define N_BALLS_KILL 5
#define N_BALLS_ADD 3

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

typedef struct {
    enum {
        ADDBALLS,
        TURN,
        GAMEOVER
    } State; /* State of game */

    Celltype Board[NCELLS][NCELLS]; /* Game board */
    Celltype Next[N_BALLS_ADD]; /* Next balls */

    int Score; /* Player's score */

    bool ispicked;
    short oldxpos, oldypos;
    short xpos, ypos;
    Celltype curtype;
    bool iscached;

    bool need_redraw;
} BallsGame;

int GETBALL(void);
bool ONBOARD(short x, short y);
bool FREE(short x, short y, BallsGame *Game);
bool BUSY(short x, short y, BallsGame *Game);
bool CORRECTMOVE(BallsGame *Game);

void Init(BallsGame *Game);
void AddBalls(BallsGame *Game);
/* Does some magic */
bool DoKill(BallsGame *Game);
/* Counts number of free cells on the board */
int FreeCellsNum(BallsGame *Game);
/* Marks all cells in Walk to which exists path form cell (x, y) */
void FillWalkableCells(bool (*Walk)[NCELLS], BallsGame *Game);
/* Checks if there is a path from cell (x1, y1) to cell (x2, y2) */
bool IsWalkable(BallsGame *Game);

#endif /* _GAME_H */
