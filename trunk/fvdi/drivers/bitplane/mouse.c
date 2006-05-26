/*
 * Bitplane mouse routine
 *
 * $Id: mouse.c,v 1.3 2006-05-26 06:50:29 johan Exp $
 *
 * Copyright 2006, Johan Klockars 
 * Copyright 2002 The EmuTOS development team
 * Copyright 1999 by Caldera, Inc. and Authors:
 * Copyright 1982 by Digital Research Inc.
 *
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "fvdi.h"

#if 0
#define UWORD unsigned short
#define ULONG unsigned long
#endif

extern short fix_shape;
extern short no_restore;


#if 0
short set_mouse_colours(Workstation *wk)
{
    int foreground, background;
    short i, colours;

    foreground = x_get_colour(wk, wk->mouse.colour.foreground);
    background = x_get_colour(wk, wk->mouse.colour.background);

    switch (wk->screen.mfdb.bitplanes) {
    case 1:
	foreground = (foreground << 1) | foreground;
	background = (background << 1) | background;
    case 2:
	foreground = (foreground << 2) | foreground;
	background = (background << 2) | background;
    case 4:
	foreground = (foreground << 4) | foreground;
	background = (background << 4) | background;
	break;
    }
    foreground <<= 7;
    background <<= 7;
    for(i = 7; i >= 0; i--) {
	colours <<= 1;
	foreground <<= 1;
	if (foreground & 0x8000)
	    colours |= 1;
	colours <<= 1;
	background <<= 1;
	if (background & 0x8000)
	    colours |= 1;
    }

    return colours;
}

void set_mouse_shape(Mouse *mouse, short *masks)
{
    int i;

    for(i = 0; i < 16; i++) {
	*masks++ = mouse->mask[i];
	*masks++ = mouse->data[i];
    }
}
#endif


#define NO_W  1
#define LOCAL_PTR 1

#if PLANES == 1
long CDECL
c_mouse_draw_1(Workstation *wk, long x, long y, Mouse *mouse)
#elif PLANES == 2
long CDECL
c_mouse_draw_2(Workstation *wk, long x, long y, Mouse *mouse)
#elif PLANES == 4
long CDECL
c_mouse_draw_4(Workstation *wk, long x, long y, Mouse *mouse)
#else
long CDECL
c_mouse_draw_8(Workstation *wk, long x, long y, Mouse *mouse)
#endif
{
#if !LOCAL_PTR
  UWORD *dst, *save_w, *mask_start;
#endif
  long state;
  static long old_colours = 0;
  static short colours = 0xaaaa;
  volatile static long save_state = 0;
  static short mouse_data[16 * 2] = {
    0xffff, 0x0000, 0x7ffe, 0x3ffc, 0x3ffc, 0x1ff8, 0x1ff8, 0x0ff0,
    0x0ff0, 0x07e0, 0x07e0, 0x03c0, 0x03c0, 0x0180, 0x0180, 0x0000
  };
  static short saved[16 * 8 * 2];
  
  if ((long)mouse > 7) {   /* New mouse shape */
    if (fix_shape)
      return 0;
    if (*(long *)&wk->mouse.colour != old_colours) {
      colours = set_mouse_colours(wk);
      old_colours = *(long *)&wk->mouse.colour;
    }

    set_mouse_shape(mouse, mouse_data);
    return 0;
  }
  

  state = save_state;
  if (state && !no_restore &&
      (((long)mouse == 0) || ((long)mouse == 2))) { /* Move or Hide */
#if LOCAL_PTR
    UWORD *dst, *save_w;
#endif
#if 0
    short h, plane, wrap;
#else
    short h, wrap;
#endif
#if 0
    wrap = (wk->screen.wrap >> 1) - PLANES;
    dst = (UWORD *)((long)wk->screen.mfdb.address +
		    (short)((state >> 10) & 0x3fff) * (long)wk->screen.wrap +
		    (short)(state & 0x3ff) * (long)PLANES * 2);
#else
    wrap = wk->screen.wrap - PLANES * 2;
    dst = (UWORD *)((long)wk->screen.mfdb.address + (state & 0x00ffffff));
#endif
    h = ((unsigned long)state >> 24) & 0x0f;
    save_w = saved;

#if 0
    if ((state >> 28) & 0x01) {  	/* Long? */
#else
    if (state & 0x80000000L) {  	/* Long? */
#endif
#if 0
      for(; h >= 0; h--) {
#else
      do {
#endif
#if 0
        plane = PLANES - 1;
        do {
          *dst++ = *save_w++;
          *(dst + PLANES - 1) = *save_w++;
        } while (--plane >= 0);
	dst += wrap;
#else
        switch (PLANES) {
        case 8:
          *dst++ = *save_w++;
          *(dst + PLANES - 1) = *save_w++;
          *dst++ = *save_w++;
          *(dst + PLANES - 1) = *save_w++;
          *dst++ = *save_w++;
          *(dst + PLANES - 1) = *save_w++;
          *dst++ = *save_w++;
          *(dst + PLANES - 1) = *save_w++;
        case 4:
          *dst++ = *save_w++;
          *(dst + PLANES - 1) = *save_w++;
          *dst++ = *save_w++;
          *(dst + PLANES - 1) = *save_w++;
        case 2:
          *dst++ = *save_w++;
          *(dst + PLANES - 1) = *save_w++;
        case 1:
          *dst++ = *save_w++;
          *(dst + PLANES - 1) = *save_w++;
          break;
        }
	dst = (UWORD *)((long)dst + wrap);
#endif
#if 0
      }
#else
      } while (--h >= 0);
#endif
    } else {   	/* Word */
#if 0
      for(; h >= 0; h--) {
#else
      do {
#endif
#if 0
	plane = PLANES - 1;
	do {
	  *dst++ = *save_w++;
	} while (--plane >= 0);
	dst += wrap;
#else
        switch (PLANES) {
        case 8:
          *dst++ = *save_w++;
          *dst++ = *save_w++;
          *dst++ = *save_w++;
          *dst++ = *save_w++;
        case 4:
          *dst++ = *save_w++;
          *dst++ = *save_w++;
        case 2:
          *dst++ = *save_w++;
        case 1:
          *dst++ = *save_w++;
          break;
        }
	dst = (UWORD *)((long)dst + wrap);
#endif
#if 0
      }
#else
      } while (--h >= 0);
#endif
    }

    save_state = 0;
  }

  if (((long)mouse == 0) || ((long)mouse == 3)) { /* Move or Show */
#if LOCAL_PTR
    UWORD *dst, *save_w, *mask_start;
#endif
    UWORD cdb_fgbg;
    short h, plane, wrap, shft, op;
#if !NO_W
    int w, xs, ys;
#endif

    x -= wk->mouse.hotspot.x;
    y -= wk->mouse.hotspot.y;
#if !NO_W
    w = 16;
#endif
    h = 16;

    op = 0;   /* Assume long operations */

    mask_start = mouse_data;
    if (y < wk->screen.coordinates.min_y) {
      int ys;
      ys = wk->screen.coordinates.min_y - y;
      h -= ys;
      y = wk->screen.coordinates.min_y;
      mask_start += ys << 1;
    }
    if (y + h - 1 > wk->screen.coordinates.max_y)
      h = wk->screen.coordinates.max_y - y + 1;

    shft = x & 0xf;
    if (!shft)
      op = 2;

    if (x < wk->screen.coordinates.min_x) {
      int xs;
      xs = wk->screen.coordinates.min_x - x;
#if !NO_W
      w -= xs;
#endif
      x = wk->screen.coordinates.min_x;
      op = 1;              /* Left clip */
    } else {
#if !NO_W
      if (x + w - 1 > wk->screen.coordinates.max_x) {
	w = wk->screen.coordinates.max_x - x + 1;
#else
      if (x + 16 - 1 > wk->screen.coordinates.max_x) {
#endif
	op = 2;            /* Right clip */
      }
    }

    wrap = (wk->screen.wrap >> 1) - PLANES;
    dst = (UWORD *)((long)wk->screen.mfdb.address +
		    (short)y * (long)wk->screen.wrap +
		    (short)(x >> 4) * (long)PLANES * 2);
      
    save_w = saved;

    if (h) {
#if 0
      state = 0x80000000L;
      state |= (long)(op == 0) << 28;
#else
      state |= (long)(op == 0) << 31;
#endif
      state |= (long)((h - 1) & 0x0f) << 24;
#if 0
      state |= y << 10;
      state |= x >> 4;
#else
      state |= (long)dst - (long)wk->screen.mfdb.address;
#endif
    } else {
      state = 0;
      return 0;
    }
      
    switch (op) {
    case 0:    /* Long word */
      {
	ULONG bits, fg, bg;

	cdb_fgbg = colours;
	shft = 16 - shft;

	h--;
	do {
	  bg = *mask_start++;
	  bg <<= shft;
	  fg = *mask_start++;
	  fg <<= shft;
	  
	  plane = PLANES - 1;
	  do {
	    bits = *(ULONG *)dst & 0xffff0000;
	    bits += *(ULONG *)(dst + PLANES) >> 16;
	    *(ULONG *)save_w = bits;
	    save_w += 2;
	    
	    cdb_fgbg = (cdb_fgbg >> 1) | (cdb_fgbg << 15);
	    if (cdb_fgbg & 0x8000)
	      bits |= bg;
	    else
	      bits &= ~bg;
	    
	    cdb_fgbg = (cdb_fgbg >> 1) | (cdb_fgbg << 15);
	    if (cdb_fgbg & 0x8000)
	      bits |= fg;
	    else
	      bits &= ~fg;
	    
	    *(dst + PLANES) = (UWORD)bits;
	    bits = (bits << 16) | (bits >> 16);
	    *dst++ = (UWORD)bits;
	    
	  } while (--plane >= 0);
	  dst += wrap;
	} while (--h >= 0);
      }
      break;
      
    case 1:      /* Right word only */
      {
	UWORD bits, fg, bg;
	
	cdb_fgbg = colours;
	shft = 16 - shft;
	
	h--;
	do {
	  bg = *mask_start++;
	  bg <<= shft;
	  fg = *mask_start++;
	  fg <<= shft;
	  
	  plane = PLANES - 1;
	  do {
	    bits = *dst;
	    *save_w++ = bits;
	    
	    cdb_fgbg = (cdb_fgbg >> 1) | (cdb_fgbg << 15);
	    if (cdb_fgbg & 0x8000)
	      bits |= bg;
	    else
	      bits &= ~bg;
	    
	    cdb_fgbg = (cdb_fgbg >> 1) | (cdb_fgbg << 15);
	    if (cdb_fgbg & 0x8000)
	      bits |= fg;
	    else
	      bits &= ~fg;
	    
	    *dst++ = bits;
	    
	  } while (--plane >= 0);
	  dst += wrap;
	} while (--h >= 0);
      }
      break;
      
    default:     /* Left word only */
      {
	UWORD bits, fg, bg;
	
	cdb_fgbg = colours;
	
	h--;
	do {
	  bg = *mask_start++;
	  bg >>= shft;
	  fg = *mask_start++;
	  fg >>= shft;
	  
	  plane = PLANES - 1;
	  do {
	    bits = *dst;
	    *save_w++ = bits;
	    
	    cdb_fgbg = (cdb_fgbg >> 1) | (cdb_fgbg << 15);
	    if (cdb_fgbg & 0x8000)
	      bits |= bg;
	    else
	      bits &= ~bg;
	    
	    cdb_fgbg = (cdb_fgbg >> 1) | (cdb_fgbg << 15);
	    if (cdb_fgbg & 0x8000)
	      bits |= fg;
	    else
	      bits &= ~fg;
	    
	    *dst++ = bits;
	    
	  } while (--plane >= 0);
	  dst += wrap;
	} while (--h >= 0);
      }
      break;	  
    }
    
    save_state = state;
  }

  return 0; 
}


#if 0
  case 0:
      cdb_bg = background;
      cdb_fg = foreground;
      for(plane = wk->screen.mfdb.bitplanes - 1; plane >= 0; plane--) {
        int row;
        UWORD *src, *dst;
	
        src = mask_start;
        dst = addr++;

	  for(row = h - 1; row >= 0; row--) {
            ULONG bits, fg, bg;
	    
	    bits = ((ULONG)*dst << 16) | *(dst + inc);
	    *save_l++ = bits;

            bg = ((ULONG)*src++ << 16) >> shft;
            fg = ((ULONG)*src++ << 16) >> shft;

            if (cdb_bg & 0x0001)
                bits |= bg;
            else
                bits &= ~bg;

            if (cdb_fg & 0x0001)
                bits |= fg;
            else
                bits &= ~fg;

	    *(dst + inc) = (UWORD)bits;
	    *dst = (UWORD)(bits >> 16);
	    
            dst += wrap;
	  }

        cdb_bg >>= 1;
        cdb_fg >>= 1;
      }


  case 1:
	    bits = *dst;
	    *save_w++ = bits;
...
	    *dst = bits;


  case 2:
	    bits = *dst;
	    *save_w++ = bits;

            bg = *src++ >> shft;
            fg = *src++ >> shft;
...
            *dst = bits;
#endif


#if 0
#define FAST_MOUSE

long CDECL
c_mouse_draw(Workstation *wk, long x, long y, Mouse *mouse)
{
    static short mask[16], data[16];
#ifndef FAST_MOUSE
    static short saved[16 * 8];
    static short old_x = 0;
    static short old_y = 0;
    static short old_w = 0;
    short foreground, background;
#else
    static long saved[16 * 8];
    static void *saves[] = {
	&&savew, &&save1, &&save2, 0, &&save4, 0, 0, 0, &&save8
    };
    static void *restores[] = {
	&&restorew, &&restore1, &&restore2, 0, &&restore4, 0, 0, 0, &&restore8
    };
    void *copier;
    static short old_saver;
    static long *old_addr = 0;
    long *src_addr, *dst_addr;
    short saver;
    short wrap;
    int j, words;
    static long old_colours = 0;
    static short foreground = 0, background = 0;
#endif
    static short old_h = 0;
    int i;
    int xs, ys, w, h;
    MFDB src, dst;

#ifndef FAST_MOUSE
    foreground = wk->mouse.colour.foreground;  /* x_get_colour in blit */
    background = wk->mouse.colour.background;
#else
    if (*(long *)&wk->mouse.colour != old_colours) {
	old_colours = *(long *)&wk->mouse.colour;
	foreground = x_get_colour(wk, wk->mouse.colour.foreground);
	background = x_get_colour(wk, wk->mouse.colour.background);
    }

    saver = wk->screen.mfdb.bitplanes;
#endif

    xs = ys = 0;
    x -= wk->mouse.hotspot.x;
    y -= wk->mouse.hotspot.y;
    w = h = 16;

    if (x < wk->screen.coordinates.min_x) {
	xs = wk->screen.coordinates.min_x - x;
	w -= xs;
	x = wk->screen.coordinates.min_x;
    }
    if (x + w - 1 > wk->screen.coordinates.max_x) {
	w = wk->screen.coordinates.max_x - x + 1;
	saver >>= 1;
    }

    if (y < wk->screen.coordinates.min_y) {
	ys = wk->screen.coordinates.min_y - y;
	h -= ys;
	y = wk->screen.coordinates.min_y;
    }
    if (y + h - 1 > wk->screen.coordinates.max_y)
	h = wk->screen.coordinates.max_y - y + 1;

    if ((w < 0) || (h < 0))
	w = h = 0;

    src.width     = dst.width     = 16;
    src.height    = dst.height    = 16;
    src.wdwidth   = dst.wdwidth   = 1;
    src.standard  = dst.standard  = 0;
//    src.bitplanes = dst.bitplanes = 1;

#ifdef FAST_MOUSE
 #if 0
    words = wk->screen.mfdb.bitplanes - 1;
 #endif
    wrap = wk->screen.wrap - saver * 4;
#endif

    if (((long)mouse == 0) || ((long)mouse == 2)) { /* Move or Hide */
#ifndef FAST_MOUSE
        /* Restore old background */
	src.address = saved;
	src.bitplanes = wk->screen.mfdb.bitplanes;
	x_blit_area(wk, &src, 0, 0, 0, old_x, old_y, old_w, old_h, 3);
#else
        /* Restore old background */
        copier = restores[old_saver];
	src_addr = saved;
	dst_addr = old_addr;
	for(i = old_h - 1; i >= 0; i--) {
 #if 0
	    for(j = words; j >= 0; j--) {
		*dst_addr++ = *src_addr++;
	    }
 #else
	goto *copier;
	restorew:
	    *(short *)dst_addr = *(short *)src_addr;
	    src_addr = (long *)((long)src_addr + 2);
            goto restore0;
	restore8:
	    *dst_addr++ = *src_addr++;
	    *dst_addr++ = *src_addr++;
	    *dst_addr++ = *src_addr++;
	    *dst_addr++ = *src_addr++;
	restore4:
	    *dst_addr++ = *src_addr++;
	    *dst_addr++ = *src_addr++;
	restore2:
	    *dst_addr++ = *src_addr++;
	restore1:
	    *dst_addr++ = *src_addr++;
	restore0:
 #endif
	    dst_addr = (long *)((long)dst_addr + wrap);
	}
#endif
    }

    if (((long)mouse == 0) || ((long)mouse == 3)) { /* Move or Show */
#ifndef FAST_MOUSE
	/* Save new background */
	dst.address = saved;
	dst.bitplanes = wk->screen.mfdb.bitplanes;
	x_blit_area(wk, 0, x, y, &dst, 0, 0, w, h, 3);
	old_x = x;
	old_y = y;
	old_w = w;
	old_h = h;
#else
	/* Save new background */
	src_addr = (long *)((long)wk->screen.mfdb.address +
                            (short)y * (long)wk->screen.wrap +
                            (x >> 4) * wk->screen.mfdb.bitplanes * 2);
        copier = saves[saver];
	old_saver = saver;
	old_addr = src_addr;
	old_h = h;
	dst_addr = saved;
	for(i = h - 1; i >= 0; i--) {
 #if 0
	    for(j = words; j >= 0; j--) {
		*dst_addr++ = *src_addr++;
	    }
 #else
	    goto *copier;
	savew:
	    *(short *)dst_addr = *(short *)src_addr;
	    dst_addr = (long *)((long)dst_addr + 2);
            goto save0;
	save8:
	    *dst_addr++ = *src_addr++;
	    *dst_addr++ = *src_addr++;
	    *dst_addr++ = *src_addr++;
	    *dst_addr++ = *src_addr++;
	save4:
	    *dst_addr++ = *src_addr++;
	    *dst_addr++ = *src_addr++;
	save2:
	    *dst_addr++ = *src_addr++;
	save1:
	    *dst_addr++ = *src_addr++;
	save0:
 #endif
	    src_addr = (long *)((long)src_addr + wrap);
	}
#endif
	/* Draw mask */
	src.address = mask;
	src.bitplanes = 1;
	x_expand_area(wk, &src, xs, ys, 0, x, y, w, h, 2, background);
	/* Draw shape */
	src.address = data;
	src.bitplanes = 1;
	x_expand_area(wk, &src, xs, ys, 0, x, y, w, h, 2, foreground);
    }

    if ((long)mouse > 7) {   /* New mouse shape */
	for(i = 0; i < 16; i++) {
	    mask[i] = mouse->mask[i];
	    data[i] = mouse->data[i];
	}
    }

    return 0; 
}
#endif
