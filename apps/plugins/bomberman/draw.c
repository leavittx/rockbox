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

/* Bitmaps */
#include "pluginbitmaps/bomberman_player.h"
#include "pluginbitmaps/bomberman_box.h"
#include "pluginbitmaps/bomberman_block.h"
#include "pluginbitmaps/bomberman_bomb.h"
#include "pluginbitmaps/bomberman_explode.h"
#include "pluginbitmaps/bomberman_player_move.h"
#include "pluginbitmaps/bomberman_player_death.h"
#include "pluginbitmaps/bomberman_cc.h"

#include "game.h"
#include "draw.h"

#define SQUARE_SIZE 16 //todo
#define XMAPOFFSET 25
#define YMAPOFFSET 30

void Draw(Game *game)
{	
	int i, j;
	
	rb->lcd_clear_display();	
	
	/* Chaos constructions logo */
	rb->lcd_bitmap_transparent(bomberman_cc,
		0,
		(LCD_HEIGHT - BMPHEIGHT_bomberman_cc) - 26,
		BMPWIDTH_bomberman_cc,
		BMPHEIGHT_bomberman_cc);
	
	/* Different objects on the field */
	for (i = 0; i < MAP_W; i++)
		for (j = 0; j < MAP_H; j++)
			switch (game->field.map[i][j])
			{
				case SQUARE_FREE:
					break;
				case SQUARE_BOX:
					/*
					rb->lcd_bitmap_transparent(bomberman_box,
						i * SQUARE_SIZE + XMAPOFFSET,
						j * SQUARE_SIZE + YMAPOFFSET,
						BMPWIDTH_bomberman_box,
						BMPHEIGHT_bomberman_box);
					*/
					rb->lcd_bitmap_transparent_part(bomberman_box,
						game->field.boxes[i][j].state * SQUARE_SIZE,
						0,
						STRIDE(SCREEN_MAIN, BMPWIDTH_bomberman_box, BMPHEIGHT_bomberman_box),
						i * SQUARE_SIZE + XMAPOFFSET,
						j * SQUARE_SIZE + YMAPOFFSET,
						SQUARE_SIZE,
						SQUARE_SIZE);
					break;
				case SQUARE_BLOCK:
					rb->lcd_bitmap_transparent(bomberman_block,
						i * SQUARE_SIZE + XMAPOFFSET,
						j * SQUARE_SIZE + YMAPOFFSET,
						BMPWIDTH_bomberman_block,
						BMPHEIGHT_bomberman_block);
					break;
				case SQUARE_BOMB:
					/*
					rb->lcd_bitmap_transparent(bomberman_bomb,
						i * SQUARE_SIZE + XMAPOFFSET,
						j * SQUARE_SIZE + YMAPOFFSET,
						BMPWIDTH_bomberman_bomb,
						BMPHEIGHT_bomberman_bomb);
					*/
					rb->lcd_bitmap_transparent_part(bomberman_bomb,
						game->field.det[i][j] * SQUARE_SIZE,
						0,
						STRIDE(SCREEN_MAIN, BMPWIDTH_bomberman_bomb, BMPHEIGHT_bomberman_bomb),
						i * SQUARE_SIZE + XMAPOFFSET,
						j * SQUARE_SIZE + YMAPOFFSET,
						SQUARE_SIZE,
						SQUARE_SIZE);
					break;
				/*case SQUARE_FIRE:
					rb->lcd_bitmap_transparent_part(bomberman_explode,
						0,
						0,
						STRIDE(SCREEN_MAIN, BMPWIDTH_bomberman_explode, BMPHEIGHT_bomberman_explode),
						i * SQUARE_SIZE + XMAPOFFSET,
						j * SQUARE_SIZE + YMAPOFFSET,
						SQUARE_SIZE,
						SQUARE_SIZE);
					break;*/
			}
	/*
	for (i = 0; i < BOMBS_MAX_NUM; i++)
	{
		else
		if (game->field.bombs[i].state == BOMB_PLACED)
		{
			rb->lcd_bitmap_transparent(bomberman_bomb,
				game->field.bombs[i].xpos * SQUARE_SIZE + XMAPOFFSET,
				game->field.bombs[i].ypos * SQUARE_SIZE + YMAPOFFSET,
				BMPWIDTH_bomberman_bomb,
				BMPHEIGHT_bomberman_bomb);
		}
	}
	*/
	
	// player without movement
	/*
	rb->lcd_bitmap_transparent(bomberman_player,
		game->players[i].xpos * SQUARE_SIZE + XMAPOFFSET,
		game->players[i].ypos * SQUARE_SIZE + YMAPOFFSET -
			(BMPHEIGHT_bomberman_player - SQUARE_SIZE),
		BMPWIDTH_bomberman_player,
		BMPHEIGHT_bomberman_player);
	*/
	
	/* Player (with movement animation) */
	//int xcoord[3] = { 1, 6, 12 };
	int xcoord[3] = { -4, 0, 4 };
	//int ycoord[3] = { 3, 9, 14 };
	//int ycoord[3] = { 1, 6, 12 };
	//int ycoord[3] = { 12, 6, 1 };
	int ycoord[3] = { 9, 3, -2 };
	
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		if (game->players[i].ismove)
		{
			int curphase;
			
			if (game->players[i].move_phase <= 3)
				curphase = game->players[i].move_phase;
			else if (game->players[i].move_phase == 4)
				curphase = 2;
			else if (game->players[i].move_phase == 5)
				curphase = 0;
				
			// todo: make common code for different direction
			if (game->players[i].look == LOOK_UP)
			{
				rb->lcd_bitmap_transparent_part(bomberman_player_move,
					curphase * BMPWIDTH_bomberman_player,
					game->players[i].look * BMPHEIGHT_bomberman_player,
					STRIDE(SCREEN_MAIN, BMPWIDTH_bomberman_player_move, BMPHEIGHT_bomberman_player_move),
					game->players[i].xpos * SQUARE_SIZE + XMAPOFFSET + 
						xcoord[game->players[i].rxpos + 1],
					game->players[i].ypos * SQUARE_SIZE + YMAPOFFSET -
						(BMPHEIGHT_bomberman_player - SQUARE_SIZE) -
						 ycoord[game->players[i].rypos + 1] - game->players[i].move_phase,
					BMPWIDTH_bomberman_player,
					BMPHEIGHT_bomberman_player);
			}
			else if (game->players[i].look == LOOK_DOWN)
			{
				rb->lcd_bitmap_transparent_part(bomberman_player_move,
					curphase * BMPWIDTH_bomberman_player,
					game->players[i].look * BMPHEIGHT_bomberman_player,
					STRIDE(SCREEN_MAIN, BMPWIDTH_bomberman_player_move, BMPHEIGHT_bomberman_player_move),
					game->players[i].xpos * SQUARE_SIZE + XMAPOFFSET + 
						xcoord[game->players[i].rxpos + 1],
					game->players[i].ypos * SQUARE_SIZE + YMAPOFFSET -
						(BMPHEIGHT_bomberman_player - SQUARE_SIZE) -
						 ycoord[game->players[i].rypos + 1] + game->players[i].move_phase,
					BMPWIDTH_bomberman_player,
					BMPHEIGHT_bomberman_player);
			}
			else if (game->players[i].look == LOOK_RIGHT)
			{
				rb->lcd_bitmap_transparent_part(bomberman_player_move,
					curphase * BMPWIDTH_bomberman_player,
					game->players[i].look * BMPHEIGHT_bomberman_player,
					STRIDE(SCREEN_MAIN, BMPWIDTH_bomberman_player_move, BMPHEIGHT_bomberman_player_move),
					game->players[i].xpos * SQUARE_SIZE + XMAPOFFSET + 
						xcoord[game->players[i].rxpos + 1] + game->players[i].move_phase,
					game->players[i].ypos * SQUARE_SIZE + YMAPOFFSET -
						(BMPHEIGHT_bomberman_player - SQUARE_SIZE) -
						 ycoord[game->players[i].rypos + 1],
					BMPWIDTH_bomberman_player,
					BMPHEIGHT_bomberman_player);
			}
			else /* LOOK_LEFT */
			{
				rb->lcd_bitmap_transparent_part(bomberman_player_move,
					curphase * BMPWIDTH_bomberman_player,
					game->players[i].look * BMPHEIGHT_bomberman_player,
					STRIDE(SCREEN_MAIN, BMPWIDTH_bomberman_player_move, BMPHEIGHT_bomberman_player_move),
					game->players[i].xpos * SQUARE_SIZE + XMAPOFFSET + 
						xcoord[game->players[i].rxpos + 1] - game->players[i].move_phase,
					game->players[i].ypos * SQUARE_SIZE + YMAPOFFSET -
						(BMPHEIGHT_bomberman_player - SQUARE_SIZE) -
						 ycoord[game->players[i].rypos + 1],
					BMPWIDTH_bomberman_player,
					BMPHEIGHT_bomberman_player);
			}
		}
		else
		{
			rb->lcd_bitmap_transparent_part(bomberman_player_move,
				0,
				game->players[i].look * BMPHEIGHT_bomberman_player,
				STRIDE(SCREEN_MAIN, BMPWIDTH_bomberman_player_move, BMPHEIGHT_bomberman_player_move),
				game->players[i].xpos * SQUARE_SIZE + XMAPOFFSET +
					xcoord[game->players[i].rxpos + 1],
				game->players[i].ypos * SQUARE_SIZE + YMAPOFFSET -
					(BMPHEIGHT_bomberman_player - SQUARE_SIZE) -
					ycoord[game->players[i].rypos + 1],
				BMPWIDTH_bomberman_player,
				BMPHEIGHT_bomberman_player);
		}
	}
	
	/* Explosions */
	for (i = 0; i < MAP_W; i++)
		for (j = 0; j < MAP_H; j++)
		{
			if (game->field.firemap[i][j].state > BOMB_PLACED)
			{
				if (game->field.firemap[i][j].dir == FIRE_CENTER)
				{
					rb->lcd_bitmap_transparent_part(bomberman_explode,
						(game->field.firemap[i][j].state - 2) * SQUARE_SIZE,
						0,
						STRIDE(SCREEN_MAIN, BMPWIDTH_bomberman_explode, BMPHEIGHT_bomberman_explode),
						i * SQUARE_SIZE + XMAPOFFSET,
						j * SQUARE_SIZE + YMAPOFFSET,
						SQUARE_SIZE,
						SQUARE_SIZE);
				}
				else
				{
					rb->lcd_bitmap_transparent_part(bomberman_explode,
						game->field.firemap[i][j].dir * SQUARE_SIZE,
						SQUARE_SIZE + SQUARE_SIZE * (game->field.firemap[i][j].state - 2) * 2 +
							SQUARE_SIZE * game->field.firemap[i][j].isend,
						STRIDE(SCREEN_MAIN, BMPWIDTH_bomberman_explode, BMPHEIGHT_bomberman_explode),
						i * SQUARE_SIZE + XMAPOFFSET,
						j * SQUARE_SIZE + YMAPOFFSET,
						SQUARE_SIZE,
						SQUARE_SIZE);
				}
			}
		}
	
	// Possibly add some demo effects
	/*
	FOR_NB_SCREENS(i)
    {
		struct screen *display = rb->screens[i];
		
		display->drawline(0, 10, LCD_WIDTH, 10);
		
		display->update();
	}
	*/
	rb->lcd_update();
}
