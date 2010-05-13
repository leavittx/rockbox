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

#define BOMBS_MAX_NUM 100
#define BOMB_DELAY_DET (HZ * 2) /* Two seconds */
#define BOMB_DELAY_PHASE1 (HZ * 2.1)
#define BOMB_DELAY_PHASE2 (HZ * 2.2)
#define BOMB_DELAY_PHASE3 (HZ * 2.3)
#define BOMB_DELAY_PHASE4 (HZ * 2.4)

typedef enum {
	SQUARE_FREE,
	SQUARE_BOX,
	SQUARE_BLOCK,
	SQUARE_BOMB
} SqType;

typedef enum {
	LOOK_UP,
	LOOK_DOWN,
	LOOK_RIGHT,
	LOOK_LEFT
} LookSide;

typedef enum {
	BOMB_PWR_SINGLE,
	BOMB_PWR_DOUBLE,
	BOMB_PWR_TRIPLE,
	BOMB_PWR_QUAD,
	BOMB_PWR_KILLER
} BombPower;

typedef struct {
	bool isalive;
	int xpos, ypos;
	LookSide look;
	int speed;
	int bombs_max;
	int bombs_placed;
	BombPower bomb_power;
} Player;

typedef enum {
	BOMB_NONE,
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
	FIRE_RIGNT,
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

typedef struct {
	SqType map[MAP_W][MAP_H];
	Bomb bombs[BOMBS_MAX_NUM];
	Fire firemap[MAP_W][MAP_H];
	//Bonus bonuses[BONUSES_MAX_NUM];
} Field;

typedef struct {
	Field field;
	Player player;
	int bomb_rad[BOMB_PWR_KILLER + 1];
} Game;

void PlayerMoveUp(Game *game, Player *player);
void PlayerMoveDown(Game *game, Player *player);
void PlayerMoveRight(Game *game, Player *player);
void PlayerMoveLeft(Game *game, Player *player);
void PlayerPlaceBomb(Game *game, Player *player);
void BombsUpdate(Game *game);

#endif /* _GAME_H */
