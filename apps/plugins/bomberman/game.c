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
 * Bomberman plugin
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

inline unsigned long get_msec(void)
{
	return *rb->current_tick / HZ * 1000;
}

inline unsigned long get_tick(void)
{
	return *rb->current_tick;
}

void PlayerMoveUp(Game *game, Player *player)
{
	if (player->ypos > 0 && game->field.map[player->xpos][player->ypos - 1] == SQUARE_FREE)
		player->ypos--;
}

void PlayerMoveDown(Game *game, Player *player)
{
	if (player->ypos < MAP_H - 1 && game->field.map[player->xpos][player->ypos + 1] == SQUARE_FREE)
		player->ypos++;
}

void PlayerMoveRight(Game *game, Player *player)
{
	if (player->xpos < MAP_W - 1 && game->field.map[player->xpos + 1][player->ypos] == SQUARE_FREE)
		player->xpos++;
}

void PlayerMoveLeft(Game *game, Player *player)
{
	if (player->xpos > 0 && game->field.map[player->xpos - 1][player->ypos] == SQUARE_FREE)
		player->xpos--;
}

void PlayerPlaceBomb(Game *game, Player *player)
{
	int i;
		
	if (player->bombs_placed >= player->bombs_max &&
		player->bombs_max != -1) /* Infinity */
		return;
	
	for (i = 0; i < BOMBS_MAX_NUM; i++)
		if (game->field.bombs[i].state == BOMB_NONE)
		{
			game->field.bombs[i].state = BOMB_PLACED;
			game->field.bombs[i].xpos = player->xpos;
			game->field.bombs[i].ypos = player->ypos;
			game->field.bombs[i].power = player->bomb_power;
			game->field.bombs[i].place_time = get_tick();
			game->field.bombs[i].owner = player;
			game->field.map[player->xpos][player->ypos] = SQUARE_BOMB;
			player->bombs_placed++;
			break;
		}
}

static bool IsTransparentSquare(SqType type)
{
	if (type == SQUARE_FREE ||
		type == SQUARE_BOMB)
		return true;
		
	return false;
}

static void FirePhaseEnd(Game *game, int x, int y, int rad, FireDir dir)
{
	int j;
	
	for (j = 1; j <= rad; j++)
	{
		int curx, cury;
		
		switch (dir)
		{
			case FIRE_RIGNT:
				curx = x + j;
				cury = y;
				break;
			case FIRE_DOWN:
				curx = x;
				cury = y + j;
				break;
			case FIRE_LEFT:
				curx = x - j;
				cury = y;
				break;
			case FIRE_UP:
				curx = x;
				cury = y - j;
				break;
			default:
				break;
		}
		
		if ((dir == FIRE_RIGNT && curx < MAP_W - 1) ||
			(dir == FIRE_DOWN  && cury < MAP_H - 1) ||
			(dir == FIRE_LEFT  && curx >= 0) ||
			(dir == FIRE_UP    && cury >= 0))
		{
			if (IsTransparentSquare(game->field.map[curx][cury]))
			{
				game->field.firemap[curx][cury].state = BOMB_NONE;
			}
			if (game->field.map[curx][cury] == SQUARE_BOX)
			{
				game->field.firemap[curx][cury].state = BOMB_NONE;
				game->field.map[curx][cury] = SQUARE_FREE;
				break;
			}
			if (game->field.map[curx][cury] == SQUARE_BLOCK)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
}

static void FirePhase4(Game *game, int x, int y, int rad, FireDir dir)
{
	int j;
	
	for (j = 1; j <= rad; j++)
	{
		int curx, cury;
		
		switch (dir)
		{
			case FIRE_RIGNT:
				curx = x + j;
				cury = y;
				break;
			case FIRE_DOWN:
				curx = x;
				cury = y + j;
				break;
			case FIRE_LEFT:
				curx = x - j;
				cury = y;
				break;
			case FIRE_UP:
				curx = x;
				cury = y - j;
				break;
			default:
				break;
		}
		
		if ((dir == FIRE_RIGNT && curx < MAP_W - 1) ||
			(dir == FIRE_DOWN  && cury < MAP_H - 1) ||
			(dir == FIRE_LEFT  && curx >= 0) ||
			(dir == FIRE_UP    && cury >= 0))
		{
			if (IsTransparentSquare(game->field.map[curx][cury]))
			{
				game->field.firemap[curx][cury].state = BOMB_EXPL_PHASE4;
			}
			else if (game->field.map[curx][cury] == SQUARE_BOX)
			{
				game->field.firemap[curx][cury].state = BOMB_EXPL_PHASE4;
				break;
			}
			else if (game->field.map[curx][cury] == SQUARE_BLOCK)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
}

static void FirePhase3(Game *game, int x, int y, int rad, FireDir dir)
{
	int j;
	
	for (j = 1; j <= rad; j++)
	{
		int curx, cury;
		
		switch (dir)
		{
			case FIRE_RIGNT:
				curx = x + j;
				cury = y;
				break;
			case FIRE_DOWN:
				curx = x;
				cury = y + j;
				break;
			case FIRE_LEFT:
				curx = x - j;
				cury = y;
				break;
			case FIRE_UP:
				curx = x;
				cury = y - j;
				break;
			default:
				break;
		}

		if ((dir == FIRE_RIGNT && curx < MAP_W - 1) ||
			(dir == FIRE_DOWN  && cury < MAP_H - 1) ||
			(dir == FIRE_LEFT  && curx >= 0) ||
			(dir == FIRE_UP    && cury >= 0))
		{
			if (IsTransparentSquare(game->field.map[curx][cury]))
			{
				game->field.firemap[curx][cury].state = BOMB_EXPL_PHASE3;
			}
			else if (game->field.map[curx][cury] == SQUARE_BOX)
			{
				game->field.firemap[curx][cury].state = BOMB_EXPL_PHASE3;
				break;
			}
			else if (game->field.map[curx][cury] == SQUARE_BLOCK)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
}

static void FirePhase2(Game *game, int x, int y, int rad, FireDir dir)
{
	int j;
	
	for (j = 1; j <= rad; j++)
	{
		int curx, cury;
		
		switch (dir)
		{
			case FIRE_RIGNT:
				curx = x + j;
				cury = y;
				break;
			case FIRE_DOWN:
				curx = x;
				cury = y + j;
				break;
			case FIRE_LEFT:
				curx = x - j;
				cury = y;
				break;
			case FIRE_UP:
				curx = x;
				cury = y - j;
				break;
			default:
				break;
		}

		if ((dir == FIRE_RIGNT && curx < MAP_W - 1) ||
			(dir == FIRE_DOWN  && cury < MAP_H - 1) ||
			(dir == FIRE_LEFT  && curx >= 0) ||
			(dir == FIRE_UP    && cury >= 0))
		{
			if (IsTransparentSquare(game->field.map[curx][cury]))
			{
				game->field.firemap[curx][cury].state = BOMB_EXPL_PHASE2;
			}
			else if (game->field.map[curx][cury] == SQUARE_BOX)
			{
				game->field.firemap[curx][cury].state = BOMB_EXPL_PHASE2;
				break;
			}
			else if (game->field.map[curx][cury] == SQUARE_BLOCK)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
}

static void FirePhase1(Game *game, int x, int y, int rad, FireDir dir)
{
	int j;
	
	for (j = 1; j <= rad; j++)
	{
		int curx, cury;
		int prevx, prevy;
		
		switch (dir)
		{
			case FIRE_RIGNT:
				curx = x + j;
				cury = y;
				prevx = curx - 1;
				prevy = cury;
				break;
			case FIRE_DOWN:
				curx = x;
				cury = y + j;
				prevx = curx;
				prevy = cury - 1;
				break;
			case FIRE_LEFT:
				curx = x - j;
				cury = y;
				prevx = curx + 1;
				prevy = cury;
				break;
			case FIRE_UP:
				curx = x;
				cury = y - j;
				prevx = curx;
				prevy = cury + 1;
				break;
			default:
				break;
		}
		
		if ((dir == FIRE_RIGNT && curx < MAP_W - 1) ||
			(dir == FIRE_DOWN  && cury < MAP_H - 1) ||
			(dir == FIRE_LEFT  && curx >= 0) ||
			(dir == FIRE_UP    && cury >= 0))
		{
			if (IsTransparentSquare(game->field.map[curx][cury]))
			{
				game->field.firemap[curx][cury].state = BOMB_EXPL_PHASE1;
				game->field.firemap[curx][cury].dir = dir;
				game->field.firemap[curx][cury].isend = false;
			}
			else if (game->field.map[curx][cury] == SQUARE_BOX)
			{
				game->field.firemap[curx][cury].state = BOMB_EXPL_PHASE1;
				game->field.firemap[curx][cury].dir = dir;
				game->field.firemap[curx][cury].isend = true;
				break;
			}
			else if (game->field.map[curx][cury] == SQUARE_BLOCK)
			{
				if (j > 1)
					game->field.firemap[prevx][prevy].isend = true;
				break;
			}
		}
		else
		{
			if (j > 1)
				game->field.firemap[prevy][prevx].isend = true;
			break;
		}
	}
}

void BombsUpdate(Game *game)
{
	int i;
	
	for (i = 0; i < BOMBS_MAX_NUM; i++)
	{
		if (game->field.bombs[i].state < BOMB_PLACED)
			continue;
		
		int x = game->field.bombs[i].xpos, y = game->field.bombs[i].ypos;
		int rad = game->bomb_rad[game->field.bombs[i].power];
		
		if (get_tick() - game->field.bombs[i].place_time >= BOMB_DELAY_PHASE4)
		{
			game->field.bombs[i].state = BOMB_NONE;
			
			game->field.firemap[x][y].state = BOMB_NONE;
			
			FirePhaseEnd(game, x, y, rad, FIRE_RIGNT);
			FirePhaseEnd(game, x, y, rad, FIRE_DOWN);
			FirePhaseEnd(game, x, y, rad, FIRE_LEFT);
			FirePhaseEnd(game, x, y, rad, FIRE_UP);
			
			game->field.bombs[i].owner->bombs_placed--;
			game->field.map[x][y] = SQUARE_FREE;
		}
		else if (get_tick() - game->field.bombs[i].place_time >= BOMB_DELAY_PHASE3)
		{
			//game->field.map[game->field.bombs[i].xpos][game->field.bombs[i].ypos] = SQUARE_FIRE;
			game->field.bombs[i].state = BOMB_EXPL_PHASE4;
			
			game->field.firemap[x][y].state = BOMB_EXPL_PHASE4;

			FirePhase4(game, x, y, rad, FIRE_RIGNT);
			FirePhase4(game, x, y, rad, FIRE_DOWN);
			FirePhase4(game, x, y, rad, FIRE_LEFT);
			FirePhase4(game, x, y, rad, FIRE_UP);
		}
		else if (get_tick() - game->field.bombs[i].place_time >= BOMB_DELAY_PHASE2)
		{
			//game->field.map[game->field.bombs[i].xpos][game->field.bombs[i].ypos] = SQUARE_FIRE;
			game->field.bombs[i].state = BOMB_EXPL_PHASE3;
			
			game->field.firemap[x][y].state = BOMB_EXPL_PHASE3;
			
			FirePhase3(game, x, y, rad, FIRE_RIGNT);
			FirePhase3(game, x, y, rad, FIRE_DOWN);
			FirePhase3(game, x, y, rad, FIRE_LEFT);
			FirePhase3(game, x, y, rad, FIRE_UP);
		}
		else if (get_tick() - game->field.bombs[i].place_time >= BOMB_DELAY_PHASE1)
		{
			//game->field.map[game->field.bombs[i].xpos][game->field.bombs[i].ypos] = SQUARE_FIRE;
			game->field.bombs[i].state = BOMB_EXPL_PHASE2;
			
			game->field.firemap[x][y].state = BOMB_EXPL_PHASE2;
			
			FirePhase2(game, x, y, rad, FIRE_RIGNT);
			FirePhase2(game, x, y, rad, FIRE_DOWN);
			FirePhase2(game, x, y, rad, FIRE_LEFT);
			FirePhase2(game, x, y, rad, FIRE_UP);
		}
		else if (get_tick() - game->field.bombs[i].place_time >= BOMB_DELAY_DET)
		{
			//game->field.map[game->field.bombs[i].xpos][game->field.bombs[i].ypos] = SQUARE_FIRE;
			game->field.bombs[i].state = BOMB_EXPL_PHASE1;
			
			game->field.firemap[x][y].state = BOMB_EXPL_PHASE1;
			game->field.firemap[x][y].dir = FIRE_CENTER;
			
			FirePhase1(game, x, y, rad, FIRE_RIGNT);
			FirePhase1(game, x, y, rad, FIRE_DOWN);
			FirePhase1(game, x, y, rad, FIRE_LEFT);
			FirePhase1(game, x, y, rad, FIRE_UP);
		}
	}
}
