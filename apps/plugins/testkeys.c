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
 * Test some buttons
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

#define NELEMS(a) (sizeof(a) / sizeof(a[0]))

#define DEFAULT_WAIT_TIME 3

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

/*
 * Main function
 */

int plugin_main(void)
{
    int action;
    int sleep_time = DEFAULT_WAIT_TIME;
    /* without unsigned I got warning on line before "switch(action)" */
    unsigned int i;
    bool show_none_button = true;

    int key[] = {
					PLA_LEFT,              BUTTON_MINUS,
					PLA_RIGHT,             BUTTON_PLUS,
					BUTTON_MENU,	PLA_SELECT,
					PLA_CANCEL,          BUTTON_MENU,
					BUTTON_PLUS,      BUTTON_MINUS,
					PLA_UP,	PLA_DOWN };
				
	char *keyname[] = {
					"PLA_LEFT",              "BUTTON_MINUS",
					"PLA_RIGHT",             "BUTTON_PLUS",
					"BUTTON_MENU",          "PLA_SELECT",
					"PLA_CANCEL",          "BUTTON_MENU",
					"BUTTON_PLUS",       "BUTTON_MINUS",
					"PLA_UP", 	"PLA_DOWN" };
					
    while (true)
    {
			rb->sleep(sleep_time);
				
			action = pluginlib_getaction(TIMEOUT_NOBLOCK,
										 plugin_contexts,
										 NB_ACTION_CONTEXTS);
										 
			
			for (i = 0; i < NELEMS(key); i ++)
				if (action == key[i])
				{
					rb->lcd_clear_display();
					rb->lcd_update();
					rb->splash(HZ*2, keyname[i]);
					break;
				}
				
			if (action == PLA_EXIT)
			{
				rb->lcd_clear_display();
				rb->lcd_update();
				rb->splash(HZ*2, "PLA_EXIT :(");
				cleanup(NULL);
				return PLUGIN_OK;
			}
			
			if(false)
			if (i == NELEMS(key))
			switch (action)
			{
				case PLA_EXIT:
					rb->lcd_clear_display();
					rb->lcd_update();
					rb->splash(HZ*2, "PLA_EXIT :(");
					cleanup(NULL);
					return PLUGIN_OK;

				case BUTTON_NONE:
					if (show_none_button)
					{
						rb->lcd_clear_display();
						rb->lcd_update();
						rb->splash(HZ*2, "BUTTON_NONE (:");
						show_none_button = false;
					}
					break;
					
					//BUTTON_POWER
					
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
