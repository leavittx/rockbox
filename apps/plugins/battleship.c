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
 * Battleship game plugin
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

/* Bitmaps */
#include "pluginbitmaps/battleship_huy.h"

/* For math */
#include "lib/pdbox-lib.h"

/* This macros must always be included. Should be placed at the top by
   convention, although the actual position doesn't matter */
PLUGIN_HEADER

#define SLEEP_TIME 0

#ifdef HAVE_LCD_COLOR
struct Color
{
	uint32_t r,g,b;
};
#endif

#define RED     {255,   0,   0}
#define GREEN   {  0, 255,   0}
#define BLUE    { 93, 120, 208}
#define PINK    {234,  91, 219}
#define ORANGE  {231,  58,  34}
#define YELLOW  {209, 237,  18}
#define CYAN    { 39, 236, 215}

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

#ifdef HAVE_LCD_COLOR
#define COLOR_RGBPACK(color) \
	LCD_RGBPACK((color)->r, (color)->g, (color)->b)

void color_apply(struct Color * color, struct screen * display)
{
	if (display->is_color){
		unsigned foreground=
			SCREEN_COLOR_TO_NATIVE(display,COLOR_RGBPACK(color));
		display->set_foreground(foreground);
	}
}
#endif

/* Code */

int plugin_main(void)
{
    int action;
	int i;
	bool need_redraw = true;
	int x, y;
	int dirx = 1, diry = -1;
	
    FOR_NB_SCREENS(i)
    {
        struct screen *display = rb->screens[i];

        if (display->is_color)
            display->set_background(LCD_BLACK);
    }
    
    x = rb->rand() % (LCD_WIDTH - BMPWIDTH_battleship_huy);
    y = rb->rand() % (LCD_HEIGHT - BMPHEIGHT_battleship_huy);

    while (true)
    {
		if (need_redraw)
		{
			FOR_NB_SCREENS(i)
			{
				struct screen *display = rb->screens[i];
				
				rb->lcd_clear_display();
				
				display->bitmap(battleship_huy, x, y, BMPWIDTH_battleship_huy, BMPHEIGHT_battleship_huy);
				
				display->update();
			}
			//need_redraw = false;
		}
		
		if (x + BMPWIDTH_battleship_huy == LCD_WIDTH || x == 0)
			dirx *= -1;
		if (y + BMPHEIGHT_battleship_huy == LCD_HEIGHT || y == 0)
			diry *= -1;
		
		x += dirx;
		y += diry;
		
		//rb->sleep(SLEEP_TIME);
		rb->yield();

		action = pluginlib_getaction(TIMEOUT_NOBLOCK,
									 plugin_contexts,
									 NB_ACTION_CONTEXTS);
		switch (action)
		{
			case PLA_QUIT:
				cleanup(NULL);
				return PLUGIN_OK;
				
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
