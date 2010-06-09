#undef WIN32

#ifndef MIKMOD_SUPP
#define MIKMOD_SUPP 1

#include "codeclib.h"
#include <string.h>
extern struct codec_api *ci;
//#define strncpy     ci->strncpy
#define strstr      ci->strcasestr

#ifndef INT_MAX
#define INT_MAX 32767
#endif

#ifndef SIMULATOR
typedef void FILE;
#endif

#define BMALLOC 1
#ifdef BMALLOC
#define codec_malloc(x)		dmalloc(x)
#define codec_calloc(x,y)		dcalloc(x,y)
#define codec_realloc(x,y)	drealloc(x,y)
#define codec_free(x)			dfree(x)
#endif

#define sprintf(...)
#define fprintf(...)
#define printf(...)     DEBUGF(__VA_ARGS__)
#define fputs(...)
#define puts(...)
#define putchar(...)

#endif

