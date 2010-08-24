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
 * Circle demo plugin
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

#define DEFAULT_WAIT_TIME 3

/* Key assignement */
#define MY_QUIT PLA_EXIT
#define MY_LESS_R PLA_DOWN
#define MY_MORE_R PLA_UP

const struct button_mapping *plugin_contexts[] = {
	pla_main_ctx,
#if defined(HAVE_REMOTE_LCD)
    pla_remote_ctx,
#endif
};

#define NB_ACTION_CONTEXTS \
    (sizeof(plugin_contexts) / sizeof(struct button_mapping*))

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

void circle_points( int cx, int cy, int x, int y, struct screen *display )
{
	if (x == 0) {
		display->drawpixel(cx, cy + y);
		display->drawpixel(cx, cy - y);
		display->drawpixel(cx + y, cy);
		display->drawpixel(cx - y, cy);
	} else 
	if (x == y) {
		display->drawpixel(cx + x, cy + y);
		display->drawpixel(cx - x, cy + y);
		display->drawpixel(cx + x, cy - y);
		display->drawpixel(cx - x, cy - y);
	} else 
	if (x < y) {
		display->drawpixel(cx + x, cy + y);
		display->drawpixel(cx - x, cy + y);
		display->drawpixel(cx + x, cy - y);
		display->drawpixel(cx - x, cy - y);
		display->drawpixel(cx + y, cy + x);
		display->drawpixel(cx - y, cy + x);
		display->drawpixel(cx + y, cy - x);
		display->drawpixel(cx - y, cy - x);
	}
}

void draw_circle( int xCenter, int yCenter, int radius, struct screen *display )
{
	int x = 0;
	int y = radius;
	int p = (5 - radius*4)/4;

	circle_points(xCenter, yCenter, x, y, display);
	while (x < y) {
		x++;
		if (p < 0) {
			p += 2*x+1;
		} else {
			y--;
			p += 2*(x-y)+1;
		}
		circle_points(xCenter, yCenter, x, y, display);
	}
}

/*
 * Main function
 */

int plugin_main(void)
{
    int action;
    int sleep_time = DEFAULT_WAIT_TIME;
	int i, W, H;
	int R, Xc, Yc;
	
    FOR_NB_SCREENS(i)
    {
#ifdef HAVE_LCD_COLOR
        struct screen *display = rb->screens[i];
        if (display->is_color)
            display->set_background(LCD_BLACK);
            
        W = display->getwidth();
		H = display->getheight();
		
		Xc = W / 2;
		Yc = H / 2;
		R  = H / 3;
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
            
#ifdef HAVE_LCD_COLOR
            color_apply(&color, display);
#endif
            
            draw_circle(Xc, Yc, R, display);
            
            display->update();
               
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
					
				case MY_MORE_R:
					if (R < H / 2)
						R ++;
					break;
					
				case MY_LESS_R:
					if (R > 0)
						R --;
					break;
					
				default:
					if (rb->default_event_handler_ex(action, cleanup, NULL)
						== SYS_USB_CONNECTED)
						return PLUGIN_USB_CONNECTED;
					break;
			}
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
