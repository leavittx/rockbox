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
#include "pluginbitmaps/bomberman_ai1_move.h"
#include "pluginbitmaps/bomberman_ai2_move.h"
#include "pluginbitmaps/bomberman_ai3_move.h"
#include "pluginbitmaps/bomberman_ai4_move.h"
#include "pluginbitmaps/bomberman_player_death.h"
#include "pluginbitmaps/bomberman_player_win.h"
#include "pluginbitmaps/bomberman_ai1_win.h"
#include "pluginbitmaps/bomberman_ai2_win.h"
#include "pluginbitmaps/bomberman_ai3_win.h"
#include "pluginbitmaps/bomberman_ai4_win.h"
#include "pluginbitmaps/bomberman_bonus.h"
#include "pluginbitmaps/bomberman_gameover.h"
#include "pluginbitmaps/bomberman_youwon.h"

#include "game.h"
#include "draw.h"

#define SQUARE_SIZE 16
#define XMAPOFFSET 25
#define YMAPOFFSET 30

//int xcoord[3] = {1, 6, 12};
static const int xcoord[3] = {-4, 0, 4};
//int ycoord[3] = {3, 9, 14};
//int ycoord[3] = {1, 6, 12};
//int ycoord[3] = {12, 6, 1};
static const int ycoord[3] = {9, 3, -2};

void Draw(Game *game)
{
    int i, j;

    if (game->state == GAME_GAME)
	rb->lcd_clear_display();

    if (game->state == GAME_GAME)
    {
	/* Different objects on the field */
	for (i = 0; i < MAP_W; i++)
            for (j = 0; j < MAP_H; j++)
                switch (game->field.map[i][j])
                {
                case SQUARE_FREE:
                    if (game->field.bonuses[i][j] != BONUS_NONE)
                    {
                        rb->lcd_bitmap_transparent_part(bomberman_bonus,
                            0,
                            game->field.bonuses[i][j] * SQUARE_SIZE,
                            STRIDE(SCREEN_MAIN,
                                   BMPWIDTH_bomberman_bonus,
                                   BMPHEIGHT_bomberman_bonus),
                            i * SQUARE_SIZE + XMAPOFFSET,
                            j * SQUARE_SIZE + YMAPOFFSET,
                            SQUARE_SIZE,
                            SQUARE_SIZE);
                    }
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
                        STRIDE(SCREEN_MAIN,
                               BMPWIDTH_bomberman_box,
                               BMPHEIGHT_bomberman_box),
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
                        STRIDE(SCREEN_MAIN,
                               BMPWIDTH_bomberman_bomb,
                               BMPHEIGHT_bomberman_bomb),
                        i * SQUARE_SIZE + XMAPOFFSET,
                        j * SQUARE_SIZE + YMAPOFFSET,
                        SQUARE_SIZE,
                        SQUARE_SIZE);
                    break;
                }

        /* player without movement */
        /*
        rb->lcd_bitmap_transparent(bomberman_player,
            game->players[i].xpos * SQUARE_SIZE + XMAPOFFSET,
            game->players[i].ypos * SQUARE_SIZE + YMAPOFFSET -
            (BMPHEIGHT_bomberman_player - SQUARE_SIZE),
            BMPWIDTH_bomberman_player,
            BMPHEIGHT_bomberman_player);
        */

	/* Player and ai's (with movement animation) */
	for (i = 0; i < MAX_PLAYERS; i++)
	{
            if (game->draw_order[i]->status.state > GONNA_DIE)
            {
                if (game->draw_order[i]->status.state < DEAD)
                {
                    rb->lcd_bitmap_transparent_part(bomberman_player_death,
                        (game->draw_order[i]->status.state - 2) * BMPWIDTH_bomberman_player,
                        0,
                        STRIDE(SCREEN_MAIN,
                               BMPWIDTH_bomberman_player_death,
                               BMPHEIGHT_bomberman_player_death),
                        game->draw_order[i]->xpos * SQUARE_SIZE + XMAPOFFSET +
                                        xcoord[game->draw_order[i]->rxpos + 1],
                        game->draw_order[i]->ypos * SQUARE_SIZE + YMAPOFFSET -
                        (BMPHEIGHT_bomberman_player - SQUARE_SIZE) -
                                        ycoord[game->draw_order[i]->rypos + 1],
                        BMPWIDTH_bomberman_player,
                        BMPHEIGHT_bomberman_player);
                }
                else if (game->draw_order[i]->status.state > DEAD)
                {
                    const fb_data *win_bitmap;

                    switch (game->draw_order[i]->num)
                    {
                        case 0:
                            win_bitmap = bomberman_player_win;
                            break;
                        case 1:
                            win_bitmap = bomberman_ai1_win;
                            break;
                        case 2:
                            win_bitmap = bomberman_ai2_win;
                            break;
                        case 3:
                            win_bitmap = bomberman_ai3_win;
                            break;
                        case 4:
                            win_bitmap = bomberman_ai4_win;
                            break;
                        default:
                            win_bitmap = bomberman_ai1_win;
                            break;
                    }

                    rb->lcd_bitmap_transparent_part(win_bitmap,
                        (game->draw_order[i]->status.state % 2) * BMPWIDTH_bomberman_player,
                        0,
                        STRIDE(SCREEN_MAIN,
                               BMPWIDTH_bomberman_player_win,
                               BMPHEIGHT_bomberman_player_win),
                        game->draw_order[i]->xpos * SQUARE_SIZE + XMAPOFFSET +
                                        xcoord[game->draw_order[i]->rxpos + 1],
                        game->draw_order[i]->ypos * SQUARE_SIZE + YMAPOFFSET -
                                        (BMPHEIGHT_bomberman_player - SQUARE_SIZE) -
                                        ycoord[game->draw_order[i]->rypos + 1],
                        BMPWIDTH_bomberman_player,
                        BMPHEIGHT_bomberman_player);
                }
            }
            else
            {
                const fb_data *move_bitmap;

                switch (game->draw_order[i]->num)
                {
                    case 0:
                        move_bitmap = bomberman_player_move;
                        break;
                    case 1:
                        move_bitmap = bomberman_ai1_move;
                        break;
                    case 2:
                        move_bitmap = bomberman_ai2_move;
                        break;
                    case 3:
                        move_bitmap = bomberman_ai3_move;
                        break;
                    case 4:
                        move_bitmap = bomberman_ai4_move;
                        break;
                    default:
                        move_bitmap = bomberman_ai1_move;
                        break;
                }

                if (game->draw_order[i]->ismove)
                {
                    int curphase = 0;

                    if (game->draw_order[i]->move_phase <= 3)
                        curphase = game->draw_order[i]->move_phase;
                    else if (game->draw_order[i]->move_phase == 4)
                        curphase = 2;
                    else if (game->draw_order[i]->move_phase == 5)
                        curphase = 0;

                    // todo: make common code for different directions
                    if (game->draw_order[i]->look == LOOK_UP)
                    {
                        rb->lcd_bitmap_transparent_part(move_bitmap,
                            curphase * BMPWIDTH_bomberman_player,
                            game->draw_order[i]->look * BMPHEIGHT_bomberman_player,
                            STRIDE(SCREEN_MAIN, BMPWIDTH_bomberman_player_move, BMPHEIGHT_bomberman_player_move),
                            game->draw_order[i]->xpos * SQUARE_SIZE + XMAPOFFSET +
                            xcoord[game->draw_order[i]->rxpos + 1],
                            game->draw_order[i]->ypos * SQUARE_SIZE + YMAPOFFSET -
                            (BMPHEIGHT_bomberman_player - SQUARE_SIZE) -
                            ycoord[game->draw_order[i]->rypos + 1] - game->draw_order[i]->move_phase,
                            BMPWIDTH_bomberman_player,
                            BMPHEIGHT_bomberman_player);
                    }
                    else if (game->draw_order[i]->look == LOOK_DOWN)
                    {
                        rb->lcd_bitmap_transparent_part(move_bitmap,
                            curphase * BMPWIDTH_bomberman_player,
                            game->draw_order[i]->look * BMPHEIGHT_bomberman_player,
                            STRIDE(SCREEN_MAIN, BMPWIDTH_bomberman_player_move, BMPHEIGHT_bomberman_player_move),
                            game->draw_order[i]->xpos * SQUARE_SIZE + XMAPOFFSET +
                            xcoord[game->draw_order[i]->rxpos + 1],
                            game->draw_order[i]->ypos * SQUARE_SIZE + YMAPOFFSET -
                            (BMPHEIGHT_bomberman_player - SQUARE_SIZE) -
                            ycoord[game->draw_order[i]->rypos + 1] + game->draw_order[i]->move_phase,
                            BMPWIDTH_bomberman_player,
                            BMPHEIGHT_bomberman_player);
                    }
                    else if (game->draw_order[i]->look == LOOK_RIGHT)
                    {
                        rb->lcd_bitmap_transparent_part(move_bitmap,
                            curphase * BMPWIDTH_bomberman_player,
                            game->draw_order[i]->look * BMPHEIGHT_bomberman_player,
                            STRIDE(SCREEN_MAIN, BMPWIDTH_bomberman_player_move, BMPHEIGHT_bomberman_player_move),
                            game->draw_order[i]->xpos * SQUARE_SIZE + XMAPOFFSET +
                            xcoord[game->draw_order[i]->rxpos + 1] + game->draw_order[i]->move_phase,
                            game->draw_order[i]->ypos * SQUARE_SIZE + YMAPOFFSET -
                            (BMPHEIGHT_bomberman_player - SQUARE_SIZE) -
                            ycoord[game->draw_order[i]->rypos + 1],
                            BMPWIDTH_bomberman_player,
                            BMPHEIGHT_bomberman_player);
                    }
                    else /* LOOK_LEFT */
                    {
                        rb->lcd_bitmap_transparent_part(move_bitmap,
                            curphase * BMPWIDTH_bomberman_player,
                            game->draw_order[i]->look * BMPHEIGHT_bomberman_player,
                            STRIDE(SCREEN_MAIN, BMPWIDTH_bomberman_player_move, BMPHEIGHT_bomberman_player_move),
                            game->draw_order[i]->xpos * SQUARE_SIZE + XMAPOFFSET +
                            xcoord[game->draw_order[i]->rxpos + 1] - game->draw_order[i]->move_phase,
                            game->draw_order[i]->ypos * SQUARE_SIZE + YMAPOFFSET -
                            (BMPHEIGHT_bomberman_player - SQUARE_SIZE) -
                            ycoord[game->draw_order[i]->rypos + 1],
                            BMPWIDTH_bomberman_player,
                            BMPHEIGHT_bomberman_player);
                    }
                }
                else
                {
                    rb->lcd_bitmap_transparent_part(move_bitmap,
                        0,
                        game->draw_order[i]->look * BMPHEIGHT_bomberman_player,
                        STRIDE(SCREEN_MAIN, BMPWIDTH_bomberman_player_move, BMPHEIGHT_bomberman_player_move),
                        game->draw_order[i]->xpos * SQUARE_SIZE + XMAPOFFSET +
                        xcoord[game->draw_order[i]->rxpos + 1],
                        game->draw_order[i]->ypos * SQUARE_SIZE + YMAPOFFSET -
                        (BMPHEIGHT_bomberman_player - SQUARE_SIZE) -
                        ycoord[game->draw_order[i]->rypos + 1],
                        BMPWIDTH_bomberman_player,
                        BMPHEIGHT_bomberman_player);
                }
            }
	}

	/* Explosions */
        for (i = 0; i < MAP_W; i++)
            for (j = 0; j < MAP_H; j++)
            {
                int dir;

                for (dir = 0; dir < 5; dir++)
                {
                    bool is_set = game->field.firemap[i][j] & (BITMASK_ALL_PHASES << (dir * 4));

                    if (is_set)
                    {
                        bool is_end = game->field.firemap[i][j] & (BITMASK_IS_END << dir);
                        int phase;

                        for (phase = 0; phase < 4; phase++)
                        {
                            if (game->field.firemap[i][j] & ((BITMASK_SING_PHASE << (dir * 4)) << phase))
                                break;
                        }

                        if (dir == FIRE_CENTER)
                        {
                            rb->lcd_bitmap_transparent_part(bomberman_explode,
                                (phase) * SQUARE_SIZE,
                                0,
                                STRIDE(SCREEN_MAIN,
                                       BMPWIDTH_bomberman_explode,
                                       BMPHEIGHT_bomberman_explode),
                                i * SQUARE_SIZE + XMAPOFFSET,
                                j * SQUARE_SIZE + YMAPOFFSET,
                                SQUARE_SIZE,
                                SQUARE_SIZE);
                        }
                        else
                        {
                            rb->lcd_bitmap_transparent_part(bomberman_explode,
                                dir * SQUARE_SIZE,
                                SQUARE_SIZE + SQUARE_SIZE * (phase) * 2 +
                                SQUARE_SIZE * is_end,
                                STRIDE(SCREEN_MAIN,
                                       BMPWIDTH_bomberman_explode,
                                       BMPHEIGHT_bomberman_explode),
                                i * SQUARE_SIZE + XMAPOFFSET,
                                j * SQUARE_SIZE + YMAPOFFSET,
                                SQUARE_SIZE,
                                SQUARE_SIZE);
                        }
                    }
                }
            }
    }
    else if (game->state == GAME_GAMEOVER)
    {
        rb->lcd_bitmap(bomberman_gameover, 0, 0,
                       BMPWIDTH_bomberman_gameover, BMPHEIGHT_bomberman_gameover);
    }
    else if (game->state == GAME_WON)
    {
        rb->lcd_bitmap(bomberman_youwon, 0, 0,
                       BMPWIDTH_bomberman_youwon, BMPHEIGHT_bomberman_youwon);
    }

    rb->lcd_update();
}
