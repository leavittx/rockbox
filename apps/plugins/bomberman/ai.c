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

#include "plugin.h"
#include "game.h"

#define _abs(x) ((x) >= 0 ? (x) : -(x))

#define UNREAL_F 999

#define PATH_OFFSET 1
#define MOVE_COST 10

typedef struct
{
  bool IsWalkable;
  bool IsOnOpen;
  bool IsOnClose;
  int G, H, F;
  int ParentX, ParentY;
} NODE;

typedef struct
{
  int X, Y;
} PATHELEM;

typedef struct
{
  PATHELEM Path[MAP_W * MAP_H];
  int Distance;
} PATH;

typedef struct
{
  int ClosestPlayer;
  bool Danger;
  PATHELEM SafetyPlace;
  int Bombs;
} AiVars;

NODE Nodes[MAP_W][MAP_H]; 
AiVars AI[MAX_PLAYERS];

bool GetNode(Field *field, int x, int y, bool UseBoxes)
{  
	if (field->map[x][y] == SQUARE_FREE
	   || (UseBoxes && field->map[x][y] == SQUARE_BOX))
		return true;

	return false;
}

void InitNodes(Field *F, bool UseBoxes)
{
  int x, y;

  for (x = 0; x < MAP_W; x++)
    for (y = 0; y < MAP_H; y++)
    {
	    Nodes[x][y].IsWalkable = GetNode(F, x, y, UseBoxes);
	    Nodes[x][y].IsOnOpen = false;
	    Nodes[x][y].IsOnClose = false;
	    Nodes[x][y].G = 0;
	    Nodes[x][y].H = 0;
	    Nodes[x][y].F = 0;
	    Nodes[x][y].ParentX = 0;
	    Nodes[x][y].ParentY = 0;
	}
}

int FindPath(Game *G, PATH *Path, int StartX, int StartY,
                int EndX, int EndY, bool UseBoxes)
{
	int x = 0, y = 0; // for running through the nodes
	int dx, dy; // for the 8 (4) squares adjacent to each node
	int cx = StartX, cy = StartY;
	int lowestf = UNREAL_F; // start with the lowest being the highest
	int count = 0;
  
	// debug info
	/*
	int desc;
	char logStr[100] = "\n";
  
	if ((desc = rb->open(PLUGIN_GAMES_DIR "/path.txt", O_WRONLY | O_APPEND | O_CREAT, 0666)) < 0)
	{ 
	  rb->splash(HZ, "Cant open");
	  return;
	}
	rb->write(desc, logStr, rb->strlen(logStr));
	rb->memset(logStr, 0, 100);
	rb->snprintf(logStr, 5, "%i %i\n", n, m);
	rb->write(desc, logStr, rb->strlen(logStr));
	*/
  
  InitNodes(&G->field, UseBoxes);
  // add starting node to open list
  Nodes[StartX][StartY].IsOnOpen = true;
  Nodes[StartX][StartY].IsOnClose = false;
  
  while (cx != EndX || cy != EndY)
  {
	count++;
	
	if (count > 100)
	{
		// debug
		/*
		rb->memset(logStr, 0, 100);
		rb->snprintf(logStr, 8, "%s\n", "Return");
		rb->write(desc, logStr, rb->strlen(logStr));
		*/

		return 0;
        }

    // look for lowest F cost node on open list - this becomes the current node
	lowestf = UNREAL_F;
	for (x = 0; x < MAP_W; x++)
	{
	  for (y = 0; y < MAP_H; y++)
	  {
		Nodes[x][y].F = Nodes[x][y].G + Nodes[x][y].H;
		if (Nodes[x][y].IsOnOpen)
		{
			if (Nodes[x][y].F < lowestf)
			{
				cx = x;
				cy = y;
				lowestf = Nodes[x][y].F;
			}
		}
	  }
	}
	
	// we found it, so now put that node on the closed list
	Nodes[cx][cy].IsOnOpen = false;
	Nodes[cx][cy].IsOnClose = true;
	
	// for each of the 8 (4) adjacent node
	for (dx = -1; dx <= 1; dx++)
	{
	  for (dy = -1; dy <= 1; dy++)
	  {
		// we don't use diagonals in bomberman
		if (dx != -dy && dx != dy)
		{
		  if ((cx + dx) < MAP_W && (cx + dx) > -1 && 
		       (cy + dy) < MAP_H && (cy + dy) > -1)
		  {
			 // if its walkable and not on the closed list
			 if (Nodes[cx+dx][cy+dy].IsWalkable == true
				&& Nodes[cx+dx][cy+dy].IsOnClose == false)
			 {
			    // if its not on open list
				if (Nodes[cx+dx][cy+dy].IsOnOpen == false)
				{
				  // add it to open list
				  Nodes[cx+dx][cy+dy].IsOnOpen = true;
				  Nodes[cx+dx][cy+dy].IsOnClose = false;
				  // make the current node its parent
				  Nodes[cx+dx][cy+dy].ParentX = cx;
				  Nodes[cx+dx][cy+dy].ParentY = cy;
				  
				  // debug
				  /*
				  rb->memset(logStr, 0, 100);
	              rb->snprintf(logStr, 21 , "%i C: %i %i P: %i %i\n", n, cx+dx, cy+dy, cx, cy);
	              rb->write(desc, logStr, rb->strlen(logStr));
	              */
	              
				  // work out G
				  Nodes[cx+dx][cy+dy].G = MOVE_COST; // straights cost 10
				  // work out H
				  // MANHATTAN METHOD
				  Nodes[cx+dx][cy+dy].H =
							(_abs(EndX-(cx+dx)) + 
							 _abs(EndY-(cy+dy))) * MOVE_COST;
				  Nodes[cx+dx][cy+dy].F =
							Nodes[cx+dx][cy+dy].G + 
							Nodes[cx+dx][cy+dy].H;
				}
			} // end if walkable and not on closed list
		    }
		}
	  }
	}
  }
  
  // follow all the parents back to the start
  cx = EndX;
  cy = EndY;
  Path->Distance = 0;
  Path->Path[Path->Distance].X = cx;
  Path->Path[Path->Distance].Y = cy;
  Path->Distance++;
  
  while (cx != StartX || cy != StartY)
  {
	Path->Path[Path->Distance].X = Nodes[cx][cy].ParentX;
	Path->Path[Path->Distance].Y = Nodes[cx][cy].ParentY;
	cx = Path->Path[Path->Distance].X;
	cy = Path->Path[Path->Distance].Y;
	
	// debug
	/*
	rb->memset(logStr, 0, 100);
	rb->snprintf(logStr, 12 , "%i PP: %i %i\n", n, Path->Path[Path->Distance].X, 
	             Path->Path[Path->Distance].Y);
	rb->write(desc, logStr, rb->strlen(logStr));
	*/
	
	Path->Distance++;
	if (Path->Distance > 100)
	  return 0;
  }
  
	// debug
	//rb->close(desc);
  return 1;
}



void CopyPaths(PATH *Dst, PATH *Src)
{
	int i;
	
	Dst->Distance = Src->Distance;
	
	for (i = Src->Distance - 1; i >= 0; i--)
	{
	  Dst->Path[(Src->Distance - 1) - i].X = Src->Path[i].X;
	  Dst->Path[(Src->Distance - 1) - i].Y = Src->Path[i].Y;
	}
}

void LogPath(PATH *P)
{
    int file;
    int i;
    char logStr[100] = "\n";

    if ((file = rb->open(PLUGIN_GAMES_DIR "/safe_path.txt", 
	O_WRONLY | O_CREAT | O_APPEND, 0666)) < 0)
    {
	return;
    }
    
    rb->write(file, logStr, 100);
    
    for (i = 0; i < P->Distance; i++)
    {
	rb->memset(logStr, 0, 100);
	rb->snprintf(logStr, 7, "%i %i\n", P->Path[i].X, P->Path[i].Y);
	rb->write(file, logStr, 100);
    }

    rb->close(file);
}

/*int CheckIfThereAnyWall(Game *G, Player *P)
{
  int i, j;
  int ProtectWalls = 0;
  
  for(i = 0; i < MAX_BOMBS; i++)
  {
    ProtectWalls = 0;
    if(G->field.bombs[i] >= BOMB_PLACED)
    {
      if(G->field.bombs[i].ypos == P->ypos)
      {
       if(G->field.bombs[i].xpos < P->xpos)
       {
         for(j = G->field.bombs[i].xpos; j < P->xpos; j++)
	   if(G->field.map[j][P->ypos] == SQUARE_BLOCK
	    || G->field.map[j][P->ypos] == SQUARE_BOX)
	    {
	      ProtectWalls++; 
	      break;
	    }
       }
       else
       {
         for(j = P->xpos; j < G->field.bombs[i].xpos; j++)
	   if(G->field.map[j][P->ypos] == SQUARE_BLOCK
	    || G->field.map[j][P->ypos] == SQUARE_BOX)
	    {
	      ProtectWalls++; 
	      break;
	    }
       }
     }
     
     if(G->field.bombs[i].xpos == P->xpos)
     {
       if(G->field.bombs[i].ypos < P->ypos)
       {
         for(j = G->field.bombs[i].ypos; j < P->ypos; j++)
	   if(G->field.map[P->xpos][j] == SQUARE_BLOCK
	    || G->field.map[P->xpos][j] == SQUARE_BOX)
	    {
	      ProtectWalls++; 
	      break;
	    }
       }
       else
       {
         for(j = P->ypos; j < G->field.bombs[i].ypos; j++)
	   if(G->field.map[P->xpos][j] == SQUARE_BLOCK
	    || G->field.map[P->xpos][j] == SQUARE_BOX)
	    {
	      ProtectWalls++; 
	      break;
	    }
       }
     }
     
     if(ProtectWalls >= 1)
       continue;
     return 0;
    }
  }
  return 1;
}*/


int FoundDangerBombs(Game *G, int x, int y)
{
  int i, j;
  int ProtectWalls = 0;
  int Checked = 0, BombPlaced = 0;
  
  if(x < 0 || y < 0 
    || x >= MAP_W 
    || y >= MAP_H)
    return -1;
  
  for(i = 0; i < MAX_BOMBS; i++)
  {
    ProtectWalls = 0;
    Checked = 0;
    BombPlaced = 0;
    if(G->field.bombs[i].state >= BOMB_PLACED)
    {
      BombPlaced = 1;
      if(G->field.bombs[i].ypos == y)
      {
       Checked = 1;
       if(_abs(G->field.bombs[i].xpos - x) > BOMB_PWR_KILLER + 1)
         ProtectWalls++;
       else if(G->field.bombs[i].xpos < x)
       {
	 
         for(j = G->field.bombs[i].xpos; j < x; j++)
	   if(G->field.map[j][y] == SQUARE_BLOCK
	    || G->field.map[j][y] == SQUARE_BOX)
	    {
	      ProtectWalls++; 
	      break;
	    }
       }
       else if(G->field.bombs[i].xpos > x)
       {
	 
         for(j = x; j < G->field.bombs[i].xpos; j++)
	   if(G->field.map[j][y] == SQUARE_BLOCK
	    || G->field.map[j][y] == SQUARE_BOX)
	    {
	      ProtectWalls++; 
	      break;
	    }
       }
     }
     
     if(G->field.bombs[i].xpos == x)
     {
       Checked = 1;
       if(_abs(G->field.bombs[i].ypos - y) > BOMB_PWR_KILLER + 1)
         ProtectWalls++;
       else if(G->field.bombs[i].ypos < y)
       {
	
         for(j = G->field.bombs[i].ypos; j < y; j++)
	   if(G->field.map[x][j] == SQUARE_BLOCK
	    || G->field.map[x][j] == SQUARE_BOX)
	    {
	      ProtectWalls++; 
	      break;
	    }
       }
       else if(G->field.bombs[i].ypos > y)
       {
	 
         for(j = y; j < G->field.bombs[i].ypos; j++)
	   if(G->field.map[x][j] == SQUARE_BLOCK
	    || G->field.map[x][j] == SQUARE_BOX)
	    {
	      ProtectWalls++; 
	      break;
	    }
       }
     }
     
     if(ProtectWalls >= 1
       || (!Checked && BombPlaced))
       continue;

     return 1;
    }
  }
  
  return 0;
}

int IsPlayerNearPlayer(Player *P1, Player *P2)
{
  if(P1->xpos == P2->xpos)
  {
    if(_abs(P1->ypos - P2->ypos) <= BOMB_PWR_SINGLE + 1)
     return 1;
  }
  else if(P1->ypos == P2->ypos)
  {
    if(_abs(P1->xpos - P2->xpos) <= BOMB_PWR_SINGLE + 1)
     return 1;
  }
  
  return 0;
}

int FindSafetyPlace(Game *G, AiVars *P,  PATH *Path, Player *Pl)
{
  int dx, dy;
  int MinDist = UNREAL_F;
  PATHELEM Tmp;
  int x = Pl->xpos, y = Pl->ypos;
  
  Tmp.X = 0;
  Tmp.Y = 0;
  for(dx = -MAP_W; dx <= MAP_W; dx++)
    for(dy = -MAP_H; dy <= MAP_H; dy++)
    {
      if ((x + dx) < MAP_W && (x + dx) > -1 
	  && (y + dy) < MAP_H && (y + dy) > -1)
	  {
	    if(G->field.map[x + dx][y + dy] == SQUARE_FREE)
	    {
	      if(FoundDangerBombs(G, x + dx, y + dy) == 0)
	        if(FindPath(G, Path, x, y, x + dx, y + dy, false))
		{
		  if(Path->Distance < MinDist)
		  {
		    MinDist = Path->Distance;
		    Tmp.X = x + dx;
		    Tmp.Y = y + dy;
		  }
		}
	    }
	  }
    }
  if(MinDist < UNREAL_F)
  {
    P->SafetyPlace.X = Tmp.X;
    P->SafetyPlace.Y = Tmp.Y;
    return 1;
  }
  return 0;
}

inline static int IsABox(Game *G, PATHELEM *P)
{
  return (G->field.map[P->X][P->Y] == SQUARE_BOX);
}

void MovePlayer(Game *G, Player *P, PATH *Path)
{
    if (Path->Distance > 1)
    {
	if (P->xpos < Path->Path[PATH_OFFSET].X)
	    PlayerMoveRight(G, P);
	else if (P->xpos > Path->Path[PATH_OFFSET].X)
	    PlayerMoveLeft(G, P);
	else if (P->ypos < Path->Path[PATH_OFFSET].Y)
	    PlayerMoveDown(G, P);
	else if (P->ypos > Path->Path[PATH_OFFSET].Y) 
	    PlayerMoveUp(G, P);  
   }
}

inline static int CheckFire(Game *G, int x, int y)
{
  return !(G->field.firemap[x][y].state == BOMB_NONE);
}

void UpdateAI(Game *G, Player *Players)
{
  int i, j, Danger = 0;
  PATH Path, CurPath;
  int MinDist = UNREAL_F;
  
  
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    
    MinDist = UNREAL_F;
    
    if (Players[i].isAI == true &&
        Players[i].ismove == false &&
        Players[i].status.state == ALIVE)
    {
	if(FoundDangerBombs(G, Players[i].xpos, Players[i].ypos) == 1)
	{
	  Danger = 1;
	}
	if(Danger == 0)
	{
	  AI[i].Danger = false;
	  for (j = 0; j < MAX_PLAYERS; j++)
	   {
		  if (j == i || Players[j].status.state > ALIVE)
		    continue;
	      
		  FindPath(G, &Path, Players[i].xpos, Players[i].ypos, 
			  Players[j].xpos, Players[j].ypos, true);

		  if (Path.Distance < MinDist)
		  {
		    MinDist = Path.Distance;
		    CopyPaths(&CurPath, &Path);
		    AI[i].ClosestPlayer = j;
		  }
	  }
	}
      if(Danger)
      {
        if(FindSafetyPlace(G, &AI[i], &Path, &Players[i]))
	{
	  AI[i].Danger = true;
	}
      }
      if(AI[i].Danger)
      {
	if(FoundDangerBombs(G, Players[i].xpos, Players[i].ypos) == 1)
	  FindSafetyPlace(G, &AI[i], &Path, &Players[i]);
        if(FindPath(G, &Path, Players[i].xpos, Players[i].ypos,
		    AI[i].SafetyPlace.X, AI[i].SafetyPlace.Y, false))
	   {
	     CopyPaths(&CurPath, &Path);
	     
	     //if(!CheckFire(G, CurPath.Path[PATH_OFFSET].X, CurPath.Path[PATH_OFFSET].Y))
	      MovePlayer(G, &Players[i], &CurPath); 
	   }
      }
      else if(IsPlayerNearPlayer(&Players[i], &Players[AI[i].ClosestPlayer])
              && AI[i].Danger == false)
      {
        PlayerPlaceBomb(G, &Players[i]);
      }
      else
        if(!CheckFire(G, CurPath.Path[PATH_OFFSET].X, CurPath.Path[PATH_OFFSET].Y)
	  && !Players[i].bombs_placed)
	{
	  if(IsABox(G, &CurPath.Path[PATH_OFFSET]) && AI[i].Danger == false)
	  {
	    PlayerPlaceBomb(G, &Players[i]);
	  }
	  else
            MovePlayer(G, &Players[i], &CurPath);
	}
    }
			  
			
  }
}
