/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2010 Lev Panov, Nick Petrov
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

#include "ai.h"

/* PATHELEM Path[MAP_W][MAP_H]; */
static NODE Nodes[MAP_W][MAP_H]; 

bool GetNode( const Field const *F, int x, int y )
{
  if(F->map[x][y] == SQUARE_FREE)
    return TRUE;
  return FALSE;
}


void InitNodes( const Field const *F )
{
  int x, y;
  for(x = 0; x < MAP_W; x++)
    for(y = 0; y < MAP_H; y++)
    {
	  Nodes[x][y].IsWalkable = GetNode(F, x, y);
	  Nodes[x][y].IsOnOpen = FALSE;
	  Nodes[x][y].IsOnClose = FALSE;
	  Nodes[x][y].G = 0;
	  Nodes[x][y].H = 0;
	  Nodes[x][y].F = 0;
	  Nodes[x][y].ParentX = 0;
	  Nodes[x][y].ParentY = 0;
	}
}

void FindPath( PATH *Path, int StartX, int StartY, 
                int EndX, int EndY )
{
  int x = 0, y = 0; // for running through the nodes
  int dx, dy; // for the 8 squares adjacent to each node
  int cx = StartX, cy = StartY;
  int lowestf = UNREAL_F; // start with the lowest being the highest
  int count = 0;
  // add starting node to open list
  Nodes[StartX][StartY].IsOnOpen = TRUE;
  Nodes[StartX][StartY].IsOnClose = FALSE;
//////////////////////LOOP BEGINS HERE/////////////////////////
  while ( cx != EndX || cy != EndY )
  {
    //look for lowest F cost node on open list - this becomes the current node
	lowestf = UNREAL_F;
	for ( x = 0; x < MAP_W; x++ )
	{
	  for ( y = 0; y < MAP_H; y++)
	  {
	    Noded[x][y].F = Nodes[x][y].G + Nodes[x][y].H;
		if ( Nodes[x][y].IsOnOpen )
		{
		  if ( Nodes[x][y].F < lowestf )
		  {
		    cx = x;
			cy = y;
			lowestf = Nodes[x][y].F;
		  }
		}
		
	  }
	}
	// we found it, so now put that node on the closed list
	Nodes[cx][cy].IsOnOpen = FALSE;
	Nodes[cx][cy].IsOnClose = TRUE;
	// for each of the 8 adjacent node
	for ( dx = -1; dx <= 1; dx++ )
	{
	  for ( dy = -1; dy <= 1; dy++ )
	  {
		if ( (dx != 0) || (dy != 0) )
		{
		  if ( (cx + dx) < MAP_W && (cx + dx) > -1 && 
		       (cy + dy) < MAP_H && (cy + dy) >-1 )
		  {
			 // if its walkable and not on the closed list
			 if ( Nodes[cx+dx][cy+dy].IsWalkable == TRUE 
			   && Nodes[cx+dx][cy+dy].IsOnClose == FALSE )
			 {
			    //if its not on open list
				if ( Nodes[cx+dx][cy+dy].IsOnOpen == FALSE )
				{
				  //add it to open list
				  Nodes[cx+dx][cy+dy].IsOnOpen = TRUE;
				  Nodes[cx+dx][cy+dy].IsOnClose = FALSE;
				  //make the current node its parent
				  Nodes[cx+dx][cy+dy].ParentX = cx;
				  Nodes[cx+dx][cy+dy].ParentY = cy;
				  //work out G
				  if ( dx != 0 && dy != 0) 
				    Nodes[cx+dx][cy+dy].G = 14; // diagonals cost 14
				  else 
				    Nodes[cx+dx][cy+dy].G = 10; // straights cost 10
				  //work out H
				  //MANHATTAN METHOD
				  Nodes[cx+dx][cy+dy].H =
							(rb->abs(endx-(cx+dx)) + 
							 rb->abs(endy-(cy+dy)))*10;
				  Nodes[cx+dx][cy+dy].F =
							Nodes[cx+dx][cy+dy]->G + 
							Nodes[cx+dx][cy+dy]->H;

							
				}
				//otherwise it is on the open list
				else if ( Nodes[cx+dx][cy+dy].IsOnClose == FALSE 
						  && Nodes[cx+dx][cy+dy].IsOnOpen == TRUE )
				{
				  if ( dx == 0 || dy == 0)   // if its not a diagonal
				  {
				     if ( Nodes[cx+dx][cy+dy].G == 14 )   //and it was previously
					 {
					   Nodes[cx+dx][cy+dy].G = 10; // straight score 10
					   //change its parent because its a shorter distance
					   Nodes[cx+dx][cy+dy].ParentX = cx;
					   Nodes[cx+dx][cy+dy].ParentY = cy;
					   //recalc H
					   Nodes[cx+dx][cy+dy].H = 
					  					 (rb->abs(endx-(cx+dx)) + 
										  rb->abs(endy-(cy+dy))) * 10;
					   //recalc F
					   Nodes[cx+dx][cy+dy].F = 
										 Nodes[cx+dx][cy+dy].G 
										 + Nodes[cx+dx][cy+dy].H;
					 }
				  } 
				}//end else
			}// end if walkable and not on closed list
		}
	}
  }
  
  //follow all the parents back to the start
  
  cx = EndX;
  cy = EndY;
  
  while ( cx != StartX || cy != StartY )
  {
	Path->Path[count].X = Nodes[cx][cy].ParentX;
	Path->Path[count].Y = Nodes[cx][cy].ParentY;
	cx = Nodes[cx][cy].ParentX;
	cy = Nodes[cx][cy].ParentY;
	Path->Distance++;
	if ( count > 100 )
	  break;
  }
  //return Path; //we're done, return a pointer to the final path;
}

void UpdateAI( Player* Players )
{
  int i;
  int size = sizeof(Players) / sizeof(Players[0]);
  PATH Path = {0};
  
  for(i = 0; i < size; i++)
  {
    rb->memset(&Path, 0, sizeof(PATH));
    //FindPath( &Path,  
  }
}

