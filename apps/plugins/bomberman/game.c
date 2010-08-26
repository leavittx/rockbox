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
	if (player->ismove)
		return;
		
	if (player->rxpos != 0)
	{
		if (player->rxpos == -1)
			PlayerMoveRight(game, player);
		else
			PlayerMoveLeft(game, player);
		return;
	}
	
	if (player->look != LOOK_UP)
	{
		player->look = LOOK_UP;
		return;
	}
	
	if ((player->ypos > 0 && game->field.map[player->xpos][player->ypos - 1] == SQUARE_FREE)
		|| player->rypos == 1)
	{
		player->ismove = true;
		player->move_phase = 1;
		player->move_start_time = get_tick();
	}
}

void PlayerMoveDown(Game *game, Player *player)
{		
	if (player->ismove)
		return;
		
	if (player->rxpos != 0)
	{
		if (player->rxpos == -1)
			PlayerMoveRight(game, player);
		else
			PlayerMoveLeft(game, player);
		return;
	}
	
	if (player->look != LOOK_DOWN)
	{
		player->look = LOOK_DOWN;
		return;
	}
	
	if ((player->ypos < MAP_H - 1 && game->field.map[player->xpos][player->ypos + 1] == SQUARE_FREE)
		|| player->rypos == -1)
	{
		player->ismove = true;
		player->move_phase = 1;
		player->move_start_time = get_tick();
	}
}

void PlayerMoveRight(Game *game, Player *player)
{
	if (player->ismove)
		return;
		
	if (player->rypos != 0)
	{
		if (player->rypos == -1)
			PlayerMoveDown(game, player);
		else
			PlayerMoveUp(game, player);
		return;
	}
	
	if (player->look != LOOK_RIGHT)
	{
		player->look = LOOK_RIGHT;
		return;
	}
	
	if ((player->xpos < MAP_W - 1 && game->field.map[player->xpos + 1][player->ypos] == SQUARE_FREE)
		|| player->rxpos == -1)
	{
		player->ismove = true;
		player->move_phase = 1;
		player->move_start_time = get_tick();
	}
}

void PlayerMoveLeft(Game *game, Player *player)
{
	if (player->ismove)
		return;
		
	if (player->rypos != 0)
	{
		if (player->rypos == -1)
			PlayerMoveDown(game, player);
		else
			PlayerMoveUp(game, player);
		return;
	}
	
	if (player->look != LOOK_LEFT)
	{
		player->look = LOOK_LEFT;
		return;
	}
	
	if ((player->xpos > 0 && game->field.map[player->xpos - 1][player->ypos] == SQUARE_FREE)
		|| player->rxpos == 1)
	{
		player->ismove = true;
		player->move_phase = 1;
		player->move_start_time = get_tick();
	}
}

void UpdatePlayer(Player *player)
{
	if (player->ismove)
	{
		if ((get_tick() - player->move_start_time) / PLAYER_MOVE_PART_TIME > player->move_phase)
		{
			if (player->move_phase == 5)
			{
				player->ismove = false;
				player->move_phase = 0;
				
				if (player->look == LOOK_UP)
				{
					if (player->rypos == -1)
					{
						player->ypos--;
						player->rypos = 1;
					}
					else
						player->rypos--;
				}
				else if (player->look == LOOK_DOWN)
				{
					if (player->rypos == 1)
					{
						player->ypos++;
						player->rypos = -1;
					}
					else
						player->rypos++;
				}
				else if (player->look == LOOK_RIGHT)
				{
					if (player->rxpos == 1)
					{
						player->xpos++;
						player->rxpos = -1;
					}
					else
						player->rxpos++;
				}
				else /* LOOK_LEFT */
				{
					if (player->rxpos == -1)
					{
						player->xpos--;
						player->rxpos = 1;
					}
					else
						player->rxpos--;
				}
			}
			else
				player->move_phase++;
		}
	}
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
			game->field.det[player->xpos][player->ypos] = DET_PHASE1;
			player->bombs_placed++;
			break;
		}
}

static bool IsTransparentSquare(Field *field, int x, int y)
{
	if (field->map[x][y] == SQUARE_FREE ||
		field->map[x][y] == SQUARE_BOMB ||
		(field->map[x][y] == SQUARE_BOX && field->boxes[x][y].state > HUNKY))
		return true;
		
	return false;
}

static void FirePhaseEnd(Game *game, int x, int y, int rad, FireDir dir)
{
	int j;
	
	for (j = 1; j <= rad; j++)
	{
		int curx = 0, cury = 0;
		
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
			if (IsTransparentSquare(&game->field, curx, cury))
			{
				game->field.firemap[curx][cury].state = BOMB_NONE;
			}
			else if (game->field.map[curx][cury] == SQUARE_BOX)
			{
				game->field.firemap[curx][cury].state = BOMB_NONE;
				//game->field.map[curx][cury] = SQUARE_FREE;
				game->field.boxes[curx][cury].state = BOX_EXPL_PHASE1;
				game->field.boxes[curx][cury].expl_time = get_tick();
				break;
			}
			else if (game->field.map[curx][cury] == SQUARE_BLOCK)
			{
				break;
			}
			if (game->players[0].xpos == curx && game->players[0].ypos == cury)
			{
				game->players[0].status.health = 0;
				game->players[0].status.state = EXPL_PHASE1;
				game->players[0].status.time_of_death = get_tick();
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
		int curx = 0, cury = 0;
		
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
			if (IsTransparentSquare(&game->field, curx, cury))
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
		int curx = 0, cury = 0;
		
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
			if (IsTransparentSquare(&game->field, curx, cury))
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
		int curx = 0, cury = 0;
		
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
			if (IsTransparentSquare(&game->field, curx, cury))
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
		int curx = 0, cury = 0;
		int prevx = 0, prevy = 0;
		
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
			if (IsTransparentSquare(&game->field, curx, cury))
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

void UpdateBombs(Game *game)
{
	int i;
	static int detphases[4] = { 0, 1, 2, 1 }; // This helps with detonation animation
	
	for (i = 0; i < BOMBS_MAX_NUM; i++)
	{
		if (game->field.bombs[i].state < BOMB_PLACED)
			continue;
		
		int x = game->field.bombs[i].xpos, y = game->field.bombs[i].ypos;
		int rad = game->bomb_rad[game->field.bombs[i].power];
		
		/* Update detonation animation */
		game->field.det[x][y] = detphases[((get_tick() - game->field.bombs[i].place_time) / BOMB_DELAY_DET_ANIM) % 4];
		
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

void UpdateBoxes(Game *game)
{
	int i, j;
	
	for (i = 0; i < MAP_W; i++)
		for (j = 0; j < MAP_H; j++)
			if (game->field.map[i][j] == SQUARE_BOX && game->field.boxes[i][j].state > HUNKY)
			{
				//game->field.boxes[i][j].state++;
				//	break;
				game->field.boxes[i][j].state = 
					(get_tick() - game->field.boxes[i][j].expl_time) / BOX_DELAY_EXPLOSION_ANIM + 1;
				if (game->field.boxes[i][j].state > BOX_EXPL_PHASE5)
					game->field.map[i][j] = SQUARE_FREE;
			}
}