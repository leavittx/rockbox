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
 * Drawnote plugin
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
#include "lib/pluginlib_touchscreen.c"
#include "lib/helper.h"

/* This macros must always be included. Should be placed at the top by
   convention, although the actual position doesn't matter */
PLUGIN_HEADER

#define SLEEP_TIME 3

struct Color
{
	uint32_t r,g,b;
};

#define RED     {255,   0,   0}
#define GREEN   {  0, 255,   0}
#define BLUE    { 93, 120, 208}
#define PINK    {234,  91, 219}
#define ORANGE  {231,  58,  34}
#define YELLOW  {209, 237,  18}
#define CYAN    { 39, 236, 215}

const struct button_mapping *plugin_contexts[] = { 
	generic_directions,
	generic_actions,
#if defined(HAVE_REMOTE_LCD)
    remote_directions
#endif
};

#define NB_ACTION_CONTEXTS \
    sizeof(plugin_contexts) / sizeof(struct button_mapping*)

void cleanup(void * parameter)
{
	(void)parameter;

	backlight_use_settings();
#ifdef HAVE_REMOTE_LCD
	remote_backlight_use_settings();
#endif
}

#define COLOR_RGBPACK(color) \
	LCD_RGBPACK((color)->r, (color)->g, (color)->b)

void color_apply(struct Color * color)
{
	unsigned foreground = COLOR_RGBPACK(color);
	rb->lcd_set_foreground(foreground);
}

int plugin_main(void)
{
    int action, button; /* Key/touchscreen actions */
    short x = LCD_WIDTH / 2, y = LCD_HEIGHT / 2, oldx = x, oldy = y;
    struct Color Red = {255, 0, 0};

	//rb->splashf(HZ*3, "%i %i", LCD_WIDTH, LCD_HEIGHT);
	
    //rb->lcd_set_background(LCD_BLACK);
    rb->lcd_clear_display();
    rb->lcd_update();

    color_apply(&Red);

    rb->touchscreen_set_mode(TOUCHSCREEN_POINT);

	/* Main loop */
    while (true)
    {
        action = rb->get_action(CONTEXT_STD, HZ);
        
        if (action == BUTTON_POWER)
        {
            cleanup(NULL);
            return PLUGIN_OK;
        }
        else if (action == ACTION_TOUCHSCREEN)
        {
            button = rb->action_get_touchscreen_press(&x, &y);
            
            if (button == BUTTON_TOUCHSCREEN)
            {
                //rb->lcd_drawpixel(x, y);
                //rb->lcd_update();
                
                oldx = x;
                oldy = y;
            }
            else if (button == BUTTON_REPEAT)
            {
                rb->lcd_drawline(oldx, oldy, x, y);
                rb->lcd_update();
                
                oldx = x;
                oldy = y;
            }
        }
				
		rb->sleep(SLEEP_TIME);
		//rb->yield();
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
