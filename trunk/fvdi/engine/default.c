/*
 * fVDI default drawing function code
 *
 * $Id: default.c,v 1.5 2005-08-04 10:14:54 johan Exp $
 *
 * Copyright 2003, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 *
 * The Bresenham line drawing variation with perfect clipping is
 * based on code from the Graphics Gems series of books.
 */

#include "fvdi.h"
#include "function.h"


extern call_draw_line(Virtual *vwk, DrvLine *line);

#if 0
#define LEFT            1
#define RIGHT           2
#define BOTTOM          4
#define TOP             8

#define SWAP(x, y)      { int _t = x; x = y; y = _t; }

#define OUTCODE(x, y, outcode, type)                                     \
{                                                                        \
  if (x < clip[0]) {                                                     \
    outcode = LEFT;                                                      \
    type = 1;                                                            \
  } else if (x > clip[2]) {                                              \
    outcode = RIGHT;                                                     \
    type = 1;                                                            \
  } else                                                                 \
    outcode = type = 0;                                                  \
  if (y < clip[3]) {                                                     \
    outcode |= BOTTOM;                                                   \
    type++;                                                              \
  } else if (y > clip[1]) {                                              \
    outcode |= TOP;                                                      \
    type++;                                                              \
  }                                                                      \
}

#define CLIP(a1, a2, b1, da, da2, db2, as, bs, sa, sb,                   \
             amin, AMIN, amax, AMAX, bmin, BMIN, bmax, BMAX)             \
{                                                                        \
  long ca, cb;                                                           \
  if (out1) {                                                            \
    if (out1 & AMIN) {                                                   \
      ca = db2 * (amin - a1);                                            \
      as = amin;                                                         \
    } else if (out1 & AMAX) {                                            \
      ca = db2 * (a1 - amax);                                            \
      as = amax;                                                         \
    }                                                                    \
    if (out1 & BMIN) {                                                   \
      cb = da2 * (bmin - b1);                                            \
      bs = bmin;                                                         \
    } else if (out1 & BMAX) {                                            \
      cb = da2 * (b1 - bmax);                                            \
      bs = bmax;                                                         \
    }                                                                    \
    if (type1 == 2)                                                      \
      out1 &= (ca + da < cb + !dir) ? ~(AMIN | AMAX) : ~(BMAX | BMIN);   \
    if (out1 & (AMIN | AMAX)) {                                          \
      cb = (ca + da - !dir) / da2;                                       \
      if (sb >= 0) {                                                     \
        if ((bs = b1 + cb) > bmax)                                       \
          return ptr;                                                    \
      } else {                                                           \
        if ((bs = b1 - cb) < bmin)                                       \
          return ptr;                                                    \
      }                                                                  \
      r += ca - da2 * cb;                                                \
    } else {                                                             \
      ca = (cb - da + db2 - dir) / db2;                                  \
      if (sa >= 0) {                                                     \
        if ((as = a1 + ca) > amax)                                       \
          return ptr;                                                    \
      } else {                                                           \
        if ((as = a1 - ca) < amin)                                       \
          return ptr;                                                    \
      }                                                                  \
      r += db2 * ca - cb;                                                \
    }                                                                    \
  } else {                                                               \
    as = a1;                                                             \
    bs = b1;                                                             \
  }                                                                      \
  alt = 0;                                                               \
  if (out2) {                                                            \
    if (type2 == 2) {                                                    \
      ca = db2 * ((out2 & AMIN) ? a1 - amin : amax - a1);                \
      cb = da2 * ((out2 & BMIN) ? b1 - bmin : bmax - b1);                \
      out2 &= (cb + da < ca + dir) ? ~(AMIN | AMAX) : ~(BMIN | BMAX);    \
    }                                                                    \
    if (out2 & (AMIN | AMAX))                                            \
      n = (out2 & AMIN) ? as - amin : amax - as;                         \
    else {                                                               \
      n = (out2 & BMIN) ? bs - bmin : bmax - bs;                         \
      alt = 1;                                                           \
    }                                                                    \
  } else                                                                 \
    n = (a2 >= as) ? a2 - as : as - a2;                                  \
}


short *clip_line(short x1, short y1, short x2, short y2,
                 short clip[], short draw_last, short *ptr,
                 short *total, short *first, short *drawn)
/*
 *      If dir = 0, round towards (x1, y1)
 *      If dir = 1, round towards (x2, y2)
 */
{
  short dir = 0;
  short adx, ady, adx2, ady2, sx, sy;
  short out1, out2, type1, type2;
  short r, xs, ys, n, alt;

  OUTCODE(x1, y1, out1, type1);
  OUTCODE(x2, y2, out2, type2);
  if (out1 & out2)
    return ptr;

#if 0
  /* Swap coordinates in some cases for efficiency */
  if ((type1 != 0 && type2 == 0) || (type1 == 2 && type2 == 1)) {
    SWAP(out1, out2);
    SWAP(type1, type2);
    SWAP(x1, x2);
    SWAP(y1, y2);
    dir ^= 1;
  }
#endif

  xs = x1;
  ys = y1;
  sx = 1;
  adx = x2 - x1;
  if (adx < 0) {
    adx = -adx;
    sx = -1;
  }
  sy = 1;
  ady = y2 - y1;
  if (ady < 0) {
    ady = -ady;
    sy = -1;
  }
  adx2 = 2 * adx;
  ady2 = 2 * ady;

  *first = *drawn = 0;

  if (adx >= ady) {
    /*
     *      Line is semi-horizontal
     */
    *total = adx + 1;
    r = ady2 - adx - !dir;
    CLIP(x1, x2, y1, adx, adx2, ady2, xs, ys, sx, sy,
         clip[0], LEFT, clip[2], RIGHT, clip[3], BOTTOM, clip[1], TOP);
    r -= ady2;
    *first = xs;
    if (alt) {
      for(;; xs += sx) {       /* Alternate Bresenham */
        *ptr++ = xs;
        *ptr++ = ys;
        r += ady2;
        if (r >= 0) {
          if (--n < 0)
            break;
          r -= adx2;
          ys += sy;
        }
      }
    } else {
      do {       /* Standard Bresenham */
        *ptr++ = xs;
        *ptr++ = ys;
        r += ady2;
        if (r >= 0) {
          r -= adx2;
          ys += sy;
        }
        xs += sx;
      } while (n--);
    }
    *drawn = ABS(ptr[-2] - *first + 1);
    *first = ABS(*first - x1);
  } else {
    /*
     *      Line is semi-vertical
     */
    *total = ady + 1;
    r = adx2 - ady - !dir;
    CLIP(y1, y2, x1, ady, ady2, adx2, ys, xs, sy, sx,
         clip[3], BOTTOM, clip[1], TOP, clip[0], LEFT, clip[2], RIGHT);
    r -= adx2;
    *first = ys;
    if (alt) {
      for(;; ys += sy) {       /* Alternate Bresenham */
        *ptr++ = xs;
        *ptr++ = ys;
        r += adx2;
        if (r >= 0) {
          if (--n < 0)
            break;
          r -= ady2;
          xs += sx;
        }
      }
    } else {
      do {       /* Standard Bresenham */
        *ptr++ = xs;
        *ptr++ = ys;
        r += adx2;
        if (r >= 0) {
          r -= ady2;
          xs += sx;
        }
        ys += sy;
      } while (n--);
    }
    *drawn = ABS(ptr[-1] - *first + 1);
    *first = ABS(*first - y1);
  }

  if (!draw_last && (ptr[-2] == x2) && (ptr[-1] == y2)) {
    ptr -= 2;
    *drawn -= 1;
    *total -= 1;
  }

  return ptr;
}


static void polymline(short table[], int length, short index[], int moves,
                      short clip[], short *points)
{
	short n;
	short x1, y1, x2, y2, init_x, init_y;
	short movepnt;
	short *ptr, *oldptr;
	short draw_last;
	short total, first, drawn;

	ptr = points;
	oldptr = points;

	x1 = *table++;
	y1 = *table++;

	if (moves) {
		init_x = x1;
		init_y = y1;
	} else {
		init_x = index[0];
		init_y = index[1];
	}

	movepnt = -1;
	if (index) {
		moves--;
		if (index[moves] == -4)
			moves--;
		if (index[moves] == -2)
			moves--;
		if (moves >= 0)
			movepnt = (index[moves] + 4) / 2;
	}

	for(n = 1; n < length; n++) {
		x2 = *table++;
		y2 = *table++;
		if (n == movepnt) {
			if (--moves >= 0)
				movepnt = (index[moves] + 4) / 2;
			else
				movepnt = -1;		/* Never again equal to n */
			init_x = x1 = x2;
			init_y = y1 = y2;
			continue;
		}

		if (((n == length - 1) || (n == movepnt - 1)) &&
		    ((x2 != init_x) || (y2 != init_y)))
			draw_last = 1;		/* Draw last pixel in the line */
		else
			draw_last = 0;		/* Do not draw last pixel in the line */

		newptr = clip_line(x1, y1, x2, y2, clip, draw_last, ptr,
		                   &total, &first, &drawn);

		if (first) {
			/* Beginning was clipped, so draw previous part */
			draw...
			if (total - first != drawn) {
				/* End was clipped, so draw this part */
				draw...	rotate_left(pattern, (previous + first) % 16);
				pattern = rotate_left(pattern, (previous + total) % 16);
				oldptr = ptr = points;
			} else {
				pattern = rotate_left(pattern, (previous + first) % 16);
				oldptr = ptr;
				ptr = newptr;
			}
		} else if (total - first != drawn) {
			/* End was clipped, so draw previous and this part */
			draw...;
			pattern = rotate_left(pattern, (previous + total) % 16);
			oldptr = ptr = points;
		}

		if (not enough space left for a max length line) {
			draw current points
			pattern = rotate_left(pattern, all % 16);
			oldptr = ptr = points;
		}


		x1 = x2;
		y1 = y2;
	}
}


/*
 * Pixel by pixel line routine
 * In:   a0      VDI struct (odd address marks table operation)
 *       d0      line colour
 *       d1      x1 or table address
 *       d2      y1 or table length (high) and type (0 - coordinate pairs, 1 - pairs+moves)
 *       d3      x2 or move point count
 *       d4      y2 or move index address
 *       d5      pattern
 *       d6      mode
 */
int default_line(Virtual *vwk, long x1, long y1, long x2, long y2,
                long pattern, long colour, long mode)
{
	ulong foreground, background;
	ulong pat;
	short *table, length, *index, moves;
	short ltab[4];

	table = index = 0;
	length = moves = 0;
	if ((long)vwk & 1) {
		if ((unsigned long)(y1 & 0xffff) > 1)
			return -1;		/* Don't know about this kind of table operation */
		table = (short *)x1;
		length = (y1 >> 16) & 0xffff;
		if ((y1 & 0xffff) == 1) {
			index = (short *)y2;
			moves = x2 & 0xffff;
		}
		vwk = (Virtual *)((long)vwk - 1);
		x1 = table[0];
		y1 = table[1];
		x2 = table[2];
		y2 = table[3];
	} else {
		table = ltab;
		length = 2;
		table[0] = x1;
		table[1] = y1;
		table[2] = x2;
		table[3] = y2;
	}

	get_colours_r(vwk, colour, &foreground, &background);

	if (moves)
		polymline(table, length, index, moves,
		          (short *)&vwk->clip.rectangle, points);
	else {
		short connected = (x1 == table[(length - 1) * 2]) &&
		                  (y1 == table[(length - 1) * 2 + 1]);
		index = table;		/* To check for connection in polymline */

		table += 4;
		for(--length; length > 0; length--) {
			short skip_last = connected || (length != 1);
			if (x1 == x2) {
				if (y1 < y2) {
					short y_max = y2 - skip_last;
					if (y1 < vwk->clip.rectangle.y1)
						y1 = vwk->clip.rectangle.y1;
					if (y_max > vwk->clip.rectangle.y2)
						y_max = vwk->clip.rectangle.y2;
					for(; y1 <= y_max; y1++) {
						*ptr++ = x1;
						*ptr++ = y1;
					}
				} else {
					short y_min = y2 + skip_last;
					if (y1 > vwk->clip.rectangle.y2)
						y1 = vwk->clip.rectangle.y2;
					if (y_min < vwk->clip.rectangle.y1)
						y_min = vwk->clip.rectangle.y1;
					for(; y1 >= y_min; y1--) {
						*ptr++ = x1;
						*ptr++ = y1;
					}
				}
			} else if (y1 == y2) {
				if (x1 < x2) {
					short x_max = x2 - skip_last;
					if (x1 < vwk->clip.rectangle.x1)
						x1 = vwk->clip.rectangle.x1;
					if (x_max > vwk->clip.rectangle.x2)
						x_max = vwk->clip.rectangle.x2;
					for(; x1 <= x_max; x1++) {
						*ptr++ = x1;
						*ptr++ = y1;
					}
				} else {
					short x_min = x2 + skip_last;
					if (x1 > vwk->clip.rectangle.x2)
						x1 = vwk->clip.rectangle.x2;
					if (x_min < vwk->clip.rectangle.x1)
						x_min = vwk->clip.rectangle.x1;
					for(; x1 >= x_min; x1--) {
						*ptr++ = x1;
						*ptr++ = y1;
					}
				}
			} else {
				length++;
				table -= 4;
				break;
			}
			x1 = x2;
			y1 = y2;
			x2 = *table++;
			y2 = *table++;
		}
		if (length) {
			polymline(table, length, index, 0, (short *)&vwk->clip.rectangle, points);
		}
	}

	return 1;
}



/*
 * Fill a multiple bitplane area using a monochrome pattern
 * c_fill_area(Virtual *vwk, long x, long y, long w, long h, short *pattern, long colour, long mode, long interior_style)
 * In:	a0	VDI struct (odd address marks table operation)
 *	d0	colours
 *	d1	x1 destination or table address
 *	d2	y1    - " -    or table length (high) and type (0 - y/x1/x2 spans)
 *	d3-d4	x2,y2 destination
 *	d5	pattern address
 *	d6	mode
 *	d7	interior/style
 */
long c_fill_area(Virtual *vwk, long x, long y, long w, long h,
                short *pattern, long colour, long mode, long interior_style)
{
	ulong foreground, background;
	short *table;

	table = 0;
	if ((long)vwk & 1) {
		if ((y & 0xffff) != 0)
			return -1;		/* Don't know about this kind of table operation */
		table = (short *)x;
		h = (y >> 16) & 0xffff;
		vwk = (Virtual *)((long)vwk - 1);
	}
	
	get_colours_r(vwk, colour, &foreground, &background);

	if ((interior_style >> 16) >= 2 && (interior_style & 0xffffL) != 8) {	
		/* Pattern */
		ulong pat0, pat1;
		pat0 = (((long)pattern[3] & 0x00ff) << 24) | (((long)pattern[2] & 0x00ff) << 16) | (((long)pattern[1] & 0x00ff) << 8) | ((pattern[0] & 0x00ff));
		pat1 = (((long)pattern[7] & 0x00ff) << 24) | (((long)pattern[6] & 0x00ff) << 16) | (((long)pattern[5] & 0x00ff) << 8) | ((pattern[4] & 0x00ff));

		write_raw_pattern(register_base, pat0, pat1);			/* Six register writes */
#if 0
	} else if ((interior_style >> 16) == 0) {
		/* Setup pixel data path */
		reg_w(register_base, DP_SRC, FRGD_SRC_BKGD_CLR);
#endif
	} else {
		/* Setup pixel data path */
		reg_w(register_base, DP_SRC, FRGD_SRC_FRGD_CLR);
	}

	/* Perform rectangle fill. */
	if (!table) {
		dst_yx(x, y);
		dst_hw(w, h);
	} else {
		for(h = h - 1; h >= 0; h--) {
			y = *table++;
			x = *table++;
			w = *table++ - x + 1;
#ifdef CHECK_FIFO
			while ((swap16(reg_r(register_base, FIFO_STAT) & 0xFFFF)) > ((unsigned int)(0x8000 >> 2)));
#endif
			dst_yx(x, y);
			dst_hw(w, 1);
		}
	}
	
	return 1;
}


retry_set_pixel(Virtual *vwk, MFDB *mfdb, long x, long y, long colour,
                long pattern, long mode)
{
	short *table;
	short length;

	if ((long)vwk & 1) {
		if ((unsigned long)(y & 0xffff) > 1)
			return -1;		/* Don't know about this kind of table operation */
		table = (short *)x;
		length = (y >> 16) & 0xffff;
		vwk = (Virtual *)((long)vwk - 1);
	} else
		return;

	if ((y & 0xffff) == 0) {
		for(--length; length >= 0; length--) {
			x = *table++;
			y = *table++;
			c_set_pixel(vwk, mfdb, x, y, colour);
		}
	} else {
		ushort pat;
		ulong foreground, background;
		get_colours_r(vwk, colour, &foreground, &background);
		pat = pattern;
		for(--length; length >= 0; length--) {
			x = *table++;
			y = *table++;
			switch (mode) {
			case 1:
				if (pat & 0x8000)
					c_set_pixel(vwk, mfdb, x, y, foreground);
				else
					c_set_pixel(vwk, mfdb, x, y, background);
				break;
			case 2:
				if (pat & 0x8000)
					c_set_pixel(vwk, mfdb, x, y, foreground);
				break;
			case 3:
				if (pat & 0x8000)
					c_set_pixel(vwk, mfdb, x, y,
					            ~c_get_pixel(vwk, mfdb, x, y));
				break;
			case 4:
				if (!(pat & 0x8000))
					c_set_pixel(vwk, mfdb, x, y, foreground);
				break;
			}
			pat = (pat << 1) || (pat >> 15);
		}			
	}
}
#endif


#if 0
	tst.l		d0
	lbgt		.l1,1
	lbmi		.l2,2
	move.l		_fallback_line,d0
	bra		give_up

 label .l1,1
	movem.l		(a7)+,d0-d2/a0-a2
	rts
#endif

#if 0
retry_line(Virtual *vwk, long x1, long y1, long x2, long y2,
           long pattern, long colour, long mode, long draw_last)
#else
long retry_line(Virtual *vwk, DrvLine *pars)
#endif
{
	short *table, length, *index, moves;
	short n;
	short init_x, init_y, x2, y2;
	short movepnt;
	DrvLine line;

	table = index = 0;
	length = moves = 0;
	if ((long)vwk & 1) {
		if ((unsigned long)(pars->y1 & 0xffff) > 1)
			return;		/* Don't know about this kind of table operation */
		table = (short *)pars->x1;
		length = (pars->y1 >> 16) & 0xffff;
		if ((pars->y1 & 0xffff) == 1) {
			index = (short *)pars->y2;
			moves = pars->x2 & 0xffff;
		}
		vwk = (Virtual *)((long)vwk - 1);
	} else
		return;

	/* Clear high words of coordinate unions? */
	line.pattern = pars->pattern;
	line.colour = pars->colour;
	line.mode = pars->mode;

	init_x = line.x1 = *table++;
	init_y = line.y1 = *table++;

	movepnt = -1;
	if (index) {
		moves--;
		if (index[moves] == -4)
			moves--;
		if (index[moves] == -2)
			moves--;
		if (moves >= 0)
			movepnt = (index[moves] + 4) / 2;
	}

	for(n = 1; n < length; n++) {
		x2 = *table++;
		y2 = *table++;
		if (n == movepnt) {
			if (--moves >= 0)
				movepnt = (index[moves] + 4) / 2;
			else
				movepnt = -1;		/* Never again equal to n */
			init_x = line.x1 = x2;
			init_y = line.y1 = y2;
			continue;
		}

		if (((n == length - 1) || (n == movepnt - 1)) &&
		    ((x2 != init_x) || (y2 != init_y)))
			line.draw_last = 1;	/* Draw last pixel in the line */
		else
			line.draw_last = 0;	/* Do not draw last pixel in the line */

#if 0
		c_draw_line(vwk, x1, y1, x2, y2, pattern, colour, mode, draw_last);
#else
		line.x2 = x2;
		line.y2 = y2;
		call_draw_line(vwk, &line);
#endif

		line.x1 = x2;
		line.y1 = y2;
	}
}


int colour_entry(Virtual *vwk, long subfunction, short *intin, short *intout)
{
  switch(subfunction) {
  case 0:     /* v_color2value */
    puts("v_color2value not yet supported\x0a\x0d");
    return 2;
  case 1:     /* v_value2color */
    puts("v_value2color not yet supported\x0a\x0d");
    return 6;
  case 2:     /* v_color2nearest */
    puts("v_color2nearest not yet supported\x0a\x0d");
    return 6;
  case 3:     /* vq_px_format */
    puts("vq_px_format not yet supported\x0a\x0d");
    *(long *)&intout[0] = 1;
    *(long *)&intout[2] = 0x03421820;
    return 4;
  default:
    puts("Unknown colour entry operation\x0a\x0d");
    return 0;
  }
}


extern int set_col_table(Virtual *vwk, long colours, long start, void *values);

int set_colour_table(Virtual *vwk, long subfunction, short *intin)
{
  COLOR_TAB *ctab;

  switch(subfunction) {
  case 0:     /* vs_ctab */
    ctab = (COLOR_TAB *)intin;
#if 0
    puts("vs_ctab still not verified\x0a\x0d");
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
      puts("\x0a\x0d");
      for(i = 0; i < 6; i++) {
	ltoa(buf, *lptr++, 16);
	puts(buf);
	puts(" ");
      }
      puts("\x0a\x0d");
      sptr = (short *)&ctab->colors;
      for(j = 0; j < 64; j++) {
	for(i = 0; i < 16; i++) {
	  ltoa(buf, *sptr++ & 0xffffL, 16);
	  puts(buf);
	  puts(" ");
	}
	puts("\x0a\x0d");
      }
    }
#endif
    set_col_table(vwk, ctab->no_colors, 0, &ctab->colors);
    return vwk->real_address->screen.palette.size;    /* Not really correct */
  case 1:     /* vs_ctab_entry */
    puts("vs_ctab_entry not yet supported\x0a\x0d");
    return 1;      /* Seems to be the only possible value for non-failure */
  case 2:     /* vs_dflt_ctab */
    puts("vs_dflt_ctab not yet supported\x0a\x0d");
    return 256;    /* Not really correct */
  default:
    puts("Unknown set colour table operation\x0a\x0d");
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
      puts("vq_ctab not yet really supported\x0a\x0d");
      if (length > *(long *)&intin[0]) {
	char buf[10];
	puts("  Too little space available for ctab (");
	ltoa(buf, *(long *)&intin[0], 10);
	puts(buf);
	puts(" when ctab needs ");
	ltoa(buf, length, 10);
	puts(buf);
	puts(")!\x0a\x0d");
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
	ctab->colors[i].rgb.red   = i & 0xe0;
	ctab->colors[i].rgb.green = (i & 0x1c) << 3;
	ctab->colors[i].rgb.blue  = (i & 0x03) << 6;
      }
      return 256;    /* Depending on palette size */
    }
  case 1:     /* vq_ctab_entry */
    puts("vq_ctab_entry not yet supported\x0a\x0d");
    return 6;
  case 2:     /* vq_ctab_id */
    puts("vq_ctab_id not yet supported\x0a\x0d");
    *(long *)&intout[0] = 0xbadc0de1;   /* Not really correct */
    return 2;
  case 3:     /* v_ctab_idx2vdi */
    if (vwk->real_address->driver->device->clut != 1)
      intout[0] = intin[0];
    else if (intin[0] == vwk->real_address->screen.palette.size - 1)
      intout[0] = 1;
    else if ((intin[0] >= 16) && (intin[0] != 255))
      intout[0] = intin[0];
    else {
      static signed char vdi_colours[] = {0,2,3,6,4,7,5,8,9,10,11,14,12,15,13,-1};
      intout[0] = vdi_colours[intin[0]];
      if (intout[0] < 0)
	intout[0] = vwk->real_address->screen.palette.size - 1;
    }
    return 1;
  case 4:     /* v_ctab_vdi2idx */
    if (vwk->real_address->driver->device->clut != 1)
      intout[0] = intin[0];
    else if (intin[0] == 255)
      intout[0] = 15;
    else if (intin[0] >= 16)
      intout[0] = intin[0];
    else {
      static signed char tos_colours[] = {0,-1,1,2,4,6,3,5,7,8,9,10,12,14,11,13};
      intout[0] = tos_colours[intin[0]];
      if (intout[0] < 0)
	intout[0] = vwk->real_address->screen.palette.size - 1;
    }
    return 1;
  case 5:     /* v_ctab_idx2value */
    puts("v_ctab_idx2value not yet supported\x0a\x0d");
    return 2;
  case 6:     /* v_get_ctab_id */
    puts("v_get_ctab_id not yet supported\x0a\x0d");
    *(long *)&intout[0] = 0xbadc0de1;   /* Should always be different */
    return 2;
  case 7:     /* vq_dflt_ctab */
    puts("vq_dflt_ctan not yet supported\x0a\x0d");
    return 256;    /* Depending on palette size */
  case 8:     /* v_create_ctab */
    puts("v_create_ctab not yet supported\x0a\x0d");
    return 2;
  case 9:     /* v_delete_ctab */
    puts("v_delete_ctab not yet supported\x0a\x0d");
    intout[0] = 1;   /* OK */
    return 1;
  default:
    puts("Unknown colour table operation\x0a\x0d");
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
      puts("v_create_itab not yet supported\x0a\x0d");
      *(long *)&intout[0] = 0xbadc0de1;
      return 2;
    }
  case 1:     /* v_delete_itab */
    puts("v_delete_itab not yet supported\x0a\x0d");
    intout[0] = 1;   /* OK */
    return 1;
  default:
    puts("Unknown inverse colour table operation\x0a\x0d");
    return 0;
  }
}


/* This is in a rather adhoc state at the moment, unfortunately.
 * But it does seem to work, for what it does, and is not complicated.
 */
void vr_transfer_bits(Virtual *vwk, GCBITMAP *src_bm, GCBITMAP *dst_bm,
                      RECT16 *src_rect, RECT16 *dst_rect, long mode)
{
  char buf[10];
  char *error;

  error = 0;

  do {
    if ((src_rect->x2 - src_rect->x1 != dst_rect->x2 - dst_rect->x1) ||
        (src_rect->y2 - src_rect->y1 != dst_rect->y2 - dst_rect->y1)) {
	error = "No support yet for scaling";
	break;
    }

    if (!src_bm) {
	error = "No support yet for screen->memory/screen";
	break;
    }

    if (dst_bm) {
      if (src_bm->px_format != dst_bm->px_format) {
	if ((src_bm->px_format == 0x01020101) &&
            (dst_bm->px_format == 0x01020808)) {
	  int x, y;

	  for(y = src_rect->y1; y <= src_rect->y2; y++) {
	    long *src;
	    char *dst;
	    unsigned long v, mask;

	    src = (long *)(src_bm->addr + src_bm->width * y + (src_rect->x1 / 32) * 4);
	    mask = 1 << (31 - src_rect->x1 % 32);
	    dst = dst_bm->addr +
	      dst_bm->width * (dst_rect->y1 - src_rect->y1 + y) +
	      dst_rect->x1;
	    v = *src++;
	    if (mode == 33) {
	      for(x = src_rect->x2 - src_rect->x1; x >= 0; x--) {
		if (v & mask)
		  *dst = ~0;
		dst++;
		mask = (mask >> 1) | (mask << 31);
		if ((long)mask < 0)
		  v = *src++;
	      }
	    } else {
	      for(x = src_rect->x2 - src_rect->x1; x >= 0; x--) {
		if (v & mask)
		  *dst++ = ~0;
		else
		  *dst++ = 0;
		mask = (mask >> 1) | (mask << 31);
		if ((long)mask < 0)
		  v = *src++;
	      }
	    }
	  }
	  mode = 0;  /* Just to skip error printout at the end */
	} else if ((src_bm->px_format == 0x01020101) &&
		   (dst_bm->px_format == 0x03421820)) {
	  int x, y;

	  for(y = src_rect->y1; y <= src_rect->y2; y++) {
	    long *src, *dst;
	    unsigned long v, mask;

	    src = (long *)(src_bm->addr + src_bm->width * y + (src_rect->x1 / 32) * 4);
	    mask = 1 << (31 - src_rect->x1 % 32);
	    dst = (long *)(dst_bm->addr +
			   dst_bm->width * (dst_rect->y1 - src_rect->y1 + y)) +
	      dst_rect->x1;
	    v = *src++;
	    if (mode == 33) {
	      for(x = src_rect->x2 - src_rect->x1; x >= 0; x--) {
		if (v & mask)
		  *dst = ~0;
		dst++;
		mask = (mask >> 1) | (mask << 31);
		if ((long)mask < 0)
		  v = *src++;
	      }
	    } else {
	      for(x = src_rect->x2 - src_rect->x1; x >= 0; x--) {
		if (v & mask)
		  *dst++ = ~0;
		else
		  *dst++ = 0;
		mask = (mask >> 1) | (mask << 31);
		if ((long)mask < 0)
		  v = *src++;
	      }
	    }
	  }
	  mode = 0;  /* Just to skip error printout at the end */
	} else {
	  error = "No support yet for memory->memory between these different pixmap formats";
	  break;
	}
      } else {
	if (src_bm->px_format == 0x01020101) { /* PX_PREF1 */
	  error = "No support yet for 1 bit memory->memory";
	  break;
	} else if (src_bm->px_format == 0x01020808) { /* PX_PREF8 */
	  int x, y;

	  for(y = src_rect->y1; y <= src_rect->y2; y++) {
	    char *src, *dst;
	    src = src_bm->addr + src_bm->width * y + src_rect->x1;
	    dst = dst_bm->addr +
	      dst_bm->width * (dst_rect->y1 - src_rect->y1 + y) +
	      dst_rect->x1;
	    for(x = src_rect->x2 - src_rect->x1; x >= 0; x--)
	      *dst++ = *src++;
	  }
	} else if (src_bm->px_format == 0x03421820) { /* PX_PREF32 */
	  int x, y;

	  for(y = src_rect->y1; y <= src_rect->y2; y++) {
	    long *src, *dst;
	    src = (long *)(src_bm->addr + src_bm->width * y) + src_rect->x1;
	    dst = (long *)(dst_bm->addr +
			   dst_bm->width * (dst_rect->y1 - src_rect->y1 + y)) +
	      dst_rect->x1;
	    for(x = src_rect->x2 - src_rect->x1; x >= 0; x--)
	      *dst++ = *src++;
	  }
	} else {
	  puts("Unsupported pixel format ($");
	  ltoa(buf, src_bm->px_format, 16);
	  puts(buf);
	  error = ") for memory->memory";
	  break;
	}
      }
    } else {
      if (src_bm->px_format == 0x01020101) { /* PX_PREF1 */
	int x, y, i;
	char *block;
	long *palette;
	MFDB mfdb;
	short coords[8];

#if 0
	if (!src_bm->ctab) {
	  error = "Need a colour table for 1 bit->TC";
	  break;
	}
	if (src_bm->ctab->color_space != 1) {
	  error = "Need an RGB colour table for 1 bit->TC";
	  break;
	}
#endif
	
	if (!(block = (char *)allocate_block(0))) {
	  error = "Could not allocate memory block";
	  break;
	}
	
#if 0
	palette = (long *)block;
	for(i = 0; i < src_bm->ctab->no_colors; i++) {
	  palette[i] = ((long)(src_bm->ctab->colors[i].rgb.red & 0xff) << 16) |
	    ((long)(src_bm->ctab->colors[i].rgb.green & 0xff) << 8) |
	    ((long)(src_bm->ctab->colors[i].rgb.blue & 0xff));
	}
#endif
	
	mfdb.address   = (short *)&block[src_bm->ctab->no_colors * sizeof(*palette)];
	mfdb.width     = src_rect->x2 - src_rect->x1 + 1;
	mfdb.height    = 1;
	mfdb.wdwidth   = (mfdb.width + 15) / 16;
	mfdb.standard  = 0;
	mfdb.bitplanes = 32;
	
	coords[0] = 0;
	coords[1] = 0;
	coords[2] = src_rect->x2 - src_rect->x1 + 1;
	coords[3] = 0;
	
	coords[4] = dst_rect->x1;
	coords[5] = dst_rect->y1;
	coords[6] = dst_rect->x2;
	coords[7] = dst_rect->y1;
	
	for(y = src_rect->y1; y <= src_rect->y2; y++) {
	  long *src, *dst;
	  unsigned long v, mask;
	  
	  src = (long *)(src_bm->addr + src_bm->width * y) + (src_rect->x1 / 32);
	  mask = 1 << (31 - src_rect->x1 % 32);
	  dst = (long *)&block[src_bm->ctab->no_colors * sizeof(*palette)];
	  v = *src++;
	  if (mode == 33) {
	    short coords2[8];
	    for(i = 0; i < 4; i++) {
	      coords2[i] = coords[i + 4];
	      coords2[i + 4] = coords[i];
	    }
	    lib_vdi_spppp(&lib_vro_cpyfm, vwk, 3, coords2, 0, &mfdb, 0);
	    for(x = src_rect->x2 - src_rect->x1; x >= 0; x--) {
	      if (v & mask)
		*dst = ~0;
	      dst++;
	      mask = (mask >> 1) | (mask << 31);
	      if ((long)mask < 0)
		v = *src++;
	    }
	  } else {
	    for(x = src_rect->x2 - src_rect->x1; x >= 0; x--) {
	      if (v & mask)
		*dst++ = ~0;
	      else
		*dst++ = 0;
	      mask = (mask >> 1) | (mask << 31);
	      if ((long)mask < 0)
		v = *src++;
	    }
	  }
	  lib_vdi_spppp(&lib_vro_cpyfm, vwk, 3, coords, &mfdb, 0, 0);
	  coords[5]++;
	  coords[7]++;
	}
	
	free_block(block);
#if 0
	mode = 0;  /* Just to skip error printout at the end */
#else
	error = "OK?";
#endif
      } else if (src_bm->px_format == 0x01020808) { /* PX_PREF8 */
	char *block;
	long *palette;
	unsigned char *src;
	long *dst;
	MFDB mfdb;
	short coords[8];
	int x, y, i;

	if (!src_bm->ctab) {
	  error = "Need a colour table for 8 bit->TC";
	  break;
	}
	if (src_bm->ctab->color_space != 1) {
	  error = "Need an RGB colour table for 8 bit->TC";
	  break;
	}
	
	if (!(block = (char *)allocate_block(0))) {
	  error = "Could not allocate memory block";
	  break;
	}
	
	palette = (long *)block;
	for(i = 0; i < src_bm->ctab->no_colors; i++) {
	  palette[i] = ((long)(src_bm->ctab->colors[i].rgb.red & 0xff) << 16) |
	    ((long)(src_bm->ctab->colors[i].rgb.green & 0xff) << 8) |
	    ((long)(src_bm->ctab->colors[i].rgb.blue & 0xff));
	}
	
	mfdb.address   = (short *)&block[src_bm->ctab->no_colors * sizeof(*palette)];
	mfdb.width     = src_rect->x2 - src_rect->x1 + 1;
	mfdb.height    = 1;
	mfdb.wdwidth   = (mfdb.width + 15) / 16;
	mfdb.standard  = 0;
	mfdb.bitplanes = 32;
	
	coords[0] = 0;
	coords[1] = 0;
	coords[2] = src_rect->x2 - src_rect->x1 + 1;
	coords[3] = 0;
	
	coords[4] = dst_rect->x1;
	coords[5] = dst_rect->y1;
	coords[6] = dst_rect->x2;
	coords[7] = dst_rect->y1;
	
	for(y = src_rect->y1; y <= src_rect->y2; y++) {
	  src = src_bm->addr + src_bm->width * y + src_rect->x1;
	  dst = (long *)&block[src_bm->ctab->no_colors * sizeof(*palette)];
	  for(x = src_rect->x2 - src_rect->x1; x >= 0; x--)
	    *dst++ = palette[*src++];
	  lib_vdi_spppp(&lib_vro_cpyfm, vwk, 3, coords, &mfdb, 0, 0);
	  coords[5]++;
	  coords[7]++;
	}
	
	free_block(block);
      } else if (src_bm->px_format == 0x03421820) { /* PX_PREF32 */
	MFDB mfdb;
	short coords[8];

	mfdb.address   = (short *)src_bm->addr;
	mfdb.width     = src_bm->xmax - src_bm->xmin;
	mfdb.height    = src_bm->ymax - src_bm->ymin;
	mfdb.wdwidth   = (mfdb.width + 15) / 16;
	mfdb.standard  = 0;
	mfdb.bitplanes = 32;
	
	coords[0] = src_rect->x1;
	coords[1] = src_rect->y1;
	coords[2] = src_rect->x2;
	coords[3] = src_rect->y2;
	
	coords[4] = dst_rect->x1;
	coords[5] = dst_rect->y1;
	coords[6] = dst_rect->x2;
	coords[7] = dst_rect->y2;
	
	lib_vdi_spppp(&lib_vro_cpyfm, vwk, 3, coords, &mfdb, 0, 0);
      } else {
	puts("Unsupported source pixel format ($");
	ltoa(buf, src_bm->px_format, 16);
	puts(buf);
	error = ") for !TC->TC";
	break;
      }
    }
  } while(0);

  if (error) {
    int i;

    puts(error);
    puts("\x0a\x0d");

    puts("vr_transfer_bits mode "),
    ltoa(buf, mode, 10);
    puts(buf);
    puts("\x0a\x0d");

    for(i = 0; i < 2; i++) {
      GCBITMAP *bm = (i == 0) ? src_bm : dst_bm;
      if (bm) {
	puts((i == 0) ? "SRC" : "DST");
	puts(" $");
	ltoa(buf, (long)bm->addr, 16);
	puts(buf);
	puts("  Width: ");
	ltoa(buf, bm->width, 10);
	puts(buf);
	puts("  Bits: ");
	ltoa(buf, bm->bits, 10);
	puts(buf);
	puts("  Format: $");
	ltoa(buf, bm->px_format, 16);
	puts(buf);
	if (bm->xmin || bm->ymin) {
	  puts("  Base: ");
	  ltoa(buf, bm->xmin, 10);
	  puts(buf);
	  puts(",");
	  ltoa(buf, bm->ymin, 10);
	  puts(buf);
	}
	puts("  Dim: ");
	ltoa(buf, bm->xmax - bm->xmin, 10);
	puts(buf);
	puts(",");
	ltoa(buf, bm->ymax - bm->ymin, 10);
	puts(buf);
	puts("\x0a\x0d");

	puts("    C/ITAB: $");
	ltoa(buf, (long)bm->ctab, 16);
	puts(buf);
	puts("/$");
	ltoa(buf, (long)bm->itab, 16);
	puts(buf);

	if (bm->ctab) {
	  puts("  ID: $");
	  ltoa(buf, bm->ctab->map_id, 16);
	  puts(buf);
	  puts("  Space: ");
	  ltoa(buf, bm->ctab->color_space, 10);
	  puts(buf);
	  puts("  Flags: $");
	  ltoa(buf, bm->ctab->flags, 16);
	  puts(buf);
	  puts("  Colours: ");
	  ltoa(buf, bm->ctab->no_colors, 10);
	  puts(buf);
	}

	puts("\x0a\x0d");
      }
    }

    puts("--------\x0a\x0d");
  } else if ((mode != 0) && (mode != 32)) {
    ltoa(buf, mode, 10);
    puts("\x0a\x0d");
    puts("vr_transform_bits mode ");
    puts(buf);
    puts("\x0a\x0d\x0a\x0d");
  }
}


#if 0
retry_fill(Virtual *vwk, long x, long y, long w, long h, short *pattern,
           long colour, long mode, long interior_style)
{
	short *table;
	short h;

	table = 0;
	if ((long)vwk & 1) {
		if ((y & 0xffff) != 0)
			return -1;		/* Don't know about this kind of table operation */
		table = (short *)x;
		h = (y >> 16) & 0xffff;
		vwk = (Virtual *)((long)vwk - 1);
	} else
		return;

	for(h = h - 1; h >= 0; h--) {
		y = *table++;
		x = *table++;
		w = *table++ - x + 1;
		c_fill_area(vwk, x, y, w, 1, pattern, colour, mode, interior_style);
	}
}
#endif


#if 0
	dc.b	0,"default_line",0
* _default_line - Pixel by pixel line routine
* In:	a0	VDI struct (odd address marks table operation)
*	d0	Colour
*	d1	x1 or table address
*	d2	y1 or table length (high) and type (0 - coordinate pairs, 1 - pairs+moves)
*	d3	x2 or move point count
*	d4	y2 or move index address
*	d5.w	Pattern
*	d6	Logic operation
* Call:	a0	VDI struct, 0 (destination MFDB)
*	d1-d2.w	Coordinates
*	a3-a4	Set/get pixel
_default_line:
	movem.l	d6-d7/a1/a3-a4,-(a7)

	move.w	a0,d7
	and.w	#1,d7
	sub.w	d7,a0

	move.l	vwk_real_address(a0),a1
	move.l	wk_r_get_colour(a1),a1	; Index to real colour
	jsr	(a1)

	clr.l	-(a7)			; No MFDB => draw on screen
	move.l	a0,-(a7)

	move.w	d6,-(a7)
	bsr	setup_plot		; Setup pixel plot functions (a1/a3/a4)
	addq.l	#2,a7

	tst.w	d7
	bne	.multiline

	bsr	clip_line
	bvs	.skip_draw

	move.l	a7,a0			; a0 no longer -> VDI struct!

	bsr	.draw
.skip_draw:

	move.l	(a7),a0
	addq.l	#8,a7

	movem.l	(a7)+,d6-d7/a1/a3-a4
	rts

.draw:
	move.l	#$00010001,d7		; d7 = y-step, x-step

	sub.w	d1,d3			; d3 = dx
	bge	.ok1
	neg.w	d3
	neg.w	d7	
.ok1:
	sub.w	d2,d4			; d4 = dy
	bge	.ok2
	neg.w	d4
	swap	d7
	neg.w	d7
	swap	d7
.ok2:
	and.l	#$ffff,d5
	cmp.w	d3,d4
	bls	.xmajor
	or.l	#$80000000,d5
	exg	d3,d4
.xmajor:
	add.w	d4,d4			; d4 = incrE = 2dy
	move.w	d4,d6
	sub.w	d3,d6			; d6 = lines, d = 2dy - dx
	swap	d4
	move.w	d6,d4
	sub.w	d3,d4			; d4 = incrE, incrNE = 2(dy - dx)

	rol.w	#1,d5
	jsr	(a1)

	swap	d1
	move.w	d2,d1
	swap	d1			; d1 = y, x
	bra	.loop_end1

.loop1:
	tst.w	d6
	bgt	.both
	swap	d4
	add.w	d4,d6
	swap	d4
	tst.l	d5
	bmi	.ymajor
	add.w	d7,d1
	bra	.plot
.ymajor:
	swap	d7
	swap	d1
	add.w	d7,d1
	swap	d7
	swap	d1
	bra	.plot
.both:
	add.w	d4,d6
;	add.l	d7,d1
	add.w	d7,d1
	swap	d7
	swap	d1
	add.w	d7,d1
	swap	d7
	swap	d1
.plot:
	move.l	d1,d2
	swap	d2
	rol.w	#1,d5
	jsr	(a1)

.loop_end1:
	dbra	d3,.loop1
	rts

.multiline:				; Transform multiline to single ones
	cmp.w	#1,d2
	bhi	.line_done		; Only coordinate pairs and pairs+marks available so far
	beq	.use_marks
	moveq	#0,d3			; Move count
.use_marks:
	swap	d3
	move.w	#1,d3			; Current index in high word
	swap	d3
	movem.l	d0/d2/d3/d5/a0/a5-a6,-(a7)
	move.l	d1,a5			; Table address
	move.l	d4,a6			; Move index address
	tst.w	d3			;  may not be set
	beq	.no_start_move
	add.w	d3,a6
	add.w	d3,a6
	subq.l	#2,a6
	cmp.w	#-4,(a6)
	bne	.no_start_movex
	subq.l	#2,a6
	sub.w	#1,d3
.no_start_movex:
	cmp.w	#-2,(a6)
	bne	.no_start_move
	subq.l	#2,a6
	sub.w	#1,d3
.no_start_move:
	bra	.line_loop_end
.line_loop:
	movem.w	(a5),d1-d4
	move.l	7*4(a7),a0
	bsr	clip_line
	bvs	.no_draw
	move.l	0(a7),d6		; Colour
	move.l	3*4(a7),d5		; Pattern
;	move.l	xxx(a7),d0		; Logic operation
	lea	7*4(a7),a0
	bsr	.draw
.no_draw:
	move.l	2*4(a7),d3
	tst.w	d3
	beq	.no_marks
	swap	d3
	addq.w	#1,d3
	move.w	d3,d4
	add.w	d4,d4
	subq.w	#4,d4
	cmp.w	(a6),d4
	bne	.no_move
	subq.l	#2,a6
	addq.w	#1,d3
	swap	d3
	subq.w	#1,d3
	swap	d3
	addq.l	#4,a5
	subq.w	#1,1*4(a7)
.no_move:
	swap	d3
	move.l	d3,2*4(a7)
.no_marks:
	addq.l	#4,a5
.line_loop_end:
	subq.w	#1,1*4(a7)
	bgt	.line_loop
	movem.l	(a7)+,d0/d2/d3/d5/a0/a5-a6
.line_done:
	move.l	(a7),a0
	addq.l	#8,a7

	movem.l	(a7)+,d6-d7/a1/a3-a4
	rts

	dc.b	0,"default_fill",0
* _default_fill - Line by line (or pixel by pixel) fill routine
* In:	a0	VDI struct (odd address marks table operation)
*	d0	Colours
*	d1	x1 destination or table address
*	d2	y1   - "" -     or table length (high) and type (0 - y/x1/x2 spans)
*	d3-d4.w	x2,y2 destination
*	d5	Pointer to pattern
*	d6	Mode
*	d7	Interior/style
* Call:	a0	VDI struct, 0 (destination MFDB)
*	d0	Colours
*	d1-d2.w	Start coordinates
*	d3-d4.w	End coordinates
*	d5	Pattern
_default_fill:
	movem.l	d1-d7/a0-a2,-(a7)

	move.w	a0,d7
	and.w	#1,d7
	sub.w	d7,a0

	move.l	d5,a2

	cmp.w	#1,d6			; Not replace?
	bne	.pattern

	move.l	a2,a1
	moveq	#8-1,d5
	move.l	d6,-(a7)
.check_pattern:
	move.l	(a1)+,d6		; All ones?
	addq.l	#1,d6
	dbne	d5,.check_pattern
	beq	.no_pattern
	move.l	(a7)+,d6
	bra	.pattern
.no_pattern:
	move.l	(a7)+,d6
	moveq	#-1,d5

	move.l	vwk_real_address(a0),a1
	move.l	wk_r_line(a1),a1
	cmp.l	#_default_line,a1	; No real acceleration?
	beq	.pattern

	tst.w	d7
	bne	.table_lfill

	move.w	d4,d7
	moveq	#0,d6
	move.w	vwk_mode(a0),d6
.loopy_sl:
	move.w	d2,d4
	jsr	(a1)
	addq.w	#1,d2
	cmp.w	d7,d2
	ble	.loopy_sl

.end_default_fill:
	movem.l	(a7)+,d1-d7/a0-a2
	rts

.table_lfill:
	tst.w	d2
	bne	.end_default_fill	; Only y/x1/x2 spans available so far
	move.l	d2,d6
	swap	d6
	subq.w	#1,d6
	blt	.end_default_fill
	move.l	d1,a2
.tlfill_loop:
	moveq	#0,d2
	move.w	(a2)+,d2
	move.w	d2,d4
	moveq	#0,d1
	move.w	(a2)+,d1
	move.w	(a2)+,d3
	jsr	(a1)
	dbra	d6,.tlfill_loop
	bra	.end_default_fill

* Call:	a0	VDI struct, 0 (destination MFDB)
*	d0	Colour values
*	d1-d2.w	Coordinates
*	d6	Mode
*	a3-a4	Set/get pixel
.pattern:
	movem.l	a3-a4,-(a7)

	move.l	vwk_real_address(a0),a1
	move.l	wk_r_get_colour(a1),a1	; Index to real colour
	jsr	(a1)

	move.w	d6,-(a7)
	bsr	setup_plot
	addq.l	#2,a7

;	move.l	d5,a2

	clr.l	-(a7)			; No MFDB => draw on screen
	move.l	a0,-(a7)
	move.l	a7,a0			; a0 no longer -> VDI struct!

	tst.w	d7
	bne	.table_pfill

	move.w	d1,d6
.loopy_pp:
	move.w	d6,d1			; x
	move.w	d2,d5
	and.w	#$000f,d5
	add.w	d5,d5
	move.w	0(a2,d5.w),d5
	rol.w	d1,d5
.loopx_pp:
	rol.w	#1,d5
	jsr	(a1)
	addq.w	#1,d1
	cmp.w	d3,d1
	ble	.loopx_pp
	addq.w	#1,d2
	cmp.w	d4,d2
	ble	.loopy_pp
.end_pfill:
	move.l	(a7),a0
	addq.l	#8,a7
	movem.l	(a7)+,a3-a4
	bra	.end_default_fill

.table_pfill:
	tst.w	d2
	bne	.end_pfill		; Only y/x1/x2 spans available so far
	move.l	d2,d6
	swap	d6
	subq.w	#1,d6
	blt	.end_pfill

	move.l	a5,-(a7)
	move.l	d1,a5
.tploopy_pp:
	moveq	#0,d2
	move.w	(a5)+,d2
	moveq	#0,d1
	move.w	(a5)+,d1
	move.w	(a5)+,d3

	move.w	d2,d5
	and.w	#$000f,d5
	add.w	d5,d5
	move.w	0(a2,d5.w),d5
	rol.w	d1,d5
.tploopx_pp:
	rol.w	#1,d5
	jsr	(a1)
	addq.w	#1,d1
	cmp.w	d3,d1
	ble	.tploopx_pp
	dbra	d6,.tploopy_pp
	move.l	(a7)+,a5
	bra	.end_pfill

#endif
