/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2002 Bj�rn Stenberg
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#include "menu.h"
#include "tree.h"
#include "credits.h"
#include "lcd.h"
#include "button.h"
#include "kernel.h"

void show_splash(void);

#ifdef HAVE_LCD_BITMAP

#include "screensaver.h"
extern void tetris(void);

/* recorder menu */
enum Main_Menu_Ids {
    Tetris, Screen_Saver, Splash, Credits
};

struct menu_items items[] = {
    { Tetris,       "Tetris",       tetris      },
    { Screen_Saver, "Screen Saver", screensaver },
    { Splash,       "Splash",       show_splash },
    { Credits,      "Credits",      show_credits },
};

#else

/* player menu */
enum Main_Menu_Ids {
    Splash, Credits
};

struct menu_items items[] = {
    { Splash,       "Splash",       show_splash },
    { Credits,      "Credits",      show_credits },
};

#endif

#ifdef HAVE_LCD_BITMAP
int show_logo(void)
{
    unsigned char buffer[112 * 8];

    int failure;
    int width=0;
    int height=0;

    failure = read_bmp_file("/rockbox112.bmp", &width, &height, buffer);

    debugf("read_bmp_file() returned %d, width %d height %d\n",
           failure, width, height);

    if (failure) {
        debugf("Unable to find \"/rockbox112.bmp\"\n");
        return -1;
    } else {

        int i;
        int eline;

        for(i=0, eline=0; i< height; i+=8, eline++) {
            int x,y;
      
            /* the bitmap function doesn't work with full-height bitmaps
               so we "stripe" the logo output */

            lcd_bitmap(&buffer[eline*width], 0, 10+i, width,
                       (height-i)>8?8:height-i, false);
      
#if 0
            /* for screen output debugging */
            for(y=0; y<8 && (i+y < height); y++) {
                for(x=0; x < width; x++) {

                    if(buffer[eline*width + x] & (1<<y)) {
                        printf("*");
                    }
                    else
                        printf(" ");
                }
                printf("\n");
            }
#endif
        }
    }

    return 0;
}
#endif

void show_splash(void)
{
    int i;
    char *rockbox = "ROCKbox!";
    lcd_clear_display();

#ifdef HAVE_LCD_BITMAP
    if (show_logo() != 0) 
        return;
#else
    lcd_puts(0, 0, rockbox);
#endif

    lcd_update();
    for ( i=0;i<10;i++) {
        sleep(HZ/10);
        if (button_get())
            break;
    }
}

void main_menu(void)
{
    menu_init( items, sizeof(items)/sizeof(struct menu_items) );
    menu_run();
}
