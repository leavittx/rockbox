#include "codeclib.h"
#include "libmikmod/mikmod.h"

CODEC_HEADER

extern void *dmalloc(size_t);
extern void dfree(void *);
extern void *drealloc(void *, size_t);
extern int add_pool(void *start, size_t size);
extern MREADER *_mm_new_mem_reader(const void *buffer, int len);

#ifdef SIMULATOR
extern void print_used();
#endif

static unsigned char* membuf IBSS_ATTR;
static size_t membufsize;

static unsigned char* reqbuf IBSS_ATTR;
static size_t reqbufsize;

#define SAMPLE_RATE 44100

MODULE *module IBSS_ATTR;
struct MREADER* reader IBSS_ATTR;

long int seekindex[128];

#define CHUNK_SIZE (512*2)
char samples_all[CHUNK_SIZE] IBSS_ATTR;
static unsigned int samples_written;

/* this is the codec entry point */
enum codec_status codec_main(void)
{
    unsigned int st;
    unsigned int i;
    unsigned int length;
    int scratch_hid;

    int status = CODEC_OK;
    
    if (codec_init()) {
        DEBUGF("codec init failed\n");
        return CODEC_ERROR;
    }
    
#ifdef BMALLOC
    membuf = ci->codec_get_buffer(&membufsize);
    add_pool(membuf, membufsize);
    
    membufsize = (1048576 * MEM) / 4;  //todo - find a better way to get this value
    membuf = ci->get_scratch(membufsize, &scratch_hid);
    DEBUGF("scratch_hid %d\n", scratch_hid);
    if ( scratch_hid > 0 ) {
        add_pool(membuf, membufsize);
    }
#endif

    MikMod_RegisterDriver(&drv_nos);
    MikMod_RegisterLoader(&load_xm);
    MikMod_RegisterLoader(&load_it);
    MikMod_RegisterLoader(&load_s3m);
    MikMod_RegisterLoader(&load_mod);
    
    md_mixfreq = SAMPLE_RATE;
    
    if (MikMod_Init(""))  {
        DEBUGF("Unable to Init: %s\n", MikMod_strerror(MikMod_errno));
        return CODEC_ERROR;
    }
    
next_track:
    
    while (!*ci->taginfo_ready && !ci->stop_codec)
        ci->sleep(1);
        
    codec_set_replaygain(ci->id3);
    
    samples_written = 0;
    ci->memset(samples_all, 0, sizeof(samples_all));
    ci->seek_buffer(0);
    
    reqbuf = ci->request_buffer(&reqbufsize, ci->filesize);
    if (!reqbuf || (size_t)reqbufsize < (size_t)ci->filesize) {
        DEBUGF("Unable to load file\n");
        status = CODEC_ERROR;
        goto quit;
    }

    reader = _mm_new_mem_reader(reqbuf, ci->filesize);
    if ( reader == NULL ) {
        DEBUGF("Unable to create new reader: %s\n", MikMod_strerror(MikMod_errno));
        status = CODEC_ERROR;
        goto quit;
    }

    module = Player_LoadGeneric(reader, 64, 0);
    if ( module == NULL ) {
        DEBUGF("Unable to load module: %s\n", MikMod_strerror(MikMod_errno));
        status = CODEC_ERROR;
        goto quit;
    }
    
    DEBUGF("Loaded %s\n", ci->id3->path);
    
#ifdef SIMULATOR
    print_used();
#endif

    module->loop = module->wrap = 0;

    Player_Start(module);
    
    st = ((1.0 / ((module->bpm * 2) / 5)) * 1000) * module->sngspd;
    length = 0;
    for( i=0; i<(int)module->numpos; i++ ) {
        Player_SetPosition(i);
        seekindex[i] = length;
        length += module->pattrows[module->positions[i]] * st;
    }
    ci->id3->length = length;
    Player_SetPosition(0);
    
    ci->configure(DSP_SET_FREQUENCY, SAMPLE_RATE);
    ci->configure(DSP_SET_SAMPLE_DEPTH, 16);
    ci->configure(DSP_SET_STEREO_MODE, STEREO_INTERLEAVED);
    
    ci->set_elapsed(0);

    while ( Player_Active() )
    {
        ci->yield();
        if (ci->stop_codec || ci->new_track) {
            break;
        }

        if (ci->seek_time) {
            /* New time is ready in ci->seek_time */
            i=0;
            while( i<(int)module->numpos && seekindex[i] < ci->seek_time-1 ) {
                i++;
            }
            Player_SetPosition(i);
            module->sngtime = (seekindex[i]/1000) << 10;

            ci->seek_complete();
            ci->set_elapsed(seekindex[i]);
            ci->seek_complete();
        }
        else {
            ci->set_elapsed((int)(module->sngtime >> 10)*1000);
        }

        samples_written = VC_WriteBytes(samples_all, CHUNK_SIZE) >> 2;
        ci->pcmbuf_insert(samples_all, NULL, samples_written);
    }

    Player_Stop();
    Player_Free(module);
    free(reader);
    DEBUGF("Unloaded %s\n", ci->id3->path);
    
#ifdef SIMULATOR
    print_used();
#endif

    if (ci->request_next_track())
        goto next_track;

quit:
    MikMod_Exit();

    if ( scratch_hid > 0 ) {
        ci->close_scratch(scratch_hid);
        DEBUGF("closing scratch %d\n", scratch_hid);
    }
    
    return status;
}
