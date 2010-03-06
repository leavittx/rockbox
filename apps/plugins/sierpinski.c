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

/* For math */
#include "lib/pdbox-lib.h"

/* This macros must always be included. Should be placed at the top by
   convention, although the actual position doesn't matter */
PLUGIN_HEADER

#define SLEEP_TIME 3
//#define MAX_ITER 9
#define MAX_ITER 20
#define SQRT3BY2 0.86602540378444

#define DOT(a, b)	((a).x * (b).x + (a).y * (b).y)
#define NORMALIZE(a)  do {\
	double len = rb_sqrt(DOT(a, a));\
	(a).x /= len; (a).y /= len;\
    } while(0);

/* By this we will determine, if the current iteration is first or not.
 * Needed for optimization and avoiding draw collisions */
int StartIter;

struct vec2 {
	double x, y;
};

const struct button_mapping *plugin_contexts[]
= {generic_directions, generic_actions,
#if defined(HAVE_REMOTE_LCD)
    remote_directions
#endif
};
#define NB_ACTION_CONTEXTS \
    sizeof(plugin_contexts)/sizeof(struct button_mapping*)

#ifdef HAVE_LCD_COLOR
struct Color
{
	uint32_t r,g,b;
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

/* Draw an iteration of triangle effect */
void DrawIterT(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct screen *display)
{
    struct Color color;
    
    if (Iter < 0)
        return;

    /* Set color to red.
     * TODO: iteration-depending color handling */
    color.r = 255;
    color.g = color.b = 0;
    color_apply(&color, display);
        
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
  
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len;

    /* We need to normalize normal vector. */
    NORMALIZE(n);
    
    len = rb_sqrt(s.x * s.x + s.y * s.y) / 2;
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    DrawIterT(Iter - 1, p1, m, dir * (-1), display);
    DrawIterT(Iter - 1, m, p2, dir * (-1), display);
}

/* Draw an iteration of Lévy C curve */
void DrawIterLC(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct screen *display)
{
    struct Color color;
    
    if (Iter < 0)
        return;

    /* Set color to red.
     * TODO: iteration-depending color handling */
    color.r = 255;
    color.g = color.b = 0;
    color_apply(&color, display);
        
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
  
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * (-dir), (p1.x - p2.x) * (-dir)};
    double len;

    /* We need to normalize normal vector. */
    NORMALIZE(n);
    
    len = rb_sqrt(s.x * s.x + s.y * s.y) / 2;
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    DrawIterLC(Iter - 1, p1, m, dir, display);
    DrawIterLC(Iter - 1, m, p2, dir, display);
}

/* Draw an iteration of dragon curve */
void DrawIterDC(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct screen *display)
{
    struct Color color;
    
    if (Iter < 0)
        return;

    /* Set color to red.
     * TODO: iteration-depending color handling */
    color.r = 255;
    color.g = color.b = 0;
    color_apply(&color, display);
        
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
  
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len;

    /* We need to normalize normal vector. */
    NORMALIZE(n);
    
    len = rb_sqrt(s.x * s.x + s.y * s.y) / 2;
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    DrawIterDC(Iter - 1, p1, m, dir, display);
    DrawIterDC(Iter - 1, m, p2, dir * (-1), display);
}

/* Draw an iteration of blood effect */
void DrawIterB(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct screen *display)
{
    struct Color color;
    
    if (Iter < 0)
        return;

    /* Set color to red.
     * TODO: iteration-depending color handling */
    color.r = 255;
    color.g = color.b = 0;
    color_apply(&color, display);
        
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
  
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len = rb_sqrt(s.x * s.x + s.y * s.y) * SQRT3BY2;
    
    /* We need to normalize normal vector. */
    NORMALIZE(n);
    /*
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    */
    s.x = (p1.x + p2.x) / 3;
    s.y = (p1.y + p2.y) / 3;
    DrawIterB(Iter - 1, p1, s, dir, display);
    
    len = rb_sqrt((s.x * s.x + s.y * s.y) / 3) * SQRT3BY2;
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    DrawIterB(Iter - 1, s, m, dir, display);
    
    s.x = (p1.x + p2.x) * 2 / 3;
    s.y = (p1.y + p2.y) * 2 / 3;
    DrawIterB(Iter - 1, m, s, dir, display);
    
    DrawIterB(Iter - 1, s, p2, dir, display);
}

/* Draw an iteration of blood effect, CurveType 2 */
void DrawIterB2(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct screen *display)
{
    struct Color color;
    
    if (Iter < 0)
        return;

    /* Set color to red.
     * TODO: iteration-depending color handling */
    color.r = 255;
    color.g = color.b = 0;
    color_apply(&color, display);
        
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
  
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len = rb_sqrt(s.x * s.x + s.y * s.y) * SQRT3BY2;
    
    /* We need to normalize normal vector. */
    NORMALIZE(n);
    /*
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    */
    s.x = (p1.x + 0.5*p2.x) / 1.5;
    s.y = (p1.y + 0.5*p2.y) / 1.5;
    DrawIterB2(Iter - 1, p1, s, dir, display);
    
    len = rb_sqrt((s.x * s.x + s.y * s.y) / 3) * SQRT3BY2;
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    DrawIterB2(Iter - 1, s, m, dir, display);
    
    s.x = (p1.x + p2.x) * 2 / 3;
    s.y = (p1.y + p2.y) * 2 / 3;
    DrawIterB2(Iter - 1, m, s, dir, display);
    
    DrawIterB2(Iter - 1, s, p2, dir, display);
}

/* Draw an iteration of Koch curve */
void DrawIterKC(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct screen *display)
{
    struct Color color;
    
    if (Iter < 0)
        return;

    /* Set color to red.
     * TODO: iteration-depending color handling */
    color.r = 255;
    color.g = color.b = 0;
    color_apply(&color, display);
        
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
  
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 s1;
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len;

    s1.x = (p1.x + 0.5*p2.x) / 1.5;
    s1.y = (p1.y + 0.5*p2.y) / 1.5;
    DrawIterKC(Iter - 1, p1, s1, dir, display);
    
    /* We need to normalize normal vector. */
    NORMALIZE(n);
    
    len = rb_sqrt(s.x * s.x + s.y * s.y) / 3 * SQRT3BY2;
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    DrawIterKC(Iter - 1, s1, m, dir, display);
    
    s.x = (p1.x + 2*p2.x) / 3;
    s.y = (p1.y + 2*p2.y) / 3;
    DrawIterKC(Iter - 1, m, s, dir, display);
    DrawIterKC(Iter - 1, s, p2, dir, display);
}

/* Draw an iteration of flower, CurveType 2 */
void DrawIterFlower2(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct screen *display)
{
    struct Color color;
    
    if (Iter < 0)
        return;
    
    /* Set color to red.
     * TODO: iteration-depending color handling */
    color.r = 255;
    color.g = color.b = 0;
    color_apply(&color, display);
    
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len = rb_sqrt(s.x * s.x + s.y * s.y) * SQRT3BY2;
    
    /* We need to normalize normal vector. */
    NORMALIZE(n);
    
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
    
    s.x = (p1.x + m.x) / 2;
    s.y = (p1.y + m.y) / 2;
    if (Iter & 1)
        DrawIterFlower2(Iter - 1, p1, s, dir * (-1), display);
    else
        DrawIterFlower2(Iter - 1, p1, s, dir, display);
    
    n.x = (p2.x + m.x) / 2;
    n.y = (p2.y + m.y) / 2;
    if (Iter & 1)
    {
        DrawIterFlower2(Iter - 1, s, n, dir * (-1), display);
        DrawIterFlower2(Iter - 1, n, p2, dir * (-1), display);
    }
    else
    {
        DrawIterFlower2(Iter - 1, s, n, dir, display);
        DrawIterFlower2(Iter - 1, n, p2, dir, display);
    }
}

/* Draw an iteration of beautiful flower */
void DrawIterFlower(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct screen *display)
{
    struct Color color;
    
    if (Iter < 0)
        return;
    
    /* Set color to red.
     * TODO: iteration-depending color handling */
    color.r = 255;
    color.g = color.b = 0;
    color_apply(&color, display);
    
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len = rb_sqrt(s.x * s.x + s.y * s.y) * SQRT3BY2;
    
    /* We need to normalize normal vector. */
    NORMALIZE(n);
    
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
    
    s.x = (p1.x + m.x) / 2;
    s.y = (p1.y + m.y) / 2;
    DrawIterFlower(Iter - 1, p1, s, dir * (-1), display);
    
    n.x = (p2.x + m.x) / 2;
    n.y = (p2.y + m.y) / 2;
    DrawIterFlower(Iter - 1, s, n, dir * (-1), display);
    DrawIterFlower(Iter - 1, n, p2, dir * (-1), display);
}

/* Draw an iteration of Sierpinski triangle CurveType 1 */
void DrawIterST1(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct screen *display)
{
    struct Color color;
    
    if (Iter < 0)
        return;

    /* Set color to red.
     * TODO: iteration-depending color handling */
    color.r = 255;
    color.g = color.b = 0;
    color_apply(&color, display);
        
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
  
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len = rb_sqrt(s.x * s.x + s.y * s.y) * SQRT3BY2;
    
    /* We need to normalize normal vector. */
    NORMALIZE(n);
    
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    s.x = (p1.x + m.x) / 2;
    s.y = (p1.y + m.y) / 2;
    DrawIterST1(Iter - 1, p1, s, dir * (-1), display);
    
    n.x = (p2.x + m.x) / 2;
    n.y = (p2.y + m.y) / 2;
    DrawIterST1(Iter - 1, s, n, dir, display);
    DrawIterST1(Iter - 1, n, p2, dir * (-1), display);
}

/* Draw an iteration of Sierpinski triangle CurveType 2 */
void DrawIterST2(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct screen *display)
{
    struct Color color;
    
    if (Iter < 0)
        return;
    
    /* Draw two lines, just test.
     * Will be removed shortly.
    static int W, H;
    W = display->getwidth();
    H = display->getheight();
    
    color.r = color.g = color.b = 255;
    color_apply(&color, display);
    display->drawline(0, 0, W, H);

    color.r = 255;
    color.g = color.b = 0;
    color_apply(&color, display);
    display->drawline(0, H, W, 0);
    */

    /* Set color to red.
     * TODO: iteration-depending color handling */
    color.r = 255;
    color.g = color.b = 0;
    color_apply(&color, display);
    
    /* Thanks to np3 for this math stuff */
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len = rb_sqrt(s.x * s.x + s.y * s.y) * SQRT3BY2;
    
    /* We need to normalize normal vector.
     * Use macro instead of this code.
    double len = rb_sqrt(n.x * n.x + n.y * n.y);
    n.x /= len;
    n.y /= len; */
    NORMALIZE(n);
    
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    /* This old code leads to some draw collisions.
     * (Same lines drawed multiple times)
    display->drawline(p1.x, p1.y, p2.x, p2.y);
    display->drawline(p2.x, p2.y, m.x, m.y);
    display->drawline(m.x, m.y, p1.x, p1.y);
    */
    if (Iter == StartIter)
    {
        display->drawline(p1.x, p1.y, p2.x, p2.y);
        display->drawline(m.x, m.y, p1.x, p1.y);
    }
    display->drawline(p2.x, p2.y, m.x, m.y);
    
    s.x = (p1.x + m.x) / 2;
    s.y = (p1.y + m.y) / 2;
    DrawIterST2(Iter - 1, p1, s, dir * (-1), display);
    
    s.x = (p2.x + m.x) / 2;
    s.y = (p2.y + m.y) / 2;
    DrawIterST2(Iter - 1, p2, s, dir, display);
    
    s.x = (m.x + p2.x) / 2;
    s.y = (m.y + p2.y) / 2;
    DrawIterST2(Iter - 1, m, s, dir * (-1), display);
}

int plugin_main(void)
{
    int action;
	int i;
    int W, H;
    int Iter = 0;
    bool need_redraw = true;
    
    void(* DrawIter[])(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct screen *display) = {
        DrawIterST2, DrawIterST1,
        DrawIterFlower, DrawIterFlower2,
        DrawIterB, DrawIterB2, DrawIterT,
        DrawIterKC, DrawIterDC,
        DrawIterLC
        };
    int CurveType = 0;
	
    FOR_NB_SCREENS(i)
    {
        struct screen *display = rb->screens[i];
        
        W = display->getwidth();
        H = display->getheight();
        
        if (display->is_color)
            display->set_background(LCD_BLACK);
    }
    
    while (true)
    {
        FOR_NB_SCREENS(i)
        {
            struct screen *display = rb->screens[i];

            if (need_redraw)
            {
                struct vec2 p1, p2;
                
                if (DrawIter[CurveType] == DrawIterDC)
                {
                    p1.x = W - H + 10;
                    p1.y = H - 70;
                    
                    p2.x = W - 10;
                    p2.y = H - 70;
                }
                
                else if (DrawIter[CurveType] == DrawIterLC)
                {
                    p1.x = (W - H) + 5;
                    p1.y = 70;
                    
                    p2.x = H - 5;
                    p2.y = 70;
                }
                
                else
                {
                    p1.x = (W - H) / 2;
                    p1.y = H - 15;
    
                    p2.x = W - (W - H) / 2;
                    p2.y = H - 15;
                }
                
                rb->lcd_clear_display();
                
                StartIter = Iter;
                //DrawIterST2(Iter, p1, p2, 1, display);
                DrawIter[CurveType](Iter, p1, p2, 1, display);
                need_redraw = false;
                
                display->update();
            }

            rb->sleep(SLEEP_TIME);

            action = pluginlib_getaction(TIMEOUT_NOBLOCK,
                                         plugin_contexts,
                                         NB_ACTION_CONTEXTS);
            switch (action)
            {
                case PLA_QUIT:
                    cleanup(NULL);
                    return PLUGIN_OK;
                    
                case PLA_FIRE:
                    if (Iter <= MAX_ITER)
                    {
                        Iter++;
                        need_redraw = true;
                    }
                    break;
                    
                case PLA_START:
                    if (Iter > 0)
                    {
                        Iter--;
                        need_redraw = true;
                    }
                    break;
                    
                case PLA_MENU:
                    if ((unsigned int)CurveType < ARRAYLEN(DrawIter) - 1)
                        CurveType++;
                    else
                        CurveType = 0;
                    
                    need_redraw = true;
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
