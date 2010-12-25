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

#ifndef _GAME_H
#define _GAME_H

#define MAP_W 17
#define MAP_H 11

#define MAX_PLAYERS 4

#define MAX_BOMBS 10

#define CYCLETIME 40

#define BOMB_DELAY_DET (HZ * 4 / (CYCLETIME / 10)) /* Delay before bomb detanates */
#define BOMB_DELAY_DET_ANIM /*(BOMB_DELAY_DET / 90 / (CYCLETIME / 10))*/(1)
#define BOMB_DELAY_PHASE1 (HZ * 4.02 / (CYCLETIME / 10))
#define BOMB_DELAY_PHASE2 (HZ * 4.03 / (CYCLETIME / 10))
#define BOMB_DELAY_PHASE3 (HZ * 4.05 / (CYCLETIME / 10))
#define BOMB_DELAY_PHASE4 (HZ * 4.06 / (CYCLETIME / 10))

#define BOX_DELAY_EXPLOSION_ANIM (HZ * 0.04 / (CYCLETIME / 10))

#define PLAYER_MOVE_PART_TIME (HZ * 0.01 / (CYCLETIME / 10))
#define PLAYER_DELAY_DEATH_ANIM (HZ * 0.09 / (CYCLETIME / 10))
#define PLAYER_DELAY_WIN_ANIM (HZ * 0.1 / (CYCLETIME / 10))
#define PLAYER_DELAY_WIN_ANIM_DUR (HZ * 5 / (CYCLETIME / 10))

#define AFTERGAME_DUR (HZ * 3 / (CYCLETIME / 10))

typedef enum {
	SQUARE_FREE = 0,
	SQUARE_BOX,
	SQUARE_BLOCK,
	SQUARE_BOMB
} SqType;

typedef enum {
	LOOK_UP = 0,
	LOOK_DOWN,
	LOOK_RIGHT,
	LOOK_LEFT
} LookSide;

typedef enum {
	BOMB_PWR_SINGLE = 0,
	BOMB_PWR_DOUBLE,
	BOMB_PWR_TRIPLE,
	BOMB_PWR_QUAD,
	BOMB_PWR_KILLER
} BombPower;

typedef enum {
	ALIVE = 0,
	GONNA_DIE,
	EXPL_PHASE1,
	EXPL_PHASE2,
	EXPL_PHASE3,
	EXPL_PHASE4,
	DEAD,
	WIN_PHASE1,
	WIN_PHASE2
} PlayerState;

typedef struct {
	PlayerState state;
	int health;
	unsigned long time_of_death;
} PlayerStatus;

typedef struct {
	PlayerStatus status;
	
	int xpos, ypos;
	LookSide look;
	int speed;
	
	int bombs_max;
	int bombs_placed;
	BombPower bomb_power;
	
	int rxpos, rypos;
	bool ismove;
	int move_phase;
	unsigned long move_start_time;
	
	bool IsAIPlayer;
	
	int num;
} Player;

/*
 * rxpos & rypos - position of player in single cell
 * 
 __________________
 | -1  |  0  |  1  |
 | -1  | -1  | -1  |
 |_____|_____|_____|
 | -1  |  0  |  1  |
 |  0  |  0  |  0  |   <--- one cell
 |_____|_____|_____|
 | -1  |  0  |  1  |
 |  1  |  1  |  1  |
 |_____|_____|_____|
 */
 
typedef enum {
	BOMB_NONE = 0,
	BOMB_PLACED,
	BOMB_EXPL_PHASE1,
	BOMB_EXPL_PHASE2,
	BOMB_EXPL_PHASE3,
	BOMB_EXPL_PHASE4
} BombState;

typedef struct {
	BombState state;
	int xpos, ypos;
	BombPower power;
	unsigned long place_time;
	Player *owner;
} Bomb;

typedef enum {
	FIRE_RIGNT = 0,
	FIRE_DOWN,
	FIRE_LEFT,
	FIRE_UP,
	FIRE_CENTER
} FireDir;

typedef struct {
	BombState state;
	FireDir dir;
	bool isend;
} Fire;

typedef enum {
	DET_PHASE1 = 0,
	DET_PHASE2,
	DET_PHASE3
} BombDetonation;

typedef enum {
	HUNKY = 0,
	BOX_EXPL_PHASE1,
	BOX_EXPL_PHASE2,
	BOX_EXPL_PHASE3,
	BOX_EXPL_PHASE4,
	BOX_EXPL_PHASE5
} BoxState;

typedef struct {
	BoxState state;
	unsigned long expl_time;
} BoxDetonation;

typedef enum {
	BONUS_EXTRABOMB, // extra bomb
	BONUS_POWER, // increase bomb explosion radius
	BONUS_SPEEDUP, // increase player speed
	BONUS_FULLPOWER, // destroy all destroyable objects in radius
	BONUS_NONE // no bonus
} BonusType;

typedef struct {
	SqType map[MAP_W][MAP_H];
	Bomb bombs[MAX_BOMBS];
	Fire firemap[MAP_W][MAP_H];
	BombDetonation det[MAP_W][MAP_H];
	BoxDetonation boxes[MAP_W][MAP_H];
	BonusType bonuses[MAP_W][MAP_H];
} Field;

typedef enum {
	GAME_GAME = 0,
	GAME_GAMEOVER,
	GAME_WON,
	GAME_GREETZ
} GameState;

typedef struct {
	Field field;
	Player players[MAX_PLAYERS];
	Player *draw_order[MAX_PLAYERS];
	int nplayers;
	int bomb_rad[5];
	GameState state;
	int aftergame_y;
} Game;

void PlayerMoveUp(Game *game, Player *player);
void PlayerMoveDown(Game *game, Player *player);
void PlayerMoveRight(Game *game, Player *player);
void PlayerMoveLeft(Game *game, Player *player);
int UpdatePlayer(Game *game, Player *player);
void PlayerPlaceBomb(Game *game, Player *player);
void UpdateBombs(Game *game);
void UpdateBoxes(Game *game);
void UpdateAftergame(Game *game);
unsigned long get_tick(void);

extern unsigned long tick;

#endif /* _GAME_H */
