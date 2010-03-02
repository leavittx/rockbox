/* c-ray-f - a simple raytracing filter.
 * Copyright (C) 2006 John Tsiombikas <nuclear@siggraph.org>
 *
 * You are free to use, modify and redistribute this program under the
 * terms of the GNU General Public License v2 or (at your option) later.
 * see "http://www.gnu.org/licenses/gpl.txt" for details.
 * ---------------------------------------------------------------------
 * Scene file format:
 *   # sphere (many)
 *   s  x y z  rad   r g b   shininess   reflectivity
 *   # light (many)
 *   l  x y z
 *   # camera (one)
 *   c  x y z  fov   tx ty tz
 * ---------------------------------------------------------------------
 */

/* Rockbox specific headers */
#include "plugin.h"
#include "lib/pluginlib_actions.h"
#include "lib/helper.h"

/* you cant just include headers from other plugins
 * like #include "pdbox/pdbox.h"
 * So I've stolen some functions from pdbox
 * For math */
#include "lib/pdbox-lib.h"

/* For memory allocation */
#include "codecs/lib/tlsf/src/tlsf.h"

//#define ALIGN(stack, size) ((stack) += ((size) - (long)(stack)) & ((size) - 1));
#define PTR_MASK    (sizeof(void *) - 1)

/* In the original version of c-ray we have some type defenitions
 * (uint8_t, uint32_t), this will make everything for us */
#include <inttypes.h>

/* This macros must always be included. 
 * Should be placed at the top by convention,
 * although the actual position doesn't matter */
PLUGIN_HEADER

//#define MEMORY_POOL_SIZE 307200
#define SLEEP_TIME 10
enum { xres = 320, yres = 240 };
bool DEBUG = false;

struct vec3 {
	double x, y, z;
};

struct ray {
	struct vec3 orig, dir;
};

struct material {
	struct vec3 col;	/* color */
	double spow;		/* specular power */
	double refl;		/* reflection intensity */
};

struct sphere {
	struct vec3 pos;
	double rad;
	struct material mat;
	struct sphere *next;
};

struct spoint {
	struct vec3 pos, normal, vref;	/* position, normal and view reflection */
	double dist;		/* parametric distance of intersection along the ray */
};

struct camera {
	struct vec3 pos, targ;
	double fov;
};

void render(int xsz, int ysz, uint32_t *fb, int samples);
struct vec3 trace(struct ray ray, int depth);
struct vec3 shade(struct sphere *obj, struct spoint *sp, int depth);
struct vec3 reflect(struct vec3 v, struct vec3 n);
struct vec3 cross_product(struct vec3 v1, struct vec3 v2);
struct ray get_primary_ray(int x, int y, int sample);
struct vec3 get_sample_pos(int x, int y, int sample);
struct vec3 jitter(int x, int y, int s);
int ray_sphere(const struct sphere *sph, struct ray ray, struct spoint *sp);
void load_scene(int fd);
unsigned long get_msec(void);

#define MAX_LIGHTS		16				/* maximum number of lights */
#define RAY_MAG			1000.0			/* trace rays of this magnitude */
#define MAX_RAY_DEPTH	5				/* raytrace recursion limit */
#define FOV				0.78539816		/* field of view in rads (pi/4) */
#define HALF_FOV		(FOV * 0.5)
#define ERR_MARGIN		1e-6			/* an arbitrary error margin to avoid surface acne */

/* bit-shift ammount for packing each color into a 32bit uint */
#ifdef ROCKBOX_LITTLE_ENDIAN
#define RSHIFT	16
#define BSHIFT	0
#else	/* big endian */
#define RSHIFT	0
#define BSHIFT	16
#endif	/* endianess */
#define GSHIFT	8	/* this is the same in both byte orders */

/* some helpful macros... */
#define SQ(x)		((x) * (x))
/* We already have these macroses in 'firmware/export/system.h'
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define MIN(a, b)	((a) < (b) ? (a) : (b)) */
#define DOT(a, b)	((a).x * (b).x + (a).y * (b).y + (a).z * (b).z)
#define NORMALIZE(a)  do {\
	double len = rb_sqrt(DOT(a, a));\
	(a).x /= len; (a).y /= len; (a).z /= len;\
} while(0);

/* global state */
/* not using this now
int xres = 800;
int yres = 600; */
double aspect = 1.333333;
struct sphere *object_list; /* obj_list -- name conflict with 'pdbox/PDa/src/m_pd.h' */
struct vec3 lights[MAX_LIGHTS];
int lnum = 0;
struct camera cam;

#define NRAN	1024
#define JMASK	(NRAN - 1) /* Originally simply 'MASK', due to name conflict changed to 'JMASK' */
struct vec3 urand[NRAN];
int irand[NRAN];
/*
const char *usage = {
	"Usage: c-ray-f [options]\n"
	"  Reads a scene file from stdin, writes the image to stdout, and stats to stderr.\n\n"
	"Options:\n"
	"  -s WxH	 where W is the width and H the height of the image\n"
	"  -r <rays>  shoot <rays> rays per pixel (antialiasing)\n"
	"  -i <file>  read from <file> instead of stdin\n"
	"  -o <file>  write to <file> instead of stdout\n"
	"  -h		 this help screen\n\n"
};*/

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
#endif /* #ifdef HAVE_LCD_COLOR */

int plugin_main(void) {
	int i, j;
	unsigned long rend_time, start_time;
	uint32_t *pixels;
	//uint32_t pixels[xres * yres * sizeof(uint32_t)];
	int rays_per_pixel = 1;
	int infile, outfile;
	//unsigned char memory_pool[MEMORY_POOL_SIZE];
	unsigned char *memory_pool;
	
/*
	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-' && argv[i][2] == 0) {
			char *sep;
			switch(argv[i][1]) {
			case 's':
				if(!isdigit(argv[++i][0]) || !(sep = strchr(argv[i], 'x')) || !isdigit(*(sep + 1))) {
					fputs("-s must be followed by something like \"640x480\"\n", stderr);
					return EXIT_FAILURE;
				}
				xres = atoi(argv[i]);
				yres = atoi(sep + 1);
				aspect = (double)xres / (double)yres;
				break;

			case 'i':
				if(!(infile = fopen(argv[++i], "r"))) {
					fprintf(stderr, "failed to open input file %s: %s\n", argv[i], strerror(errno));
					return EXIT_FAILURE;
				}
				break;

			case 'o':
				if(!(outfile = fopen(argv[++i], "w"))) {
					fprintf(stderr, "failed to open output file %s: %s\n", argv[i], strerror(errno));
					return EXIT_FAILURE;
				}
				break;

			case 'r':
				if(!isdigit(argv[++i][0])) {
					fputs("-r must be followed by a number (rays per pixel)\n", stderr);
					return EXIT_FAILURE;
				}
				rays_per_pixel = atoi(argv[i]);
				break;

			case 'h':
				fputs(usage, stdout);
				return 0;
				
			default:
				fprintf(stderr, "unrecognized argument: %s\n", argv[i]);
				fputs(usage, stderr);
				return EXIT_FAILURE;
			}
		} else {
			fprintf(stderr, "unrecognized argument: %s\n", argv[i]);
			fputs(usage, stderr);
			return EXIT_FAILURE;
		}
	}
	*/

/* We will use static memory allocation, at least for now */
/*	if(!(pixels = malloc(xres * yres * sizeof *pixels))) {
		perror("pixel buffer allocation failed");
		return EXIT_FAILURE;
	}
*/

	size_t size;
	memory_pool = rb->plugin_get_audio_buffer(&size);
	if (DEBUG)
		rb->splashf(HZ*5, "plugin_get_audio_buffer() allocated %lu bytes", size);
	//ALIGN(memory_pool, sizeof(uint32_t));
	while ((unsigned long)memory_pool & PTR_MASK)
	{
		memory_pool ++;
		size --;
	}
	init_memory_pool(size, memory_pool);
	pixels = tlsf_malloc(xres * yres * sizeof(uint32_t));
	
	if ((infile = rb->open(PLUGIN_DEMOS_DIR "/scene.txt", O_RDONLY)) < 0)
	{
		rb->splash(HZ*3, "Can't open scene file :(");
		return PLUGIN_ERROR;
	}
	
	if ((outfile = rb->open(PLUGIN_DEMOS_DIR "/out.pnm", O_WRONLY|O_CREAT)) < 0)
	{
		rb->close(infile);
		rb->splash(HZ*3, "Can't create output file :(");
		return PLUGIN_ERROR;
	}
	
	if (DEBUG)
	{
		rb->lcd_clear_display();
		rb->lcd_update();
		rb->splashf(HZ*3, "This thing is %s, INT_MAX = %iu, sizeof(uint32_t) = %lu",
					(RSHIFT == 16 && BSHIFT == 0) ? "little-endian" : "big-endian",
					INT_MAX, sizeof(uint32_t));
	}
	
	//init_memory_pool(MEMORY_POOL_SIZE, memory_pool);
	
	if (DEBUG)
	{
		rb->lcd_clear_display();
		rb->lcd_update();
		rb->splash(HZ/2, "Will load scene now...");
	}
	
	load_scene(infile);
	
	if (DEBUG)
	{
		rb->lcd_clear_display();
		rb->lcd_update();
		rb->splash(HZ/2, "Scene loaded, rendering...");
	}

	/* initialize the random number tables for the jitter */
	rb->srand(*rb->current_tick);
	for(i=0; i<NRAN; i++) urand[i].x = (double)rb->rand() / RAND_MAX - 0.5;
	for(i=0; i<NRAN; i++) urand[i].y = (double)rb->rand() / RAND_MAX - 0.5;
	for(i=0; i<NRAN; i++) irand[i] = (uint32_t)(NRAN * ((double)rb->rand() / RAND_MAX));
	
	start_time = get_msec();
	render(xres, yres, pixels, rays_per_pixel);
	rend_time = get_msec() - start_time;
	
	/* output statistics */
	rb->lcd_clear_display();
	rb->lcd_update();
	rb->splashf(HZ*3, "Rendering took: %lu seconds (%lu milliseconds)", rend_time / 1000, rend_time);

	uint32_t W, H;
	FOR_NB_SCREENS(j)
	{
#ifdef HAVE_LCD_COLOR
		struct screen *display = rb->screens[j];
			
		W = display->getwidth();
		H = display->getheight();
		
		if (DEBUG)
		{
			rb->lcd_clear_display();
			rb->lcd_update();
			rb->splashf(HZ*2, "W = %lu, H = %lu, Screen number = %i", W, H, j);
		}
#endif
	}

	/* output and draw the image */
	rb->lcd_clear_display();
	
	rb->fdprintf(outfile, "P6\n%d %d\n255\n", xres, yres);
	for (i = 0; i < xres*yres; i++) {
	/*	Original
		fputc((pixels[i] >> RSHIFT) & 0xff, outfile);
		fputc((pixels[i] >> GSHIFT) & 0xff, outfile);
		fputc((pixels[i] >> BSHIFT) & 0xff, outfile);	*/
		
	/*	We need a buffer for write() call, so this is wrong
	    rb->write(outfile, (pixels[i] >> RSHIFT) & 0xff, 1);
		rb->write(outfile, (pixels[i] >> GSHIFT) & 0xff, 1);
		rb->write(outfile, (pixels[i] >> BSHIFT) & 0xff, 1);	*/
		
		unsigned char tmp;
		struct Color color;
		
		tmp = (pixels[i] >> RSHIFT) & 0xff;
		rb->write(outfile, &tmp, 1);
		color.r = tmp;
		
		tmp = (pixels[i] >> GSHIFT) & 0xff;
		rb->write(outfile, &tmp, 1);
		color.g = tmp;
		
		tmp = (pixels[i] >> BSHIFT) & 0xff;
		rb->write(outfile, &tmp, 1);
		color.b = tmp;
		
		FOR_NB_SCREENS(j)
		{
			struct screen *display = rb->screens[j];
			
			color_apply(&color, display);
			display->drawpixel(i % xres, i / xres);
			
			/* Too slow? */
			//display->update();
		}
	}
	
	/* We don't have such thing on rockbox... */
	/* fflush(outfile); */

	rb->close(infile);
	rb->close(outfile);
	
	FOR_NB_SCREENS(j)
	{
		struct screen *display = rb->screens[j];
		display->update();
	}
	
	int action;
    while (true)
    {
		rb->sleep(SLEEP_TIME);
			
		action = pluginlib_getaction(TIMEOUT_NOBLOCK,
									 plugin_contexts,
									 NB_ACTION_CONTEXTS);
		switch (action)
		{
			case PLA_QUIT:
				cleanup(NULL);
				return PLUGIN_OK;
				
			default:
				if (rb->default_event_handler_ex(action, cleanup, NULL)
					== SYS_USB_CONNECTED)
					return PLUGIN_USB_CONNECTED;
				break;
        }
    }
}

/* render a frame of xsz/ysz dimensions into the provided framebuffer */
void render(int xsz, int ysz, uint32_t *fb, int samples) {
	int i, j, s;
	double rcp_samples = 1.0 / (double)samples;

	/* for each subpixel, trace a ray through the scene, accumulate the
	 * colors of the subpixels of each pixel, then pack the color and
	 * put it into the framebuffer.
	 * XXX: assumes contiguous scanlines with NO padding, and 32bit pixels.
	 */
	for(j=0; j<ysz; j++) {
		for(i=0; i<xsz; i++) {
			double r, g, b;
			r = g = b = 0.0;
			
			for(s=0; s<samples; s++) {
				struct vec3 col = trace(get_primary_ray(i, j, s), 0);
				r += col.x;
				g += col.y;
				b += col.z;
			}

			r = r * rcp_samples;
			g = g * rcp_samples;
			b = b * rcp_samples;
				
			*fb++ = ((uint32_t)(MIN(r, 1.0) * 255.0) & 0xff) << RSHIFT |
					((uint32_t)(MIN(g, 1.0) * 255.0) & 0xff) << GSHIFT |
					((uint32_t)(MIN(b, 1.0) * 255.0) & 0xff) << BSHIFT;
		}
	}
}

/* trace a ray throught the scene recursively (the recursion happens through
 * shade() to calculate reflection rays if necessary).
 */
struct vec3 trace(struct ray ray, int depth) {
	struct vec3 col;
	struct spoint sp, nearest_sp;
	struct sphere *nearest_obj = 0;
	struct sphere *iter = object_list->next;

	/* if we reached the recursion limit, bail out */
	if(depth >= MAX_RAY_DEPTH) {
		col.x = col.y = col.z = 0.0;
		return col;
	}
	
	/* find the nearest intersection ... */
	while(iter) {
		if(ray_sphere(iter, ray, &sp)) {
			if(!nearest_obj || sp.dist < nearest_sp.dist) {
				nearest_obj = iter;
				nearest_sp = sp;
			}
		}
		iter = iter->next;
	}

	/* and perform shading calculations as needed by calling shade() */
	if(nearest_obj) {
		col = shade(nearest_obj, &nearest_sp, depth);
	} else {
		col.x = col.y = col.z = 0.0;
	}

	return col;
}

/* Calculates direct illumination with the phong reflectance model.
 * Also handles reflections by calling trace again, if necessary.
 */
struct vec3 shade(struct sphere *obj, struct spoint *sp, int depth) {
	int i;
	struct vec3 col = {0, 0, 0};

	/* for all lights ... */
	for(i=0; i<lnum; i++) {
		double ispec, idiff;
		struct vec3 ldir;
		struct ray shadow_ray;
		struct sphere *iter = object_list->next;
		int in_shadow = 0;

		ldir.x = lights[i].x - sp->pos.x;
		ldir.y = lights[i].y - sp->pos.y;
		ldir.z = lights[i].z - sp->pos.z;

		shadow_ray.orig = sp->pos;
		shadow_ray.dir = ldir;

		/* shoot shadow rays to determine if we have a line of sight with the light */
		while(iter) {
			if(ray_sphere(iter, shadow_ray, 0)) {
				in_shadow = 1;
				break;
			}
			iter = iter->next;
		}

		/* and if we're not in shadow, calculate direct illumination with the phong model. */
		
		if(!in_shadow) {
			NORMALIZE(ldir);

			idiff = MAX(DOT(sp->normal, ldir), 0.0);
		/*	ispec = obj->mat.spow > 0.0 ? pow(MAX(DOT(sp->vref, ldir), 0.0), obj->mat.spow) : 0.0; */
			/* btw: pow(x, y ) = exp(y * log(x)) */
			ispec = obj->mat.spow > 0.0 ? rb_pow(MAX(DOT(sp->vref, ldir), 0.0), obj->mat.spow) : 0.0;

			col.x += idiff * obj->mat.col.x + ispec;
			col.y += idiff * obj->mat.col.y + ispec;
			col.z += idiff * obj->mat.col.z + ispec;
		}
	}

	/* Also, if the object is reflective, spawn a reflection ray, and call trace()
	 * to calculate the light arriving from the mirror direction.
	 */
	if(obj->mat.refl > 0.0) {
		struct ray ray;
		struct vec3 rcol;

		ray.orig = sp->pos;
		ray.dir = sp->vref;
		ray.dir.x *= RAY_MAG;
		ray.dir.y *= RAY_MAG;
		ray.dir.z *= RAY_MAG;

		rcol = trace(ray, depth + 1);
		col.x += rcol.x * obj->mat.refl;
		col.y += rcol.y * obj->mat.refl;
		col.z += rcol.z * obj->mat.refl;
	}

	return col;
}

/* calculate reflection vector */
struct vec3 reflect(struct vec3 v, struct vec3 n) {
	struct vec3 res;
	double dot = v.x * n.x + v.y * n.y + v.z * n.z;
	res.x = -(2.0 * dot * n.x - v.x);
	res.y = -(2.0 * dot * n.y - v.y);
	res.z = -(2.0 * dot * n.z - v.z);
	return res;
}

struct vec3 cross_product(struct vec3 v1, struct vec3 v2) {
	struct vec3 res;
	res.x = v1.y * v2.z - v1.z * v2.y;
	res.y = v1.z * v2.x - v1.x * v2.z;
	res.z = v1.x * v2.y - v1.y * v2.x;
	return res;
}

/* determine the primary ray corresponding to the specified pixel (x, y) */
struct ray get_primary_ray(int x, int y, int sample) {
	struct ray ray;
	float m[3][3];
	struct vec3 i, j = {0, 1, 0}, k, dir, orig, foo;

	k.x = cam.targ.x - cam.pos.x;
	k.y = cam.targ.y - cam.pos.y;
	k.z = cam.targ.z - cam.pos.z;
	NORMALIZE(k);

	i = cross_product(j, k);
	j = cross_product(k, i);
	m[0][0] = i.x; m[0][1] = j.x; m[0][2] = k.x;
	m[1][0] = i.y; m[1][1] = j.y; m[1][2] = k.y;
	m[2][0] = i.z; m[2][1] = j.z; m[2][2] = k.z;
	
	ray.orig.x = ray.orig.y = ray.orig.z = 0.0;
	ray.dir = get_sample_pos(x, y, sample);
	ray.dir.z = 1.0 / HALF_FOV;
	ray.dir.x *= RAY_MAG;
	ray.dir.y *= RAY_MAG;
	ray.dir.z *= RAY_MAG;
	
	dir.x = ray.dir.x + ray.orig.x;
	dir.y = ray.dir.y + ray.orig.y;
	dir.z = ray.dir.z + ray.orig.z;
	foo.x = dir.x * m[0][0] + dir.y * m[0][1] + dir.z * m[0][2];
	foo.y = dir.x * m[1][0] + dir.y * m[1][1] + dir.z * m[1][2];
	foo.z = dir.x * m[2][0] + dir.y * m[2][1] + dir.z * m[2][2];

	orig.x = ray.orig.x * m[0][0] + ray.orig.y * m[0][1] + ray.orig.z * m[0][2] + cam.pos.x;
	orig.y = ray.orig.x * m[1][0] + ray.orig.y * m[1][1] + ray.orig.z * m[1][2] + cam.pos.y;
	orig.z = ray.orig.x * m[2][0] + ray.orig.y * m[2][1] + ray.orig.z * m[2][2] + cam.pos.z;

	ray.orig = orig;
	ray.dir.x = foo.x + orig.x;
	ray.dir.y = foo.y + orig.y;
	ray.dir.z = foo.z + orig.z;
	
	return ray;
}


struct vec3 get_sample_pos(int x, int y, int sample) {
	struct vec3 pt;
	/*double xsz = 2.0, ysz = xres / aspect;*/
	static double sf = 0.0;

	if(sf == 0.0) {
		sf = 2.0 / (double)xres;
	}

	pt.x = ((double)x / (double)xres) - 0.5;
	pt.y = -(((double)y / (double)yres) - 0.65) / aspect;

	if(sample) {
		struct vec3 jt = jitter(x, y, sample);
		pt.x += jt.x * sf;
		pt.y += jt.y * sf / aspect;
	}
	return pt;
}

/* jitter function taken from Graphics Gems I. */
struct vec3 jitter(int x, int y, int s) {
	struct vec3 pt;
	pt.x = urand[(x + (y << 2) + irand[(x + s) & JMASK]) & JMASK].x;
	pt.y = urand[(y + (x << 2) + irand[(y + s) & JMASK]) & JMASK].y;
	return pt;
}

/* Calculate ray-sphere intersection, and return {1, 0} to signify hit or no hit.
 * Also the surface point parameters like position, normal, etc are returned through
 * the sp pointer if it is not NULL.
 */
int ray_sphere(const struct sphere *sph, struct ray ray, struct spoint *sp) {
	double a, b, c, d, sqrt_d, t1, t2;
	
	a = SQ(ray.dir.x) + SQ(ray.dir.y) + SQ(ray.dir.z);
	b = 2.0 * ray.dir.x * (ray.orig.x - sph->pos.x) +
				2.0 * ray.dir.y * (ray.orig.y - sph->pos.y) +
				2.0 * ray.dir.z * (ray.orig.z - sph->pos.z);
	c = SQ(sph->pos.x) + SQ(sph->pos.y) + SQ(sph->pos.z) +
				SQ(ray.orig.x) + SQ(ray.orig.y) + SQ(ray.orig.z) +
				2.0 * (-sph->pos.x * ray.orig.x - sph->pos.y * ray.orig.y - sph->pos.z * ray.orig.z) - SQ(sph->rad);
	
	if((d = SQ(b) - 4.0 * a * c) < 0.0) return 0;

	//sqrt_d = rb->fp_sqrt(d, 16); wtf???
	sqrt_d = rb_sqrt(d);
	t1 = (-b + sqrt_d) / (2.0 * a);
	t2 = (-b - sqrt_d) / (2.0 * a);

	if((t1 < ERR_MARGIN && t2 < ERR_MARGIN) || (t1 > 1.0 && t2 > 1.0)) return 0;

	if(sp) {
		if(t1 < ERR_MARGIN) t1 = t2;
		if(t2 < ERR_MARGIN) t2 = t1;
		sp->dist = t1 < t2 ? t1 : t2;
		
		sp->pos.x = ray.orig.x + ray.dir.x * sp->dist;
		sp->pos.y = ray.orig.y + ray.dir.y * sp->dist;
		sp->pos.z = ray.orig.z + ray.dir.z * sp->dist;
		
		sp->normal.x = (sp->pos.x - sph->pos.x) / sph->rad;
		sp->normal.y = (sp->pos.y - sph->pos.y) / sph->rad;
		sp->normal.z = (sp->pos.z - sph->pos.z) / sph->rad;

		sp->vref = reflect(ray.dir, sp->normal);
		NORMALIZE(sp->vref);
	}
	return 1;
}

/* Load the scene from an extremely simple scene description file */
#define DELIM	" \t\n"
/* In rockbox we don't have 'FILE *' type,
 * we will use file descriptor instead */
void load_scene(int fd) {
	char line[256], *ptr, type;
	char *store = NULL;
	
	//store = tlsf_malloc(256);

	object_list = tlsf_malloc(sizeof(struct sphere));
	
	object_list->next = 0;
	
	while((rb->read_line(fd, line, 256)) > 0) {
		int i;
		struct vec3 pos, col;
		double rad, spow, refl;
		
		ptr = line;
		
		while(*ptr == ' ' || *ptr == '\t') ptr++;
		if(*ptr == '#' || *ptr == '\n') continue;

		if(!(ptr = rb->strtok_r(line, DELIM, &store))) continue;
		type = *ptr;
		
		for(i=0; i<3; i++) {
			if(!(ptr = rb->strtok_r(0, DELIM, &store))) break;
			*((double*)&pos.x + i) = rb_atof(ptr);
		}

		if(type == 'l') {
			if (DEBUG)
			{
				rb->lcd_clear_display();
				rb->lcd_update();
				rb->splash(HZ/2, "Light");
			}
			
			lights[lnum++] = pos;
			continue;
		}

		if(!(ptr = rb->strtok_r(0, DELIM, &store))) continue;
		rad = rb_atof(ptr);

		for(i=0; i<3; i++) {
			if(!(ptr = rb->strtok_r(0, DELIM, &store))) break;
			*((double*)&col.x + i) = rb_atof(ptr);
		}

		if(type == 'c') {
			if (DEBUG)
			{
				rb->lcd_clear_display();
				rb->lcd_update();
				rb->splash(HZ/2, "Camera");
			}
			
			cam.pos = pos;
			cam.targ = col;
			cam.fov = rad;
			continue;
		}

		if(!(ptr = rb->strtok_r(0, DELIM, &store))) continue;
		spow = rb_atof(ptr);

		if(!(ptr = rb->strtok_r(0, DELIM, &store))) continue;
		refl = rb_atof(ptr);

		if(type == 's') {
			if (DEBUG)
			{
				rb->lcd_clear_display();
				rb->lcd_update();
				rb->splash(HZ/2, "Sphere");
			}
			
			struct sphere *sph = tlsf_malloc(sizeof *sph);
			sph->next = object_list->next;
			object_list->next = sph;

			sph->pos = pos;
			sph->rad = rad;
			sph->mat.col = col;
			sph->mat.spow = spow;
			sph->mat.refl = refl;
		} else {
			
			rb->lcd_clear_display();
			rb->lcd_update();
			
			rb->splashf(HZ*3, "unknown type: %c", type);
		}
	}
}

/* provide a millisecond-resolution timer for rockbox */
unsigned long get_msec(void) {
	return *rb->current_tick * 1000 / HZ;
}

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
