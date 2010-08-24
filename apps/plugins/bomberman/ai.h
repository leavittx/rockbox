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


#ifndef _AI_H_
#define _AI_H_

#include "game.h"

#define UNREAL_F 10000


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
  //int F;
} PATHELEM;

typedef struct
{
  PATHELEM Path[MAP_W * MAP_H];
  int Distance;
} PATH;

#endif /* _AI_H_ */
