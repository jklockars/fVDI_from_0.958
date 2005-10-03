/*
 * fVDI workstation functions
 * 
 * $Id: workstn.c,v 1.10 2005-10-03 22:51:42 johan Exp $
 *
 * Copyright 2000/2003, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "fvdi.h"
#include "utility.h"
#include "function.h"
#include "globals.h"

#define NEG_PAL_N	9	/* Number of negative palette entries */


Virtual **handle_link = 0;

int lib_vq_extnd(Virtual *vwk, long subfunction, long flag,
		 short *intout, short *ptsout);


void
linea_setup(Workstation *wk)
{
	unsigned short *linea, width, height, bitplanes;
	static unsigned short linea_orig[12];
	static short init = 1;

	linea = wk->screen.linea;
	
	if (init) {
		init = 0;
		linea_orig[0] = linea[-0x2b4 / 2];
		linea_orig[1] = linea[-0x2b2 / 2];
		linea_orig[2] = linea[-0x00c / 2];
		linea_orig[3] = linea[-0x004 / 2];
		linea_orig[4] = linea[-0x306 / 2];
		linea_orig[5] = linea[0];
		linea_orig[6] = linea[1];
		linea_orig[7] = linea[-1];
		linea_orig[8] = linea[-0x304 / 2];
		linea_orig[9] = linea[-0x266 / 2];
		linea_orig[10] = linea[-0x26e / 2];
		linea_orig[11] = linea[-0x29a / 2];
	}

	/* Copy a few things into the lineA variables */
	width = wk->screen.mfdb.width;
	height = wk->screen.mfdb.height;
	linea[-0x2b4 / 2] = width - 1;
	linea[-0x2b2 / 2] = height - 1;
	linea[-0x00c / 2] = width;
	linea[-0x004 / 2] = height;
	if (lineafix) {			/* Should cover more really */
		bitplanes = wk->screen.mfdb.bitplanes;
		linea[-0x306 / 2] = bitplanes;
		linea[0] = bitplanes;	/* Can this perhaps be done always? */
		width = (width * bitplanes) >> 3;	/* Bug (<8 planes) here in original */
		linea[1] = width;
		linea[-1] = width;	/* Really the same? */
	} else if (!init) {
		linea[-0x306 / 2] = linea_orig[4];
		linea[0] = linea_orig[5];
		linea[1] = linea_orig[6];
		linea[-1] = linea_orig[7];
	}

#if 0
	linea[-0x256 / 2] = 0;	/* Fix for ImageCopy (requires LineA drawing to be disabled) */	
#endif
	linea[-0x304 / 2] = wk->screen.look_up_table;		/* 1/0 */
	linea[-0x266 / 2] = wk->screen.palette.possibilities;	/* 0 */
	linea[-0x26e / 2] = wk->screen.colour;			/* 1 */
	linea[-0x29a / 2] = wk->screen.palette.size;		/* 0x100 */
}


/* Find virtual workstation entry for a handle */
static Virtual** find_handle_entry(short hnd)
{
	short handles;
	Virtual **link, **handle_table;

	handles = HANDLES;
	if (hnd < handles)
		return &handle[hnd];

	link = handle_link;
	while (link) {
		hnd -= handles;
		if (debug) {
			puts("Looking for handle in extra table\x0a\x0d");
		}
		handles = (long)handle_table[-2];
		if (hnd < handles)
			return &handle_table[hnd];
		link = (Virtual **)handle_table[-1];
	}

	return 0;
}

/* Find (or create, if necessary) a free handle */
static short find_free_handle(Virtual ***handle_entry)
{
	short hnd, handles;
	Virtual ***link, ***last, **handle_table;

	handles = HANDLES;
	for(hnd = 1; hnd < handles; hnd++) {
		if (handle[hnd] == non_fvdi_vwk) {
			*handle_entry = &handle[hnd];
			return hnd;
		}
	}

	link = &handle_link;
	last = link;
	while (handle_table = *link) {
		if (debug) {
			puts("Looking for free handle in extra table\x0a\x0d");
		}
		handles += (long)handle_table[-2];
		for(; hnd < handles; hnd++) {
			if (handle_table[hnd] == non_fvdi_vwk) {
				*handle_entry = &handle[hnd];
				return hnd;
			}
		}
		last = link;
		link = (Virtual ***)&handle_table[-1];
	}

	handle_table = (Virtual **)malloc(64 * sizeof(Virtual *));
	if (handle_table) {
		handles = *(long *)handle_table / sizeof(Virtual *) - 2;
		if (debug) {
			puts("Allocated space for ");
			ltoa((char *)handle_table, handles, 10);
			puts((char *)handle_table);
			puts(" extra handles\x0a\x0d");
		}
		handle_table[0] = (Virtual *)((long)handles);
		handle_table[1] = 0;
		for(handles--; handles >= 0; handles--)
			handle_table[handles] = non_fvdi_vwk;
		*last = &handle_table[2];
		*handle_entry = &handle_table[2];
		return hnd;
	}
				
	return 0;
}


// Needs to deal with virtuals on non-screen workstations!
void v_opnvwk(Virtual *vwk, VDIpars *pars)
{
	short *intin;
	short hnd, width, height, bitplanes, lwidth, dummy, extra_size;
	long size;
	Workstation *wk, *new_wk;
	Virtual *new_vwk, **handle_entry;
	MFDB *mfdb;

	pars->control->handle = 0;	/* Assume failure */
	if (!(hnd = find_free_handle(&handle_entry)))
		return;

	/* Check if really v_opnbm */
	if ((pars->control->subfunction != 1) || (pars->control->l_intin < 20)) {
		extra_size = 32;
		if (!(new_vwk = malloc(sizeof(Virtual) + extra_size)))
			return;
		copymem(vwk->real_address->driver->default_vwk, new_vwk, sizeof(Virtual));
		vwk = new_vwk;
	} else {
		mfdb = (MFDB *)pars->control->addr1;
		intin = pars->intin;

		/* Doesn't allow the EdDI v1.1 variant yet */
		if (!intin[15] || !intin[16] || !intin[17] ||
		    !intin[18] || !intin[19])
			return;

		wk = vwk->real_address;

		extra_size = sizeof(Workstation) + sizeof(Virtual) + 32;	/* 32 - user fill pattern */

		if (mfdb->address || intin[11] || intin[12]) {
			width = mfdb->width;
			height = mfdb->height;
		} else {
			width = wk->screen.mfdb.width;		/* vwk/wk bug here in assembly file */
			height = wk->screen.mfdb.height;
		}
		width = (width + 15) & 0xfff0;

		bitplanes = mfdb->bitplanes;
		if ((bitplanes != 1) &&
		    (bitplanes != wk->screen.mfdb.bitplanes))
			if (bitplanes)			/* Only same as physical or one allowed */
				return;
			else
				bitplanes = wk->screen.mfdb.bitplanes;

		lwidth = (width >> 3) * bitplanes;	/* >> 4 in assembly file */
		size = (long)lwidth * height;

		if (!mfdb->address)
			extra_size += size;

		/* New vwk, but it should really not always be for this driver! */
		if (!(new_vwk = malloc(extra_size)))
			return;

		new_wk = (Workstation *)((long)new_vwk + sizeof(Virtual) + 32);
		copymem(wk, new_wk, sizeof(Workstation));

		if (!mfdb->address) {
			mfdb->standard = 0;
			mfdb->address = (short *)((long)new_wk + sizeof(Workstation));
			setmem(mfdb->address, size, 0);
		}

		new_wk->screen.mfdb.address = mfdb->address;
		mfdb->width = new_wk->screen.mfdb.width = width;
		mfdb->height = new_wk->screen.mfdb.height = height;
		mfdb->wdwidth = new_wk->screen.mfdb.wdwidth = (width >> 4);	/* Right? */
		mfdb->bitplanes = new_wk->screen.mfdb.bitplanes = bitplanes;
		mfdb->reserved[0] = new_wk->screen.mfdb.reserved[0] = 0;
		mfdb->reserved[1] = new_wk->screen.mfdb.reserved[1] = 0;
		mfdb->reserved[2] = new_wk->screen.mfdb.reserved[2] = 0;
		new_wk->screen.mfdb.standard = 0;
		if (mfdb->standard)	/* Need to convert input MFDB to device dependent format? */
			lib_vdi_pp(&lib_vr_trn_fm, new_vwk, mfdb, mfdb);

		new_wk->screen.type = 0;
		new_wk->screen.wrap = lwidth;		/* Right? */
		new_wk->screen.shadow.buffer = 0;
		new_wk->screen.shadow.address = 0;
		new_wk->screen.shadow.wrap = 0;

		new_wk->screen.pixel.width = intin[13];
		new_wk->screen.pixel.height = intin[14];

		new_wk->screen.coordinates.course = 0;	/* ? */
		new_wk->screen.coordinates.min_x = 0;
		new_wk->screen.coordinates.min_y = 0;
		new_wk->screen.coordinates.max_x = width - 1;
		new_wk->screen.coordinates.max_y = height - 1;

		/* Probably OK to mark all these as unavailable */
		new_wk->various.input_type = 0;
		new_wk->various.inking = 0;
		new_wk->various.buttons = 0;
		new_wk->various.cursor_movement = 0;
		new_wk->various.number_entry = 0;
		new_wk->various.selection = 0;
		new_wk->various.typing = 0;
		new_wk->various.workstation_type = 0;
		new_wk->mouse.type = 0;				/* Enough? */

		copymem(wk->driver->default_vwk, new_vwk, sizeof(Virtual));

		new_vwk->real_address = new_wk;
		vwk = new_vwk;
	}

	vwk->fill.user.pattern.in_use = (short *)((long)vwk + sizeof(Virtual));
	vwk->fill.user.pattern.extra = 0;
	vwk->fill.user.multiplane = 0;

#if 0
	opnvwk_values(vwk, pars);		/* Return information about workstation */
#else
	/* Return information about workstation */
	lib_vq_extnd(vwk, 0, 0, pars->intout, pars->ptsout);
#endif

	pars->control->handle = vwk->standard_handle = hnd;
	*handle_entry = vwk;

	/* Call various setup functions (most with supplied data) */
#if 0
	lib_vsl_type(vwk, pars->intin[1]);
	lib_vsl_colour(vwk, pars->intin[2]);
	lib_vsm_type(vwk, pars->intin[3]);
	lib_vsm_colour(vwk, pars->intin[4]);
	lib_vst_font(vwk, pars->intin[5]);
	/* Default to 10 point font (or less) (should really depend on resolution) */
	lib_vst_point(vwk, 10, &dummy, &dummy, &dummy, &dummy);
	lib_vst_colour(vwk, pars->intin[6]);
	lib_vsf_interior(vwk, pars->intin[7]);
	lib_vsf_style(vwk, pars->intin[8]);
	lib_vsf_colour(vwk, pars->intin[9]);
	lib_vs_clip(vwk, 0, 0);			/* No clipping (set to max size) */
	/* Should also take care of the coordinate values that come now */
#else
#if 0
	dummy = 0;
#else
	lib_vdi_s(&lib_vsl_type, vwk, pars->intin[1]);
	lib_vdi_s(&lib_vsl_color, vwk, pars->intin[2]);
	lib_vdi_s(&lib_vsm_type, vwk, pars->intin[3]);
	lib_vdi_s(&lib_vsm_color, vwk, pars->intin[4]);
#if 0
	lib_vdi_s(&lib_vst_font, vwk, pars->intin[5]);
	/* Default to 10 point font (or less) (should really depend on resolution) */
	lib_vdi_spppp(&lib_vst_point, vwk, 10, &dummy, &dummy, &dummy, &dummy);
	lib_vdi_s(&lib_vst_color, vwk, pars->intin[6]);
#else
	lib_vst_font(vwk, pars->intin[5]);
	/* Default to 10 point font (or less) (should really depend on resolution) */
	lib_vst_point(vwk, 10, &dummy, &dummy, &dummy, &dummy);
	lib_vst_color(vwk, pars->intin[6]);
#endif
	lib_vdi_s(&lib_vsf_interior, vwk, pars->intin[7]);
	lib_vdi_s(&lib_vsf_style, vwk, pars->intin[8]);
	lib_vdi_s(&lib_vsf_color, vwk, pars->intin[9]);
	lib_vdi_sp(&lib_vs_clip, vwk, 0, 0);			/* No clipping (set to max size) */
	/* Should also take care of the coordinate values that come now */
#endif
#endif

	return;
}


void v_opnwk(VDIpars *pars)
{
	Driver *driver;
	Virtual *vwk, **handle_entry;
	Workstation *wk;
#if 0
	unsigned short *linea, width, height, bitplanes, hnd, oldhnd;
#else
	unsigned short hnd, oldhnd;
#endif

	/* For now, just assume that any
	 * workstation handle >10 is non-fVDI
	 */
	if (pars->intin[0] > 10) {
		pars->control->handle = 0;	/* Assume failure */
		vwk = 0;
		if ((old_gdos != -2) &&		/* No pass-through without old GDOS */
		    (hnd = find_free_handle(&handle_entry)) &&
		    (vwk = malloc(6)) &&
		    (oldhnd = call_other(pars, 0))) {	/* Dummy handle for call */
			vwk->real_address = non_fvdi_wk;
			/* Mark as pass-through handle */
			vwk->standard_handle = oldhnd | 0x8000;
			*handle_entry = vwk;
			pars->control->handle = hnd;
		} else if (vwk)
			free(vwk);		/* Couldn't open */
		return;
	}

	/* Experimenting, 001217/010109 */
	if (!old_wk_handle && !stand_alone) {
		short intout[45], ptsout[12];
		old_wk_handle = scall_v_opnwk(1, intout, ptsout);
	}

#if 0
	if (driver_list->type != ...)		/* Sanity check */
#endif
	driver = (Driver *)driver_list->value;
#if 0
	if (driver->flags & 1)
#endif

	if (vwk = ((Virtual *(*)(Virtual *))(driver->opnwk))(default_virtual))
		;				/* Should probably do something */
	else
		vwk = driver->default_vwk;

	/* To accomodate mouse drawing (only for one screen wk) */
	screen_wk = wk = vwk->real_address;

#if 0
	/* Copy a few things into the lineA variables */
	linea = wk->screen.linea;
	width = wk->screen.mfdb.width;
	height = wk->screen.mfdb.height;
	linea[-0x2b4 / 2] = width - 1;
	linea[-0x2b2 / 2] = height - 1;
	linea[-0x00c / 2] = width;
	linea[-0x004 / 2] = height;
	if (lineafix) {			/* Should cover more really */
		bitplanes = wk->screen.mfdb.bitplanes;
		linea[-0x306 / 2] = bitplanes;
		linea[0] = bitplanes;	/* Can this perhaps be done always? */
		width = (width * bitplanes) >> 3;	/* Bug (<8 planes) here in original */
		linea[1] = width;
		linea[-1] = width;	/* Really the same? */
	}

	linea[-0x304 / 2] = wk->screen.look_up_table;		/* 1/0 */
	linea[-0x266 / 2] = wk->screen.palette.possibilities;	/* 0 */
	linea[-0x26e / 2] = wk->screen.colour;			/* 1 */
	linea[-0x29a / 2] = wk->screen.palette.size;		/* 0x100 */
#else
	linea_setup(wk);
#endif

	if (wk->mouse.type && !stand_alone)	/* Old mouse? */
		link_mouse_routines();

	if (stand_alone)
		setup_vbl_handler();

#if 0
{
	unsigned short *linea;
	linea = wk->screen.linea;
	linea[-0x154] = 0;
	linea[-0x256 / 2] = 0;	/* Fix for ImageCopy (requires LineA drawing to be disabled) */	
}
#endif

	v_opnvwk(vwk, pars);
}


void v_clsvwk(Virtual *vwk, VDIpars *pars)
{
	Virtual **handle_entry;
	short hnd;

	hnd = vwk->standard_handle;
	if (!hnd)	/* Check if default VDI structure */
		return;
	else if (hnd < 0) {
		call_other(pars, hnd & 0x7fff);
		hnd = pars->control->handle;
	} else {
		if (vwk->fill.user.pattern.extra)
			free(vwk->fill.user.pattern.extra);
		if (vwk->palette)
			free((void *)(((long)vwk->palette & ~1) - NEG_PAL_N * sizeof(Colour)));
	}

	free(vwk);	/* This will work for off-screen bitmaps too, fortunately */

	/* Reset VDI structure address to default */
	handle_entry = find_handle_entry(hnd);
	if (!handle_entry)
		puts("Handle could not be found in tables!\x0a\x0d");
	else if (*handle_entry != vwk)
		puts("Wrong handle entry!\x0a\x0d");
	else
	        *handle_entry = non_fvdi_vwk;

	return;
}


// Needs to be able to deal with multiple fVDI workstations!
void v_clswk(Virtual *vwk, VDIpars *pars)
{
#if 0
	Driver *driver;

	v_clsvwk(vwk, pars);

	unlink_mouse_routines();

	screen_wk = 0;
 #if 0
	if (driver_list->type != ...)		/* Sanity check */
 #endif
	driver = (Driver *)driver_list->value;
 #if 0
	if (driver->flags & 1)
 #endif

	((void (*)(Virtual *))(driver->clswk))(vwk);

	if (old_wk_handle)
		scall_v_clswk(old_wk_handle);
#else
	Driver *driver;
	Workstation *wk;

	wk = vwk->real_address;
	v_clsvwk(vwk, pars);

	if (wk != non_fvdi_wk) {
	    unlink_mouse_routines();
	    shutdown_vbl_handler();

	    screen_wk = 0;
 #if 0
	    if (driver_list->type != ...)		/* Sanity check */
 #endif
	    driver = (Driver *)driver_list->value;
 #if 0
	    if (driver->flags & 1)
 #endif

	    ((void (*)(Virtual *))(driver->clswk))(vwk);

	    if (old_wk_handle)
		scall_v_clswk(old_wk_handle);
	}
#endif

	return;
}


int lib_vq_extnd(Virtual *vwk, long subfunction, long flag,
		 short *intout, short *ptsout)
{
  Workstation *wk = vwk->real_address;

  if ((flag == 2) && (subfunction == 1)) {
    copymem_aligned(wk->driver->device, intout, sizeof(Device));
    ((Device *)intout)->address = wk->screen.mfdb.address;
    ((Device *)intout)->byte_width = wk->screen.wrap;
    return 1;
  } else if (flag) {
    *intout++ = wk->screen.type;
    *intout++ = wk->screen.bkg_colours;
    *intout++ = wk->writing.effects;
    *intout++ = wk->raster.scaling;
    *intout++ = wk->screen.mfdb.bitplanes;
    *intout++ = wk->screen.look_up_table;
    *intout++ = wk->raster.performance;
    *intout++ = wk->drawing.flood_fill;
    *intout++ = wk->writing.rotation.type;
    *intout++ = wk->drawing.writing_modes;
    *intout++ = wk->various.input_type;
    *intout++ = wk->writing.justification;
    *intout++ = wk->various.inking;
    *intout++ = wk->drawing.rubber_banding;
    *intout++ = wk->various.max_ptsin;
    *intout++ = wk->various.max_intin;
    *intout++ = wk->various.buttons;
    *intout++ = wk->drawing.line.wide.types_possible;
    *intout++ = wk->drawing.line.wide.writing_modes;
    *intout++ = vwk->clip.on;  /* 19 - originally reserved from here on */
    *intout++ = 0;   /* No pixel sizes below (1 - 0.1 um, 2 - 0.01, 3 - 0.001 */
    *intout++ = 0;   /* Pixel width */
    *intout++ = 0;   /* Pixel height */
    *intout++ = 0;   /* Horizontal dpi */
    *intout++ = 0;   /* Vertical dpi */
    *intout++ = 0;   /* Bit image rotation on printer (PC-GEM/3) */
    *intout++ = 0;   /* AES buffer address (PC-GEM/3) */
    *intout++ = 0;   /*         -    "    -           */
    *intout++ = 2;   /* Beziers! */
    *intout++ = 0;   /* Unused */
    *intout++ = 0;   /* 1 - bitmap scale, 2 - new raster functions, 4 - vr_clip_rects_xxx */
    *intout++ = 0;   /* 9 from here used to be reserved */
    *intout++ = 1;   /* New style colour routines (at least some of them) */
    *intout++ = 0;
    *intout++ = 0;
    *intout++ = 0;
    *intout++ = 0;
    *intout++ = 0;
    *intout++ = 0;
    *intout++ = 0;
    *intout++ = 0;   /* Unusable left border */
    *intout++ = 0;   /*          upper       */
    *intout++ = 0;   /*          right       */
    *intout++ = 0;   /*          lower       */
    *intout++ = 0;   /* Paper format (default/A3/A4/A5/B5/letter/half/legal/double/broad */

    *ptsout++ = vwk->clip.rectangle.x1;
    *ptsout++ = vwk->clip.rectangle.y1;
    *ptsout++ = vwk->clip.rectangle.x2;
    *ptsout++ = vwk->clip.rectangle.y2;
    *ptsout++ = 0;   /* Reserved from here on */
    *ptsout++ = 0;
    *ptsout++ = 0;
    *ptsout++ = 0;
    *ptsout++ = 0;
    *ptsout++ = 0;
    *ptsout++ = 0;
    *ptsout++ = 0;

    return 0;
  } else {
    long attributes;
    int i, n;
    *intout++ = wk->screen.coordinates.max_x;
    *intout++ = wk->screen.coordinates.max_y;
    *intout++ = wk->screen.coordinates.course;
    *intout++ = wk->screen.pixel.width;
    *intout++ = wk->screen.pixel.height;
    *intout++ = wk->writing.size.possibilities;
    *intout++ = wk->drawing.line.types;
    *intout++ = wk->drawing.line.wide.width.possibilities;
    *intout++ = wk->drawing.marker.types;
    *intout++ = wk->drawing.marker.size.possibilities;
    *intout++ = 1;     /* Fonts */
    *intout++ = wk->drawing.fill.patterns;
    *intout++ = wk->drawing.fill.hatches;
    *intout++ = wk->screen.palette.size;
    *intout++ = wk->drawing.primitives.supported;

    attributes = wk->drawing.primitives.attributes;
    n = 0;
    for(i = 9; i >= 0; i--) {
      if (attributes & 0x07) {
	n++;
	intout[i]      = 10 - i;
	intout[i + 10] = (attributes & 0x07) - 1;
      }
      attributes >>= 3;
    }
    if (n < 10)
      intout[i + n] = -1;
    intout += 2 * 10;

    *intout++ = wk->screen.colour;
    *intout++ = wk->writing.rotation.possible;
    *intout++ = wk->drawing.fill.possible;
    *intout++ = wk->drawing.cellarray.available;
    *intout++ = wk->screen.palette.possibilities;
    *intout++ = wk->various.cursor_movement;
    *intout++ = wk->various.number_entry;
    *intout++ = wk->various.selection;
    *intout++ = wk->various.typing;
    *intout++ = wk->various.workstation_type;

    *ptsout++ = wk->writing.size.width.min;
    *ptsout++ = wk->writing.size.height.min;
    *ptsout++ = wk->writing.size.width.max;
    *ptsout++ = wk->writing.size.height.max;
    *ptsout++ = wk->drawing.line.wide.width.min;
    *ptsout++ = 0;
    *ptsout++ = wk->drawing.line.wide.width.max;
    *ptsout++ = 0;
    *ptsout++ = wk->drawing.marker.size.width.min;
    *ptsout++ = wk->drawing.marker.size.height.min;
    *ptsout++ = wk->drawing.marker.size.width.max;
    *ptsout++ = wk->drawing.marker.size.height.max;
    return 0;
  }
}
