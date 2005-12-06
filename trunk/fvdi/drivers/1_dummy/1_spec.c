/*
 * fVDI device driver specific setup
 *
 * Copyright 1998-2000, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "fvdi.h"
#include "relocate.h"

#include "driver.h"


char red[] = {6};
char green[] = {6};
char blue[] = {6};
char none[] = {0};

unsigned char tos_colours[] = {0, 1};

Mode mode[1] =
	{{1, CHECK_PREVIOUS, {red, green, blue, none, none, none}, 0, 0, 1, 1}};

extern Device device;

char driver_name[] = "Monochrome";

extern Driver *me;
extern Access *access;

extern void *write_pixel;
extern void *read_pixel;
extern void *set_colours;
extern void *get_colour;

void *write_pixel_r = &write_pixel;
void *read_pixel_r  = &read_pixel;
void *line_draw_r   = 0;
void *expand_area_r = 0;
void *fill_area_r   = 0;
void *fill_poly_r   = 0;
void *blit_area_r   = 0;
void *text_area_r   = 0;
void *mouse_draw_r  = 0;
void *set_colours_r = &set_colours;
void *get_colours_r = 0;
void *get_colour_r  = &get_colour;

long wk_extend = 0;

short accel_s = A_SET_PAL | A_GET_COL | A_SET_PIX | A_GET_PIX;
short accel_c = 0;

Mode *graphics_mode = &mode[0];

short colour_bits = 18;

short debug = 0;

short depth = 0;


/*
 * Handle any driver specific parameters
 */
void check_token(char *token, const char **ptr)
{
}


/*
 * Do whatever setup work might be necessary on boot up
 * and which couldn't be done directly while loading.
 * Supplied is the default fVDI virtual workstation.
 */
long initialize(Virtual *vwk)
{
	Workstation *wk;

	debug = access->funcs.misc(0, 1, 0);
	
	vwk = me->default_vwk;	/* This is what we're interested in */
	wk = vwk->real_address;

        if (wk->screen.pixel.width > 0)        /* Starts out as screen width */
                wk->screen.pixel.width = (wk->screen.pixel.width * 1000L) / wk->screen.mfdb.width;
        else                                   /*   or fixed DPI (negative) */
                wk->screen.pixel.width = 25400 / -wk->screen.pixel.width;
        if (wk->screen.pixel.height > 0)        /* Starts out as screen height */
                wk->screen.pixel.height = (wk->screen.pixel.height * 1000L) / wk->screen.mfdb.height;
        else                                    /*   or fixed DPI (negative) */
                wk->screen.pixel.height = 25400 / -wk->screen.pixel.height;

	device.byte_width = wk->screen.wrap;
	device.address = wk->screen.mfdb.address;

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
	return 0;
}

/*
 * 'Deinitialize'
 */
void clswk(Virtual *vwk)
{
}
