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
 * GEF demo plugin
 * Code is ugly :P
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
#include "lib/fixedpoint.h"

#ifdef HAVE_LCD_BITMAP
#include "lib/pluginlib_actions.h"
#include "lib/helper.h"

/* This macros must always be included. Should be placed at the top by
   convention, although the actual position doesn't matter */
PLUGIN_HEADER

#define DEFAULT_WAIT_TIME 3

#define pi 180
enum { MAX = 4 };
double b = pi / 3, c = pi / 9, d = pi / 3;
double k1 = 0.5, k2 = 0.5, k3 = 0.9;
int rec_lvl = 0;
bool need_redraw = true;

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

void RecursiveDrawIter( int x, int y, double l, double a, struct screen *display )
{
/*
  double x1 = x * fp14_cos(a) - y * fp14_sin(a), y1 = x * fp14_sin(a) + y * fp14_cos(a);
*/
  double x1, y1;
  //static int rec_lvl = 0;

  if (rec_lvl > MAX || l < 3/* || inp(0x60) == 1*/)
    return;
  rec_lvl ++;

  x1 = x + l * fp14_cos(a) / 16384;
  y1 = y - l * fp14_sin(a) / 16384;

  display->drawline(x, y, (int)x1, (int)y1);
  display->update();
              //TGR_RGB(rec_lvl * 10 - 100, rec_lvl * 10 - 20, rec_lvl * 20)
              /* TGR_RGB(134, 22, 44) */ 

  RecursiveDrawIter(x1, y1, l * k3, pi / 2, display);
  RecursiveDrawIter(x1, y1, l * k1, a - b, display);
  RecursiveDrawIter(x1, y1, l * k1, a + b, display);
  RecursiveDrawIter(x1, y1, l * k2, a - c, display);
  RecursiveDrawIter(x1, y1, l * k2, a + c, display);
//  RecursiveDrawIter(x1, y1, l * k3, a - d);

  rec_lvl --;
}

/*
 * Main function
 */

int plugin_main(void)
{
    int action;
    int sleep_time = DEFAULT_WAIT_TIME;
	int i, W, H;
	//static bool need_redraw = true;
	
    FOR_NB_SCREENS(i)
    {
#ifdef HAVE_LCD_COLOR
        struct screen *display = rb->screens[i];
        if (display->is_color)
            display->set_background(LCD_BLACK);
            
        W = display->getwidth();
	H = display->getheight();
#endif
    }
    
#ifdef HAVE_LCD_COLOR
    struct line_color color;
    color_init(&color);
#endif

	FOR_NB_SCREENS(i)
        {
            struct screen *display = rb->screens[i];
            
			{
#ifdef HAVE_LCD_COLOR
            color_apply(&color, display);
#endif
				
				rb->lcd_clear_display();
				rb->lcd_update();
				
				RecursiveDrawIter(W / 2, H, (W + H) / 12, pi / 2, display);
				
				display->update();
				
				need_redraw = false;
			}
        }
	
    while (true)
    {
		/*if (need_redraw == true)
        FOR_NB_SCREENS(i)
        {
            struct screen *display = rb->screens[i];
            
			{
#ifdef HAVE_LCD_COLOR
            color_apply(&color, display);
#endif
				
				rb->lcd_clear_display();
				rb->lcd_update();
				
				RecursiveDrawIter(W / 2, H, (W + H) / 15, pi / 2, display);
				
				display->update();
				
				need_redraw = false;
			}
        }*/
        
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
