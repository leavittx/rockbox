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
 * Battleship plugin
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

#define SLEEP_TIME 3

/* Bitmaps */
#include "pluginbitmaps/battleship_huy.h"
#include "pluginbitmaps/battleship_map.h"
#include "pluginbitmaps/battleship_placeships.h"
#include "pluginbitmaps/battleship_yourturn.h"
#include "pluginbitmaps/battleship_player1.h"
//#include "pluginbitmaps/battleship_player1_transp.h"
#include "pluginbitmaps/battleship_player2.h"
#include "pluginbitmaps/battleship_n0.h"
#include "pluginbitmaps/battleship_n1.h"
#include "pluginbitmaps/battleship_n2.h"
#include "pluginbitmaps/battleship_n3.h"
#include "pluginbitmaps/battleship_n4.h"
#include "pluginbitmaps/battleship_n5.h"
#include "pluginbitmaps/battleship_n6.h"
#include "pluginbitmaps/battleship_n7.h"
#include "pluginbitmaps/battleship_n8.h"
#include "pluginbitmaps/battleship_n9.h"
#include "pluginbitmaps/battleship_nsep.h"
#include "pluginbitmaps/battleship_aim.h"
#include "pluginbitmaps/battleship_missed.h"
#include "pluginbitmaps/battleship_shipone_notplaced.h"
#include "pluginbitmaps/battleship_shipone.h"
#include "pluginbitmaps/battleship_shiponedead.h"
#include "pluginbitmaps/battleship_noships.h"

unsigned short const *numbitmaps[] = {
	battleship_n0, battleship_n1,
	battleship_n2, battleship_n3,
	battleship_n4, battleship_n5,
	battleship_n6, battleship_n7,
	battleship_n8, battleship_n9
};

/* States of game */
enum {
	WAIT_FOR_PLAYER1_TO_PLACE_SHIPS,
	PLACE_SHIPS_PLAYER1,
	WAIT_FOR_PLAYER2_TO_PLACE_SHIPS,
	PLACE_SHIPS_PLAYER2,
	WAIT_FOR_PLAYER1_TO_MAKE_A_TURN,
	TURN_PLAYER1,
	SHOW_FIELD_AFTER_TURN_OF_PLAYER1,
	WAIT_FOR_PLAYER2_TO_MAKE_A_TURN,
	TURN_PLAYER2,
	SHOW_FIELD_AFTER_TURN_OF_PLAYER2,
	GAME_OVER
} State;

typedef enum {
	VERT,
	HORIZ
} Orientation;

/* Square type */
typedef enum { 
	FREE, 		/* Nothing */
	SHIP, 		/* A bit of ship */
	SHOTSHIP, 	/* Partly shot or entirely dead */
	SHOTWATER,	/* Missed! */
	NOSHIPS 	/* Being a ship in this square isn't possible */
} Sqtype;

/* Coordinates */
#define X 	0
#define Y 	1

/* Number of deck */
#define D1	0
#define D2	1
#define D3	2
#define D4	3

/* Amount of decks on ship of each type */
#define NUM_OF_D_S1 1
#define NUM_OF_D_S2 2
#define NUM_OF_D_S3 3
#define NUM_OF_D_S4 4

/* First ship, second, ... */
#define N1	0
#define N2	1
#define N3	2
#define N4	3

/* Amount of ships of each type */
#define NUM_OF_S1 4
#define NUM_OF_S2 3
#define NUM_OF_S3 2
#define NUM_OF_S4 1

/* Everything about ships of player */
typedef struct {
	int nS1, nS2, nS3, nS4;
	int S1[NUM_OF_S1][1][2], S2[NUM_OF_S2][2][2], S3[NUM_OF_S3][3][2], S4[NUM_OF_S4][4][2];
	bool issunkS1[NUM_OF_S1], issunkS2[NUM_OF_S2], issunkS3[NUM_OF_S3], issunkS4[NUM_OF_S4];
	bool isshotS1[NUM_OF_S1][NUM_OF_D_S1], isshotS2[NUM_OF_S2][NUM_OF_D_S2],
		 isshotS3[NUM_OF_S3][NUM_OF_D_S3], isshotS4[NUM_OF_S4][NUM_OF_D_S4];
	Orientation /*oriS1[NUM_OF_S1],*/ oriS2[NUM_OF_S2], oriS3[NUM_OF_S3], oriS4[NUM_OF_S4];
} Ships;

/* Shot result */
enum { 
	MISSED,
	GOT,
	DUMB
} shotres;

/* Field length */
#define FLD_LEN 10
/* X-axis field offest in pixels */
#define XOFFSET 20
/* Y-axis field offest in pixels */
#define YOFFSET 20
/* Length of square in pixels */
#define SQSIZE 	22
/* Delay after each turn in seconds */
#define DELAY_AFTER_TURN 2

Sqtype battlefield_p1[FLD_LEN][FLD_LEN];
Sqtype battlefield_p2[FLD_LEN][FLD_LEN];
Ships ships_p1;
Ships ships_p2;

/* For buttons */
const struct button_mapping *plugin_contexts[]
= {generic_directions, generic_actions,
#if defined(HAVE_REMOTE_LCD)
    remote_directions
#endif
};
#define NB_ACTION_CONTEXTS \
    sizeof(plugin_contexts)/sizeof(struct button_mapping*)

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

unsigned long get_sec(void) {
	return *rb->current_tick / HZ;
}

void ClearField(Sqtype (*bf)[FLD_LEN])
{
	int i, j;
	
	for (i = 0; i < FLD_LEN; i++)
		for (j = 0; j < FLD_LEN; j++)
			bf[i][j] = FREE;
}

void InitFields(void)
{
	int i, j;
	
	ClearField(battlefield_p1);
	ClearField(battlefield_p2);
		
	for (i = 0; i < NUM_OF_S1; i++)
	{
		ships_p1.issunkS1[i] = true;
		ships_p2.issunkS1[i] = true;
	}
	for (i = 0; i < NUM_OF_S2; i++)
	{
		ships_p1.issunkS2[i] = true;
		ships_p2.issunkS2[i] = true;
		for (j = 0; j < NUM_OF_D_S2; j++)
		{
			ships_p1.isshotS2[i][j] = true;
			ships_p2.isshotS2[i][j] = true;
		}
	}
	for (i = 0; i < NUM_OF_S3; i ++)
	{
		ships_p1.issunkS3[i] = true;
		ships_p2.issunkS3[i] = true;
		for (j = 0; j < NUM_OF_D_S3; j++)
		{
			ships_p1.isshotS3[i][j] = true;
			ships_p2.isshotS3[i][j] = true;
		}
	}
	for (i = 0; i < NUM_OF_S4; i ++)
	{
		ships_p1.issunkS4[i] = true;
		ships_p2.issunkS4[i] = true;
		for (j = 0; j < NUM_OF_D_S4; j++)
		{
			ships_p1.isshotS4[i][j] = true;
			ships_p2.isshotS4[i][j] = true;
		}
	}
}

void RegenField(Sqtype (*bf)[FLD_LEN], Ships *ships)
{
	int i;
	
	for (i = 0; i < ships->nS1; i++)
		bf[ships->S1[i][D1][X]][ships->S1[i][D1][Y]] = SHIP;
	for (i = 0; i < ships->nS2; i++)
	{
		bf[ships->S2[i][D1][X]][ships->S1[i][D1][Y]] = SHIP;
		bf[ships->S2[i][D2][X]][ships->S1[i][D2][Y]] = SHIP;
	}
	for (i = 0; i < ships->nS3; i++)
	{
		bf[ships->S3[i][D1][X]][ships->S3[i][D1][Y]] = SHIP;
		bf[ships->S3[i][D2][X]][ships->S3[i][D2][Y]] = SHIP;
		bf[ships->S3[i][D3][X]][ships->S3[i][D3][Y]] = SHIP;
	}
	for (i = 0; i < ships->nS4; i++)
	{
		bf[ships->S4[i][D1][X]][ships->S4[i][D1][Y]] = SHIP;
		bf[ships->S4[i][D2][X]][ships->S4[i][D2][Y]] = SHIP;
		bf[ships->S4[i][D3][X]][ships->S4[i][D3][Y]] = SHIP;
		bf[ships->S4[i][D4][X]][ships->S4[i][D4][Y]] = SHIP;
	}
}

void PlaceShipToBF(int xpos, int ypos, Orientation ori, int len, int num, Sqtype (*bf)[FLD_LEN], Ships *ships)
{
	int i;
	
	if (len == NUM_OF_D_S1)
	{
		bf[xpos][ypos] = SHIP;
		ships->nS1 = num + 1;
	}
	else if (len == NUM_OF_D_S2)
	{
		for (i = 0; i < len; i++)
		{
			if (ori == VERT)
			{
				bf[xpos][ypos + i] = SHIP;
			}
			else /* HORIZ */
			{
				bf[xpos + i][ypos] = SHIP;
			}
		}
		ships->nS2 = num + 1;
	}
	else if (len == NUM_OF_D_S3)
	{
		for (i = 0; i < len; i++)
		{
			if (ori == VERT)
			{
				bf[xpos][ypos + i] = SHIP;
			}
			else /* HORIZ */
			{
				bf[xpos + i][ypos] = SHIP;
			}
		}
		ships->nS3 = num + 1;
	}
	else /* NUM_OF_D_S4 */
	{
		for (i = 0; i < len; i++)
		{
			if (ori == VERT)
			{
				bf[xpos][ypos + i] = SHIP;
			}
			else /* HORIZ */
			{
				bf[xpos + i][ypos] = SHIP;
			}
		}
		ships->nS4 = num + 1;
	}
}

void PlaceShip(int xpos, int ypos, Orientation ori, int len, int num, Sqtype (*bf)[FLD_LEN], Ships *ships)
{
	int i;
	
	/* Avoid compiler warning about unused parameter.
	 * TODO: remove this parameter
	 */
	(void)bf;
	
	if (len == NUM_OF_D_S1)
	{
		//bf[xpos][ypos] = SHIP;
		ships->S1[num][D1][X] = xpos;
		ships->S1[num][D1][Y] = ypos;
		ships->issunkS1[num] = false;
		ships->isshotS1[num][D1] = false;
		//ships->nS1 = num + 1;
	}
	else if (len == NUM_OF_D_S2)
	{
		for (i = 0; i < len; i++)
		{
			if (ori == VERT)
			{
				//bf[xpos][ypos + i] = SHIP;
				ships->S2[num][i][X] = xpos;
				ships->S2[num][i][Y] = ypos + i;
			}
			else /* HORIZ */
			{
				//bf[xpos + i][ypos] = SHIP;
				ships->S2[num][i][X] = xpos + i;
				ships->S2[num][i][Y] = ypos;
			}
			ships->isshotS2[num][i] = false;
		}
		ships->issunkS2[num] = false;
		//ships->nS2 = num + 1;
	}
	else if (len == NUM_OF_D_S3)
	{
		for (i = 0; i < len; i++)
		{
			if (ori == VERT)
			{
				//bf[xpos][ypos + i] = SHIP;
				ships->S3[num][i][X] = xpos;
				ships->S3[num][i][Y] = ypos + i;
			}
			else /* HORIZ */
			{
				//bf[xpos + i][ypos] = SHIP;
				ships->S3[num][i][X] = xpos + i;
				ships->S3[num][i][Y] = ypos;
			}
			ships->isshotS3[num][i] = false;
		}
		ships->issunkS3[num] = false;
		//ships->nS3 = num + 1;
	}
	else /* NUM_OF_D_S4 */
	{
		for (i = 0; i < len; i++)
		{
			if (ori == VERT)
			{
				//bf[xpos][ypos + i] = SHIP;
				ships->S4[num][i][X] = xpos;
				ships->S4[num][i][Y] = ypos + i;
			}
			else /* HORIZ */
			{
				//bf[xpos + i][ypos] = SHIP;
				ships->S4[num][i][X] = xpos + i;
				ships->S4[num][i][Y] = ypos;
			}
			ships->isshotS4[num][i] = false;
		}
		ships->issunkS4[num] = false;
		//ships->nS4 = num + 1;
	}
}

bool CorrectPosition(int xpos, int ypos, Orientation ori, int len, Sqtype (*bf)[FLD_LEN])
{
	int i;
	int xdir, ydir;
	
	for (i = 0; i < len; i++)
		for (xdir = -1; xdir <= 1; xdir++)
			for (ydir = -1; ydir <= 1; ydir++)
			{
				if (ori == VERT)
				{
					if (xpos + xdir >= 0 && xpos + xdir < FLD_LEN &&
						ypos + i + ydir >= 0 && ypos + i + ydir < FLD_LEN &&
						bf[xpos + xdir][ypos + i + ydir] != FREE)
							return false;
				}
				else		
				{
					if (xpos + i + xdir >= 0 && xpos + i + xdir < FLD_LEN &&
						ypos + ydir >= 0 && ypos + ydir < FLD_LEN &&
						bf[xpos + i + xdir][ypos + ydir] != FREE)
							return false;
				}
			}
	
	return true;
}

bool NextThing(int *curlen, int *curnum)
{	
	if (*curlen == NUM_OF_D_S4)
	{
		if (*curnum == NUM_OF_S4 - 1)
		{
			(*curlen)--;
			*curnum = 0;
		}
		else
		{
			(*curnum)++;
		}
	}
	else if (*curlen == NUM_OF_D_S3)
	{
		if (*curnum == NUM_OF_S3 - 1)
		{
			(*curlen)--;
			*curnum = 0;
		}
		else
		{
			(*curnum)++;
		}
	}
	else if (*curlen == NUM_OF_D_S2)
	{
		if (*curnum == NUM_OF_S2 - 1)
		{
			(*curlen)--;
			*curnum = 0;
		}
		else
		{
			(*curnum)++;
		}
	}
	else if (*curlen == NUM_OF_D_S1)
	{
		if (*curnum == NUM_OF_S1 - 1)
		{
			return true;
		}
		else
		{
			(*curnum)++;
		}
	}
	
	return false;
}

int Shoot(int xpos, int ypos, Sqtype (*bf)[FLD_LEN], Ships *ships)
{	
	int i, j, k;
	int alivedecks = 0;
	
	if (bf[xpos][ypos] == FREE)
	{
		bf[xpos][ypos] = SHOTWATER;
		
		return MISSED;
	}
	if (bf[xpos][ypos] == SHIP)
	{
		bf[xpos][ypos] = SHOTSHIP;
		
		for (i = 0; i < NUM_OF_S1; i++)
		{
			if (!ships->issunkS1[i] && ships->S1[i][D1][X] == xpos && ships->S1[i][D1][Y] == ypos)
			{
				ships->issunkS1[i] = true;
				
				if (xpos != 0)
				{
					if (bf[xpos - 1][ypos] == FREE)
						bf[xpos - 1][ypos] = NOSHIPS;
					if (ypos != 0)
						if (bf[xpos - 1][ypos - 1] == FREE)
							bf[xpos - 1][ypos - 1] = NOSHIPS;
					if (ypos != FLD_LEN - 1)
						if (bf[xpos - 1][ypos + 1] == FREE)
							bf[xpos - 1][ypos + 1] = NOSHIPS;
				}
				if (xpos != FLD_LEN - 1)
				{
					if (bf[xpos + 1][ypos] == FREE)
						bf[xpos + 1][ypos] = NOSHIPS;
					if (ypos != 0)
						if (bf[xpos + 1][ypos - 1] == FREE)
							bf[xpos + 1][ypos - 1] = NOSHIPS;
					if (ypos != FLD_LEN - 1)
						if (bf[xpos + 1][ypos + 1] == FREE)
							bf[xpos + 1][ypos + 1] = NOSHIPS;
				}
				if (ypos != 0)
					if (bf[xpos][ypos - 1] == FREE)
						bf[xpos][ypos - 1] = NOSHIPS;
				if (ypos != FLD_LEN - 1)
					if (bf[xpos][ypos + 1] == FREE)
						bf[xpos][ypos + 1] = NOSHIPS;
						
				return GOT;
			}
		}
		
		for (i = 0; i < NUM_OF_S2; i++)
		{
			if (!ships->issunkS2[i])
				for (j = 0; j < NUM_OF_D_S2; j++)
					if (ships->S2[i][j][X] == xpos && ships->S2[i][j][Y] == ypos)
					{
						ships->isshotS2[i][j] = true;
						
						for (k = 0; k < NUM_OF_D_S2; k++)
						{
							if (!ships->isshotS2[i][k])
								alivedecks++;
						}
						
						if (!alivedecks)
						{
							ships->issunkS2[i] = true;
							
							for (k = 0; k < NUM_OF_D_S2; k++)
							{
								if (ships->S2[i][k][X] != 0)
								{
									if (bf[ships->S2[i][k][X] - 1][ships->S2[i][k][Y]] == FREE)
										bf[ships->S2[i][k][X] - 1][ships->S2[i][k][Y]] = NOSHIPS;
									if (ships->S2[i][k][Y] != 0)
										if (bf[ships->S2[i][k][X] - 1][ships->S2[i][k][Y] - 1] == FREE)
											bf[ships->S2[i][k][X] - 1][ships->S2[i][k][Y] - 1] = NOSHIPS;
									if (ships->S2[i][k][Y] != FLD_LEN - 1)
										if (bf[ships->S2[i][k][X] - 1][ships->S2[i][k][Y] + 1] == FREE)
											bf[ships->S2[i][k][X] - 1][ships->S2[i][k][Y] + 1] = NOSHIPS;
								}
								if (ships->S2[i][k][X] != FLD_LEN - 1)
								{
									if (bf[ships->S2[i][k][X] + 1][ships->S2[i][k][Y]] == FREE)
										bf[ships->S2[i][k][X] + 1][ships->S2[i][k][Y]] = NOSHIPS;
									if (ships->S2[i][k][Y] != 0)
										if (bf[ships->S2[i][k][X] + 1][ships->S2[i][k][Y] - 1] == FREE)
											bf[ships->S2[i][k][X] + 1][ships->S2[i][k][Y] - 1] = NOSHIPS;
									if (ships->S2[i][k][Y] != FLD_LEN - 1)
										if (bf[ships->S2[i][k][X] + 1][ships->S2[i][k][Y] + 1] == FREE)
											bf[ships->S2[i][k][X] + 1][ships->S2[i][k][Y] + 1] = NOSHIPS;
								}
								if (ships->S2[i][k][Y] != 0)
									if (bf[ships->S2[i][k][X]][ships->S2[i][k][Y] - 1] == FREE)
										bf[ships->S2[i][k][X]][ships->S2[i][k][Y] - 1] = NOSHIPS;
								if (ships->S2[i][k][Y] != FLD_LEN - 1)
									if (bf[ships->S2[i][k][X]][ships->S2[i][k][Y] + 1] == FREE)
										bf[ships->S2[i][k][X]][ships->S2[i][k][Y] + 1] = NOSHIPS;
							}
						}
						
						return GOT;
					}
		}
		
		for (i = 0; i < NUM_OF_S3; i++)
		{
			if (!ships->issunkS3[i])
				for (j = 0; j < NUM_OF_D_S3; j++)
					if (ships->S3[i][j][X] == xpos && ships->S3[i][j][Y] == ypos)
					{
						ships->isshotS3[i][j] = true;
						
						for (k = 0; k < NUM_OF_D_S3; k++)
						{
							if (!ships->isshotS3[i][k])
								alivedecks++;
						}
						
						if (!alivedecks)
						{
							ships->issunkS3[i] = true;
							
							for (k = 0; k < NUM_OF_D_S3; k++)
							{
								if (ships->S3[i][k][X] != 0)
								{
									if (bf[ships->S3[i][k][X] - 1][ships->S3[i][k][Y]] == FREE)
										bf[ships->S3[i][k][X] - 1][ships->S3[i][k][Y]] = NOSHIPS;
									if (ships->S3[i][k][Y] != 0)
										if (bf[ships->S3[i][k][X] - 1][ships->S3[i][k][Y] - 1] == FREE)
											bf[ships->S3[i][k][X] - 1][ships->S3[i][k][Y] - 1] = NOSHIPS;
									if (ships->S3[i][k][Y] != FLD_LEN - 1)
										if (bf[ships->S3[i][k][X] - 1][ships->S3[i][k][Y] + 1] == FREE)
											bf[ships->S3[i][k][X] - 1][ships->S3[i][k][Y] + 1] = NOSHIPS;
								}
								if (ships->S3[i][k][X] != FLD_LEN - 1)
								{
									if (bf[ships->S3[i][k][X] + 1][ships->S3[i][k][Y]] == FREE)
										bf[ships->S3[i][k][X] + 1][ships->S3[i][k][Y]] = NOSHIPS;
									if (ships->S3[i][k][Y] != 0)
										if (bf[ships->S3[i][k][X] + 1][ships->S3[i][k][Y] - 1] == FREE)
											bf[ships->S3[i][k][X] + 1][ships->S3[i][k][Y] - 1] = NOSHIPS;
									if (ships->S3[i][k][Y] != FLD_LEN - 1)
										if (bf[ships->S3[i][k][X] + 1][ships->S3[i][k][Y] + 1] == FREE)
											bf[ships->S3[i][k][X] + 1][ships->S3[i][k][Y] + 1] = NOSHIPS;
								}
								if (ships->S3[i][k][Y] != 0)
									if (bf[ships->S3[i][k][X]][ships->S3[i][k][Y] - 1] == FREE)
										bf[ships->S3[i][k][X]][ships->S3[i][k][Y] - 1] = NOSHIPS;
								if (ships->S3[i][k][Y] != FLD_LEN - 1)
									if (bf[ships->S3[i][k][X]][ships->S3[i][k][Y] + 1] == FREE)
										bf[ships->S3[i][k][X]][ships->S3[i][k][Y] + 1] = NOSHIPS;
							}
						}
						
						return GOT;
					}
		}
	
		for (i = 0; i < NUM_OF_S4; i++)
		{
			if (!ships->issunkS4[i])
				for (j = 0; j < NUM_OF_D_S4; j++)
					if (ships->S4[i][j][X] == xpos && ships->S4[i][j][Y] == ypos)
					{				
						ships->isshotS4[i][j] = true;
						
						for (k = 0; k < NUM_OF_D_S4; k++)
						{
							if (!ships->isshotS4[i][k])
								alivedecks++;
						}
						
						if (!alivedecks)
						{
							ships->issunkS4[i] = true;
							
							for (k = 0; k < NUM_OF_D_S4; k++)
							{
								if (ships->S4[i][k][X] != 0)
								{
									if (bf[ships->S4[i][k][X] - 1][ships->S4[i][k][Y]] == FREE)
										bf[ships->S4[i][k][X] - 1][ships->S4[i][k][Y]] = NOSHIPS;
									if (ships->S4[i][k][Y] != 0)
										if (bf[ships->S4[i][k][X] - 1][ships->S4[i][k][Y] - 1] == FREE)
											bf[ships->S4[i][k][X] - 1][ships->S4[i][k][Y] - 1] = NOSHIPS;
									if (ships->S4[i][k][Y] != FLD_LEN - 1)
										if (bf[ships->S4[i][k][X] - 1][ships->S4[i][k][Y] + 1] == FREE)
											bf[ships->S4[i][k][X] - 1][ships->S4[i][k][Y] + 1] = NOSHIPS;
								}
								if (ships->S4[i][k][X] != FLD_LEN - 1)
								{
									if (bf[ships->S4[i][k][X] + 1][ships->S4[i][k][Y]] == FREE)
										bf[ships->S4[i][k][X] + 1][ships->S4[i][k][Y]] = NOSHIPS;
									if (ships->S4[i][k][Y] != 0)
										if (bf[ships->S4[i][k][X] + 1][ships->S4[i][k][Y] - 1] == FREE)
											bf[ships->S4[i][k][X] + 1][ships->S4[i][k][Y] - 1] = NOSHIPS;
									if (ships->S4[i][k][Y] != FLD_LEN - 1)
										if (bf[ships->S4[i][k][X] + 1][ships->S4[i][k][Y] + 1] == FREE)
											bf[ships->S4[i][k][X] + 1][ships->S4[i][k][Y] + 1] = NOSHIPS;
								}
								if (ships->S4[i][k][Y] != 0)
									if (bf[ships->S4[i][k][X]][ships->S4[i][k][Y] - 1] == FREE)
										bf[ships->S4[i][k][X]][ships->S4[i][k][Y] - 1] = NOSHIPS;
								if (ships->S4[i][k][Y] != FLD_LEN - 1)
									if (bf[ships->S4[i][k][X]][ships->S4[i][k][Y] + 1] == FREE)
										bf[ships->S4[i][k][X]][ships->S4[i][k][Y] + 1] = NOSHIPS;
							}
						}
						
						return GOT;
					}
		}
	}
	
	return DUMB;
}

int plugin_main(void)
{
    int action;
	int i, j, n;
	bool need_redraw = true; /* Do we need redraw field? */
	unsigned long startsec, prevsec = 0, sec, min = 0, delay = 0; /* Time stuff */
	int xpos, ypos;
	int xdir, ydir;
	Orientation ori = HORIZ;
	int curlen, curnum;
	
    FOR_NB_SCREENS(n)
    {
        struct screen *display = rb->screens[n];

        if (display->is_color)
            display->set_background(LCD_BLACK);
    }

	/* Set all squares free */
	InitFields();

	State = WAIT_FOR_PLAYER1_TO_PLACE_SHIPS;
	xpos = XOFFSET + 1;
	ypos = YOFFSET + 1;
	xdir = 1;
	ydir = -1;
	
	/* Get start second */
	startsec = get_sec();

	/* Main loop */
    while (true)
    {
		/* Measure time */
		sec = get_sec() - startsec;
		/* Update display once a second */
		if (sec != prevsec)
		{
			min = sec / 60;
			sec -= min * 60;
			min %= 60; /* > 60 minutes */
			need_redraw = true;
		}
		
		/* Draw everything */
		if (need_redraw)
		{
			FOR_NB_SCREENS(n)
			{
				struct screen *display = rb->screens[n];
				
				rb->lcd_clear_display();
				
				/* Battle map */
				display->bitmap(battleship_map,
								0,
								0,
								BMPWIDTH_battleship_map,
								BMPHEIGHT_battleship_map);
				
				if (State == WAIT_FOR_PLAYER1_TO_PLACE_SHIPS ||
					State == WAIT_FOR_PLAYER1_TO_MAKE_A_TURN)
				{
					display->bitmap(battleship_player1,
									xpos,
									ypos,
									BMPWIDTH_battleship_player1,
									BMPHEIGHT_battleship_player1);
					/*
					rb->lcd_bitmap_transparent(battleship_player1_transp,
									xpos,
									ypos,
									BMPWIDTH_battleship_player1_transp,
									BMPHEIGHT_battleship_player1_transp);
					*/
				}
				else if (State == WAIT_FOR_PLAYER2_TO_PLACE_SHIPS ||
						 State == WAIT_FOR_PLAYER2_TO_MAKE_A_TURN)
				{
					display->bitmap(battleship_player2,
									xpos,
									ypos,
									BMPWIDTH_battleship_player2,
									BMPHEIGHT_battleship_player2);
				}
				else
				{
					/* Player's name */
					if (State == PLACE_SHIPS_PLAYER1 ||
						State == TURN_PLAYER1 ||
						State == SHOW_FIELD_AFTER_TURN_OF_PLAYER1)
					{
						display->bitmap(battleship_player1,
										BMPWIDTH_battleship_map,
										0,
										BMPWIDTH_battleship_player1,
										BMPHEIGHT_battleship_player1);
					}
					else if (State == PLACE_SHIPS_PLAYER2 ||
							 State == TURN_PLAYER2 ||
							 State == SHOW_FIELD_AFTER_TURN_OF_PLAYER2)
					{
						display->bitmap(battleship_player2,
										BMPWIDTH_battleship_map,
										0,
										BMPWIDTH_battleship_player2,
										BMPHEIGHT_battleship_player2);
					}

					/* Objects on field */
					for (i = 0; i < FLD_LEN; i++)
						for (j = 0; j < FLD_LEN; j++)
						{
							if (State == TURN_PLAYER1 ||
								State == SHOW_FIELD_AFTER_TURN_OF_PLAYER1)
							{
								if (battlefield_p2[i][j] == SHOTWATER)
									display->bitmap(battleship_missed,
													XOFFSET + i * SQSIZE + (SQSIZE - BMPWIDTH_battleship_missed) / 2,
													YOFFSET + j * SQSIZE + (SQSIZE - BMPWIDTH_battleship_missed) / 2,
													BMPWIDTH_battleship_missed,
													BMPHEIGHT_battleship_missed);
								else if (battlefield_p2[i][j] == SHOTSHIP)
									display->bitmap(battleship_shiponedead,
													XOFFSET + i * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shiponedead) / 2,
													YOFFSET + j * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shiponedead) / 2,
													BMPWIDTH_battleship_shiponedead,
													BMPHEIGHT_battleship_shiponedead);
								else if (battlefield_p2[i][j] == NOSHIPS)
									display->bitmap(battleship_noships,
													XOFFSET + i * SQSIZE + (SQSIZE - BMPWIDTH_battleship_noships) / 2,
													YOFFSET + j * SQSIZE + (SQSIZE - BMPWIDTH_battleship_noships) / 2,
													BMPWIDTH_battleship_noships,
													BMPHEIGHT_battleship_noships);
							}
							else if (State == TURN_PLAYER2 ||
								State == SHOW_FIELD_AFTER_TURN_OF_PLAYER2)
							{
								if (battlefield_p1[i][j] == SHOTWATER)
									display->bitmap(battleship_missed,
													XOFFSET + i * SQSIZE + (SQSIZE - BMPWIDTH_battleship_missed) / 2,
													YOFFSET + j * SQSIZE + (SQSIZE - BMPWIDTH_battleship_missed) / 2,
													BMPWIDTH_battleship_missed,
													BMPHEIGHT_battleship_missed);
								else if (battlefield_p1[i][j] == SHOTSHIP)
									display->bitmap(battleship_shiponedead,
													XOFFSET + i * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shiponedead) / 2,
													YOFFSET + j * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shiponedead) / 2,
													BMPWIDTH_battleship_shiponedead,
													BMPHEIGHT_battleship_shiponedead);
								else if (battlefield_p1[i][j] == NOSHIPS)
									display->bitmap(battleship_noships,
													XOFFSET + i * SQSIZE + (SQSIZE - BMPWIDTH_battleship_noships) / 2,
													YOFFSET + j * SQSIZE + (SQSIZE - BMPWIDTH_battleship_noships) / 2,
													BMPWIDTH_battleship_noships,
													BMPHEIGHT_battleship_noships);
							}
							else if (State == PLACE_SHIPS_PLAYER1)
							{
								if (battlefield_p1[i][j] == SHIP)
									display->bitmap(battleship_shipone,
													XOFFSET + i * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shipone) / 2,
													YOFFSET + j * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shipone) / 2,
													BMPWIDTH_battleship_shipone,
													BMPHEIGHT_battleship_shipone);
							}
							else if (State == PLACE_SHIPS_PLAYER2)
							{
								if (battlefield_p2[i][j] == SHIP)
									display->bitmap(battleship_shipone,
													XOFFSET + i * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shipone) / 2,
													YOFFSET + j * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shipone) / 2,
													BMPWIDTH_battleship_shipone,
													BMPHEIGHT_battleship_shipone);
							}
						}
					
					/* Ships placing is in progress... */					
					if (State == PLACE_SHIPS_PLAYER1 ||
						State == PLACE_SHIPS_PLAYER2)
					{
						display->bitmap(battleship_placeships,
										BMPWIDTH_battleship_map,
										BMPHEIGHT_battleship_player1,
										BMPWIDTH_battleship_placeships,
										BMPHEIGHT_battleship_placeships);
						for (i = 0; i < curlen; i++)
						{
							if (ori == VERT)
								display->bitmap(battleship_shipone_notplaced,
												XOFFSET + xpos * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shipone_notplaced) / 2,
												YOFFSET + (ypos + i) * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shipone_notplaced) / 2,
												BMPWIDTH_battleship_shipone_notplaced,
												BMPHEIGHT_battleship_shipone_notplaced);
							else
								display->bitmap(battleship_shipone_notplaced,
												XOFFSET + (xpos + i) * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shipone_notplaced) / 2,
												YOFFSET + ypos * SQSIZE + (SQSIZE - BMPWIDTH_battleship_shipone_notplaced) / 2,
												BMPWIDTH_battleship_shipone_notplaced,
												BMPHEIGHT_battleship_shipone_notplaced);
						}
					}
					
					/* Aim, your turn */
					if (State == TURN_PLAYER1 ||
						State == TURN_PLAYER2)
					{
						display->bitmap(battleship_aim,
										XOFFSET + xpos * SQSIZE + (SQSIZE - BMPWIDTH_battleship_aim) / 2,
										YOFFSET + ypos * SQSIZE + (SQSIZE - BMPHEIGHT_battleship_aim) / 2,
										BMPWIDTH_battleship_aim,
										BMPHEIGHT_battleship_aim);
						display->bitmap(battleship_yourturn,
										BMPWIDTH_battleship_map,
										BMPHEIGHT_battleship_player1,
										BMPWIDTH_battleship_yourturn,
										BMPHEIGHT_battleship_yourturn);
					}
					
					/* Time */
					display->bitmap(numbitmaps[min / 10],
									BMPWIDTH_battleship_map,
									LCD_HEIGHT - BMPHEIGHT_battleship_n0 - 5,
									BMPWIDTH_battleship_n0,
									BMPHEIGHT_battleship_n0);
									
					display->bitmap(numbitmaps[min % 10],
									BMPWIDTH_battleship_map + BMPWIDTH_battleship_n0 - 2,
									LCD_HEIGHT - BMPHEIGHT_battleship_n0 - 5,
									BMPWIDTH_battleship_n0,
									BMPHEIGHT_battleship_n0);
					
					display->bitmap(battleship_nsep,
									BMPWIDTH_battleship_map + BMPWIDTH_battleship_n0 * 2 - 4,
									LCD_HEIGHT - BMPHEIGHT_battleship_n0 - 5,
									BMPWIDTH_battleship_nsep,
									BMPHEIGHT_battleship_nsep);
					
					display->bitmap(numbitmaps[sec / 10],
									BMPWIDTH_battleship_map + BMPWIDTH_battleship_n0 * 2 - 4 + BMPWIDTH_battleship_nsep,
									LCD_HEIGHT - BMPHEIGHT_battleship_n0 - 5,
									BMPWIDTH_battleship_n0,
									BMPHEIGHT_battleship_n0);
									
					display->bitmap(numbitmaps[sec % 10],
									BMPWIDTH_battleship_map + BMPWIDTH_battleship_n0 * 3 - 6 + BMPWIDTH_battleship_nsep,
									LCD_HEIGHT - BMPHEIGHT_battleship_n0 - 5,
									BMPWIDTH_battleship_n0,
									BMPHEIGHT_battleship_n0);
				}
				
				display->update();
			}
			
			need_redraw = false;
		}
		
		if (State == WAIT_FOR_PLAYER1_TO_PLACE_SHIPS ||
			State == WAIT_FOR_PLAYER2_TO_PLACE_SHIPS ||
			State == WAIT_FOR_PLAYER1_TO_MAKE_A_TURN ||
			State == WAIT_FOR_PLAYER2_TO_MAKE_A_TURN)
		{
			if (xpos == XOFFSET || xpos + BMPWIDTH_battleship_player1 == XOFFSET + SQSIZE * FLD_LEN)
				xdir *= -1;
			if (ypos == YOFFSET || ypos + BMPHEIGHT_battleship_player1 == YOFFSET + SQSIZE * FLD_LEN)
				ydir *= -1;
			xpos += xdir;
			ypos += ydir;
			need_redraw = true;
		}
		else if (State == SHOW_FIELD_AFTER_TURN_OF_PLAYER1)
		{
			if (get_sec() - delay >= DELAY_AFTER_TURN)
			{
				State = WAIT_FOR_PLAYER2_TO_MAKE_A_TURN;
				xpos = XOFFSET + 1;
				ypos = YOFFSET + 1;
				xdir = 1;
				ydir = -1;
			}
		}
		else if (State == SHOW_FIELD_AFTER_TURN_OF_PLAYER2)
		{
			if (get_sec() - delay >= DELAY_AFTER_TURN)
			{
				State = WAIT_FOR_PLAYER1_TO_MAKE_A_TURN;
				xpos = XOFFSET + 1;
				ypos = YOFFSET + 1;
				xdir = 1;
				ydir = -1;
			}
		}
		
		rb->sleep(SLEEP_TIME);
		//rb->yield();

		action = pluginlib_getaction(TIMEOUT_NOBLOCK,
									 plugin_contexts,
									 NB_ACTION_CONTEXTS);
		switch (action)
		{
			case PLA_QUIT:
				cleanup(NULL);
				return PLUGIN_OK;
				
			case PLA_UP:
				if (State == WAIT_FOR_PLAYER1_TO_PLACE_SHIPS)
				{
					State = PLACE_SHIPS_PLAYER1;
					curlen = NUM_OF_D_S4;
					curnum = N1;
					xpos = 0;
					ypos = 0;
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p1, &ships_p1);
					ClearField(battlefield_p1);
					RegenField(battlefield_p1, &ships_p1);
				}
				else if (State == WAIT_FOR_PLAYER2_TO_PLACE_SHIPS)
				{
					State = PLACE_SHIPS_PLAYER2;
					curlen = NUM_OF_D_S4;
					curnum = N1;
					xpos = 0;
					ypos = 0;
					ori = HORIZ;
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p2, &ships_p2);
					ClearField(battlefield_p2);
					RegenField(battlefield_p2, &ships_p2);
				}
				else if (State == WAIT_FOR_PLAYER1_TO_MAKE_A_TURN)
				{
					State = TURN_PLAYER1;
					xpos = 0;
					ypos = 0;
				}
				else if (State == WAIT_FOR_PLAYER2_TO_MAKE_A_TURN)
				{
					State = TURN_PLAYER2;
					xpos = 0;
					ypos = 0;
				}
				else if (State == TURN_PLAYER1 ||
						 State == TURN_PLAYER2)
				{
					if (ypos == 0)
						ypos = FLD_LEN - 1;
					else
						ypos--;
				}
				else if (State == PLACE_SHIPS_PLAYER1)
				{
					if (ypos > 0)
						ypos--;
					
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p1, &ships_p1);
				}
				else if (State == PLACE_SHIPS_PLAYER2)
				{	
					if (ypos > 0)
						ypos--;
					
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p2, &ships_p2);
				}
				break;
				
			case PLA_DOWN:
				if (State == WAIT_FOR_PLAYER1_TO_PLACE_SHIPS)
				{
					State = PLACE_SHIPS_PLAYER1;
					curlen = NUM_OF_D_S4;
					curnum = N1;
					xpos = 0;
					ypos = 0;
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p1, &ships_p1);
					ClearField(battlefield_p1);
					RegenField(battlefield_p1, &ships_p1);
				}
				else if (State == WAIT_FOR_PLAYER2_TO_PLACE_SHIPS)
				{
					State = PLACE_SHIPS_PLAYER2;
					curlen = NUM_OF_D_S4;
					curnum = N1;
					xpos = 0;
					ypos = 0;
					ori = HORIZ;
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p2, &ships_p2);
					ClearField(battlefield_p2);
					RegenField(battlefield_p2, &ships_p2);
				}
				else if (State == WAIT_FOR_PLAYER1_TO_MAKE_A_TURN)
				{
					State = TURN_PLAYER1;
					xpos = 0;
					ypos = 0;
				}
				else if (State == WAIT_FOR_PLAYER2_TO_MAKE_A_TURN)
				{
					State = TURN_PLAYER2;
					xpos = 0;
					ypos = 0;
				}
				else if (State == TURN_PLAYER1 ||
						 State == TURN_PLAYER2)
				{
					if (ypos == FLD_LEN - 1)
						ypos = 0;
					else
						ypos++;
				}
				else if (State == PLACE_SHIPS_PLAYER1)
				{
					if (ori == VERT)
					{
						if (ypos + curlen < FLD_LEN)
							ypos++;
					}
					else
					{
						if (ypos < FLD_LEN - 1)
							ypos++;
					}
					
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p1, &ships_p1);
				}
				else if (State == PLACE_SHIPS_PLAYER2)
				{
					if (ori == VERT)
					{
						if (ypos + curlen < FLD_LEN)
							ypos++;
					}
					else
					{
						if (ypos < FLD_LEN - 1)
							ypos++;
					}
					
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p2, &ships_p2);
				}
				break;
			
			case PLA_RIGHT:
				if (State == WAIT_FOR_PLAYER1_TO_PLACE_SHIPS)
				{
					State = PLACE_SHIPS_PLAYER1;
					curlen = NUM_OF_D_S4;
					curnum = N1;
					xpos = 0;
					ypos = 0;
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p1, &ships_p1);
					ClearField(battlefield_p1);
					RegenField(battlefield_p1, &ships_p1);
				}
				else if (State == WAIT_FOR_PLAYER2_TO_PLACE_SHIPS)
				{
					State = PLACE_SHIPS_PLAYER2;
					curlen = NUM_OF_D_S4;
					curnum = N1;
					xpos = 0;
					ypos = 0;
					ori = HORIZ;
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p2, &ships_p2);
					ClearField(battlefield_p2);
					RegenField(battlefield_p2, &ships_p2);
				}
				else if (State == WAIT_FOR_PLAYER1_TO_MAKE_A_TURN)
				{
					State = TURN_PLAYER1;
					xpos = 0;
					ypos = 0;
				}
				else if (State == WAIT_FOR_PLAYER2_TO_MAKE_A_TURN)
				{
					State = TURN_PLAYER2;
					xpos = 0;
					ypos = 0;
				}
				else if (State == TURN_PLAYER1 ||
						 State == TURN_PLAYER2)
				{	
					if (xpos == FLD_LEN - 1)
						xpos = 0;
					else
						xpos++;
				}
				else if (State == PLACE_SHIPS_PLAYER1)
				{
					if (ori == VERT)
					{
						if (xpos < FLD_LEN - 1)
							xpos++;
					}
					else
					{
						if (xpos + curlen < FLD_LEN)
							xpos++;
					}
					
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p1, &ships_p1);
				}
				else if (State == PLACE_SHIPS_PLAYER2)
				{
					if (ori == VERT)
					{
						if (xpos < FLD_LEN - 1)
							xpos++;
					}
					else
					{
						if (xpos + curlen < FLD_LEN)
							xpos++;
					}
					
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p2, &ships_p2);
				}
				break;
			
			case PLA_LEFT:
				if (State == WAIT_FOR_PLAYER1_TO_PLACE_SHIPS)
				{
					State = PLACE_SHIPS_PLAYER1;
					curlen = NUM_OF_D_S4;
					curnum = N1;
					xpos = 0;
					ypos = 0;
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p1, &ships_p1);
					ClearField(battlefield_p1);
					RegenField(battlefield_p1, &ships_p1);
				}
				else if (State == WAIT_FOR_PLAYER2_TO_PLACE_SHIPS)
				{
					State = PLACE_SHIPS_PLAYER2;
					curlen = NUM_OF_D_S4;
					curnum = N1;
					xpos = 0;
					ypos = 0;
					ori = HORIZ;
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p2, &ships_p2);
					ClearField(battlefield_p2);
					RegenField(battlefield_p2, &ships_p2);
				}
				else if (State == WAIT_FOR_PLAYER1_TO_MAKE_A_TURN)
				{
					State = TURN_PLAYER1;
					xpos = 0;
					ypos = 0;
				}
				else if (State == WAIT_FOR_PLAYER2_TO_MAKE_A_TURN)
				{
					State = TURN_PLAYER2;
					xpos = 0;
					ypos = 0;
				}
				else if (State == TURN_PLAYER1 ||
						 State == TURN_PLAYER2)
				{	
					if (xpos == 0)
						xpos = FLD_LEN - 1;
					else
						xpos--;
				}
				else if (State == PLACE_SHIPS_PLAYER1)
				{
					if (xpos > 0)
						xpos--;
					
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p1, &ships_p1);
				}
				else if (State == PLACE_SHIPS_PLAYER2)
				{
					if (xpos > 0)
						xpos--;
					
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p2, &ships_p2);
				}
				break;
			
			case PLA_MENU:
				if (State == WAIT_FOR_PLAYER1_TO_PLACE_SHIPS)
				{
					State = PLACE_SHIPS_PLAYER1;
					curlen = NUM_OF_D_S4;
					curnum = N1;
					xpos = 0;
					ypos = 0;
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p1, &ships_p1);
				}
				else if (State == WAIT_FOR_PLAYER2_TO_PLACE_SHIPS)
				{
					State = PLACE_SHIPS_PLAYER2;
					curlen = NUM_OF_D_S4;
					curnum = N1;
					xpos = 0;
					ypos = 0;
					ori = HORIZ;
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p2, &ships_p2);
				}
				else if (State == WAIT_FOR_PLAYER1_TO_MAKE_A_TURN)
				{
					State = TURN_PLAYER1;
					xpos = 0;
					ypos = 0;
				}
				else if (State == WAIT_FOR_PLAYER2_TO_MAKE_A_TURN)
				{
					State = TURN_PLAYER2;
					xpos = 0;
					ypos = 0;
				}
				else if (State == PLACE_SHIPS_PLAYER1 ||
						 State == PLACE_SHIPS_PLAYER2)
				{
					ori = ori == VERT ? HORIZ : VERT;
				}
				break;
			
			case PLA_FIRE: /* Shoot / place a ship */
				if (State == WAIT_FOR_PLAYER1_TO_PLACE_SHIPS)
				{
					State = PLACE_SHIPS_PLAYER1;
					curlen = NUM_OF_D_S4;
					curnum = N1;
					xpos = 0;
					ypos = 0;
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p1, &ships_p1);
				}
				else if (State == WAIT_FOR_PLAYER2_TO_PLACE_SHIPS)
				{
					State = PLACE_SHIPS_PLAYER2;
					curlen = NUM_OF_D_S4;
					curnum = N1;
					xpos = 0;
					ypos = 0;
					ori = HORIZ;
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p2, &ships_p2);
				}
				else if (State == WAIT_FOR_PLAYER1_TO_MAKE_A_TURN)
				{
					State = TURN_PLAYER1;
					xpos = 0;
					ypos = 0;
				}
				else if (State == WAIT_FOR_PLAYER2_TO_MAKE_A_TURN)
				{
					State = TURN_PLAYER2;
					xpos = 0;
					ypos = 0;
				}
				else if (State == PLACE_SHIPS_PLAYER1)
				{
					if (CorrectPosition(xpos, ypos, ori, curlen, battlefield_p1))
					{
						PlaceShipToBF(xpos, ypos, ori, curlen, curnum, battlefield_p1, &ships_p1);
						if (NextThing(&curlen, &curnum))
						{
							State = WAIT_FOR_PLAYER2_TO_PLACE_SHIPS;
							xpos = XOFFSET + 1;
							ypos = YOFFSET + 1;
							xdir = 1;
							ydir = -1;
						}
					}
				}
				else if (State == PLACE_SHIPS_PLAYER2)
				{
					if (CorrectPosition(xpos, ypos, ori, curlen, battlefield_p2))
					{
						PlaceShipToBF(xpos, ypos, ori, curlen, curnum, battlefield_p2, &ships_p2);
						if (NextThing(&curlen, &curnum))
						{
							State = WAIT_FOR_PLAYER1_TO_MAKE_A_TURN;
							xpos = XOFFSET + 1;
							ypos = YOFFSET + 1;
							xdir = 1;
							ydir = -1;
						}
					}
				}
				else if (State == TURN_PLAYER1)
				{
					shotres = Shoot(xpos, ypos, battlefield_p2, &ships_p2);
					
					if (shotres == MISSED)
					{
						State = SHOW_FIELD_AFTER_TURN_OF_PLAYER1;
						delay = get_sec();
					}
					else if (shotres == GOT)
					{
						/*
						State = SHOW_FIELD_AFTER_TURN_OF_PLAYER1;
						delay = get_sec();
						*/
						/* Ok, nice shot, make one more */
					}
					else if (shotres == DUMB)
					{
						/* Do nothing, shoot again! */
					}
				}
				else if (State == TURN_PLAYER2)
				{
					shotres = Shoot(xpos, ypos, battlefield_p1, &ships_p1);
					
					if (shotres == MISSED)
					{
						State = SHOW_FIELD_AFTER_TURN_OF_PLAYER2;
						delay = get_sec();
					}
					else if (shotres == GOT)
					{
						/*
						State = SHOW_FIELD_AFTER_TURN_OF_PLAYER2;
						delay = get_sec();
						*/
						/* Ok, nice shot, make one more */
					}
					else if (shotres == DUMB)
					{
						/* Do nothing, shoot again! */
					}
				}
				break;
				
			case PLA_START:
				if (State == WAIT_FOR_PLAYER1_TO_PLACE_SHIPS)
				{
					State = PLACE_SHIPS_PLAYER1;
					curlen = NUM_OF_D_S4;
					curnum = N1;
					xpos = 0;
					ypos = 0;
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p1, &ships_p1);
				}
				else if (State == WAIT_FOR_PLAYER2_TO_PLACE_SHIPS)
				{
					State = PLACE_SHIPS_PLAYER2;
					curlen = NUM_OF_D_S4;
					curnum = N1;
					xpos = 0;
					ypos = 0;
					ori = HORIZ;
					PlaceShip(xpos, ypos, ori, curlen, curnum, battlefield_p2, &ships_p2);
				}
				else if (State == WAIT_FOR_PLAYER1_TO_MAKE_A_TURN)
				{
					State = TURN_PLAYER1;
					xpos = 0;
					ypos = 0;
				}
				else if (State == WAIT_FOR_PLAYER2_TO_MAKE_A_TURN)
				{
					State = TURN_PLAYER2;
					xpos = 0;
					ypos = 0;
				}
				break;
				
			default:
				if (rb->default_event_handler_ex(action, cleanup, NULL)
					== SYS_USB_CONNECTED)
					return PLUGIN_USB_CONNECTED;
				break;
		}
    }
}

/* this is the plugin entry point */
enum plugin_status plugin_start(const void* parameter)
{
	int ret;
	
    /* if you don't use the parameter, you can do like
       this to avoid the compiler warning about it */
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
