/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2010,2008 Federico Pelupessy
 *               Moon picture from Wikipedia, released to public domain,
 *               (by Tomruen, original upload 5 November 2007)
 *               moon phase code based on moontool.c by John Walker
 *               Extended by: Marek Niemiec, DB1BMN, Dec. 29th. 2009  till Jan. 12 th. 2010
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

/*
moonrock.dat:
 4 byte int: number of frames
 nframe * 4 byte float: phase of each frame
 nframe * 4 byte integer: size of each frame file 
 nframe jpg files
*/

#include "plugin.h"
#include "math.h"
#include <timefuncs.h>
#include "string.h"
#include "lib/jpeg_mem.h"

#include "lib/pluginlib_actions.h"
#include "lib/fixedpoint.h"
#include "lib/feature_wrappers.h"

#include "lib/simple_viewer.h"

#define PICSIZE  MIN(LCD_WIDTH,LCD_HEIGHT)

#define MAXNFRAME   120

#define SHOWIMAGE      defined(HAVE_LCD_BITMAP) && LCD_DEPTH>1    
#define PLUGIN_OTHER   10
#define TIMEZONE_FILE  PLUGIN_APPS_DIR "/moonrock_timezone"
#define DATA_FILE      PLUGIN_APPS_DIR "/moonrock.dat"

const struct button_mapping* plugin_contexts[]={pla_main_ctx};

#define QUIT_ACTION    PLA_EXIT
#define DATE_ACTION    PLA_SELECT
#define TOGGLE_VIEW_ACTION    PLA_CANCEL

static int get_frame_no(double phase,int nframe, float *frame_phase)
{
    int i,no;
    float d,dd;

    if (phase < 0) phase = 0.;
    if (phase > 1) phase = 1.;

    dd = 2.;
    no = -1;
    for (i = 0;i < nframe; i++)
    {
        d = phase-frame_phase[i];
        if (d >= 0.5) d = d-1;
        if (d < -0.5) d = d+1;
        if (d < 0) d=-d;
        if (d < dd) 
        {
            no = i;
            dd = d;
        } 
    }
    return no;
}

/*  Astronomical constants  */
#define EPOCH       2444238.5      /* 1980 January 0.0 */

/*  Constants defining the Sun's apparent orbit  */

#define ELONGE      278.833540     /* Ecliptic longitude of the Sun at epoch 1980.0 */
#define ELONGP      282.596403     /* Ecliptic longitude of the Sun at perigee */
#define ECCENT      0.016718       /* Eccentricity of Earth's orbit */
#define SUNSMAX     1.495985e8     /* Semi-major axis of Earth's orbit, km */
#define SUNANGSIZ   0.533128       /* Sun's angular size, degrees, at
                                      semi-major axis distance */
/*  Elements of the Moon's orbit, epoch 1980.0  */
#define MMLONG      64.975464      /* Moon's mean longitude at the epoch */
#define MMLONGP     349.383063     /* Mean longitude of the perigee at the epoch */
#define MLNODE      151.950429     /* Mean longitude of the node at the epoch */
#define MINC        5.145396       /* Inclination of the Moon's orbit */
#define MECC        0.054900       /* Eccentricity of the Moon's orbit */
#define MANGSIZ     0.5181         /* Moon's angular size at distance a
                                            from Earth */
#define MSMAX       384401.0       /* Semi-major axis of Moon's orbit in km */
#define MPARALLAX   0.9507         /* Parallax at distance a from Earth */
#define SYNMONTH    29.53058868    /* Synodic month (new Moon to new Moon) */
#define LUNATBASE   2423436.0      /* Base date for E. W. Brown's numbered
                                      series of lunations (1923 January 16) */

/*  Properties of the Earth  */

#define EARTHRAD    6378.16        /* Radius of Earth in kilometres */

/*  Handy mathematical functions  */

#define PI 3.14159265358979323846

#define SGN(x) (((x) < 0) ? -1 : ((x) > 0 ? 1 : 0))       /* Extract sign */
#ifndef ABS
#define ABS(x) ((x) < 0 ? (-(x)) : (x))                   /* Absolute val */
#endif
#define FIXANGLE(a)  ((a) < 0 ? 360.*((a)/360.-(long)((a)/360.)+1) : 360.*((a)/360.-(long)((a)/360.)))
                                                          /* angle to 0-360 */
#define FIXRANGLE(a) ((a) < 0 ? 2*PI*((a)/2/PI-(long)((a)/2/PI)+1) : 2*PI*((a)/2/PI-(long)((a)/2/PI)))
                                                          /* angle to 0-2PI */
#define TORAD(d) ((d) * (PI / 180.0))                     /* Deg->Rad     */
#define TODEG(d) ((d) * (180.0 / PI))                     /* Rad->Deg     */

static double fp_sin(double x)
{
    long s,c;
    unsigned long xx;
    xx=0xffffffff*(FIXRANGLE(x)/(2*PI));
    s=fp_sincos(xx,&c);
    return s*(1.0/0x7fffffff);
}

static double fp_cos(double x)
{
    long s,c;
    unsigned long xx;
    xx=0xffffffff*(FIXRANGLE(x)/(2*PI));
    s=fp_sincos(xx,&c);
    return c*(1.0/0x7fffffff);
}

#define dsin(x) (fp_sin(TORAD((x))))                   /* Sin from deg */
#define dcos(x) (fp_cos(TORAD((x))))                   /* Cos from deg */


/* gmtime copied from plugins/lua/gmtime.c */
/* seconds per day */
#define SPD 24*60*60

/* days per month -- nonleap! */
const short __spm[13] =
  { 0,
    (31),
    (31+28),
    (31+28+31),
    (31+28+31+30),
    (31+28+31+30+31),
    (31+28+31+30+31+30),
    (31+28+31+30+31+30+31),
    (31+28+31+30+31+30+31+31),
    (31+28+31+30+31+30+31+31+30),
    (31+28+31+30+31+30+31+31+30+31),
    (31+28+31+30+31+30+31+31+30+31+30),
    (31+28+31+30+31+30+31+31+30+31+30+31),
  };

static inline int isleap(int year) {
  /* every fourth year is a leap year except for century years that are
   * not divisible by 400. */
/*  return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)); */
  return (!(year%4) && ((year%100) || !(year%400)));
}

struct tm *gmtime(const time_t *timep) {
  static struct tm r;
  time_t i;
  register time_t work=*timep%(SPD);
  r.tm_sec=work%60; work/=60;
  r.tm_min=work%60; r.tm_hour=work/60;
  work=*timep/(SPD);
  r.tm_wday=(4+work)%7;
  for (i=1970; ; ++i) {
    register time_t k=isleap(i)?366:365;
    if (work>=k)
      work-=k;
    else
      break;
  }
  r.tm_year=i-1900;
  r.tm_yday=work;

  r.tm_mday=1;
  if (isleap(i) && (work>58)) {
    if (work==59) r.tm_mday=2; /* 29.2. */
    work-=1;
  }

  for (i=11; i && (__spm[i]>work); --i) ;
  r.tm_mon=i;
  r.tm_mday+=work-__spm[i];
  return &r;
}

/* ---------------------*/

/*  JTIME  --  Convert (GMT)  date  and  time  to  astronomical
        Julian   time  (i.e. Julian  date  plus  day  fraction,
        expressed as a double).  */
static double jtime(struct tm *t)
{
    return ( rb->mktime(t) / 86400.0 ) + 2440587.5;
}

static time_t mktimej(double jtime)
{
    return (time_t) ((jtime - 2440587.5) * 86400.0);
}

static struct tm timej(double jtime)
{
    time_t t;
    t=mktimej(jtime);
    return *gmtime(&t);
}

static struct tm tm2utc(struct tm *t, double utc_offset)
{
       struct tm tmp;         
       tmp = timej(jtime(t)- utc_offset/24);
       return tmp;
}


/*  PHASE  --  Calculate phase of moon as a fraction:
        The  argument  is  the  time  for  which  the  phase is
        requested, expressed as a Julian date and fraction.  Returns  the  terminator
        phase  angle  as a percentage of a full circle (i.e., 0 to 1), and
        stores into pointer arguments  the  illuminated  fraction  of the
        Moon's  disc, the Moon's age in days and fraction, the distance
        of the Moon from the centre of the Earth, and  the  angular
        diameter subtended  by the Moon as seen by an observer at the centre of
        the Earth.
*/

static double moon_phase(
        double  pdate,                      /* Date for which to calculate phase */
        double  *pphase,                    /* Illuminated fraction */
        double  *mage,                      /* Age of moon in days */
        double  *dist,                      /* Distance in kilometres */
        double  *angdia,                    /* Angular diameter in degrees */
        double  *sudist,                    /* Distance to Sun */
        double  *suangdia)                  /* Sun's angular diameter */
{
    double Day, N, M, Ec, Lambdasun, ml, MM, /* MN,*/ Ev, Ae, A3, MmP,
           mEc, A4, lP, V, lPP,
           /* NP, y, x, Lambdamoon, BetaM, */
           MoonAge, MoonPhase,
           MoonDist, MoonDFrac, MoonAng,
           /* MoonPar,*/
           F, SunDist, SunAng,Mrad;

    /* Calculation of the Sun's position */

    Day = pdate - EPOCH;                    /* Date within epoch */
    N = FIXANGLE((360 / 365.2422) * Day);   /* Mean anomaly of the Sun */
    M = FIXANGLE(N + ELONGE - ELONGP);      /* Convert from perigee
                                               co-ordinates to epoch 1980.0 */
    Mrad=TORAD(M);
    Ec = Mrad+2*ECCENT*fp_sin(Mrad)+
        1.25*ECCENT*ECCENT*fp_sin(2*Mrad)+
        ECCENT*ECCENT*ECCENT*fp_sin(3*Mrad); /* gives true anomaly accurate to ~arcsec
                                        (compared to usual formula, that is) */
    /*    if(Mrad > PI) Ec=Ec-2*PI; */
    Ec = TODEG(Ec);

    Lambdasun = FIXANGLE(Ec + ELONGP);      /* Sun's geocentric ecliptic
                                                longitude */
    /* Orbital distance factor */
    F = (1 + ECCENT * dcos(Ec)) / (1 - ECCENT * ECCENT);
    SunDist = SUNSMAX / F;                  /* Distance to Sun in km */
    SunAng = F * SUNANGSIZ;                 /* Sun's angular size in degrees */

    /* Calculation of the Moon's position */

    /* Moon's mean longitude */
    ml = FIXANGLE(13.1763966 * Day + MMLONG);

    /* Moon's mean anomaly */
    MM = FIXANGLE(ml - 0.1114041 * Day - MMLONGP);

    /* Moon's ascending node mean longitude */
    /*  MN = FIXANGLE(MLNODE - 0.0529539 * Day); */

    /* Evection */
    Ev = 1.2739 * dsin(2 * (ml - Lambdasun) - MM);

    /* Annual equation */
    Ae = 0.1858 * dsin(M);

    /* Correction term */
    A3 = 0.37 * dsin(M);

    /* Corrected anomaly */
    MmP = MM + Ev - Ae - A3;

    /* Correction for the equation of the centre */
    mEc = 6.2886 * dsin(MmP);

    /* Another correction term */
    A4 = 0.214 * dsin(2 * MmP);

    /* Corrected longitude */
    lP = ml + Ev + mEc - Ae + A4;

    /* Variation */
    V = 0.6583 * dsin(2 * (lP - Lambdasun));

    /* True longitude */
    lPP = lP + V;

    /* Corrected longitude of the node */
    /*    NP = MN - 0.16 * dsin(M); */

    /* Y inclination coordinate */
    /*    y = dsin(lPP - NP) * dcos(MINC); */

    /* X inclination coordinate */
    /*    x = dcos(lPP - NP); */

    /* Ecliptic longitude */
    /*    Lambdamoon = TODEG(atan2(y, x)); */
    /*    Lambdamoon += NP; */

    /* Ecliptic latitude */
    /*    BetaM = TODEG(asin(fp_sin(TORAD(lPP - NP)) * fp_sin(TORAD(MINC)))); */

    /* Calculation of the phase of the Moon */

    /* Age of the Moon in degrees */
    MoonAge = lPP - Lambdasun;

    /* Phase of the Moon */
    MoonPhase = (1 - dcos(MoonAge)) / 2;

    /* Calculate distance of moon from the centre of the Earth */

    MoonDist = (MSMAX * (1 - MECC * MECC)) /
                (1 + MECC * dcos(MmP + mEc));

    /* Calculate Moon's angular diameter */

    MoonDFrac = MoonDist / MSMAX;
    MoonAng = MANGSIZ / MoonDFrac;

    /* Calculate Moon's parallax */
    /*    MoonPar = MPARALLAX / MoonDFrac; */

    *pphase = MoonPhase;
    *mage = SYNMONTH * (FIXANGLE(MoonAge) / 360.0);
    *dist = MoonDist;
    *angdia = MoonAng;
    *sudist = SunDist;
    *suangdia = SunAng;
    return FIXANGLE(MoonAge) / 360.0;
}

#define FIXNANGLE(a) ((a) < 0 ? ((a)-(long)((a))+1) : ((a)-(long)((a))))

static double phase_search_forward(double jd,double target_phase)
{
    double phase,cphase, aom, cdist, cangdia, csund, csuang;
    double low_phase,jdlow,jdhigh;
    double prec=1./(2*30.*24.*3600);    
    int count;
    
    phase=moon_phase(jd, &cphase, &aom, &cdist, &cangdia, &csund, &csuang);
    low_phase=phase;
    jdlow=jd;
    jdhigh=jd;
    count=0;
    while( FIXNANGLE(phase-low_phase) < FIXNANGLE(target_phase-low_phase) ) 
    {
        count++; if(count>5) return -1.;
        jdlow=jdhigh;
        low_phase=phase;
        jdhigh+=10.;
        phase=moon_phase(jdhigh, &cphase, &aom, &cdist, &cangdia, &csund, &csuang);
    }

    while( ABS(phase - target_phase) > prec)
    {
        count++; if(count>50) return -2.;      
        jd=(jdlow+jdhigh)/2;
        phase=moon_phase(jd, &cphase, &aom, &cdist, &cangdia, &csund, &csuang);
        if( FIXNANGLE(phase-low_phase) < FIXNANGLE(target_phase-low_phase) ) 
        {
            jdlow=jd;
            low_phase=phase;
        } else
        {
            jdhigh=jd;
        }
    }
    return jd;
}



#if SHOWIMAGE
#define IMAGE     0
#define INFO      1
#define DATE      2
#define RESET     3
#define TIMEZONE  4
#define QUIT      5 
#else
#define INFO      0
#define DATE      1
#define RESET     2
#define TIMEZONE  3
#define QUIT      4  
#endif

static double utc_offset=0,org_utc_offset=0;
#if SHOWIMAGE    
static int foffset,nframe=0;
static float frame_phase[MAXNFRAME];
static unsigned int frame_size[MAXNFRAME];

static int read_data_info(int fd)
{
    if(fd>=0)
    {
        int ret, cur, i, totsize;
        if(!rb->read(fd,&nframe,sizeof(nframe)))
        {
            rb->splash(HZ, "(1) Corrupt moonrock.dat");
            return PLUGIN_ERROR;
        }
        nframe = letoh32(nframe);
        if(nframe<0 || nframe > MAXNFRAME)
        {
            rb->splash(HZ, "(2) Corrupt moonrock.dat");
            return PLUGIN_ERROR;
        }
        if( !rb->read( fd,&frame_phase,nframe*sizeof( (float) 0 ) ) )
        {
            rb->splash(HZ, "(3) Corrupt moonrock.dat");
            return PLUGIN_ERROR;
        }
        for ( i = 0; i < nframe; i++)
          *(uint32_t *)(frame_phase+i)=letoh32( *(uint32_t *) (frame_phase+i));
        if( !rb->read( fd,&frame_size,nframe*sizeof( (int) 0 ) ) )
        {
            rb->splash(HZ, "(4) Corrupt moonrock.dat");
            return PLUGIN_ERROR;
        }
        for ( i = 0; i < nframe; i++)
          *(frame_size+i) = letoh32( *(frame_size + i));    
        totsize=0;
        for ( i = 0; i < nframe; i++)
          totsize+=frame_size[i];
        cur=rb->lseek(fd,0,SEEK_CUR);
        ret=rb->lseek(fd,0,SEEK_END);          
        if( (ret-cur) != totsize )
        {
            rb->splash(HZ, "(5) Corrupt moonrock.dat");
            return PLUGIN_ERROR;
        }
        foffset= cur;
        return PLUGIN_OK;
    } else
        return PLUGIN_ERROR;
}

static int show_image(int fd,struct tm *tm,bool refresh, int *next_choice)
{
    static struct bitmap input_bmp={
      .width=PICSIZE,
      .height=PICSIZE,           
      .format=FORMAT_NATIVE
    };
    size_t plugin_buf_len;
    unsigned char *jpeg_buf;
    unsigned char * plugin_buf =
        (unsigned char *)rb->plugin_get_buffer(&plugin_buf_len);
    int font_w,font_h,iphase;
    double phase,jd,cphase, aom, cdist, cangdia, csund, csuang;
    int ret;
    int action;
    struct tm utc;

    utc=tm2utc(tm,utc_offset);
    jd=jtime(&utc);
    phase=moon_phase(jd, &cphase, &aom, &cdist, &cangdia, &csund, &csuang);
    
    if(fd>=0)
    {
        rb->lcd_clear_display();
        rb->lcd_getstringsize( (unsigned char *) "Calculating..", &font_w, &font_h);
        rb->lcd_putsxy((LCD_WIDTH/2) - (font_w)/2, LCD_HEIGHT/2-font_h/2,
                        (unsigned char *) "Calculating..");
        rb->lcd_update();

        if(refresh)
        {
            int i,skip;
            iphase=get_frame_no(phase,nframe,frame_phase);
            skip=0; for(i=0; i<iphase;i++) skip+=frame_size[i];

            if (frame_size[iphase] > plugin_buf_len)
              return PLUGIN_ERROR;

            rb->lseek(fd, (off_t) (skip + foffset),SEEK_SET);

            jpeg_buf=plugin_buf;
            rb->read(fd, jpeg_buf, frame_size[iphase]);

            input_bmp.data=(char*) (plugin_buf+frame_size[iphase]);

            ret = decode_jpeg_mem(jpeg_buf,frame_size[iphase], 
                                    &input_bmp, plugin_buf_len-frame_size[iphase],
                                    FORMAT_NATIVE|FORMAT_RESIZE|FORMAT_KEEP_ASPECT,
                                    &format_native);

            if (ret < 0)
            {
                rb->splash(HZ, "Could not decode image");
                return PLUGIN_ERROR;
            }
        }

        rb->lcd_bitmap( (fb_data *)input_bmp.data,
                        MAX(0,(LCD_WIDTH-input_bmp.width)/2),
                        MAX(0,(LCD_HEIGHT-input_bmp.height)/2),
                        input_bmp.width,input_bmp.height);
        rb->lcd_update();

        action=pluginlib_getaction(TIMEOUT_BLOCK, plugin_contexts, 1);
        if(action==QUIT_ACTION) *next_choice=QUIT;
        return PLUGIN_OK;
    } else
        return PLUGIN_ERROR;
}
#endif

static void save_timezone(void)
{
    int fd;
    if(org_utc_offset==utc_offset) return;
    fd = rb->open(TIMEZONE_FILE, O_WRONLY|O_CREAT, 0666);
    if (fd < 0) return;
    rb->write(fd, &utc_offset, sizeof(utc_offset));
    rb->close(fd);
}

static void load_timezone(void)
{
    int fd;
    fd = rb->open(TIMEZONE_FILE, O_RDONLY);
    if (fd < 0) return;
    if (rb->read(fd, &utc_offset, sizeof(utc_offset)) < (long)sizeof(utc_offset)) 
    {  
        utc_offset = 0;
    }
    rb->close(fd);
    org_utc_offset=utc_offset;
}


static int show_info(struct tm *tm)
{
    size_t plugin_buf_len;
    unsigned char * plugin_buf =
        (unsigned char *)rb->plugin_get_buffer(&plugin_buf_len);
    char *output=(char *) plugin_buf;
    int count;    
    static const char *desc[] = {
        "New moon", "Waxing Crescent",
        "First Quarter","Waxing Gibbous",
        "Full Moon","Waning Gibbous","Last Quarter",
        "Waning Crescent"
    };
    int iphase;
    double phase,jd,cphase, aom, cdist, cangdia, csund, csuang;
    double jdfull,jdnew;
    char phase_tendency;
    struct tm utc, tmfull,tmnew;

    utc=tm2utc(tm,utc_offset);
    jd=jtime(&utc);
    phase=moon_phase(jd, &cphase, &aom, &cdist, &cangdia, &csund, &csuang);

    rb->lcd_clear_display();

    iphase=8*phase+0.5;
    iphase=iphase%8;
        
    count=0;
    count+=rb->snprintf(output+count,plugin_buf_len-count, 
                          "UTC time:\n");
    count+=rb->snprintf(output+count,plugin_buf_len-count, 
             "     %04d-%02d-%02d, %02d:%02d:%02d\n",
             utc.tm_year+1900,utc.tm_mon+1,utc.tm_mday, utc.tm_hour, utc.tm_min, utc.tm_sec);
    count+=rb->snprintf(output+count,plugin_buf_len-count, 
                         "Local time: (UTC%s%d.%02d)\n",
                          utc_offset<0?"-":"+",(int) ABS(utc_offset),
                           (int) (ABS(utc_offset)-ABS(utc_offset))*60);
    count+=rb->snprintf(output+count,plugin_buf_len-count, 
             "     %04d-%02d-%02d, %02d:%02d:%02d\n",
             tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    count+=rb->snprintf(output+count,plugin_buf_len-count, "Julian Date:\n");            
    count+=rb->snprintf(output+count,plugin_buf_len-count, "     %ld.%04ld\n",
      (long) jd, (long) (10000*(jd- (long) jd)));            
    if (phase <= 0.5)
    {
       phase_tendency = '+'; 
    }
    else
    {
       phase_tendency = '-'; 
    }
    count+=rb->snprintf(output+count,plugin_buf_len-count,
      "Moon Phase:\n");
    count+=rb->snprintf(output+count,plugin_buf_len-count,
      "     %s (%c%d%%)\n",desc[iphase], phase_tendency, 
      (int)(cphase*100+0.5));
    count+=rb->snprintf(output+count,plugin_buf_len-count,"Distance:\n");
    count+=rb->snprintf(output+count,plugin_buf_len-count,"     %ld km\n",
                    (long)(cdist));

    jdfull=phase_search_forward(jd,0.5);  
    jdnew=phase_search_forward(jd,1.0);  
    tmfull=timej(jdfull+utc_offset/24);
    tmnew=timej(jdnew+utc_offset/24);
    if(jdnew<jdfull)
    {
    count+=rb->snprintf(output+count,plugin_buf_len-count,
      "New Moon:\n");
    count+=rb->snprintf(output+count,plugin_buf_len-count,
      "     %04d-%02d-%02d, %02d:%02d:%02d\n",
       tmnew.tm_year+1900,tmnew.tm_mon+1,tmnew.tm_mday,tmnew.tm_hour, tmnew.tm_min, tmnew.tm_sec);
    count+=rb->snprintf(output+count,plugin_buf_len-count,
      "Full Moon:\n");
    count+=rb->snprintf(output+count,plugin_buf_len-count,
      "     %04d-%02d-%02d, %02d:%02d:%02d\n",
       tmfull.tm_year+1900,tmfull.tm_mon+1,tmfull.tm_mday,tmfull.tm_hour, tmfull.tm_min, tmfull.tm_sec);
    } else
    {
    count+=rb->snprintf(output+count,plugin_buf_len-count,
      "Full Moon:\n");
    count+=rb->snprintf(output+count,plugin_buf_len-count,
      "     %04d-%02d-%02d, %02d:%02d:%02d\n",
       tmfull.tm_year+1900,tmfull.tm_mon+1,tmfull.tm_mday,tmfull.tm_hour, tmfull.tm_min, tmfull.tm_sec);
    count+=rb->snprintf(output+count,plugin_buf_len-count,
      "New Moon:\n");
    count+=rb->snprintf(output+count,plugin_buf_len-count,
      "     %04d-%02d-%02d, %02d:%02d:%02d\n",
       tmnew.tm_year+1900,tmnew.tm_mon+1,tmnew.tm_mday,tmnew.tm_hour, tmnew.tm_min, tmnew.tm_sec);
    }

    view_text("Moon info", output);
    pluginlib_getaction(TIMEOUT_BLOCK, plugin_contexts, 1);
    return PLUGIN_OK;
}

static const char* formatter(char *buffer, size_t buf_len, int val, const
char* unknown)
{
    (void) unknown;
    rb->snprintf(buffer,buf_len,"GMT%s%d.%02d hr", val<0?"":"+",val/2, val%2?30:0);
    return buffer;
}


#if SHOWIMAGE
static int moonrock_main(int fd)
#else
static int moonrock_main(void)
#endif
{
    int selection=0,menu_choice=0,itimezone=0;
    bool menu_quit = false;
    struct tm tm,tm1;
    enum plugin_status ret=PLUGIN_OK;
    bool refresh=true;

    MENUITEM_STRINGLIST(menu, "Moonrock Menu", NULL,
#if SHOWIMAGE
                        "Show image",
#endif                        
                        "Show info","Change time",
                        "Reset to current time", 
                        "Set timezone",
                        "Quit");

    rb->memcpy( &tm, rb->get_time(), sizeof(struct tm));
#if LCD_DEPTH>=2
    rb->lcd_set_backdrop(NULL);
    rb->lcd_set_background(LCD_BLACK);
    rb->lcd_set_foreground(LCD_WHITE);
#endif
    rb->lcd_clear_display();

    while(!menu_quit)
    {
        switch(menu_choice)
        {
#if SHOWIMAGE
            case IMAGE:
                menu_choice=-1;
                if(fd<0)
                {
                    rb->splash(HZ,"No image data");
                    break;
                }
                ret=show_image(fd,&tm,refresh,&menu_choice);
                refresh=false;
                if(ret!=PLUGIN_OK) return ret;
                break;
#endif                        
            case INFO:
              menu_choice=-1;
              ret=show_info(&tm);
              refresh=true;
              if(ret!=PLUGIN_OK) return ret;
              break;
            case DATE:
              menu_choice=-1;
              rb->memcpy( &tm1, &tm, sizeof(struct tm));
              rb->set_time_screen((char*) "Moonrock date/time" ,&tm1);
              if(tm1.tm_year != -1)
              {
                  rb->memcpy( &tm, &tm1, sizeof(struct tm));
                  refresh=true;
              } else
                  rb->splash(HZ, "Time NOT changed");
              break;
            case RESET:
              menu_choice=-1;
              rb->memcpy( &tm, rb->get_time(), sizeof(struct tm));
              rb->splash(HZ, "Time reset");
              refresh=true;
              break;
            case TIMEZONE:
              menu_choice=-1;
              itimezone=(int) utc_offset*2;
              rb->set_int("Timezone", "", UNIT_INT, &itimezone,
                                NULL, 1, -24, 24, formatter );
              utc_offset=itimezone/2.;
              break;
            case QUIT:
              menu_choice=-1;
              return ret;   
        }
        if(menu_choice<0)
          menu_choice=rb->do_menu(&menu, &selection, NULL, false);
    }
    return PLUGIN_ERROR;
}

/* this is the plugin entry point */
enum plugin_status plugin_start(const void* parameter)
{
    (void)parameter;
    enum plugin_status ret=PLUGIN_OK;
#if SHOWIMAGE
    int fd;
    
    fd=rb->open(DATA_FILE, O_RDONLY);
    if(fd>=0)
    {
        ret=read_data_info(fd);
        if(ret!=PLUGIN_OK) 
        {
            rb->close(fd);
            return ret;
        }
    }          
#endif
    load_timezone();
#if SHOWIMAGE
    ret=moonrock_main(fd);
#else
    ret=moonrock_main();
#endif
    save_timezone();
#if SHOWIMAGE
    if(fd>=0) rb->close(fd);
#endif
    return PLUGIN_OK;
}
