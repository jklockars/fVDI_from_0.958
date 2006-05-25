/*
 * fVDI colour handling
 *
 * $Id: colour.c,v 1.4 2006-05-25 22:44:18 johan Exp $
 *
 * Copyright 2005, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "fvdi.h"
#include "utility.h"

#define neg_pal_n  9


/* Necessary intermediate function, for now. */
extern set_palette(Virtual *vwk, DrvPalette *palette_pars);


static Colour *get_clut(Virtual *vwk)
{
  Colour *palette;
  Workstation *wk;
  char *addr;

  wk = vwk->real_address;
  palette = vwk->palette;
  if (wk->driver->device->clut == 1)          /* Hardware CLUT? (used to test look_up_table) */
    palette = wk->screen.palette.colours;     /* Actually a common global */
  else if (!palette || ((long)palette & 1)) { /* No or only negative palette allocated? */
    addr = malloc((wk->screen.palette.size + neg_pal_n) * sizeof(Colour));
    if (!addr) {                              /* If no memory for local palette, */
      palette = wk->screen.palette.colours;   /*  modify in global (BAD!) */
      puts("Could not allocate space for palette!\x0d\x0a");
    } else {
      if (!palette) {                         /* No palette allocated? */
	palette = vwk->palette = (Colour *)(addr + neg_pal_n * sizeof(Colour));   /* Point to index 0 */
      } else {                                /* Only negative palette allocated so far? */
	palette = (Colour *)((long)palette & ~1);  /* Copy the negative side first and free it */
	vwk->palette = (Colour *)(addr + neg_pal_n * sizeof(Colour));
	copymem_aligned(palette - neg_pal_n, addr, neg_pal_n * sizeof(Colour));
	free(palette);
	palette = vwk->palette;
      }
      copymem_aligned(wk->screen.palette.colours, palette, wk->screen.palette.size * sizeof(Colour));
    }
  }

  return palette;
}


void lib_vs_color(Virtual *vwk, long pen, RGB *values)
{
  Workstation *wk = vwk->real_address; 
  Colour *palette;
  DrvPalette palette_pars;

  if (pen >= wk->screen.palette.size)
    return;

  palette = get_clut(vwk);

  palette_pars.first_pen = pen;
  palette_pars.count     = 1;    /* One colour to set up */
  palette_pars.requested = (short *)values;
  palette_pars.palette   = palette;

#if 0
  ((void (*)(Virtual *, DrvPalette *))wk->r.set_palette)(vwk, &palette_pars);
#else
  set_palette(vwk, &palette_pars);
#endif
}


static int idx2vdi(Workstation *wk, int index)
{
  static signed char vdi_colours[] = {0,2,3,6,4,7,5,8,9,10,11,14,12,15,13,-1};
  int ret;

  if ((unsigned int)index >= (unsigned int)wk->screen.palette.size)
    return -1;

  /* No VDI->TOS conversion for true colour */
  if (wk->driver->device->clut != 1)  /* Hardware CLUT? (used to test look_up_table) */
    ret = index;
  else if (index == wk->screen.palette.size - 1)
    ret = 1;
  else if ((index >= 16) && (index != 255))
    ret = index;
  else {
    ret = vdi_colours[index];
    if (ret < 0)
      ret = wk->screen.palette.size - 1;
  }

  return ret;
}


static int vdi2idx(Workstation *wk, int vdi_pen)
{
  static signed char tos_colours[] = {0,-1,1,2,4,6,3,5,7,8,9,10,12,14,11,13};
  int ret;

  if ((unsigned int)vdi_pen >= (unsigned int)wk->screen.palette.size)
    return -1;

  if (wk->driver->device->clut != 1)
    ret = vdi_pen;
  else if (vdi_pen == 255)
    ret = 15;
  else if (vdi_pen >= 16)
    ret = vdi_pen;
  else {
    ret = tos_colours[vdi_pen];
    if (ret < 0)
      ret = wk->screen.palette.size - 1;
  }

  return ret;
}


int lib_vq_color(Virtual *vwk, long pen, long flag, RGB *colour)
{
  int index;
  Colour *palette;

  index = vdi2idx(vwk->real_address, pen);
  if (index < 0)
    return -1;

  palette = vwk->palette;
  /* Negative indices are always in local palette, but this can't be one of those */
  if (!palette || ((long)palette & 1))
    palette = vwk->real_address->screen.palette.colours;

  if (flag == 0) {
    colour->red   = palette[index].vdi.red;
    colour->green = palette[index].vdi.green;
    colour->blue  = palette[index].vdi.blue;
  } else {
    colour->red   = palette[index].hw.red;
    colour->green = palette[index].hw.green;
    colour->blue  = palette[index].hw.blue;
  }

  return pen;
}


static int fg_bg_index(Virtual *vwk, int subfunction, short **fg, short **bg)
{
  switch (subfunction) {
  case 0:
    *fg = &vwk->text.colour.foreground;
    *bg = &vwk->text.colour.background;
    break;
  case 1:
    *fg = &vwk->fill.colour.foreground;
    *bg = &vwk->fill.colour.background;
    break;
  case 2:
    *fg = &vwk->line.colour.foreground;
    *bg = &vwk->line.colour.background;
    break;
  case 3:
    *fg = &vwk->marker.colour.foreground;
    *bg = &vwk->marker.colour.background;
    break;
  case 4:
    /* This will be for bitmaps */
    return 0;
    break;
  default:
    return 0;
  }

  return 1;
}


int lib_vs_fg_color(Virtual *vwk, long subfunction, long colour_space, COLOR_ENTRY *values)
{
  short *fg, *bg, index;
  Colour *palette;
  void *addr;
  DrvPalette palette_pars;

  if ((unsigned int)colour_space > 1)    /* Only 0 or 1 allowed for now (current or RGB) */
    return 0;

  if (!fg_bg_index(vwk, subfunction, &fg, &bg))
    return -1;
  index = *fg = -subfunction * 2 - 2;      /* Index -2/-4... */

  palette = vwk->palette;
  if (!palette) {
    addr = malloc(neg_pal_n * sizeof(Colour));
    if (!addr) {
      puts("Could not allocate space for negative palette!\x0d\x0a");
      return -1;
    }
    palette = vwk->palette = (Colour *)(((long)addr + neg_pal_n * sizeof(Colour)) | 1);   /* Point to index 0 */
  }
  palette = (Colour *)((long)palette & ~1);
  
  palette_pars.first_pen = index;
  palette_pars.count     = 1;         /* One colour to set up */
  palette_pars.requested = (short *)((long)values | 1);    /* Odd for new style entries */
  palette_pars.palette   = palette;

#if 0
  ((void (*)(Virtual *, DrvPalette *))vwk->real_address->r.set_palette)(vwk, &palette_pars);
#else
  set_palette(vwk, &palette_pars);
#endif

  return 1;
}


int lib_vs_bg_color(Virtual *vwk, long subfunction, long colour_space, COLOR_ENTRY *values)
{
  short *fg, *bg, index;
  Colour *palette;
  void *addr;
  DrvPalette palette_pars;

  if ((unsigned int)colour_space > 1)    /* Only 0 or 1 allowed for now (current or RGB) */
    return 0;

  if (!fg_bg_index(vwk, subfunction, &fg, &bg))
    return -1;
  index = *bg = -subfunction * 2 - 3;      /* Index -3/-5... */

  palette = vwk->palette;
  if (!palette) {
    addr = malloc(neg_pal_n * sizeof(Colour));
    if (!addr) {
      puts("Could not allocate space for negative palette!\x0d\x0a");
      return -1;
    }
    palette = vwk->palette = (Colour *)(((long)addr + neg_pal_n * sizeof(Colour)) | 1);   /* Point to index 0 */
  }
  palette = (Colour *)((long)palette & ~1);

  palette_pars.first_pen = index;
  palette_pars.count     = 1;         /* One colour to set up */
  palette_pars.requested = (short *)((long)values | 1);    /* Odd for new style entries */
  palette_pars.palette   = palette;

#if 0
  ((void (*)(Virtual *, DrvPalette *))vwk->real_address->r.set_palette)(vwk, &palette_pars);
#else
  set_palette(vwk, &palette_pars);
#endif

  return 1;
}


long lib_vq_fg_color(Virtual *vwk, long subfunction, COLOR_ENTRY *colour)
{
  short *fg, *bg, index;
  Colour *palette;

  if (!fg_bg_index(vwk, subfunction, &fg, &bg))
    return -1;
  index = *fg;

  palette = vwk->palette;
  if (!palette || (((long)palette & 1) && (index >= 0)))   /* No or only part local? */
    palette = vwk->real_address->screen.palette.colours;
  palette = (Colour *)((long)palette & ~1);

  colour->rgb.reserved = 0;
  colour->rgb.red      = palette[index].vdi.red;
  colour->rgb.green    = palette[index].vdi.green;
  colour->rgb.blue     = palette[index].vdi.blue;

  return 1;    /* RGB_SPACE */
}


long lib_vq_bg_color(Virtual *vwk, long subfunction, COLOR_ENTRY *colour)
{
  short *fg, *bg, index;
  Colour *palette;

  if (!fg_bg_index(vwk, subfunction, &fg, &bg))
    return -1;
  index = *bg;

  palette = vwk->palette;
  if (!palette || (((long)palette & 1) && (index >= 0)))   /* No or only part local? */
    palette = vwk->real_address->screen.palette.colours;
  palette = (Colour *)((long)palette & ~1);

  colour->rgb.reserved = 0;
  colour->rgb.red      = palette[index].vdi.red;
  colour->rgb.green    = palette[index].vdi.green;
  colour->rgb.blue     = palette[index].vdi.blue;

  return 1;    /* RGB_SPACE */
}


int colour_entry(Virtual *vwk, long subfunction, short *intin, short *intout)
{
  switch(subfunction) {
  case 0:     /* v_color2value */
    puts("v_color2value not yet supported\x0d\x0a");
    return 2;
  case 1:     /* v_value2color */
    puts("v_value2color not yet supported\x0d\x0a");
    return 6;
  case 2:     /* v_color2nearest */
    puts("v_color2nearest not yet supported\x0d\x0a");
    return 6;
  case 3:     /* vq_px_format */
    puts("vq_px_format not yet supported\x0d\x0a");
    *(long *)&intout[0] = 1;
    *(long *)&intout[2] = 0x03421820;
    return 4;
  default:
    puts("Unknown colour entry operation\x0d\x0a");
    return 0;
  }
}


static int set_col_table(Virtual *vwk, long count, long start, COLOR_ENTRY *values)
{
  Workstation *wk = vwk->real_address;
  Colour *palette;
  DrvPalette palette_pars;

  if (start + count > wk->screen.palette.size)
    count = wk->screen.palette.size - start;

  palette = get_clut(vwk);

  palette_pars.first_pen = start;
  palette_pars.count     = count;
  palette_pars.requested = (short *)((long)values | 1);
  palette_pars.palette   = palette;

#if 0
  ((void (*)(Virtual *, DrvPalette *))wk->r.set_palette)(vwk, &palette_pars);
#else
  set_palette(vwk, &palette_pars);
#endif

  return count;
}


int set_colour_table(Virtual *vwk, long subfunction, short *intin)
{
  COLOR_TAB *ctab;

  switch(subfunction) {
  case 0:     /* vs_ctab */
    ctab = (COLOR_TAB *)intin;
#if 0
    puts("vs_ctab still not verified\x0d\x0a");
    {
      long *lptr;
      short *sptr;
      int i, j;
      char buf[10];

      lptr = (long *)ctab;
      for(i = 0; i < 6; i++) {
	ltoa(buf, *lptr++, 16);
	puts(buf);
	puts(" ");
      }
      puts("\x0d\x0a");
      for(i = 0; i < 6; i++) {
	ltoa(buf, *lptr++, 16);
	puts(buf);
	puts(" ");
      }
      puts("\x0d\x0a");
      sptr = (short *)&ctab->colors;
      for(j = 0; j < 64; j++) {
	for(i = 0; i < 16; i++) {
	  ltoa(buf, *sptr++ & 0xffffL, 16);
	  puts(buf);
	  puts(" ");
	}
	puts("\x0d\x0a");
      }
    }
#endif
    return set_col_table(vwk, ctab->no_colors, 0, ctab->colors);
  case 1:     /* vs_ctab_entry */
    puts("vs_ctab_entry not yet supported\x0d\x0a");
    return 1;      /* Seems to be the only possible value for non-failure */
  case 2:     /* vs_dflt_ctab */
    puts("vs_dflt_ctab not yet supported\x0d\x0a");
    return 256;    /* Not really correct */
  default:
    puts("Unknown set colour table operation\x0d\x0a");
    return 0;
  }
}


int colour_table(Virtual *vwk, long subfunction, short *intin, short *intout)
{
  switch(subfunction) {
  case 0:     /* vq_ctab */
    {
      COLOR_TAB *ctab = (COLOR_TAB *)&intout[0];
      int i;
#if 1
      long length = (int)((char *)&ctab->colors - (char *)&ctab->magic) +
                   256 * sizeof(COLOR_ENTRY);
#else
      long length = 48 + 256 * 8;
#endif
      Colour *palette = vwk->palette;

      /* Negative indices are always in local palette, but this can't be one of those */
      if (!palette || ((long)palette & 1))
        palette = vwk->real_address->screen.palette.colours;

#if 0
      puts("vq_ctab not yet really supported\x0d\x0a");
#endif
      if (length > *(long *)&intin[0]) {
	char buf[10];
	puts("Too little space available for ctab (");
	ltoa(buf, *(long *)&intin[0], 10);
	puts(buf);
	puts(" when ctab needs ");
	ltoa(buf, length, 10);
	puts(buf);
	puts(")!\x0d\x0a");
	return 0;
      }
      ctab->magic = str2long("ctab");
      ctab->length = length;
      ctab->format = 0;
      ctab->reserved = 0;
      ctab->map_id = 0xbadc0de1;
      ctab->color_space = 1;
      ctab->flags = 0;
      ctab->no_colors = 256;
      ctab->reserved1 = 0;
      ctab->reserved2 = 0;
      ctab->reserved3 = 0;
      ctab->reserved4 = 0;

      for(i = 0; i < 256; i++) {
	ctab->colors[i].rgb.red    = (palette[i].vdi.red   * 255L) / 1000;
	ctab->colors[i].rgb.green  = (palette[i].vdi.green * 255L) / 1000;
	ctab->colors[i].rgb.blue   = (palette[i].vdi.blue  * 255L) / 1000;
	ctab->colors[i].rgb.red   |= ctab->colors[i].rgb.red   << 8;
	ctab->colors[i].rgb.green |= ctab->colors[i].rgb.green << 8;
	ctab->colors[i].rgb.blue  |= ctab->colors[i].rgb.blue  << 8;
#if 0
	{
	  char buf[10];
	  puts("[");
	  ltoa(buf, i, 10);
	  puts(buf);
	  puts("] = ");
	  ltoa(buf, ctab->colors[i].rgb.red & 0xffff, 16);
	  puts(buf);
	  puts(",");
	  ltoa(buf, ctab->colors[i].rgb.green & 0xffff, 16);
	  puts(buf);
	  puts(",");
	  ltoa(buf, ctab->colors[i].rgb.blue & 0xffff, 16);
	  puts(buf);
	  puts("  ");
	  ltoa(buf, palette[i].vdi.red & 0xffff, 16);
	  puts(buf);
	  puts(",");
	  ltoa(buf, palette[i].vdi.green & 0xffff, 16);
	  puts(buf);
	  puts(",");
	  ltoa(buf, palette[i].vdi.blue & 0xffff, 16);
	  puts(buf);
	  puts("  ");
	  ltoa(buf, palette[i].hw.red & 0xffff, 16);
	  puts(buf);
	  puts(",");
	  ltoa(buf, palette[i].hw.green & 0xffff, 16);
	  puts(buf);
	  puts(",");
	  ltoa(buf, palette[i].hw.blue & 0xffff, 16);
	  puts(buf);
	  puts("\x0d\x0a");
	}
#endif
      }
      return 256;    /* Depending on palette size */
    }
  case 1:     /* vq_ctab_entry */
    puts("vq_ctab_entry not yet supported\x0d\x0a");
    return 6;
  case 2:     /* vq_ctab_id */
    puts("vq_ctab_id not yet supported\x0d\x0a");
    *(long *)&intout[0] = 0xbadc0de1;   /* Not really correct */
    return 2;
  case 3:     /* v_ctab_idx2vdi */
    intout[0] = idx2vdi(vwk->real_address, intin[0]);
    return 1;
  case 4:     /* v_ctab_vdi2idx */
    intout[0] = vdi2idx(vwk->real_address, intin[0]);
    return 1;
  case 5:     /* v_ctab_idx2value */
    puts("v_ctab_idx2value not yet supported\x0d\x0a");
    return 2;
  case 6:     /* v_get_ctab_id */
    puts("v_get_ctab_id not yet supported\x0d\x0a");
    *(long *)&intout[0] = 0xbadc0de1;   /* Should always be different */
    return 2;
  case 7:     /* vq_dflt_ctab */
    puts("vq_dflt_ctan not yet supported\x0d\x0a");
    return 256;    /* Depending on palette size */
  case 8:     /* v_create_ctab */
    puts("v_create_ctab not yet supported\x0d\x0a");
    return 2;
  case 9:     /* v_delete_ctab */
    puts("v_delete_ctab not yet supported\x0d\x0a");
    intout[0] = 1;   /* OK */
    return 1;
  default:
    puts("Unknown colour table operation\x0d\x0a");
    return 0;
  }
}


int inverse_table(Virtual *vwk, long subfunction, short *intin, short *intout)
{
  switch(subfunction) {
  case 0:     /* v_create_itab */
    {
      COLOR_TAB *ctab = (COLOR_TAB *)*(long *)&intin[0];
      int bits = intin[2];
      puts("v_create_itab not yet supported\x0d\x0a");
      *(long *)&intout[0] = 0xbadc0de1;
      return 2;
    }
  case 1:     /* v_delete_itab */
    puts("v_delete_itab not yet supported\x0d\x0a");
    intout[0] = 1;   /* OK */
    return 1;
  default:
    puts("Unknown inverse colour table operation\x0d\x0a");
    return 0;
  }
}


#if 0
v_get_pixel:
        uses_d1
	move.l	d2,-(a7)
        sub.w   #12,a7
        move.l  ptsin(a1),a2
        move.l  (a2),0(a7)              ; Coordinates
        move.l  intout(a1),a2
        move.l  a2,4(a7)
        addq.l  #2,a2
        move.l  a2,8(a7)
        move.l  a7,a1
	move.l	a1,-(a7)
	move.l	a0,-(a7)
        bsr     _lib_v_get_pixel
        add.w   #12+8,a7
	move.l	(a7)+,d2
        used_d1
        done_return                     ; Should be real_return

* lib_v_get_pixel - Standard Library function
* Todo: Convert when in bitplane modes
* In:   a1      Parameters   lib_v_get_pixel(x, y, &colour, &index)
*       a0      VDI struct
lib_v_get_pixel:
	move.l	d2,-(a7)
	move.l	a1,-(a7)
	move.l	a0,-(a7)
	bsr	_lib_v_get_pixel
	addq.l	#8,a7
	move.l	(a7)+,d2
	rts

void lib_v_get_pixel(Virtual *vwk, *pars)
{
	unsigned long ret;

	gp_pars->mfdb = 0;
	gp_pars->x = pars->x;
	gp_pars->y = pars->y;
	ret = vwk->real_address->r_get_pixel(vwk, gp_pars);
	*pars->colour = ret;   // word
	if (vwk->real_address->driver->device->clut == 1)  // Hardware CLUT? (used to test look_up_table)
		// This should of course convert!
	else
	{
		if (vwk->real_address->screen.mfdb.bitplanes > 16)
			*pars->index = ret >> 16;
		else
			*pars->index = -1;
	}


v_bar:
;       use_special_stack
        uses_d1
	movem.l	d2/a2,-(a7)
        move.l  ptsin(a1),a1
	move.l	a1,-(a7)
	move.l	a0,-(a7)
        jsr     _lib_v_bar
	addq.l	#8,a7
	movem.l	(a7)+,d2/a2
        used_d1
        done_return                     ; Should be real_return


* lib_v_bar - Standard Library function
* Todo: -
* In:   a1      Parameters   lib_v_bar(points)
*       a0      VDI struct
lib_v_bar:
	movem.l	d2/a2,-(a7)
	move.l	a1,-(a7)
	move.l	a0,-(a7)
	jsr	_lib_v_bar
	addq.l	#8,a7
	movem.l	(a7)+,d2/a2
	rts

void lib_v_bar(Virtual *vwk, short *points)
{
	short buf[4];

	// Not hollow, so draw,   or   hollow and perimeter, so don't draw (right?)
	if (vwk->fill.interior || !vwk->fill_perimeter)
	{
		lib_vr_recfl
		if (!vwk->fill_perimeter)
			return;
	}

	fill_pars->mode = vwk->mode;
	fill_pars->what??? = 0x00010000;    // solid (d7)
	fill_pars->
	fill_pars->colours = vwk->fill.colour;
	fill_pars->points = buf;

        move.l  _pattern_ptrs,d5

	// Draw top/bottom perimeter
	buf[0] = points[0];
	buf[1] = points[1];
	buf[2] = points[2];
	buf[3] = points[1];
	if (clip(buf))
		vwk->real_address->r_fill();

	if (points[1] == points[3])
		return;

	// Draw bottom/top perimeter
	buf[0] = points[0];
	buf[1] = points[3];
	buf[2] = points[2];
	buf[3] = points[3];
	if (clip(buf))
		vwk->real_address->r_fill();

	// Draw left perimeter
	buf[0] = points[0];
	buf[2] = points[0];
	if (points[3] < points[1]) {     // Bug compatibility
		buf[1] = points[3] + 1;
		buf[3] = points[1] - 1;
	} else {
		buf[1] = points[1] + 1;
		buf[3] = points[3] - 1;
	}
	if (clip(buf))
		vwk->real_address->r_fill();

	// Draw right perimeter
	buf[0] = points[2];
	buf[2] = points[2];
	if (points[3] < points[1]) {     // Bug compatibility
		buf[1] = points[3] + 1;
		buf[3] = points[1] - 1;
	} else {
		buf[1] = points[1] + 1;
		buf[3] = points[3] - 1;
	}
	if (clip_rect(buf))
		vwk->real_address->r_fill();
}


vr_recfl:
;       use_special_stack
        uses_d1
	movem.l	d2/a2,-(a7)
        move.l  ptsin(a1),a1
	move.l	a1,-(a7)
	move.l	a0,-(a7)
        bsr     _lib_vr_recfl
	addq.l	#8,a7
	movem.l	(a7)+,d2/a2
        used_d1
        done_return                     ; Should be real_return


* lib_vr_recfl - Standard Library function
* Todo: -
* In:   a1      Parameters   lib_vr_recfl(points)
*       a0      VDI struct
lib_vr_recfl:
	movem.l	d2/a2,-(a7)
	move.l	a1,-(a7)
	move.l	a0,-(a7)
	jsr	_lib_vr_recfl
	addq.l	#8,a7
	movem.l	(a7)+,d2/a2
	rts

void lib_vr_recfl(Virtual *vwk, short *points)
{
	// Need clear upper word for compatibility with current
	// span fill implementation (990704)
	unsigned long buf[4];

	// Bug compatibility
	if (points[2] < points[0]) {
		buf[0] = points[2];
		buf[2] = points[0];
	} else {
		buf[0] = points[0];
		buf[2] = points[2];
	}
	// (only seen in NVDI polygon code)
	if (points[3] < points[1]) {
		buf[1] = points[3];
		buf[3] = points[1];
	} else {
		buf[1] = points[1];
		buf[3] = points[3];
	}

	if (!clip_rect(buf))
		return;

	// Interior/0 (style set below, if any)
	fill_pars->interior_style = vwk->fill.interior << 16;
	if (vwk->fill.interior)
		fill_pars->colour = vwk->fill.colour;
	else
		fill_pars->colour = rot16(vwk->fill.colour);  // Hollow, so background colour

	if (vwk->fill.interior == 4)
		fill_pars->pattern = vwk->fill.user_pattern_in_use;
	else {
		fill_pars->pattern = pattern_ptrs[vwk->fill.interior];
		if (vwk->fill.interior & 2) {  // interior 2 or 3
			fill_pars->interior_style |= vwk->fill.style;
			fill_pars->pattern += vwk->fill.interior - 1) * 8; // Add style index
		}
	}
	fill_pars->mode = vwk->mode;

	vwk->real_address->r_fill(vwk, fill_pars);
}


void fill_area(Virtual *vwk, short *points)
{
	unsigned long buf[4];

	buf[0] = points[0];
	buf[1] = points[1];
	buf[2] = points[2];
	buf[3] = points[3];

	if (!clip_rect(buf))
		return;

	fill_pars->whatmmm.. = 20(a1);   colour?
	fill_pars->points = buf;
	fill_pars->patterns = pattern_ptrs;
	fill_parts->mode = 1;                 // replace mode
	fill_pars->whatever.. = 0x00010000;   // solid
	vwk->real_address->r_fill(vwk, fill_pars);
}
#endif
