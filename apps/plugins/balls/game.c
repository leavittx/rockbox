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

#include "game.h"

int GETBALL(void)
{
    return
            rb->rand() % N_BALL_TYPES + 1;
}

bool ONBOARD(short x, short y)
{
    return
            x >= 0 && x < NCELLS &&
            y >= 0 && y < NCELLS;
}

bool FREE(short x, short y, BallsGame *Game)
{
    return
            Game->Board[x][y] == CELL_FREE;
}

bool BUSY(short x, short y, BallsGame *Game)
{
    return
            Game->Board[x][y] != CELL_FREE;
}

bool CORRECTMOVE(BallsGame *Game)
{
    return
            ONBOARD(Game->xpos, Game->ypos) && \
            FREE(Game->xpos, Game->ypos, Game) && \
            IsWalkable(Game);
}

void Init(BallsGame *Game)
{
    int i;

    Game->State = ADDBALLS;
    Game->need_redraw = true;
    Game->Score = 0;
    Game->ispicked = false;
    Game->curtype = CELL_FREE;
    Game->oldxpos = Game->oldypos = 0;
    Game->xpos = Game->ypos = 0;
    Game->iscached = false;
    memset(Game->Board, 0, sizeof(Game->Board));

    /* Prepare first balls */
    for (i = 0; i < N_BALLS_ADD; i++)
        Game->Next[i] = GETBALL();
}

void AddBalls(BallsGame *Game)
{
    int i;
    int nadd, nadded = 0;

    if ((nadd = FreeCellsNum(Game)) > N_BALLS_ADD)
        nadd = N_BALLS_ADD;

    while (nadded != nadd)
    {
        Game->xpos = rb->rand() % NCELLS;
        Game->ypos = rb->rand() % NCELLS;
        if (FREE(Game->xpos, Game->ypos, Game))
            Game->Board[Game->xpos][Game->ypos] = Game->Next[nadded++];
    }

    for (i = 0; i < N_BALLS_ADD; i++)
        Game->Next[i] = GETBALL();

    if (!DoKill(Game) && nadd < N_BALLS_ADD)
        Game->State = GAMEOVER;
    else
        Game->State = TURN;

    Game->need_redraw = true;
}

/* Does some magic */
bool DoKill(BallsGame *Game)
{
    int i, j;
    bool Kill[NCELLS][NCELLS] = {{false}, {false}};
    int len;
    Celltype prev;
    int newscore = 0;

    /* Vertical */
    for (i = 0; i < NCELLS; i++)
    {
        prev = Game->Board[i][0];
        len = 1;

        for (j = 1; j < NCELLS; j++)
        {
            if (Game->Board[i][j] == prev && BUSY(i, j, Game))
                len++;
            else
            {
                if (len >= N_BALLS_KILL)
                    for (; len; len--)
                        Kill[i][j - len] = true;

                prev = Game->Board[i][j];
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
        prev = Game->Board[0][i];
        len = 1;

        for (j = 1; j < NCELLS; j++)
        {
            if (Game->Board[j][i] == prev && BUSY(j, i, Game))
                len++;
            else
            {
                if (len >= N_BALLS_KILL)
                    for (; len; len--)
                        Kill[j - len][i] = true;

                prev = Game->Board[j][i];
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
        prev = Game->Board[i][0];
        len = 1;

        for (j = i + 1; j < NCELLS; j++)
        {
            if (Game->Board[j][j - i] == prev && BUSY(j, j - i, Game))
                len++;
            else
            {
                if (len >= N_BALLS_KILL)
                    for (; len; len--)
                        Kill[j - len][j - i - len] = true;

                prev = Game->Board[j][j - i];
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
        prev = Game->Board[0][i];
        len = 1;

        for (j = i + 1; j < NCELLS; j++)
        {
            if (Game->Board[j - i][j] == prev && BUSY(j - i, j, Game))
                len++;
            else
            {
                if (len >= N_BALLS_KILL)
                    for (; len; len--)
                        Kill[j - i - len][j - len] = true;

                prev = Game->Board[j - i][j];
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
        prev = Game->Board[i][0];
        len = 1;

        for (j = i - 1; j >= 0; j--)
        {
            if (Game->Board[j][i - j] == prev && BUSY(j, i - j, Game))
                len++;
            else
            {
                if (len >= N_BALLS_KILL)
                    for (; len; len--)
                        Kill[j + len][i - j - len] = true;

                prev = Game->Board[j][i - j];
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
        prev = Game->Board[NCELLS - 1][i];
        len = 1;

        for (j = i + 1; j < NCELLS; j++)
        {
            if (Game->Board[NCELLS - 1 - (j - i)][j] == prev && BUSY(NCELLS - 1 - (j - i), j, Game))
                len++;
            else
            {
                if (len >= N_BALLS_KILL)
                    for (; len; len--)
                        Kill[NCELLS - 1 - (j - i) + len][j - len] = true;

                prev = Game->Board[NCELLS - 1 - (j - i)][j];
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
                Game->Board[i][j] = CELL_FREE;
                /* TODO: count score in proper way */
                newscore += 5;
            }

    Game->Score += newscore;

    if (newscore)
        return true;
    return false;
}

/* Counts number of free cells on the board */
int FreeCellsNum(BallsGame *Game)
{
    int i, j;
    int nfree = NCELLS * NCELLS;

    for (i = 0; i < NCELLS; i++)
        for (j = 0; j < NCELLS; j++)
            if (BUSY(i, j, Game))
                nfree--;

    return nfree;
}

/* Marks all cells in Walk to which exists path form cell (x, y) */
void FillWalkableCells(bool (*Walk)[NCELLS], BallsGame *Game)
{
    int i, j;
    bool isanynew;

    Walk[Game->oldxpos][Game->oldypos] = true;

    while (1)
    {
        isanynew = false;

        for (i = 0; i < NCELLS; i++)
            for (j = 0; j < NCELLS; j++)
                if (Walk[i][j])
                {
                    if (i < NCELLS - 1 && !Walk[i + 1][j] && FREE(i + 1, j, Game))
                        Walk[i + 1][j] = true, isanynew = true;

                    if (i > 0 && !Walk[i - 1][j] && FREE(i - 1, j, Game))
                        Walk[i - 1][j] = true, isanynew = true;

                    if (j < NCELLS - 1 && !Walk[i][j + 1] && FREE(i, j + 1, Game))
                        Walk[i][j + 1] = true, isanynew = true;

                    if (j > 0 && !Walk[i][j - 1] && FREE(i, j - 1, Game))
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
bool IsWalkable(BallsGame *Game)
{
    bool Walk[NCELLS][NCELLS] = {{false}, {false}};
    static bool WalkCache[NCELLS][NCELLS];

    if (!Game->iscached) /* Make the cache */
    {
        FillWalkableCells(Walk, Game);
        memcpy(WalkCache, Walk, sizeof(Walk));
    }

    return WalkCache[Game->xpos][Game->ypos];
}
