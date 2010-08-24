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

#define SLEEP_TIME 3
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

#ifdef HAVE_LCD_COLOR
struct Color
{
	uint32_t r,g,b;
};
#endif

typedef struct
{
    bool IsShow;
    char Name[25];
    int MaxIter;
    int CurIter;
    int StartDir;
    struct vec2 p1, p2;
    struct Color color;
    void(* DrawIter)(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display);
} Curve;

void DrawIterTrian(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display);      /* triangle effect */
void DrawIterLC(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display);         /* Lévy C curve */
void DrawIterDC(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display);         /* dragon curve */
void DrawIterB1(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display);         /* blood effect, type 1 */
void DrawIterB2(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display);         /* blood effect, type 2 */
void DrawIterKC(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display);         /* Koch curve */
void DrawIterFlower2(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display);    /* beautiful flower, type2 */
void DrawIterFlower1(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display);    /* beautiful flower, type1 */
void DrawIterST1(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display);        /* Sierpinski triangle, type 1 */
void DrawIterST2(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display);        /* Sierpinski triangle, type 2 */

/* Default start and points */
#define DEFAULT_P1  {(LCD_WIDTH - LCD_HEIGHT) / 2, LCD_HEIGHT - LCD_HEIGHT / 16}
#define DEFAULT_P2  {LCD_WIDTH - (LCD_WIDTH - LCD_HEIGHT) / 2, LCD_HEIGHT - LCD_HEIGHT / 16}
#define LEVI_P1     {(LCD_WIDTH - LCD_HEIGHT) + 5, LCD_HEIGHT / 3.4}
#define LEVI_P2     {LCD_HEIGHT - 5, LCD_HEIGHT / 3.4}
#define DRAGON_P1   {LCD_WIDTH - LCD_HEIGHT + 10, LCD_HEIGHT - LCD_HEIGHT / 3.4}
#define DRAGON_P2   {LCD_WIDTH - 10, LCD_HEIGHT - LCD_HEIGHT / 3.4}
#define FLOWER2_P1  {(LCD_WIDTH - LCD_HEIGHT) / 2, LCD_HEIGHT * 3 / 4}
#define FLOWER2_P2  {LCD_WIDTH - (LCD_WIDTH - LCD_HEIGHT) / 2, LCD_HEIGHT * 3 / 4}
/*
#define DEFAULT_P1 {(LCD_WIDTH - LCD_HEIGHT) / 2, LCD_HEIGHT - 15}
#define DEFAULT_P2 {LCD_WIDTH - (LCD_WIDTH - LCD_HEIGHT) / 2, LCD_HEIGHT - 15}
#define LEVI_P1 {(LCD_WIDTH - LCD_HEIGHT) + 5, 70}
#define LEVI_P2 {LCD_HEIGHT - 5, 70}
#define DRAGON_P1 {LCD_WIDTH - LCD_HEIGHT + 10, LCD_HEIGHT - 70}
#define DRAGON_P2 {LCD_WIDTH - 10, LCD_HEIGHT - 70}
*/

/* TODO: different colors */
#define RED     {255,   0,   0}
#define GREEN   {  0, 255,   0}
#define BLUE    { 93, 120, 208}
#define PINK    {234,  91, 219}
#define ORANGE  {231,  58,  34}
#define YELLOW  {209, 237,  18}
#define CYAN    { 39, 236, 215}

Curve Curves[] = {
    {true,  "Sierpinski triangle 2",  8, 0,  1, DEFAULT_P1, DEFAULT_P2, RED,    DrawIterST2},
    {true,  "Sierpinski triangle 1",  8, 0,  1, DEFAULT_P1, DEFAULT_P2, GREEN,  DrawIterST1},
    {false, "beautiful flower 1",     7, 0,  1, DEFAULT_P1, DEFAULT_P2, BLUE,   DrawIterFlower1},
    {true,  "beautiful flower 2",     8, 0,  1, FLOWER2_P1, FLOWER2_P2, PINK,   DrawIterFlower2},
    {true,  "Koch curve",             5, 0,  1, DEFAULT_P1, DEFAULT_P2, ORANGE, DrawIterKC},
    {false, "blood effect 1",         6, 0,  1, DEFAULT_P1, DEFAULT_P2, RED,    DrawIterB1},
    {false, "blood effect 2",         6, 0,  1, DEFAULT_P1, DEFAULT_P2, RED,    DrawIterB2},
    {true,  "dragon curve",          15, 0,  1, DRAGON_P1,  DRAGON_P2,  YELLOW, DrawIterDC},
    {true,  "Lévy C curve",          15, 0, -1, LEVI_P1,    LEVI_P2,    CYAN,   DrawIterLC},
    {false, "triangle effect",       10, 0,  1, DEFAULT_P1, DEFAULT_P2, GREEN,  DrawIterTrian}
    };

const struct button_mapping *plugin_contexts[] = {
	pla_main_ctx,
#if defined(HAVE_REMOTE_LCD)
    pla_remote_ctx,
#endif
};

#define NB_ACTION_CONTEXTS \
    (sizeof(plugin_contexts) / sizeof(struct button_mapping*))

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

int plugin_main(void)
{
    int action;
	int i;
    bool need_redraw = true;
    int CType = 0;
	
    FOR_NB_SCREENS(i)
    {
        struct screen *display = rb->screens[i];

        if (display->is_color)
            display->set_background(LCD_BLACK);
    }

    while (true)
    {
        if (need_redraw)
        {
            FOR_NB_SCREENS(i)
            {
                struct screen *display = rb->screens[i];
                
                rb->lcd_clear_display();
                
                StartIter = Curves[CType].CurIter;
                
                Curves[CType].DrawIter(Curves[CType].CurIter,
                                       Curves[CType].p1, Curves[CType].p2,
                                       Curves[CType].StartDir, Curves[CType].color, display);
                                       
                display->update();
            }
            need_redraw = false;
        }

        rb->sleep(SLEEP_TIME);

        action = pluginlib_getaction(TIMEOUT_NOBLOCK,
                                     plugin_contexts,
                                     NB_ACTION_CONTEXTS);
        switch (action)
        {
            case PLA_EXIT:
                cleanup(NULL);
                return PLUGIN_OK;
                
            case PLA_UP: //PLA_SELECT:
                if (Curves[CType].CurIter <= Curves[CType].MaxIter)
                {
                    Curves[CType].CurIter++;
                    need_redraw = true;
                }
                break;
                
            case PLA_DOWN: //PLA_START:
                if (Curves[CType].CurIter > 0)
                {
                    Curves[CType].CurIter--;
                    need_redraw = true;
                }
                break;
                
            case PLA_CANCEL:
                do
                {
                    if ((unsigned int)CType < ARRAYLEN(Curves) - 1)
                        CType++;
                    else
                        CType = 0;
                } while (!Curves[CType].IsShow);
                
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

/* Curves */

/* Draw an iteration of triangle effect */
void DrawIterTrian(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display)
{
    if (Iter < 0)
        return;

    color_apply(&color, display);
        
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
  
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len;

    /* We need to normalize normal vector. */
    NORMALIZE(n);
    
    len = rb_sqrt(DOT(s, s)) / 2;
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    DrawIterTrian(Iter - 1, p1, m, dir * (-1), color, display);
    DrawIterTrian(Iter - 1, m, p2, dir * (-1), color, display);
}

/* Draw an iteration of Lévy C curve */
void DrawIterLC(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display)
{
    if (Iter < 0)
        return;

    color_apply(&color, display);
        
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
  
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len;

    /* We need to normalize normal vector. */
    NORMALIZE(n);
    
    len = rb_sqrt(DOT(s, s)) / 2;
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    DrawIterLC(Iter - 1, p1, m, dir, color, display);
    DrawIterLC(Iter - 1, m, p2, dir, color, display);
}

/* Draw an iteration of dragon curve */
void DrawIterDC(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display)
{
    if (Iter < 0)
        return;

    color_apply(&color, display);
        
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
  
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len;

    /* We need to normalize normal vector. */
    NORMALIZE(n);
    
    len = rb_sqrt(DOT(s, s)) / 2;
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    DrawIterDC(Iter - 1, p1, m, dir, color, display);
    DrawIterDC(Iter - 1, m, p2, dir * (-1), color, display);
}

/* Draw an iteration of blood effect, type 1 */
void DrawIterB1(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display)
{
    if (Iter < 0)
        return;

    color_apply(&color, display);
        
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
  
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len = rb_sqrt(DOT(s, s)) * SQRT3BY2;
    
    /* We need to normalize normal vector. */
    NORMALIZE(n);
    
    s.x = (p1.x + p2.x) / 3;
    s.y = (p1.y + p2.y) / 3;
    DrawIterB1(Iter - 1, p1, s, dir, color, display);
    
    len = rb_sqrt((DOT(s, s)) / 3) * SQRT3BY2;
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    DrawIterB1(Iter - 1, s, m, dir, color, display);
    
    s.x = (p1.x + p2.x) * 2 / 3;
    s.y = (p1.y + p2.y) * 2 / 3;
    DrawIterB1(Iter - 1, m, s, dir, color, display);
    
    DrawIterB1(Iter - 1, s, p2, dir, color, display);
}

/* Draw an iteration of blood effect, type 2 */
void DrawIterB2(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display)
{
    if (Iter < 0)
        return;

    color_apply(&color, display);
        
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
  
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len = rb_sqrt(DOT(s, s)) * SQRT3BY2;
    
    /* We need to normalize normal vector. */
    NORMALIZE(n);

    s.x = (p1.x + 0.5*p2.x) / 1.5;
    s.y = (p1.y + 0.5*p2.y) / 1.5;
    DrawIterB2(Iter - 1, p1, s, dir, color, display);
    
    len = rb_sqrt((DOT(s, s)) / 3) * SQRT3BY2;
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    DrawIterB2(Iter - 1, s, m, dir, color, display);
    
    s.x = (p1.x + p2.x) * 2 / 3;
    s.y = (p1.y + p2.y) * 2 / 3;
    DrawIterB2(Iter - 1, m, s, dir, color, display);
    
    DrawIterB2(Iter - 1, s, p2, dir, color, display);
}

/* Draw an iteration of Koch curve */
void DrawIterKC(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display)
{
    if (Iter < 0)
        return;

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
    DrawIterKC(Iter - 1, p1, s1, dir, color, display);
    
    /* We need to normalize normal vector. */
    NORMALIZE(n);
    
    len = rb_sqrt(DOT(s, s)) / 3 * SQRT3BY2;
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    DrawIterKC(Iter - 1, s1, m, dir, color, display);
    
    s.x = (p1.x + 2*p2.x) / 3;
    s.y = (p1.y + 2*p2.y) / 3;
    DrawIterKC(Iter - 1, m, s, dir, color, display);
    DrawIterKC(Iter - 1, s, p2, dir, color, display);
}

/* Draw an iteration of beautiful flower, type2 */
void DrawIterFlower2(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display)
{
    if (Iter < 0)
        return;
    
    color_apply(&color, display);
    
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len = rb_sqrt(DOT(s, s)) * SQRT3BY2;
    
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
        DrawIterFlower2(Iter - 1, p1, s, dir * (-1), color, display);
    else
        DrawIterFlower2(Iter - 1, p1, s, dir, color, display);
    
    n.x = (p2.x + m.x) / 2;
    n.y = (p2.y + m.y) / 2;
    if (Iter & 1)
    {
        DrawIterFlower2(Iter - 1, s, n, dir * (-1), color, display);
        DrawIterFlower2(Iter - 1, n, p2, dir * (-1), color, display);
    }
    else
    {
        DrawIterFlower2(Iter - 1, s, n, dir, color, display);
        DrawIterFlower2(Iter - 1, n, p2, dir, color, display);
    }
}

/* Draw an iteration of beautiful flower, type1 */
void DrawIterFlower1(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display)
{
    if (Iter < 0)
        return;
    
    color_apply(&color, display);
    
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
    
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len = rb_sqrt(DOT(s, s)) * SQRT3BY2;
    
    /* We need to normalize normal vector. */
    NORMALIZE(n);
    
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    s.x = (p1.x + m.x) / 2;
    s.y = (p1.y + m.y) / 2;
    DrawIterFlower1(Iter - 1, p1, s, dir * (-1), color, display);
    
    n.x = (p2.x + m.x) / 2;
    n.y = (p2.y + m.y) / 2;
    DrawIterFlower1(Iter - 1, s, n, dir * (-1), color, display);
    DrawIterFlower1(Iter - 1, n, p2, dir * (-1), color, display);
}

/* Draw an iteration of Sierpinski triangle, type 1 */
void DrawIterST1(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display)
{
    if (Iter < 0)
        return;

    color_apply(&color, display);
        
    if (Iter == 0)
        display->drawline(p1.x, p1.y, p2.x, p2.y);
  
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len = rb_sqrt(DOT(s, s)) * SQRT3BY2;
    
    /* We need to normalize normal vector. */
    NORMALIZE(n);
    
    n.x *= len;
    n.y *= len;
    
    m.x += n.x;
    m.y += n.y;
    
    s.x = (p1.x + m.x) / 2;
    s.y = (p1.y + m.y) / 2;
    DrawIterST1(Iter - 1, p1, s, dir * (-1), color, display);
    
    n.x = (p2.x + m.x) / 2;
    n.y = (p2.y + m.y) / 2;
    DrawIterST1(Iter - 1, s, n, dir, color, display);
    DrawIterST1(Iter - 1, n, p2, dir * (-1), color, display);
}

/* Draw an iteration of Sierpinski triangle, type 2 */
void DrawIterST2(int Iter, struct vec2 p1, struct vec2 p2, int dir, struct Color color, struct screen *display)
{
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
     * TODO: iteration-depending color handling
    color.r = 255;
    color.g = color.b = 0; */
    
    color_apply(&color, display); 
    
    /* Thanks to np3 for this math stuff */
    struct vec2 s = {p2.x - p1.x, p2.y - p1.y};
    struct vec2 m = {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2};
    struct vec2 n = {(p2.y - p1.y) * dir, (p1.x - p2.x) * dir};
    double len = rb_sqrt(DOT(s, s)) * SQRT3BY2;
    
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
    DrawIterST2(Iter - 1, p1, s, dir * (-1), color, display);
    
    s.x = (p2.x + m.x) / 2;
    s.y = (p2.y + m.y) / 2;
    DrawIterST2(Iter - 1, p2, s, dir, color, display);
    
    s.x = (m.x + p2.x) / 2;
    s.y = (m.y + p2.y) / 2;
    DrawIterST2(Iter - 1, m, s, dir * (-1), color, display);
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
