/*
 * fVDI default drawing function code
 *
 * $Id: default.c,v 1.1 2004-10-17 17:51:18 johan Exp $
 *
 * Copyright 2003, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 *
 * The Bresenham line drawing variation with perfect clipping is
 * based on code from the Graphics Gems series of books.
 */

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


retry_line(Virtual *vwk, long x1, long y1, long x2, long y2,
           long pattern, long colour, long mode, long draw_last)
{
	short *table, length, *index, moves;
	short n;
	short init_x, init_y;
	short movepnt;

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
	} else
		return;

	x1 = *table++;
	y1 = *table++;

	init_x = x1;
	init_y = y1;

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

		c_draw_line(vwk, x1, y1, x2, y2, pattern, colour, mode, draw_last);

		x1 = x2;
		y1 = y2;
	}
}


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
*	d2	y1   - " -     or table length (high) and type (0 - y/x1/x2 spans)
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

