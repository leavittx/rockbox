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
#include "draw.h"
#include "pluginbitmaps/balls_board.h"

void HandleTouchscreen(BallsGame *Game)
{
    int button;
    short x, y;

    button = rb->action_get_touchscreen_press(&x, &y);

    if (Game->State == TURN)
    {
        Game->xpos = (x - BRDXOFFSET) / CELLSIZE;
        Game->ypos = (y - BRDYOFFSET) / CELLSIZE;

        if (button == BUTTON_TOUCHSCREEN)
        {
            if (ONBOARD(Game->xpos, Game->ypos) && BUSY(Game->xpos, Game->ypos, Game))
            {
                /* Pick the ball */
                Game->ispicked = true;
                Game->oldxpos = Game->xpos;
                Game->oldypos = Game->ypos;
                Game->curtype = Game->Board[Game->xpos][Game->ypos];
                Game->Board[Game->xpos][Game->ypos] = CELL_FREE;
                /* Create the walk cache at first touch */
                IsWalkable(Game);
                Game->iscached = true;
            }
        }
        else if (button == BUTTON_REPEAT)
        {
            if (Game->ispicked)
                if (CORRECTMOVE(Game))
                    Game->need_redraw = true;
        }
        else if (button == BUTTON_REL)
        {
            Game->Board[Game->oldxpos][Game->oldypos] = Game->curtype;
            Game->ispicked = false;
        }
        else if (button == (BUTTON_REPEAT | BUTTON_REL))
        {
            if (Game->ispicked)
            {
                /* Cache benchmark */
                /*
                unsigned long tick;

                tick = *rb->current_tick;
                for (i = 0; i < 5000; i++)
                    IsWalkable(Game->xpos, Game->ypos, Game->oldGame->xpos, Game->oldGame->ypos, true, Board);
                rb->splashf(HZ*2, "with cache: %lu", (unsigned long)((*rb->current_tick - tick) / (double)HZ * 1000));

                tick = *rb->current_tick;
                for (i = 0; i < 5000; i++)
                    IsWalkable(Game->xpos, Game->ypos, Game->oldGame->xpos, Game->oldGame->ypos, false, Board);
                rb->splashf(HZ*2, "without cache: %lu", (unsigned long)((*rb->current_tick - tick) / (double)HZ * 1000));
               */

                if (CORRECTMOVE(Game))
                {
                    Game->Board[Game->xpos][Game->ypos] = Game->curtype;

                    if (Game->oldxpos != Game->xpos || Game->oldypos != Game->ypos)
                        if (!DoKill(Game))
                            Game->State = ADDBALLS;
                }
                else
                    Game->Board[Game->oldxpos][Game->oldypos] = Game->curtype;

                Game->ispicked = false;
                Game->iscached = false;
                Game->need_redraw = true;
            }
        }
    }
}
