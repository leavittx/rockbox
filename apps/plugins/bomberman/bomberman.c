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

#include "lib/pluginlib_actions.h"
#include "lib/helper.h"
#include "lib/playback_control.h"

#include "game.h"
#include "draw.h"
#include "ai.h"

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

Game game;
unsigned long tick = 0;

void InitGame(Game *game)
{
    int i, j;
    /*
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
      {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
      {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2}
      };
     */
        /*int DefaultMap[MAP_H][MAP_W] = {
      {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
      {2,0,0,1,1,1,0,1,0,1,0,1,0,1,0,0,2},
      {2,0,2,1,1,1,1,1,1,1,1,1,1,1,2,0,2},
      {2,0,1,0,0,0,0,0,0,1,0,1,0,1,1,0,2},
      {2,0,1,1,2,1,2,0,0,1,2,0,2,1,1,0,2},
      {2,0,1,1,1,1,1,1,0,0,0,0,0,1,1,0,2},
      {2,0,1,0,0,0,0,0,2,1,2,0,2,0,1,0,2},
      {2,0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,2},
      {2,0,2,1,1,1,1,1,1,1,1,1,1,1,2,0,2},
      {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
      {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2}
      };
     */
    int DefaultMap[MAP_H][MAP_W] = {
        {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
        {2,0,0,0,1,1,0,1,0,1,0,1,1,0,0,0,2},
        {2,0,2,0,1,1,1,1,1,1,1,1,1,0,2,0,2},
        {2,1,1,1,1,0,0,0,0,1,0,1,1,1,1,1,2},
        {2,0,1,1,2,1,2,0,0,1,2,0,2,1,1,0,2},
        {2,0,1,1,1,1,1,1,0,0,0,0,0,1,1,0,2},
        {2,0,1,0,0,0,0,0,2,1,2,0,2,0,1,0,2},
        {2,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,2},
        {2,0,2,0,1,1,1,1,1,1,1,1,1,0,2,0,2},
        {2,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,2},
        {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2}
    };
    for (i = 0; i < MAP_W; i++)
        for (j = 0; j < MAP_H; j++)
        {
            game->field.map[i][j] = DefaultMap[j][i];
            game->field.firemap[i][j].state = BOMB_NONE;
            game->field.boxes[i][j].state = HUNKY;
            game->field.bonuses[i][j] = BONUS_NONE;
        }

    for (i = 0; i < MAX_BOMBS; i++)
        game->field.bombs[i].state = BOMB_NONE;

    game->nplayers = MAX_PLAYERS;

    // set radius of explosion for each bomb power
    game->bomb_rad[BOMB_PWR_SINGLE] = 1;
    game->bomb_rad[BOMB_PWR_DOUBLE] = 2;
    game->bomb_rad[BOMB_PWR_TRIPLE] = 4;
    game->bomb_rad[BOMB_PWR_QUAD] = 6;
    game->bomb_rad[BOMB_PWR_KILLER] = MAX(MAP_W, MAP_H);

    // place bonuses
    int nboxes = 0, nbonuses;
    for (i = 0; i < MAP_W; i++)
        for (j = 0; j < MAP_H; j++)
            if (game->field.map[i][j] == SQUARE_BOX)
                nboxes++;

    // not all boxes consist a bonus
    nbonuses = nboxes / 2;
    while (nbonuses)
    {
        i = rb->rand() % MAP_W;
        j = rb->rand() % MAP_H;

        if (game->field.map[i][j] == SQUARE_BOX && game->field.bonuses[i][j] == BONUS_NONE)
        {
            // choose a random bonus for this box
            // - 2 -- not all bonuses implemented yet
            game->field.bonuses[i][j] = rb->rand() % (BONUS_NONE - 2);
            nbonuses--;
        }
    }

    game->state = GAME_GAME;
}

void InitPlayer(Player *player, int num, int x, int y)
{
    player->status.state = ALIVE;
    player->status.health = 100;
    player->xpos = x;
    player->ypos = y;
    player->look = LOOK_DOWN;
    player->speed = 1;
    player->bombs_max = 1;
    player->bombs_placed = 0;
    player->bomb_power = BOMB_PWR_SINGLE;

    player->rxpos = 0;
    player->rypos = 0;
    player->ismove = false;
    player->move_phase = 0;

    player->IsAIPlayer = false;

    player->num = num;
}

void InitAI(Player *player, int num, int x, int y)
{
    player->status.state = ALIVE;
    player->status.health = 100;
    player->xpos = x;
    player->ypos = y;
    player->look = LOOK_DOWN;
    player->speed = 3;
    player->bombs_max = 1;
    player->bombs_placed = 0;
    player->bomb_power = BOMB_PWR_SINGLE;

    player->rxpos = 0;
    player->rypos = 0;
    player->ismove = false;
    player->move_phase = 0;

    player->IsAIPlayer = true;

    player->num = num;
}

int plugin_main(void)
{
    int action; /* Key action */
    int i;
    int end; /* End tick */
    
    //rb->splashf(HZ, "%f",  PLAYER_MOVE_PART_TIME);
    
    rb->srand(get_tick());
    
    InitGame(&game);

    InitPlayer(&game.players[0], 0, 1, 1);
    //InitAI(&game.players[1], 3, 9);
    //InitAI(&game.players[1], 10, 9);
    InitAI(&game.players[1], 1, 1, 9);
    InitAI(&game.players[2], 2, 15, 9);
    InitAI(&game.players[3], 3, 15, 1);

    for (i = 0; i < MAX_PLAYERS; i++)
    {
        game.draw_order[i] = &game.players[i];
        //game->draw_order[i].order = i;
    }

    /* Main loop */
    while (true)
    {
        end = get_tick() + (CYCLETIME * HZ) / 1000;

        Draw(&game);

        if (game.state == GAME_GAME)
            for (i = 0; i < MAX_PLAYERS; i++)
            {
                int upd;

                //if (!(tick % game.players[i].speed))
                //{
                upd = UpdatePlayer(&game, &game.players[i]);
                if (upd == DEAD)
                {
                    game.nplayers--;
                    if (game.nplayers == 1 || !game.players[i].IsAIPlayer)
                    {
                        for (i = 0; i < MAX_PLAYERS; i++)
                        {
                            if (game.players[i].status.state == ALIVE)
                            {
                                game.players[i].status.state = WIN_PHASE1;
                                //if (game.players[i].IsAIPlayer)
                                //	rb->splash(HZ * 5, "You lose");
                                //else
                                //	rb->splash(HZ * 5, "You won");
                            }
                        }
                    }
                }
                else if (upd == -GAME_GAMEOVER || upd == -GAME_WON)
                {
                    game.state = -upd;
                    tick = 0;
                }
                //}
            }

        if (game.state == GAME_GAMEOVER || game.state == GAME_WON)
        {
            if (tick >= AFTERGAME_DUR)
                return PLUGIN_OK;
        }

        UpdateBombs(&game);
        UpdateBoxes(&game);
        UpdateAI(&game, game.players);

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
            PlayerMoveUp(&game, &game.players[0]);
            break;

        case PLA_DOWN:
        case PLA_DOWN_REPEAT:
            PlayerMoveDown(&game, &game.players[0]);
            break;

        case PLA_RIGHT:
        case PLA_RIGHT_REPEAT:
            PlayerMoveRight(&game, &game.players[0]);
            break;

        case PLA_LEFT:
        case PLA_LEFT_REPEAT:
            PlayerMoveLeft(&game, &game.players[0]);
            break;

        case PLA_SELECT:
            PlayerPlaceBomb(&game, &game.players[0]);
            break;

        case PLA_CANCEL:
            break;
        }

        if (TIME_BEFORE(get_tick(), end))
            rb->sleep(end - get_tick());
        else
            rb->yield();

        tick++;
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