#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>

#include "system.h"
#include "metadata.h"
#include "metadata_common.h"
#include "metadata_parsers.h"
#include "rbunicode.h"


bool get_mikmod_metadata(int fd, struct mp3entry* id3, const char *filename)
{    
    /* Use the trackname part of the id3 structure as a temporary buffer */
    unsigned char buf[64];
    int read_bytes;
    char *p;
    char *end;
      

    if ((lseek(fd, 0, SEEK_SET) < 0) 
         || ((read_bytes = read(fd, buf, sizeof(buf))) < 64))
    {
        return false;
    }

    p = id3->id3v2buf;

    const char *ext = strrchr( filename, '.' );
    int tlength, toffset;

    if ( strcasecmp(ext, ".xm") == 0 )
    {
        toffset = 0x11;
        tlength = 20;
    }
    else if ( strcasecmp(ext, ".it") == 0 )
    {
        toffset = 0x04;
        tlength = 25;
    }
    else if ( strcasecmp(ext, ".s3m") == 0 )
    {
        toffset = 0x00;
        tlength = 28;
    }
    else
    {
        toffset = 0x00;
        tlength = 28;
    }
    
    memcpy(p, &buf[toffset], tlength);

    while (isspace(*p)) p++;
    end = p + strlen(p) - 1;
    while (end > p && isspace(*end)) end--;
    *(end+1) = '\0';

    id3->title = p;
    id3->bitrate = filesize(fd)/1024; /* size in kb */
    id3->frequency = 44100;
    id3->length = 120*1000;
    id3->vbr = false;
    id3->filesize = filesize(fd);
    
    return true;
}
