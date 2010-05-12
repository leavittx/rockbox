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

void BombsUpdate(Game *game)
{
	int i;
	
	for (i = 0; i < BOMBS_MAX_NUM; i++)
	{
		if (game->field.bombs[i].state < BOMB_PLACED)
			break;
		if (get_tick() - game->field.bombs[i].place_time >= BOMB_DELAY_PHASE4)
		{
			game->field.map[game->field.bombs[i].xpos][game->field.bombs[i].ypos] = SQUARE_FREE;
			game->field.bombs[i].state = BOMB_NONE;
			game->field.bombs[i].owner->bombs_placed--;
		}
		else if (get_tick() - game->field.bombs[i].place_time >= BOMB_DELAY_PHASE3)
		{
			game->field.map[game->field.bombs[i].xpos][game->field.bombs[i].ypos] = SQUARE_FIRE;
			game->field.bombs[i].state = BOMB_EXPL_PHASE4;
		}
		else if (get_tick() - game->field.bombs[i].place_time >= BOMB_DELAY_PHASE2)
		{
			game->field.map[game->field.bombs[i].xpos][game->field.bombs[i].ypos] = SQUARE_FIRE;
			game->field.bombs[i].state = BOMB_EXPL_PHASE3;
		}
		else if (get_tick() - game->field.bombs[i].place_time >= BOMB_DELAY_PHASE1)
		{
			game->field.map[game->field.bombs[i].xpos][game->field.bombs[i].ypos] = SQUARE_FIRE;
			game->field.bombs[i].state = BOMB_EXPL_PHASE2;
		}
		else if (get_tick() - game->field.bombs[i].place_time >= BOMB_DELAY_DET)
		{
			game->field.map[game->field.bombs[i].xpos][game->field.bombs[i].ypos] = SQUARE_FIRE;
			game->field.bombs[i].state = BOMB_EXPL_PHASE1;
		}
	}
}
