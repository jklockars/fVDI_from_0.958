/*
 * fVDI workstation functions
 * 
 * Copyright 2000, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#define NEG_PAL_N	9	/* Number of negative palette entries */
#define HANDLES		32	/* Max number of handles */

#include "fvdi.h"

extern Virtual *handle[];
extern Virtual *default_virtual;
extern Virtual *non_fvdi_vwk;
extern Workstation *non_fvdi_wk;
extern Workstation *screen_wk;
extern List *driver_list;
extern short lineafix;
extern long old_gdos;

void lib_vdi_s(void *, void *, short);
void lib_vdi_sp(void *, void *, short, void *);
void lib_vdi_spppp(void *, void *, short, void *, void *, void *, void *);
void lib_vdi_pp(void *, void *, void *, void *);
#ifdef __PUREC__
extern void lib_vdi_s(void *, void *, short);
extern void lib_vdi_sp(void *, void *, short, void *);
extern void lib_vdi_spppp(void *, void *, short, void *, void *, void *, void *);
extern void lib_vdi_pp(void *, void *, void *, void *);
#else
#define LIB_CALL {"224f4e92";}			/* move.l a7,a1   jsr (a2) */
#pragma inline lib_vdi_s(a2, a0, (short)) LIB_CALL
#pragma inline lib_vdi_sp(a2, a0, (short),) LIB_CALL
#pragma inline lib_vdi_spppp(a2, a0, (short),,,,) LIB_CALL
#pragma inline lib_vdi_pp(a2, a0,,) LIB_CALL
#endif

extern void link_mouse_routines(void);
extern void *lib_vst_color;
extern void *lib_vst_font;
extern void *lib_vst_point;
extern void *lib_vsl_color;
extern void *lib_vsl_type;
extern void *lib_vsm_color;
extern void *lib_vsm_type;
extern void *lib_vsf_color;
extern void *lib_vsf_interior;
extern void *lib_vsf_style;
extern void *lib_vs_clip;
extern void *lib_vr_trn_fm;
#if 0
extern void lib_vst_color(Virtual *, short);
extern void lib_vst_font(Virtual *, short);
extern void lib_vst_point(Virtual *, short, short *, short *, short *, short *);
extern void lib_vsl_color(Virtual *, short);
extern void lib_vsl_type(Virtual *, short);
extern void lib_vsm_color(Virtual *, short);
extern void lib_vsm_type(Virtual *, short);
extern void lib_vsf_color(Virtual *, short);
extern void lib_vsf_interior(Virtual *, short);
extern void lib_vsf_style(Virtual *, short);
extern void lib_vs_clip(Virtual *, short, short *);
extern void lib_vr_trn_fm(Virtual *, MFDB *, MFDB *);
#endif
extern short call_other(VDIpars *, long);
extern void opnvwk_values(Virtual *, VDIpars *);

extern void copymem(void *s, void *d, long n);
extern void setmem(void *d, long v, long n);
extern void *malloc(long size, long type);
extern void free(void *addr);
#if 0
extern void puts(char *text);
#endif
extern short scall_v_opnwk(long dev_id, short *int_out, short *pts_out);

extern short old_wk_handle;


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


/* Find a free handle */
static short find_handle(void)
{
	short hnd;

	for(hnd = 1; hnd < HANDLES; hnd++)
		if (handle[hnd] == non_fvdi_vwk)
			return hnd;
	return 0;
}


void v_opnvwk(Virtual *vwk, VDIpars *pars)
{
	short *intin;
	short hnd, width, height, bitplanes, lwidth, dummy, extra_size;
	long size;
	Workstation *wk, *new_wk;
	Virtual *new_vwk;
	MFDB *mfdb;

	pars->control->handle = 0;	/* Assume failure */
	if (!(hnd = find_handle()))
		return;

	/* Check if really v_opnbm */
	if ((pars->control->subfunction != 1) || (pars->control->l_intin < 20)) {
		extra_size = 32;
		if (!(new_vwk = malloc(sizeof(Virtual) + extra_size, 3)))
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
		if (!(new_vwk = malloc(extra_size, 3)))
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

	opnvwk_values(vwk, pars);		/* Return information about workstation */

	pars->control->handle = vwk->standard_handle = hnd;
	handle[hnd] = vwk;

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
	lib_vdi_s(&lib_vst_font, vwk, pars->intin[5]);
	/* Default to 10 point font (or less) (should really depend on resolution) */
	lib_vdi_spppp(&lib_vst_point, vwk, 10, &dummy, &dummy, &dummy, &dummy);
	lib_vdi_s(&lib_vst_color, vwk, pars->intin[6]);
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
	Virtual *vwk;
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
		    (hnd = find_handle()) &&
		    (vwk = malloc(6, 3)) &&
		    (oldhnd = call_other(pars, 0))) {	/* Dummy handle for call */
			vwk->real_address = non_fvdi_wk;
			/* Mark as pass-through handle */
			vwk->standard_handle = oldhnd | 0x8000;
			handle[hnd] = vwk;
			pars->control->handle = hnd;
		} else if (vwk)
			free(vwk);		/* Couldn't open */
		return;
	}

	/* Experimenting, 001217/010109 */
	if (!old_wk_handle) {
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

	if (wk->mouse.type)		/* Old mouse? */
		link_mouse_routines();

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
	handle[hnd] = non_fvdi_vwk;

	return;
}


void v_clswk(Virtual *vwk, VDIpars *pars)
{
	/* Should call driver specific function, among other things */

	v_clsvwk(vwk, pars);

	return;
}

