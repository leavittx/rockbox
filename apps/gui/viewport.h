
/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2008 by Jonathan Gordon
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

#include "config.h"
#include "lcd.h"
#include "font.h"
#include "kernel.h"
#include "system.h"
#include "misc.h"
#include "screen_access.h"

/* return the number of text lines in the vp viewport */
int viewport_get_nb_lines(struct viewport *vp);

#define VP_ERROR              0
#define VP_DIMENSIONS       0x1
#define VP_COLORS           0x2
#define VP_SELECTIONCOLORS  0x4
/* load a viewport struct from a config string.
   returns a combination of the above to say which were loaded ok from the string */
int viewport_load_config(const char *config, struct viewport *vp);

void viewport_set_defaults(struct viewport *vp, enum screen_type screen);

/* viewportmanager_set_statusbar() is used to specify which screens the statusbar
 * should be displayed on.
 * *usually* enabled will be VP_ALLSCREENS which means display the bar if the setting
 * is enabled. (and it will be on both screens)
 * For the WPS (and other possible exceptions) use VP_IGNORE_SB_SETTING() to
 * FORCE the statusbar on for the given screen (i.e it will show regardless of the setting
 */
#define VP_SB_ONSCREEN(screen) (1<<screen) /* turn the SB on "screen" only */
#define VP_ALLSCREENS (VP_SB_ONSCREEN(0)|VP_SB_ONSCREEN(1)) /* SB on for both screens */
#define VP_IGNORE_SB_SETTING(screen) (1<<(4+screen))
char viewportmanager_set_statusbar(char enabled);

/* callbacks for GUI_EVENT_* events */
void viewportmanager_draw_statusbars(void*data);
void viewportmanager_statusbar_changed(void* data);

