/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2002-2007 Björn Stenberg
 * Copyright (C) 2007-2008 Nicolas Pennequin
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
#include "font.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "system.h"
#include "settings.h"
#include "settings_list.h"
#include "rbunicode.h"
#include "rtc.h"
#include "audio.h"
#include "status.h"
#include "power.h"
#include "powermgmt.h"
#include "sound.h"
#include "debug.h"
#ifdef HAVE_LCD_CHARCELLS
#include "hwcompat.h"
#endif
#include "abrepeat.h"
#include "mp3_playback.h"
#include "lang.h"
#include "misc.h"
#include "splash.h"
#include "scrollbar.h"
#include "led.h"
#include "lcd.h"
#ifdef HAVE_LCD_BITMAP
#include "peakmeter.h"
/* Image stuff */
#include "bmp.h"
#include "albumart.h"
#endif
#include "dsp.h"
#include "action.h"
#include "cuesheet.h"
#include "playlist.h"
#if CONFIG_CODEC == SWCODEC
#include "playback.h"
#endif
#include "backdrop.h"
#include "viewport.h"


#include "wps_internals.h"
#include "skin_engine.h"

static bool gui_wps_redraw(struct gui_wps *gwps, unsigned refresh_mode);


bool gui_wps_display(struct gui_wps *gwps)
{
    struct screen *display = gwps->display;

    /* Update the values in the first (default) viewport - in case the user
       has modified the statusbar or colour settings */
#if LCD_DEPTH > 1
    if (display->depth > 1)
    {
        gwps->data->viewports[0].vp.fg_pattern = display->get_foreground();
        gwps->data->viewports[0].vp.bg_pattern = display->get_background();
    }
#endif
    display->clear_display();
    display->backdrop_show(BACKDROP_SKIN_WPS);
    return gui_wps_redraw(gwps, WPS_REFRESH_ALL);
}

/* update a skinned screen, update_type is WPS_REFRESH_* values.
 * Usually it should only be WPS_REFRESH_NON_STATIC
 * A full update will be done if required (state.do_full_update == true)
 */
bool skin_update(struct gui_wps *gwps, unsigned int update_type)
{
    bool retval;
    /* This maybe shouldnt be here, but while the skin is only used to 
     * display the music screen this is better than whereever we are being
     * called from. This is also safe for skined screen which dont use the id3 */
    struct mp3entry *id3 = gwps->state->id3;
    bool cuesheet_update = (id3 != NULL ? cuesheet_subtrack_changed(id3) : false);
    gwps->state->do_full_update = cuesheet_update || gwps->state->do_full_update;
    
    retval = gui_wps_redraw(gwps, gwps->state->do_full_update ? 
                                        WPS_REFRESH_ALL : update_type);
    return retval;
}


#ifdef HAVE_LCD_BITMAP

static void draw_progressbar(struct gui_wps *gwps,
                             struct wps_viewport *wps_vp)
 {
    struct screen *display = gwps->display;
    struct wps_state *state = gwps->state;
    struct progressbar *pb = wps_vp->pb;
    int y = pb->y;

    if (y < 0)
    {
        int line_height = font_get(wps_vp->vp.font)->height;
        /* center the pb in the line, but only if the line is higher than the pb */
        int center = (line_height-pb->height)/2;
        /* if Y was not set calculate by font height,Y is -line_number-1 */
        y = (-y -1)*line_height + (0 > center ? 0 : center);
    }

    if (pb->have_bitmap_pb)
        gui_bitmap_scrollbar_draw(display, pb->bm,
                                  pb->x, y, pb->width, pb->bm.height,
                                  state->id3->length ? state->id3->length : 1, 0,
                                  state->id3->length ? state->id3->elapsed
                                          + state->ff_rewind_count : 0,
                                          HORIZONTAL);
    else
        gui_scrollbar_draw(display, pb->x, y, pb->width, pb->height,
                           state->id3->length ? state->id3->length : 1, 0,
                           state->id3->length ? state->id3->elapsed
                                   + state->ff_rewind_count : 0,
                                   HORIZONTAL);
#ifdef AB_REPEAT_ENABLE
    if ( ab_repeat_mode_enabled() && state->id3->length != 0 )
        ab_draw_markers(display, state->id3->length,
                        pb->x, pb->x + pb->width, y, pb->height);
#endif

    if (state->id3->cuesheet)
        cue_draw_markers(display, state->id3->cuesheet, state->id3->length,
                         pb->x, pb->x + pb->width, y+1, pb->height-2);
}

/* clears the area where the image was shown */
static void clear_image_pos(struct gui_wps *gwps, int n)
{
    if(!gwps)
        return;
    struct wps_data *data = gwps->data;
    gwps->display->set_drawmode(DRMODE_SOLID|DRMODE_INVERSEVID);
    gwps->display->fillrect(data->img[n].x, data->img[n].y,
                            data->img[n].bm.width, data->img[n].subimage_height);
    gwps->display->set_drawmode(DRMODE_SOLID);
}

static void wps_draw_image(struct gui_wps *gwps, int n, int subimage)
{
    struct screen *display = gwps->display;
    struct wps_data *data = gwps->data;
    if(data->img[n].always_display)
        display->set_drawmode(DRMODE_FG);
    else
        display->set_drawmode(DRMODE_SOLID);

#if LCD_DEPTH > 1
    if(data->img[n].bm.format == FORMAT_MONO) {
#endif
        display->mono_bitmap_part(data->img[n].bm.data, 
                                  0, data->img[n].subimage_height * subimage,
                                  data->img[n].bm.width, data->img[n].x,
                                  data->img[n].y, data->img[n].bm.width,
                                  data->img[n].subimage_height);
#if LCD_DEPTH > 1
    } else {
        display->transparent_bitmap_part((fb_data *)data->img[n].bm.data,
                                         0, data->img[n].subimage_height * subimage,
                                         data->img[n].bm.width, data->img[n].x,
                                         data->img[n].y, data->img[n].bm.width,
                                         data->img[n].subimage_height);
    }
#endif
}

static void wps_display_images(struct gui_wps *gwps, struct viewport* vp)
{
    if(!gwps || !gwps->data || !gwps->display)
        return;

    int n;
    struct wps_data *data = gwps->data;
    struct screen *display = gwps->display;

    for (n = 0; n < MAX_IMAGES; n++)
    {
        if (data->img[n].loaded)
        {
            if (data->img[n].display >= 0)
            {
                wps_draw_image(gwps, n, data->img[n].display);
            } else if (data->img[n].always_display && data->img[n].vp == vp)
            {
                wps_draw_image(gwps, n, 0);
            }
        }
    }
    display->set_drawmode(DRMODE_SOLID);
}

#else /* HAVE_LCD_CHARCELL */

static bool draw_player_progress(struct gui_wps *gwps)
{
    struct wps_state *state = gwps->state;
    struct screen *display = gwps->display;
    unsigned char progress_pattern[7];
    int pos = 0;
    int i;

    if (!state->id3)
        return false;

    if (state->id3->length)
        pos = 36 * (state->id3->elapsed + state->ff_rewind_count)
              / state->id3->length;

    for (i = 0; i < 7; i++, pos -= 5)
    {
        if (pos <= 0)
            progress_pattern[i] = 0x1fu;
        else if (pos >= 5)
            progress_pattern[i] = 0x00u;
        else
            progress_pattern[i] = 0x1fu >> pos;
    }

    display->define_pattern(gwps->data->wps_progress_pat[0], progress_pattern);
    return true;
}

static void draw_player_fullbar(struct gui_wps *gwps, char* buf, int buf_size)
{
    static const unsigned char numbers[10][4] = {
        {0x0e, 0x0a, 0x0a, 0x0e}, /* 0 */
        {0x04, 0x0c, 0x04, 0x04}, /* 1 */
        {0x0e, 0x02, 0x04, 0x0e}, /* 2 */
        {0x0e, 0x02, 0x06, 0x0e}, /* 3 */
        {0x08, 0x0c, 0x0e, 0x04}, /* 4 */
        {0x0e, 0x0c, 0x02, 0x0c}, /* 5 */
        {0x0e, 0x08, 0x0e, 0x0e}, /* 6 */
        {0x0e, 0x02, 0x04, 0x08}, /* 7 */
        {0x0e, 0x0e, 0x0a, 0x0e}, /* 8 */
        {0x0e, 0x0e, 0x02, 0x0e}, /* 9 */
    };

    struct wps_state *state = gwps->state;
    struct screen *display = gwps->display;
    struct wps_data *data = gwps->data;
    unsigned char progress_pattern[7];
    char timestr[10];
    int time;
    int time_idx = 0;
    int pos = 0;
    int pat_idx = 1;
    int digit, i, j;
    bool softchar;

    if (!state->id3 || buf_size < 34) /* worst case: 11x UTF-8 char + \0 */
        return;

    time = state->id3->elapsed + state->ff_rewind_count;
    if (state->id3->length)
        pos = 55 * time / state->id3->length;

    memset(timestr, 0, sizeof(timestr));
    format_time(timestr, sizeof(timestr)-2, time);
    timestr[strlen(timestr)] = ':';   /* always safe */

    for (i = 0; i < 11; i++, pos -= 5)
    {
        softchar = false;
        memset(progress_pattern, 0, sizeof(progress_pattern));
        
        if ((digit = timestr[time_idx]))
        {
            softchar = true;
            digit -= '0';

            if (timestr[time_idx + 1] == ':')  /* ones, left aligned */
            {   
                memcpy(progress_pattern, numbers[digit], 4);
                time_idx += 2;
            }
            else  /* tens, shifted right */
            {
                for (j = 0; j < 4; j++)
                    progress_pattern[j] = numbers[digit][j] >> 1;

                if (time_idx > 0)  /* not the first group, add colon in front */
                {
                    progress_pattern[1] |= 0x10u;
                    progress_pattern[3] |= 0x10u;
                }
                time_idx++;
            }

            if (pos >= 5)
                progress_pattern[5] = progress_pattern[6] = 0x1fu;
        }

        if (pos > 0 && pos < 5)
        {
            softchar = true;
            progress_pattern[5] = progress_pattern[6] = (~0x1fu >> pos) & 0x1fu;
        }

        if (softchar && pat_idx < 8)
        {
            display->define_pattern(data->wps_progress_pat[pat_idx],
                                    progress_pattern);
            buf = utf8encode(data->wps_progress_pat[pat_idx], buf);
            pat_idx++;
        }
        else if (pos <= 0)
            buf = utf8encode(' ', buf);
        else
            buf = utf8encode(0xe115, buf); /* 2/7 _ */
    }
    *buf = '\0';
}

#endif /* HAVE_LCD_CHARCELL */

/* Return the index to the end token for the conditional token at index.
   The conditional token can be either a start token or a separator
   (i.e. option) token.
*/
static int find_conditional_end(struct wps_data *data, int index)
{
    int ret = index;
    while (data->tokens[ret].type != WPS_TOKEN_CONDITIONAL_END)
        ret = data->tokens[ret].value.i;

    /* ret now is the index to the end token for the conditional. */
    return ret;
}

/* Evaluate the conditional that is at *token_index and return whether a skip
   has ocurred. *token_index is updated with the new position.
*/
static bool evaluate_conditional(struct gui_wps *gwps, int *token_index)
{
    if (!gwps)
        return false;

    struct wps_data *data = gwps->data;

    int i, cond_end;
    int cond_index = *token_index;
    char result[128];
    const char *value;
    unsigned char num_options = data->tokens[cond_index].value.i & 0xFF;
    unsigned char prev_val = (data->tokens[cond_index].value.i & 0xFF00) >> 8;

    /* treat ?xx<true> constructs as if they had 2 options. */
    if (num_options < 2)
        num_options = 2;

    int intval = num_options;
    /* get_token_value needs to know the number of options in the enum */
    value = get_token_value(gwps, &data->tokens[cond_index + 1],
                            result, sizeof(result), &intval);

    /* intval is now the number of the enum option we want to read,
       starting from 1. If intval is -1, we check if value is empty. */
    if (intval == -1)
        intval = (value && *value) ? 1 : num_options;
    else if (intval > num_options || intval < 1)
        intval = num_options;

    data->tokens[cond_index].value.i = (intval << 8) + num_options;

    /* skip to the appropriate enum case */
    int next = cond_index + 2;
    for (i = 1; i < intval; i++)
    {
        next = data->tokens[next].value.i;
    }
    *token_index = next;

    if (prev_val == intval)
    {
        /* Same conditional case as previously. Return without clearing the
           pictures */
        return false;
    }

    cond_end = find_conditional_end(data, cond_index + 2);
    for (i = cond_index + 3; i < cond_end; i++)
    {
#ifdef HAVE_LCD_BITMAP
        /* clear all pictures in the conditional and nested ones */
        if (data->tokens[i].type == WPS_TOKEN_IMAGE_PRELOAD_DISPLAY)
            clear_image_pos(gwps, data->tokens[i].value.i & 0xFF);
#endif
#ifdef HAVE_ALBUMART
        if (data->tokens[i].type == WPS_TOKEN_ALBUMART_DISPLAY)
            draw_album_art(gwps, audio_current_aa_hid(), true);
#endif
    }

    return true;
}

/* Read a (sub)line to the given alignment format buffer.
   linebuf is the buffer where the data is actually stored.
   align is the alignment format that'll be used to display the text.
   The return value indicates whether the line needs to be updated.
*/
static bool get_line(struct gui_wps *gwps,
                     int line, int subline,
                     struct align_pos *align,
                     char *linebuf,
                     int linebuf_size)
{
    struct wps_data *data = gwps->data;

    char temp_buf[128];
    char *buf = linebuf;  /* will always point to the writing position */
    char *linebuf_end = linebuf + linebuf_size - 1;
    int i, last_token_idx;
    bool update = false;

    /* alignment-related variables */
    int cur_align;
    char* cur_align_start;
    cur_align_start = buf;
    cur_align = WPS_ALIGN_LEFT;
    align->left = NULL;
    align->center = NULL;
    align->right = NULL;

    /* Process all tokens of the desired subline */
    last_token_idx = wps_last_token_index(data, line, subline);
    for (i = wps_first_token_index(data, line, subline);
         i <= last_token_idx; i++)
    {
        switch(data->tokens[i].type)
        {
            case WPS_TOKEN_CONDITIONAL:
                /* place ourselves in the right conditional case */
                update |= evaluate_conditional(gwps, &i);
                break;

            case WPS_TOKEN_CONDITIONAL_OPTION:
                /* we've finished in the curent conditional case,
                    skip to the end of the conditional structure */
                i = find_conditional_end(data, i);
                break;

#ifdef HAVE_LCD_BITMAP
            case WPS_TOKEN_IMAGE_PRELOAD_DISPLAY:
            {
                struct gui_img *img = data->img;
                int n = data->tokens[i].value.i & 0xFF;
                int subimage = data->tokens[i].value.i >> 8;

                if (n >= 0 && n < MAX_IMAGES && img[n].loaded)
                    img[n].display = subimage;
                break;
            }
#endif

            case WPS_TOKEN_ALIGN_LEFT:
            case WPS_TOKEN_ALIGN_CENTER:
            case WPS_TOKEN_ALIGN_RIGHT:
                /* remember where the current aligned text started */
                switch (cur_align)
                {
                    case WPS_ALIGN_LEFT:
                        align->left = cur_align_start;
                        break;

                    case WPS_ALIGN_CENTER:
                        align->center = cur_align_start;
                        break;

                    case WPS_ALIGN_RIGHT:
                        align->right = cur_align_start;
                        break;
                }
                /* start a new alignment */
                switch (data->tokens[i].type)
                {
                    case WPS_TOKEN_ALIGN_LEFT:
                        cur_align = WPS_ALIGN_LEFT;
                        break;
                    case WPS_TOKEN_ALIGN_CENTER:
                        cur_align = WPS_ALIGN_CENTER;
                        break;
                    case WPS_TOKEN_ALIGN_RIGHT:
                        cur_align = WPS_ALIGN_RIGHT;
                        break;
                    default:
                        break;
                }
                *buf++ = 0;
                cur_align_start = buf;
                break;
            case WPS_VIEWPORT_ENABLE:
            {
                char label = data->tokens[i].value.i;
                int j;
                char temp = VP_DRAW_HIDEABLE;
                for(j=0;j<data->num_viewports;j++)
                {
                    temp = VP_DRAW_HIDEABLE;
                    if ((data->viewports[j].hidden_flags&VP_DRAW_HIDEABLE) &&
                        (data->viewports[j].label == label))
                    {
                        if (data->viewports[j].hidden_flags&VP_DRAW_WASHIDDEN)
                            temp |= VP_DRAW_WASHIDDEN;
                        data->viewports[j].hidden_flags = temp;
                    }
                }
            }
                break;
            default:
            {
                /* get the value of the tag and copy it to the buffer */
                const char *value = get_token_value(gwps, &data->tokens[i],
                                              temp_buf, sizeof(temp_buf), NULL);
                if (value)
                {
                    update = true;
                    while (*value && (buf < linebuf_end))
                        *buf++ = *value++;
                }
                break;
            }
        }
    }

    /* close the current alignment */
    switch (cur_align)
    {
        case WPS_ALIGN_LEFT:
            align->left = cur_align_start;
            break;

        case WPS_ALIGN_CENTER:
            align->center = cur_align_start;
            break;

        case WPS_ALIGN_RIGHT:
            align->right = cur_align_start;
            break;
    }

    return update;
}

static void get_subline_timeout(struct gui_wps *gwps, int line, int subline)
{
    struct wps_data *data = gwps->data;
    int i;
    int subline_idx = wps_subline_index(data, line, subline);
    int last_token_idx = wps_last_token_index(data, line, subline);

    data->sublines[subline_idx].time_mult = DEFAULT_SUBLINE_TIME_MULTIPLIER;

    for (i = wps_first_token_index(data, line, subline);
         i <= last_token_idx; i++)
    {
        switch(data->tokens[i].type)
        {
            case WPS_TOKEN_CONDITIONAL:
                /* place ourselves in the right conditional case */
                evaluate_conditional(gwps, &i);
                break;

            case WPS_TOKEN_CONDITIONAL_OPTION:
                /* we've finished in the curent conditional case,
                    skip to the end of the conditional structure */
                i = find_conditional_end(data, i);
                break;

            case WPS_TOKEN_SUBLINE_TIMEOUT:
                data->sublines[subline_idx].time_mult = data->tokens[i].value.i;
                break;

            default:
                break;
        }
    }
}

/* Calculates which subline should be displayed for the specified line
   Returns true iff the subline must be refreshed */
static bool update_curr_subline(struct gui_wps *gwps, int line)
{
    struct wps_data *data = gwps->data;

    int search, search_start, num_sublines;
    bool reset_subline;
    bool new_subline_refresh;
    bool only_one_subline;

    num_sublines = data->lines[line].num_sublines;
    reset_subline = (data->lines[line].curr_subline == SUBLINE_RESET);
    new_subline_refresh = false;
    only_one_subline = false;

    /* if time to advance to next sub-line  */
    if (TIME_AFTER(current_tick, data->lines[line].subline_expire_time - 1) ||
        reset_subline)
    {
        /* search all sublines until the next subline with time > 0
            is found or we get back to the subline we started with */
        if (reset_subline)
            search_start = 0;
        else
            search_start = data->lines[line].curr_subline;

        for (search = 0; search < num_sublines; search++)
        {
            data->lines[line].curr_subline++;

            /* wrap around if beyond last defined subline or WPS_MAX_SUBLINES */
            if (data->lines[line].curr_subline == num_sublines)
            {
                if (data->lines[line].curr_subline == 1)
                    only_one_subline = true;
                data->lines[line].curr_subline = 0;
            }

            /* if back where we started after search or
                only one subline is defined on the line */
            if (((search > 0) &&
                 (data->lines[line].curr_subline == search_start)) ||
                only_one_subline)
            {
                /* no other subline with a time > 0 exists */
                data->lines[line].subline_expire_time = (reset_subline ?
                    current_tick :
                    data->lines[line].subline_expire_time) + 100 * HZ;
                break;
            }
            else
            {
                /* get initial time multiplier for this subline */
                get_subline_timeout(gwps, line, data->lines[line].curr_subline);

                int subline_idx = wps_subline_index(data, line,
                                               data->lines[line].curr_subline);

                /* only use this subline if subline time > 0 */
                if (data->sublines[subline_idx].time_mult > 0)
                {
                    new_subline_refresh = true;
                    data->lines[line].subline_expire_time = (reset_subline ?
                        current_tick : data->lines[line].subline_expire_time) +
                        TIMEOUT_UNIT*data->sublines[subline_idx].time_mult;
                    break;
                }
            }
        }
    }

    return new_subline_refresh;
}

/* Display a line appropriately according to its alignment format.
   format_align contains the text, separated between left, center and right.
   line is the index of the line on the screen.
   scroll indicates whether the line is a scrolling one or not.
*/
static void write_line(struct screen *display,
                       struct align_pos *format_align,
                       int line,
                       bool scroll)
{
    int left_width = 0, left_xpos;
    int center_width = 0, center_xpos;
    int right_width = 0,  right_xpos;
    int ypos;
    int space_width;
    int string_height;
    int scroll_width;

    /* calculate different string sizes and positions */
    display->getstringsize((unsigned char *)" ", &space_width, &string_height);
    if (format_align->left != 0) {
        display->getstringsize((unsigned char *)format_align->left,
                                &left_width, &string_height);
    }

    if (format_align->right != 0) {
        display->getstringsize((unsigned char *)format_align->right,
                                &right_width, &string_height);
    }

    if (format_align->center != 0) {
        display->getstringsize((unsigned char *)format_align->center,
                                &center_width, &string_height);
    }

    left_xpos = 0;
    right_xpos = (display->getwidth() - right_width);
    center_xpos = (display->getwidth() + left_xpos - center_width) / 2;

    scroll_width = display->getwidth() - left_xpos;

    /* Checks for overlapping strings.
        If needed the overlapping strings will be merged, separated by a
        space */

    /* CASE 1: left and centered string overlap */
    /* there is a left string, need to merge left and center */
    if ((left_width != 0 && center_width != 0) &&
        (left_xpos + left_width + space_width > center_xpos)) {
        /* replace the former separator '\0' of left and
            center string with a space */
        *(--format_align->center) = ' ';
        /* calculate the new width and position of the merged string */
        left_width = left_width + space_width + center_width;
        /* there is no centered string anymore */
        center_width = 0;
    }
    /* there is no left string, move center to left */
    if ((left_width == 0 && center_width != 0) &&
        (left_xpos + left_width > center_xpos)) {
        /* move the center string to the left string */
        format_align->left = format_align->center;
        /* calculate the new width and position of the string */
        left_width = center_width;
        /* there is no centered string anymore */
        center_width = 0;
    }

    /* CASE 2: centered and right string overlap */
    /* there is a right string, need to merge center and right */
    if ((center_width != 0 && right_width != 0) &&
        (center_xpos + center_width + space_width > right_xpos)) {
        /* replace the former separator '\0' of center and
            right string with a space */
        *(--format_align->right) = ' ';
        /* move the center string to the right after merge */
        format_align->right = format_align->center;
        /* calculate the new width and position of the merged string */
        right_width = center_width + space_width + right_width;
        right_xpos = (display->getwidth() - right_width);
        /* there is no centered string anymore */
        center_width = 0;
    }
    /* there is no right string, move center to right */
    if ((center_width != 0 && right_width == 0) &&
        (center_xpos + center_width > right_xpos)) {
        /* move the center string to the right string */
        format_align->right = format_align->center;
        /* calculate the new width and position of the string */
        right_width = center_width;
        right_xpos = (display->getwidth() - right_width);
        /* there is no centered string anymore */
        center_width = 0;
    }

    /* CASE 3: left and right overlap
        There is no center string anymore, either there never
        was one or it has been merged in case 1 or 2 */
    /* there is a left string, need to merge left and right */
    if ((left_width != 0 && center_width == 0 && right_width != 0) &&
        (left_xpos + left_width + space_width > right_xpos)) {
        /* replace the former separator '\0' of left and
            right string with a space */
        *(--format_align->right) = ' ';
        /* calculate the new width and position of the string */
        left_width = left_width + space_width + right_width;
        /* there is no right string anymore */
        right_width = 0;
    }
    /* there is no left string, move right to left */
    if ((left_width == 0 && center_width == 0 && right_width != 0) &&
        (left_width > right_xpos)) {
        /* move the right string to the left string */
        format_align->left = format_align->right;
        /* calculate the new width and position of the string */
        left_width = right_width;
        /* there is no right string anymore */
        right_width = 0;
    }

    ypos = (line * string_height);


    if (scroll && ((left_width > scroll_width) || 
                   (center_width > scroll_width) ||
                   (right_width > scroll_width)))
    {
        display->puts_scroll(0, line,
                             (unsigned char *)format_align->left);
    }
    else
    {
#ifdef HAVE_LCD_BITMAP
        /* clear the line first */
        display->set_drawmode(DRMODE_SOLID|DRMODE_INVERSEVID);
        display->fillrect(left_xpos, ypos, display->getwidth(), string_height);
        display->set_drawmode(DRMODE_SOLID);
#endif

        /* Nasty hack: we output an empty scrolling string,
        which will reset the scroller for that line */
        display->puts_scroll(0, line, (unsigned char *)"");

        /* print aligned strings */
        if (left_width != 0)
        {
            display->putsxy(left_xpos, ypos,
                            (unsigned char *)format_align->left);
        }
        if (center_width != 0)
        {
            display->putsxy(center_xpos, ypos,
                            (unsigned char *)format_align->center);
        }
        if (right_width != 0)
        {
            display->putsxy(right_xpos, ypos,
                            (unsigned char *)format_align->right);
        }
    }
}

static bool gui_wps_redraw(struct gui_wps *gwps, unsigned refresh_mode)
{
    struct wps_data *data = gwps->data;
    struct screen *display = gwps->display;
    struct wps_state *state = gwps->state;

    if (!data || !state || !display)
        return false;

    struct mp3entry *id3 = state->id3;

    if (!id3)
        return false;

    int v, line, i, subline_idx;
    unsigned flags;
    char linebuf[MAX_PATH];

    struct align_pos align;
    align.left = NULL;
    align.center = NULL;
    align.right = NULL;

    bool update_line, new_subline_refresh;

#ifdef HAVE_LCD_BITMAP

    /* to find out wether the peak meter is enabled we
       assume it wasn't until we find a line that contains
       the peak meter. We can't use peak_meter_enabled itself
       because that would mean to turn off the meter thread
       temporarily. (That shouldn't matter unless yield
       or sleep is called but who knows...)
    */
    bool enable_pm = false;

#endif

    /* reset to first subline if refresh all flag is set */
    if (refresh_mode == WPS_REFRESH_ALL)
    {
        display->set_viewport(&data->viewports[0].vp);
        display->clear_viewport();

        for (i = 0; i <= data->num_lines; i++)
        {
            data->lines[i].curr_subline = SUBLINE_RESET;
        }
    }

#ifdef HAVE_LCD_CHARCELLS
    for (i = 0; i < 8; i++)
    {
       if (data->wps_progress_pat[i] == 0)
           data->wps_progress_pat[i] = display->get_locked_pattern();
    }
#endif

    /* disable any viewports which are conditionally displayed */
    for (v = 0; v < data->num_viewports; v++)
    {
        if (data->viewports[v].hidden_flags&VP_DRAW_HIDEABLE)
        {
            if (data->viewports[v].hidden_flags&VP_DRAW_HIDDEN)
                data->viewports[v].hidden_flags |= VP_DRAW_WASHIDDEN;
            else
                data->viewports[v].hidden_flags |= VP_DRAW_HIDDEN;
        }
    }
    for (v = 0; v < data->num_viewports; v++)
    {
        struct wps_viewport *wps_vp = &(data->viewports[v]);
        unsigned vp_refresh_mode = refresh_mode;
        display->set_viewport(&wps_vp->vp);

#ifdef HAVE_LCD_BITMAP
        /* Set images to not to be displayed */
        for (i = 0; i < MAX_IMAGES; i++)
        {
            data->img[i].display = -1;
        }
#endif
        /* dont redraw the viewport if its disabled */
        if ((wps_vp->hidden_flags&VP_DRAW_HIDDEN))
        {
            if (!(wps_vp->hidden_flags&VP_DRAW_WASHIDDEN))
                display->scroll_stop(&wps_vp->vp);
            wps_vp->hidden_flags |= VP_DRAW_WASHIDDEN;
            continue;
        }
        else if (((wps_vp->hidden_flags&
                   (VP_DRAW_WASHIDDEN|VP_DRAW_HIDEABLE))
                    == (VP_DRAW_WASHIDDEN|VP_DRAW_HIDEABLE)))
        {
            vp_refresh_mode = WPS_REFRESH_ALL;
            wps_vp->hidden_flags = VP_DRAW_HIDEABLE;
        }
        if (vp_refresh_mode == WPS_REFRESH_ALL)
        {
            display->clear_viewport();
        }
            
        for (line = wps_vp->first_line; 
             line <= wps_vp->last_line; line++)
        {
            memset(linebuf, 0, sizeof(linebuf));
            update_line = false;

            /* get current subline for the line */
            new_subline_refresh = update_curr_subline(gwps, line);

            subline_idx = wps_subline_index(data, line,
                                            data->lines[line].curr_subline);
            flags = data->sublines[subline_idx].line_type;

            if (vp_refresh_mode == WPS_REFRESH_ALL || (flags & vp_refresh_mode)
                || new_subline_refresh)
            {
                /* get_line tells us if we need to update the line */
                update_line = get_line(gwps, line, data->lines[line].curr_subline,
                                       &align, linebuf, sizeof(linebuf));
            }
#ifdef HAVE_LCD_BITMAP
            /* peakmeter */
            if (flags & vp_refresh_mode & WPS_REFRESH_PEAK_METER)
            {
                /* the peakmeter should be alone on its line */
                update_line = false;

                int h = font_get(wps_vp->vp.font)->height;
                int peak_meter_y = (line - wps_vp->first_line)* h;

                /* The user might decide to have the peak meter in the last
                    line so that it is only displayed if no status bar is
                    visible. If so we neither want do draw nor enable the
                    peak meter. */
                if (peak_meter_y + h <= display->getheight()) {
                    /* found a line with a peak meter -> remember that we must
                        enable it later */
                    enable_pm = true;
                    peak_meter_enabled = true;
                    peak_meter_screen(gwps->display, 0, peak_meter_y,
                                      MIN(h, display->getheight() - peak_meter_y));
                }
                else
                {
                    peak_meter_enabled = false;
                }
            }

#else /* HAVE_LCD_CHARCELL */

            /* progressbar */
            if (flags & vp_refresh_mode & WPS_REFRESH_PLAYER_PROGRESS)
            {
                if (data->full_line_progressbar)
                    draw_player_fullbar(gwps, linebuf, sizeof(linebuf));
                else
                    draw_player_progress(gwps);
            }
#endif

            if (update_line && 
                /* conditionals clear the line which means if the %Vd is put into the default
                   viewport there will be a blank line.
                   To get around this we dont allow any actual drawing to happen in the
                   deault vp if other vp's are defined */
                ((data->num_viewports>1 && v!=0) || data->num_viewports == 1))
            {
                if (flags & WPS_REFRESH_SCROLL)
                {
                    /* if the line is a scrolling one we don't want to update
                       too often, so that it has the time to scroll */
                    if ((vp_refresh_mode & WPS_REFRESH_SCROLL) || new_subline_refresh)
                        write_line(display, &align, line - wps_vp->first_line, true);
                }
                else
                    write_line(display, &align, line - wps_vp->first_line, false);
            }
        }

#ifdef HAVE_LCD_BITMAP
        /* progressbar */
        if (vp_refresh_mode & WPS_REFRESH_PLAYER_PROGRESS)
        {
            if (wps_vp->pb)
            {
                draw_progressbar(gwps, wps_vp);
            }
        }
        /* Now display any images in this viewport */
        wps_display_images(gwps, &wps_vp->vp);
#endif
    }

#ifdef HAVE_LCD_BITMAP
    data->peak_meter_enabled = enable_pm;
#endif

    if (refresh_mode & WPS_REFRESH_STATUSBAR)
    {
        gwps_draw_statusbars();
    }
    /* Restore the default viewport */
    display->set_viewport(NULL);

    display->update();

    return true;
}