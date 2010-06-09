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

#include "game.h"
#include "draw.h"

#define SQUARE_SIZE 16 //todo
#define XMAPOFFSET 25
#define YMAPOFFSET 30

void Draw(Game *game)
{	
	int i, j;
	
	rb->lcd_clear_display();
	
	
	for (i = 0; i < MAP_W; i++)
		for (j = 0; j < MAP_H; j++)
			switch (game->field.map[i][j])
			{
				case SQUARE_FREE:
					break;
				case SQUARE_BOX:
					rb->lcd_bitmap_transparent(bomberman_box,
						i * SQUARE_SIZE + XMAPOFFSET,
						j * SQUARE_SIZE + YMAPOFFSET,
						BMPWIDTH_bomberman_box,
						BMPHEIGHT_bomberman_box);
					break;
				case SQUARE_BLOCK:
					rb->lcd_bitmap_transparent(bomberman_block,
						i * SQUARE_SIZE + XMAPOFFSET,
						j * SQUARE_SIZE + YMAPOFFSET,
						BMPWIDTH_bomberman_block,
						BMPHEIGHT_bomberman_block);
					break;
				case SQUARE_BOMB:
					rb->lcd_bitmap_transparent(bomberman_bomb,
						i * SQUARE_SIZE + XMAPOFFSET,
						j * SQUARE_SIZE + YMAPOFFSET,
						BMPWIDTH_bomberman_bomb,
						BMPHEIGHT_bomberman_bomb);
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
	rb->lcd_bitmap_transparent(bomberman_player,
		game->player.xpos * SQUARE_SIZE + XMAPOFFSET,
		game->player.ypos * SQUARE_SIZE + YMAPOFFSET -
			(BMPHEIGHT_bomberman_player - SQUARE_SIZE),
		BMPWIDTH_bomberman_player,
		BMPHEIGHT_bomberman_player);
		
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
	
	rb->lcd_update();
}