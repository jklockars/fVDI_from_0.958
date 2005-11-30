/*
 * fVDI device driver specific setup
 *
 * $Id: spec.c,v 1.2 2005-11-30 13:25:17 johan Exp $
 *
 * Copyright 1998-2002, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "fvdi.h"
#include "relocate.h"

#include "os.h"
#include "driver.h"

#if 0
 #define FAST		/* Write in FastRAM buffer */
 #define BOTH		/* Write in both FastRAM and on screen */
#else
 #undef FAST
 #undef BOTH
#endif


char red[] = {6};
char green[] = {6};
char blue[] = {6};
char none[] = {0};

unsigned char tos_colours[] = {0, 1};

Mode mode[1] =
	{{1, CHECK_PREVIOUS, {red, green, blue, none, none, none}, 0, 0, 1, 1}};

extern Device device;

char driver_name[] = "Bitplane (shadow)";

extern Driver *me;
extern Access *access;

extern void *c_write_pixel;
extern void *c_read_pixel;
extern void *c_line_draw;
extern void *c_expand_area;
extern void *c_fill_area;
extern void *c_blit_area;
#if 0
extern void *text_area;
#endif
extern void *c_mouse_draw;
extern void *c_set_colours;
extern void *c_get_colour;

void *write_pixel_r = &c_write_pixel;
void *read_pixel_r  = &c_read_pixel;
void *line_draw_r   = &c_line_draw;
void *expand_area_r = &c_expand_area;
void *fill_area_r   = &c_fill_area;
void *fill_poly_r   = 0;
void *blit_area_r   = &c_blit_area;
void *text_area_r   = 0;
void *mouse_draw_r  = &c_mouse_draw;
void *set_colours_r = &c_set_colours;
void *get_colours_r = 0;
void *get_colour_r  = &c_get_colour;

long wk_extend = 0;

short accel_s = 0;
short accel_c = A_SET_PAL | A_GET_COL | A_SET_PIX | A_GET_PIX |
                A_BLIT | A_FILL | A_EXPAND | A_LINE | A_MOUSE;

#if 0
short graphics_mode = CHECK_PREVIOUS;
#else
Mode *graphics_mode = &mode[0];
#endif

short colour_bits = 18;

int shadow = 0;

short debug = 0;

short depth = 0;


/*
 * Handle any driver specific parameters
 */
void check_token(char *token, const char **ptr)
{
	if (access->funcs.equal(token, "shadow"))
		shadow = 1;
}


/*
 * Do whatever setup work might be necessary on boot up
 * and which couldn't be done directly while loading.
 * Supplied is the default fVDI virtual workstation.
 */
long initialize(Virtual *vwk)
{
	Workstation *wk;
	char *buf;
	int fast_w_bytes;
#if 0
	int i;
#endif

	debug = access->funcs.misc(0, 1, 0);
	
	vwk = me->default_vwk;	/* This is what we're interested in */
	wk = vwk->real_address;

#ifdef FAST
	if (shadow) {
 #if 0			/* It's not clear that this is a good idea */
		fast_w_bytes = (wk->screen.wrap + 15) & 0xfffffff0;
 #else
		fast_w_bytes = wk->screen.wrap;
 #endif
		buf = (char *)access->funcs.malloc(fast_w_bytes * wk->screen.mfdb.height + 255, 1);
		if (buf) {
			wk->screen.shadow.buffer = buf;
			wk->screen.shadow.address = (void *)(((long)buf + 255) & 0xffffff00);
			wk->screen.shadow.wrap = fast_w_bytes;
		} else {
			access->funcs.error("Can't allocate FastRAM!", 0);
			wk->screen.shadow.buffer = 0;
			wk->screen.shadow.address = 0;
		}
 #ifndef BOTH
		wk->screen.mfdb.address = wk->screen.shadow.address;
 #endif
	}
#endif
	if (!wk->screen.shadow.address)
		driver_name[10] = 0;

#if 0
	device.format = mode[depth].format;
	device.clut = mode[depth].clut;
	device.bit_depth = mode[depth].bpp;
	device.dummy1 = (1L << mode[depth].bpp) >> 16;
	device.colours = (1L << mode[depth].bpp) & 0xffff;
#endif
	device.byte_width = wk->screen.wrap;
	device.address = wk->screen.mfdb.address;
#if 0
	device.bits.red = mode[depth].bits.red[0];
	device.bits.green = mode[depth].bits.green[0];
	device.bits.blue = mode[depth].bits.blue[0];
	device.bits.alpha = mode[depth].bits.alpha[0];
	device.bits.genlock = mode[depth].bits.genlock[0];
	device.bits.unused = mode[depth].bits.unused[0];
	device.bits.organization = mode[depth].org;
	device.dummy2 = 0;
	if (device.clut == 2) {
		for(i = 0; i < mode[depth].bits.red[0]; i++)
			device.bitnumber.red[i] = mode[depth].bits.red[i + 1];
		for(i = mode[depth].bits.red[0]; i < 16; i++)
			device.bitnumber.red[i] = -1;		/* Not used */
		for(i = 0; i < mode[depth].bits.green[0]; i++)
			device.bitnumber.green[i] = mode[depth].bits.green[i + 1];
		for(i = mode[depth].bits.green[0]; i < 16; i++)
			device.bitnumber.green[i] = -1;		/* Not used */
		for(i = 0; i < mode[depth].bits.blue[0]; i++)
			device.bitnumber.blue[i] = mode[depth].bits.blue[i + 1];
		for(i = mode[depth].bits.blue[0]; i < 16; i++)
			device.bitnumber.blue[i] = -1;		/* Not used */
		for(i = 0; i < mode[depth].bits.alpha[0]; i++)
			device.bitnumber.alpha[i] = mode[depth].bits.alpha[i + 1];
		for(i = mode[depth].bits.alpha[0]; i < 16; i++)
			device.bitnumber.alpha[i] = -1;		/* Not used */
		for(i = 0; i < mode[depth].bits.genlock[0]; i++)
			device.bitnumber.genlock[i] = mode[depth].bits.genlock[i + 1];
		for(i = mode[depth].bits.genlock[0]; i < 16; i++)
			device.bitnumber.genlock[i] = -1;	/* Not used */
		for(i = 0; i < mode[depth].bits.unused[0]; i++)
			device.bitnumber.unused[i] = mode[depth].bits.unused[i + 1];
		for(i = mode[depth].bits.unused[0]; i < 32; i++)
			device.bitnumber.unused[i] = -1;	/* Not used */
		for(i = 0; i < 144; i++)
			device.reserved[i] = 0;
	} else {
		for(i = 0; i < sizeof(tos_colours); i++)
			device.vdi2pix[i] = tos_colours[i];
		if (mode[depth].bpp == 8) {
			for(; i < 255; i++)
				device.vdi2pix[i] = i;
			device.vdi2pix[255] = 15;
		} else {
			for(; i < 256; i++)
				device.vdi2pix[i] = 0;
		}
	}
#endif

#if 1
	{
	char buf[10];

	access->funcs.ltoa(buf, (long)wk->screen.mfdb.width, 10);
	access->funcs.puts(buf);
	access->funcs.puts("x");
	access->funcs.ltoa(buf, (long)wk->screen.mfdb.height, 10);
	access->funcs.puts(buf);
	access->funcs.puts("x");
	access->funcs.ltoa(buf, (long)wk->screen.mfdb.bitplanes, 10);
	access->funcs.puts(buf);
	access->funcs.puts(" screen at ");
	access->funcs.ltoa(buf, (long)wk->screen.mfdb.address, 16);
	access->funcs.puts(buf);
	access->funcs.puts("\x0a\x0d");
	}
#endif

	return 1;
}

/*
 *
 */
long setup(long type, long value)
{
	long ret;

	ret = -1;
	switch(type) {
#if 0
	case S_SCREEN:
		if (value != -1)
			old_screen = value;
		ret = old_screen;
		break;
	case S_AESBUF:
		if (value != -1)
			aes_buffer = value;
		ret = aes_buffer;
		break;
	case S_DOBLIT:
		if (value != -1)
			blit_fall_back = value;
		ret = blit_fall_back;
		break;
	case S_CACHEIMG:
		if (value != -1)
			cache_img = value;
		ret = cache_img;
		break;
#endif
	case Q_NAME:
		ret = (long)driver_name;
		break;
	}

	return ret;
}

/*
 * Initialize according to parameters (boot and sent).
 * Create new (or use old) Workstation and default Virtual.
 * Supplied is the default fVDI virtual workstation.
 */
Virtual *opnwk(Virtual *vwk)
{
	me->default_vwk->real_address->screen.mfdb.address = (void *)Physbase();

	return 0;
}

/*
 * 'Deinitialize'
 */
void clswk(Virtual *vwk)
{
}
