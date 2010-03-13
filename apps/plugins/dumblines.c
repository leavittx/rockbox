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
 * Random color lines demo plugin
 * Code is really ugly :P
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

/* Key assignement */
#define MY_QUIT PLA_QUIT

const struct button_mapping *plugin_contexts[]
= {generic_directions, generic_actions,
#if defined(HAVE_REMOTE_LCD)
    remote_directions
#endif
};
#define NB_ACTION_CONTEXTS \
    sizeof(plugin_contexts)/sizeof(struct button_mapping*)

#ifdef HAVE_LCD_COLOR
struct line_color
{
    int r,g,b;
    int current_r,current_g,current_b;
};
#endif

void cleanup(void *parameter)
{
    (void)parameter;

    backlight_use_settings();
#ifdef HAVE_REMOTE_LCD
    remote_backlight_use_settings();
#endif
}

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
    int action;
    int sleep_time = DEFAULT_WAIT_TIME;
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
    
    while (true)
    {
        FOR_NB_SCREENS(i)
        {
            struct screen *display = rb->screens[i];
            
            for (i = 0; i < 1000; i ++)
            {
                display->drawline(rb->rand() % (display->getwidth()),
                                  rb->rand() % (display->getheight()),
                                  rb->rand() % (display->getwidth()),
                                  rb->rand() % (display->getheight()));
                
#ifdef HAVE_LCD_COLOR
                color_apply(&color, display);
#endif

                display->update();
/*                
#ifdef HAVE_LCD_COLOR
                color_change(&color);
#endif
*/
/* Random colors */
#ifdef HAVE_LCD_COLOR
                color_init(&color);
#endif
/* Some delay */
                rb->sleep(DEFAULT_WAIT_TIME * 5); /* ? */
                
                /* Speed handling*/
                if (sleep_time < 0) /* full speed */
                    rb->yield();
                else
                    rb->sleep(sleep_time);
                action = pluginlib_getaction(TIMEOUT_NOBLOCK,
                                             plugin_contexts,
                                             NB_ACTION_CONTEXTS);
                switch (action)
                {
                    case MY_QUIT:
                        cleanup(NULL);
                        return PLUGIN_OK;
                    default:
                        if (rb->default_event_handler_ex(action, cleanup, NULL)
                            == SYS_USB_CONNECTED)
                            return PLUGIN_USB_CONNECTED;
                        break;
                }
            }
            
            rb->sleep(DEFAULT_WAIT_TIME * 300);
            return PLUGIN_OK;
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
