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

#ifdef HAVE_LCD_BITMAP
#include "lib/pluginlib_actions.h"
#include "lib/helper.h"

/* This macros must always be included. Should be placed at the top by
   convention, although the actual position doesn't matter */
PLUGIN_HEADER

#include "game.h"
#include "draw.h"

#define SLEEP_TIME 1

const struct button_mapping *plugin_contexts[] = {
	pla_main_ctx,
#if defined(HAVE_REMOTE_LCD)
    pla_remote_ctx,
#endif
};

#define NB_ACTION_CONTEXTS \
    (sizeof(plugin_contexts) / sizeof(struct button_mapping*))

void cleanup(void *parameter)
{
	(void)parameter;

	backlight_use_settings();
#ifdef HAVE_REMOTE_LCD
	remote_backlight_use_settings();
#endif
}

/* 
 * Main code 
 */

void InitGame(Game *game)
{
	int i, j;
	int DefaultMap[MAP_H][MAP_W] = {
		{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
		{2,0,0,1,1,1,0,1,0,1,0,1,0,1,0,0,2},
		{2,0,2,1,2,1,2,0,2,1,2,0,2,1,2,0,2},
		{2,0,0,1,1,1,1,1,0,1,0,1,0,1,0,0,2},
		{2,0,2,1,2,1,2,0,2,1,2,0,2,1,2,0,2},
		{2,0,0,1,1,1,1,1,0,1,0,1,0,1,0,0,2},
		{2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2},
		{2,0,0,1,1,1,1,1,0,1,0,1,0,1,0,0,2},
		{2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2},
		{2,0,0,1,1,1,1,1,0,1,0,1,0,1,0,0,2},
		{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2}
		};
	
	for (i = 0; i < MAP_W; i++)
		for (j = 0; j < MAP_H; j++)
		{
			game->field.map[i][j] = DefaultMap[j][i];//SQUARE_FREE;
			game->field.firemap[i][j].state = BOMB_NONE;
		}
			
	for (i = 0; i < BOMBS_MAX_NUM; i++)
		game->field.bombs[i].state = BOMB_NONE;
	
	game->bomb_rad[BOMB_PWR_SINGLE] = 1;
	game->bomb_rad[BOMB_PWR_DOUBLE] = 2;
	game->bomb_rad[BOMB_PWR_TRIPLE] = 4;
	game->bomb_rad[BOMB_PWR_QUAD] = 6;
	game->bomb_rad[BOMB_PWR_KILLER] = MAP_W;
}

void InitPlayer(Player *player)
{
	player->isalive = true;
	player->xpos = 1;
	player->ypos = 1;
	player->look = LOOK_DOWN;
	player->speed = 1;
	player->bombs_max = -1;
	player->bombs_placed = 0;
	player->bomb_power = BOMB_PWR_KILLER;
	
	player->rxpos = 0;
	player->rypos = 0;
	player->ismove = false;
	player->move_phase = 0;
}

int plugin_main(void)
{
    int action; /* Key action */
    Game game;
    
    rb->srand(*rb->current_tick);
    
    InitGame(&game);
    InitPlayer(&game.player);
    
    /* Main loop */
    while (true)
    {
		Draw(&game);
	
		UpdatePlayer(&game.player);
		UpdateBombs(&game);
		
		rb->sleep(SLEEP_TIME);
		
		action = pluginlib_getaction(TIMEOUT_NOBLOCK,
									 plugin_contexts,
									 NB_ACTION_CONTEXTS);
									 
		switch (action)
		{
			case PLA_EXIT:
				cleanup(NULL);
				return PLUGIN_OK;
				
			case PLA_UP:
			case PLA_UP_REPEAT:
				PlayerMoveUp(&game, &game.player);
				break;
				
			case PLA_DOWN:
			case PLA_DOWN_REPEAT:
				PlayerMoveDown(&game, &game.player);
				break;
			
			case PLA_RIGHT:
			case PLA_RIGHT_REPEAT:
				PlayerMoveRight(&game, &game.player);
				break;
			
			case PLA_LEFT:
			case PLA_LEFT_REPEAT:
				PlayerMoveLeft(&game, &game.player);
				break;
			
			case PLA_SELECT:
				PlayerPlaceBomb(&game, &game.player);
				break;
				
			case PLA_CANCEL:
				break;
		}
	}
}

/* this is the plugin entry point */
enum plugin_status plugin_start(const void* parameter)
{
	int ret;
	
	/* avoid the compiler warning about unused parameter */
	(void)parameter;
    
#if LCD_DEPTH > 1
	rb->lcd_set_backdrop(NULL);
#endif
	backlight_force_on(); /* backlight control in lib/helper.c */
#ifdef HAVE_REMOTE_LCD
	remote_backlight_force_on(); /* remote backlight control in lib/helper.c */
#endif

	ret = plugin_main();

	return ret;
}

#endif /* #ifdef HAVE_LCD_BITMAP */
