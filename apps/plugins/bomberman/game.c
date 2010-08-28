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

#define swap(a, b) { \
		register Player *tmp = a; \
		a = b; \
		b = tmp; \
}

inline unsigned long get_tick(void)
{
	return *rb->current_tick;
}

void PlayerMoveUp(Game *game, Player *player)
{
	if (player->ismove || player->status.state != ALIVE)
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
		player->move_start_time = tick;
	}
}

void PlayerMoveDown(Game *game, Player *player)
{
	if (player->ismove || player->status.state != ALIVE)
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
		player->move_start_time = tick;
	}
}

void PlayerMoveRight(Game *game, Player *player)
{
	if (player->ismove || player->status.state != ALIVE)
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
		player->move_start_time = tick;
	}
}

void PlayerMoveLeft(Game *game, Player *player)
{
	if (player->ismove || player->status.state != ALIVE)
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
		player->move_start_time = tick;
	}
}

static void RecalcDrawOrder(Game *game)
{
	int i, j, max;
	
	max = MAX_PLAYERS - 1;	
	for (j = max; j > 0; j--)
		for (i = 0; i < max; i++)
		{
			if (game->draw_order[i]->ypos > game->draw_order[i + 1]->ypos)
			{
				swap(game->draw_order[i], game->draw_order[i + 1]);
				//rb->splash(HZ, "swap0");
			}
			else if (game->draw_order[i]->ypos == game->draw_order[i + 1]->ypos)
			{
				if (game->draw_order[i]->rypos > game->draw_order[i + 1]->rypos)
				{
					swap(game->draw_order[i], game->draw_order[i + 1]);
					//rb->splash(HZ, "swap1");
				}
			}
		}
}

void PickBonus(Game *game, Player *player)
{
	int x = player->xpos, y = player->ypos;
	
	switch (game->field.bonuses[x][y])
	{
		case BONUS_EXTRABOMB:
			player->bombs_max++;
			break;
		case BONUS_POWER:
			if (player->bomb_power < BOMB_PWR_KILLER)
				player->bomb_power++;
			break;
		case BONUS_SPEEDUP:
			break;
		case BONUS_FULLPOWER:
			break;
		case BONUS_NONE:
			break;
		default:
			break;
	}
	
	game->field.bonuses[x][y] = BONUS_NONE;
}

int UpdatePlayer(Game *game, Player *player)
{
	if (player->status.state == ALIVE)
	{
		if (player->ismove)
		{
			if ((tick - player->move_start_time) / PLAYER_MOVE_PART_TIME > player->move_phase)
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
					
					RecalcDrawOrder(game);
					PickBonus(game, player);
				}
				else
					player->move_phase++;
			}
		}
	}
	else if (player->status.state > GONNA_DIE && player->status.state < DEAD)
	{
		// 2 -- ALIVE, GONNA_DIE
		player->status.state = 
			(tick - player->status.time_of_death) / PLAYER_DELAY_DEATH_ANIM + 2;
			
		return player->status.state;
	}
	else if (player->status.state > DEAD)
	{
		static unsigned long won = 0;
		
		if (!won)
			won = tick;
		
		if (tick - won > PLAYER_DELAY_WIN_ANIM_DUR)
		{
			return -1;
		}
		
		player->status.state =
			WIN_PHASE1 + (int)((tick - won) / PLAYER_DELAY_WIN_ANIM) % 2;
	}

	return 0;
}

void PlayerPlaceBomb(Game *game, Player *player)
{
	int i;
	
	if (player->status.state != ALIVE)
		return;
		
	if (player->bombs_placed >= player->bombs_max &&
		player->bombs_max != -1) /* Infinity */
		return;
	
	for (i = 0; i < MAX_BOMBS; i++)
		if (game->field.bombs[i].state > BOMB_NONE &&
			game->field.bombs[i].xpos == player->xpos &&
			game->field.bombs[i].ypos == player->ypos)
		{
			return;
		}
		
	for (i = 0; i < MAX_BOMBS; i++)
		if (game->field.bombs[i].state == BOMB_NONE)
		{
			game->field.bombs[i].state = BOMB_PLACED;
			game->field.bombs[i].xpos = player->xpos;
			game->field.bombs[i].ypos = player->ypos;
			game->field.bombs[i].power = player->bomb_power;
			game->field.bombs[i].place_time = tick;
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
	   (field->map[x][y] == SQUARE_BOX && field->boxes[x][y].state > HUNKY))
		return true;
		
	return false;
}

static void FirePhaseEnd(Game *game, int x, int y, int rad, FireDir dir)
{
	int j;
	int i;
	
	/* Kill player in the center of explosion */
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		if (game->players[i].xpos == x && game->players[i].ypos == y &&
		   (game->players[i].status.state == ALIVE ||
			game->players[i].status.state == GONNA_DIE))
		{
			game->players[i].status.health = 0;
			game->players[i].status.state = EXPL_PHASE1;
			game->players[i].status.time_of_death = tick;
		}
	}
	
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
				game->field.boxes[curx][cury].expl_time = tick;
				break;
			}
			else if (game->field.map[curx][cury] == SQUARE_BLOCK
				|| game->field.map[curx][cury] == SQUARE_BOMB)
			{
				break;
			}
			// todo: figure out if we need detonate other bombs on this phase or not
			for (i = 0; i < MAX_PLAYERS; i++)
			{
				if (game->players[i].xpos == curx && game->players[i].ypos == cury &&
				   (game->players[i].status.state == ALIVE ||
					game->players[i].status.state == GONNA_DIE))
				{
					game->players[i].status.health = 0;
					game->players[i].status.state = EXPL_PHASE1;
					game->players[i].status.time_of_death = tick;
				}
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
	int i;
	
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
			
			// Detonate other bombs
			else if (game->field.map[curx][cury] == SQUARE_BOMB)
			{
				for (i = 0; i < MAX_BOMBS; i++)
				{
					if (game->field.bombs[i].xpos == curx &&
						game->field.bombs[i].ypos == cury &&
						game->field.bombs[i].state == BOMB_PLACED)
					{
						game->field.bombs[i].place_time = tick - BOMB_DELAY_DET;
						break;
					}
				}
			}
			
			// Player gets killed by explosion
			for (i = 0; i < MAX_PLAYERS; i++)
			{
				if (game->players[i].xpos == curx && game->players[i].ypos == cury &&
					game->players[i].status.state == ALIVE)
				{
					game->players[i].status.state = GONNA_DIE;
				}
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
	int i;
	
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
			
			// Detonate other bombs
			else if (game->field.map[curx][cury] == SQUARE_BOMB)
			{
				for (i = 0; i < MAX_BOMBS; i++)
				{
					if (game->field.bombs[i].xpos == curx &&
						game->field.bombs[i].ypos == cury &&
						game->field.bombs[i].state == BOMB_PLACED)
					{
						game->field.bombs[i].place_time = tick - BOMB_DELAY_DET;
						break;
					}
				}
			}
			
			// Player gets killed by explosion
			for (i = 0; i < MAX_PLAYERS; i++)
			{
				if (game->players[i].xpos == curx && game->players[i].ypos == cury &&
					game->players[i].status.state == ALIVE)
				{
					game->players[i].status.state = GONNA_DIE;
				}
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
	int i;
	
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
			
			// Detonate other bombs
			else if (game->field.map[curx][cury] == SQUARE_BOMB)
			{
				for (i = 0; i < MAX_BOMBS; i++)
				{
					if (game->field.bombs[i].xpos == curx &&
						game->field.bombs[i].ypos == cury &&
						game->field.bombs[i].state == BOMB_PLACED)
					{
						game->field.bombs[i].place_time = tick - BOMB_DELAY_DET;
						break;
					}
				}
			}
			
			// Player gets killed by explosion
			for (i = 0; i < MAX_PLAYERS; i++)
			{
				if (game->players[i].xpos == curx && game->players[i].ypos == cury &&
					game->players[i].status.state == ALIVE)
				{
					game->players[i].status.state = GONNA_DIE;
				}
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
	int i;
	
	/* Kill player in the center of explosion */
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		if (game->players[i].xpos == x && game->players[i].ypos == y &&
			game->players[i].status.state == ALIVE)
		{
			game->players[i].status.state = GONNA_DIE;
		}
	}
	
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
				if (j == rad)
					game->field.firemap[curx][cury].isend = true;
				else
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
			
			// Detonate other bombs
			else if (game->field.map[curx][cury] == SQUARE_BOMB)
			{
				for (i = 0; i < MAX_BOMBS; i++)
				{
					if (game->field.bombs[i].xpos == curx &&
						game->field.bombs[i].ypos == cury &&
						game->field.bombs[i].state == BOMB_PLACED)
					{
						game->field.bombs[i].place_time = tick - BOMB_DELAY_DET;
						// todo: maybe remove it
						//if (j > 1)
						//	game->field.firemap[prevx][prevy].isend = true;
						break;
					}
				}
			}
			
			// Player gets killed by explosion
			for (i = 0; i < MAX_PLAYERS; i++)
			{
				if (game->players[i].xpos == curx && game->players[i].ypos == cury &&
					game->players[i].status.state == ALIVE)
				{
					game->players[i].status.state = GONNA_DIE;
				}
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
	
	for (i = 0; i < MAX_BOMBS; i++)
	{
		if (game->field.bombs[i].state < BOMB_PLACED)
			continue;
		
		int x = game->field.bombs[i].xpos, y = game->field.bombs[i].ypos;
		int rad = game->bomb_rad[game->field.bombs[i].power];
		
		// todo: check if it neccecary to compute this
		/* Update detonation animation */
		game->field.det[x][y] = detphases[((tick - game->field.bombs[i].place_time) / BOMB_DELAY_DET_ANIM) % 4];
		
		if (tick - game->field.bombs[i].place_time >= BOMB_DELAY_PHASE4)
		{
			game->field.bombs[i].state = BOMB_NONE;
			
			game->field.firemap[x][y].state = BOMB_NONE;
			
			FirePhaseEnd(game, x, y, rad, FIRE_RIGNT);
			FirePhaseEnd(game, x, y, rad, FIRE_DOWN);
			FirePhaseEnd(game, x, y, rad, FIRE_LEFT);
			FirePhaseEnd(game, x, y, rad, FIRE_UP);
			
			game->field.bombs[i].owner->bombs_placed--;
		}
		else if (tick - game->field.bombs[i].place_time >= BOMB_DELAY_PHASE3)
		{
			//game->field.map[game->field.bombs[i].xpos][game->field.bombs[i].ypos] = SQUARE_FIRE;
			game->field.bombs[i].state = BOMB_EXPL_PHASE4;
			
			game->field.firemap[x][y].state = BOMB_EXPL_PHASE4;

			FirePhase4(game, x, y, rad, FIRE_RIGNT);
			FirePhase4(game, x, y, rad, FIRE_DOWN);
			FirePhase4(game, x, y, rad, FIRE_LEFT);
			FirePhase4(game, x, y, rad, FIRE_UP);
		}
		else if (tick - game->field.bombs[i].place_time >= BOMB_DELAY_PHASE2)
		{
			//game->field.map[game->field.bombs[i].xpos][game->field.bombs[i].ypos] = SQUARE_FIRE;
			game->field.bombs[i].state = BOMB_EXPL_PHASE3;
			
			game->field.firemap[x][y].state = BOMB_EXPL_PHASE3;
			
			FirePhase3(game, x, y, rad, FIRE_RIGNT);
			FirePhase3(game, x, y, rad, FIRE_DOWN);
			FirePhase3(game, x, y, rad, FIRE_LEFT);
			FirePhase3(game, x, y, rad, FIRE_UP);
		}
		else if (tick - game->field.bombs[i].place_time >= BOMB_DELAY_PHASE1)
		{
			//game->field.map[game->field.bombs[i].xpos][game->field.bombs[i].ypos] = SQUARE_FIRE;
			game->field.bombs[i].state = BOMB_EXPL_PHASE2;
			
			game->field.firemap[x][y].state = BOMB_EXPL_PHASE2;
			
			FirePhase2(game, x, y, rad, FIRE_RIGNT);
			FirePhase2(game, x, y, rad, FIRE_DOWN);
			FirePhase2(game, x, y, rad, FIRE_LEFT);
			FirePhase2(game, x, y, rad, FIRE_UP);
		}
		else if (tick - game->field.bombs[i].place_time >= BOMB_DELAY_DET)
		{
			//game->field.map[game->field.bombs[i].xpos][game->field.bombs[i].ypos] = SQUARE_FIRE;
			game->field.map[x][y] = SQUARE_FREE;
			
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
					(tick - game->field.boxes[i][j].expl_time) / BOX_DELAY_EXPLOSION_ANIM + 1;
				if (game->field.boxes[i][j].state > BOX_EXPL_PHASE5)
					game->field.map[i][j] = SQUARE_FREE;
			}
}
