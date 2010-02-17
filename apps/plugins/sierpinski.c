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
 * Sierpinsky triangle demo plugin
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

#define DEFAULT_WAIT_TIME 3

#ifdef HAVE_LCD_COLOR
struct line_color
{
    int r,g,b;
    int current_r,current_g,current_b;
};
#endif

#ifdef HAVE_LCD_COLOR
void color_randomize(struct line_color * color)
{
    color->r = rb->rand()%255;
    color->g = rb->rand()%255;
    color->b = rb->rand()%255;
}

void color_init(struct line_color * color)
{
    color_randomize(color);
    color->current_r=color->r;
    color->current_g=color->g;
    color->current_b=color->b;
}

void color_change(struct line_color * color)
{
    if(color->current_r<color->r)
        ++color->current_r;
    else if(color->current_r>color->r)
        --color->current_r;
    if(color->current_g<color->g)
        ++color->current_g;
    else if(color->current_g>color->g)
        --color->current_g;
    if(color->current_b<color->b)
        ++color->current_b;
    else if(color->current_b>color->b)
        --color->current_b;

    if(color->current_r==color->r &&
       color->current_g==color->g &&
       color->current_b==color->b)
        color_randomize(color);
}

#define COLOR_RGBPACK(color) \
    LCD_RGBPACK((color)->current_r, (color)->current_g, (color)->current_b)

void color_apply(struct line_color * color, struct screen * display)
{
    if (display->is_color){
        unsigned foreground=
            SCREEN_COLOR_TO_NATIVE(display,COLOR_RGBPACK(color));
        display->set_foreground(foreground);
    }
}
#endif /* #ifdef HAVE_LCD_COLOR */

/*
 * Main function
 */

int plugin_main(void)
{
	int i;
	
	FOR_NB_SCREENS(i)
    {
#ifdef HAVE_LCD_COLOR
        struct screen *display = rb->screens[i];
        if (display->is_color)
            display->set_background(LCD_BLACK);
#endif
    }
    
#ifdef HAVE_LCD_COLOR
    struct line_color color;
    color_init(&color);
#endif
    
    FOR_NB_SCREENS(i)
    {
		struct screen * display=rb->screens[i];
		
#ifdef HAVE_LCD_COLOR
        color_apply(&color, display);
#endif
        
        for (i = 0; i < 1000; i ++)
        {
            display->drawline (rb->rand() % (display->getwidth()),
                               rb->rand() % (display->getheight()),
                               rb->rand() % (display->getwidth()),
                               rb->rand() % (display->getheight()));
                               
            display->update();
            
#ifdef HAVE_LCD_COLOR
            color_change(&color);
#endif

            rb->sleep(DEFAULT_WAIT_TIME * 10);
        }
        
		//display->drawline(10, 10, 70, 70);
		rb->sleep(DEFAULT_WAIT_TIME * 300);
    }
    
    return PLUGIN_OK;
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

    /* now go ahead and have fun! */
    //rb->splash(HZ*2, "Hello world!");
    
    ret = plugin_main();

    return ret;
}

#endif /* #ifdef HAVE_LCD_BITMAP */
