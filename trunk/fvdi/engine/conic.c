/*
 * fVDI circle/ellipse/pie/arc code
 *
 * $Id: conic.c,v 1.4 2004-10-17 17:52:55 johan Exp $
 *
 * Copyright 1999/2001-2003, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 *
 * This is extracted and modified from code with an
 * original copyright as follows.
 */

/*************************************************************************
**       Copyright 1999, Caldera Thin Clients, Inc.                     ** 
**       This software is licenced under the GNU Public License.        **
**       Please see LICENSE.TXT for further information.                ** 
**                                                                      ** 
**                  Historical Copyright                                ** 
**                                                                      **
**  Copyright (c) 1987, Digital Research, Inc. All Rights Reserved.     **
**  The Software Code contained in this listing is proprietary to       **
**  Digital Research, Inc., Monterey, California and is covered by U.S. **
**  and other copyright protection.  Unauthorized copying, adaptation,  **
**  distribution, use or display is prohibited and may be subject to    **
**  civil and criminal penalties.  Disclosure to others is prohibited.  **
**  For the terms and conditions of software code use refer to the      **
**  appropriate Digital Research License Agreement.                     **
**                                                                      **
**************************************************************************/


#include "fvdi.h"
#include "utility.h"
#include "function.h"
#include "globals.h"

#define MAX_ARC_CT 256

#if 0
#define SMUL_DIV(x,y,z)	((short)(((short)(x)*(long)((short)(y)))/(short)(z)))
#else
 #ifdef __PUREC__
  #define SMUL_DIV(x,y,z)	((short)(((x)*(long)(y))/(z)))
 #else
int SMUL_DIV(int, int, int);   //   d0d1d0d2
#pragma inline d0 = SMUL_DIV(d0, d1, d2) { "c1c181c2"; }
 #endif
#endif


void clc_arc(Virtual *vwk, long gdp_code, long xc, long yc, long xrad, long yrad,
             long beg_ang, long end_ang, long del_ang, long n_steps, long colour,
             short *pattern, short *points, long mode, long interior_style)
{
	short i, j, start, angle;
	
	if (vwk->clip.on) {
		if (((xc + xrad) < vwk->clip.rectangle.x1)
		    || ((xc - xrad) > vwk->clip.rectangle.x2)
		    || ((yc + yrad ) < vwk->clip.rectangle.y1)
		    || ((yc - yrad) > vwk->clip.rectangle.y2))
			return;
	}
	start = angle = beg_ang;	
	*points++ = SMUL_DIV(Icos(angle), xrad, 32767) + xc;
	*points++ = yc - SMUL_DIV(Isin(angle), yrad, 32767);

	for(i = 1, j = 2; i < n_steps; i++, j += 2) {
		angle = SMUL_DIV(del_ang, i, n_steps) + start;
		*points++ = SMUL_DIV(Icos(angle), xrad, 32767) + xc;
		*points++ = yc - SMUL_DIV(Isin(angle), yrad, 32767);
	}
	angle = end_ang;
	*points++ = SMUL_DIV(Icos(angle), xrad, 32767) + xc;
	*points++ = yc - SMUL_DIV(Isin(angle), yrad, 32767);

	/* If pie wedge, draw to center and then close.
	 * If arc or circle, do nothing because loop should close circle.
	 */

	if ((gdp_code == 3) || (gdp_code == 7)) {	/* Pie wedge */
		n_steps++;
		*points++ = xc;
		*points++ = yc;
	}

	if ((gdp_code == 2) || (gdp_code == 6))	/* Open arc */
		c_pline(vwk, n_steps + 1, colour, points - (n_steps + 1) * 2);
	else {
#if 0
		filled_poly(vwk, points - (n_steps + 1) * 2, n_steps + 1, colour, pattern, points, mode, interior_style);
#else
		fill_poly(vwk, points - (n_steps + 1) * 2, n_steps + 1, colour, pattern, points, mode, interior_style);
#endif
		if (vwk->fill.perimeter)
			c_pline(vwk, n_steps + 1, colour, points - (n_steps + 1) * 2);
	}
}


int clc_nsteps(int xrad, int yrad)
{
	int n_steps;

	if (xrad > yrad)
		n_steps = xrad;
	else
		n_steps = yrad;

#if 0
	n_steps = n_steps >> 2;

	if (n_steps < 16)
		n_steps = 16;
	else if (n_steps > MAX_ARC_CT)
		n_steps = MAX_ARC_CT;
#else
	n_steps = (n_steps * arc_split) >> 16;

	if (n_steps < arc_min)
		n_steps = arc_min;
	else if (n_steps > arc_max)
		n_steps = arc_max;
#endif
	
	return n_steps;
}


#if 0
void circle(Virtual *vwk, int xc, int yc, int xrad, short *points)
{
	int yrad, n_steps;
	
	yrad = SMUL_DIV(xrad, xsize, ysize); 
	n_steps = clc_nsteps(xrad, yrad);
	clc_arc(vwk, 4, xc, yc, xrad, yrad, 0, 3600, 3600, n_steps, points);
}


void ellipse(Virtual *vwk, int xc, int yc, int xrad, int yrad, short *points)
{
	int n_steps;
#if 0
	if (xfm_mode < 2) /* if xform != raster then flip */
		yrad = yres - yrad;	
#endif
	n_steps = clc_nsteps(xrad, yrad);
	clc_arc(vwk, 5, xc, yc, xrad, yrad, 0, 0, 3600, n_steps, points);
}
		

void arc(Virtual *vwk, int xc, int yc, int xrad, int beg_ang, int end_ang, short *points)
{
	int del_ang, yrad, n_steps;
	
	del_ang = end_ang - beg_ang;
	if (del_ang < 0)
	    del_ang += 3600; 

	yrad = SMUL_DIV(xrad, xsize, ysize);
	n_steps = clc_nsteps(xrad, yrad);
	n_steps = SMUL_DIV(del_ang, n_steps, 3600);
	if (n_steps == 0)
		return;
	
	clc_arc(vwk, 2, xc, yc, xrad, yrad, beg_ang, end_ang, del_ang, n_steps, points);
}


void ellipsearc(Virtual *vwk, int xc, int yc, int xrad, int yrad, int beg_ang, int end_ang, short *points)
{
	int del_ang, n_steps;
	
	del_ang = end_ang - beg_ang;
	if (del_ang < 0)
		del_ang += 3600;

#if 0
	if (xfm_mode < 2)	/* If xform != raster then flip */
		yrad = yres - yrad;
#endif

	n_steps = clc_nsteps(xrad, yrad);	
	n_steps = SMUL_DIV(del_ang, n_steps, 3600);
	if (n_steps == 0)
		return;

	clc_arc(vwk, 7, xc, yc, xrad, yrad, beg_ang, end_ang, del_ang, n_steps, points);
}
#endif


#if 0
void arb_corner(WORD * corners, WORD type)
{
    /* Local declarations. */
    REG WORD temp, typ;
    REG WORD *xy1, *xy2;

    /* Fix the x coordinate values, if necessary. */

    xy1 = corners;
    xy2 = corners + 2;
    if (*xy1 > *xy2) {
        temp = *xy1;
        *xy1 = *xy2;
        *xy2 = temp;
    }



    /* End if:  "x" values need to be swapped. */
    /* Fix y values based on whether traditional (ll, ur) or raster-op */
    /* (ul, lr) format is desired.                                     */
    xy1++;                      /* they now point to corners[1] and
                                   corners[3] */
    xy2++;

    typ = type;

    if (((typ == LLUR) && (*xy1 < *xy2)) ||
        ((typ == ULLR) && (*xy1 > *xy2))) {
        temp = *xy1;
        *xy1 = *xy2;
        *xy2 = temp;
    }                           /* End if:  "y" values need to be swapped. */
}                               /* End "arb_corner". */
#endif


#if 0
    case 7: /* GDP Rounded Box */
      ltmp_end = line_beg;
      line_beg = SQUARED;
      rtmp_end = line_end;
      line_end = SQUARED;
#endif
void rounded_box(Virtual *vwk, long gdp_code, long x1, long y1, long x2, long y2, long colour,
                 short *pattern, short *points, long mode, long interior_style)
{
	short i, j;
	short rdeltax, rdeltay;
	short xc, yc, xrad, yrad;
	Workstation *wk = vwk->real_address;
	int n_steps;
#if 0
	arb_corner(PTSIN, LLUR);
#endif
	rdeltax = (x2 - x1) / 2;
	rdeltay = (y2 - y1) / 2;

	xrad = wk->screen.mfdb.width >> 6;
	if (xrad > rdeltax)
	    xrad = rdeltax;

	yrad = SMUL_DIV(xrad, wk->screen.pixel.width, wk->screen.pixel.height);
	if (yrad > rdeltay) {
	    yrad = rdeltay;
		xrad = SMUL_DIV(yrad, wk->screen.pixel.height, wk->screen.pixel.width);
	}
	yrad = -yrad;

	n_steps = clc_nsteps(xrad, yrad);

#if 0
	PTSIN[0] = 0;
	PTSIN[1] = yrad;
	PTSIN[2] = SMUL_DIV(Icos(675), xrad, 32767);
	PTSIN[3] = SMUL_DIV(Isin(675), yrad, 32767);
	PTSIN[4] = SMUL_DIV(Icos(450), xrad, 32767);
	PTSIN[5] = SMUL_DIV(Isin(450), yrad, 32767);
	PTSIN[6] = SMUL_DIV(Icos(225), xrad, 32767);
	PTSIN[7] = SMUL_DIV(Isin(225), yrad, 32767);
	PTSIN[8] = xrad;
	PTSIN[9] = 0;
#else
	for(i = 0; i < 5; i++) {
		points[i * 2]     = SMUL_DIV(Icos(900 - 225 * i), xrad, 32767);
		points[i * 2 + 1] = SMUL_DIV(Isin(900 - 225 * i), yrad, 32767);
	}
#endif

	xc = x2 - xrad;
	yc = y1 - yrad;
	j = 10;
	for(i = 9; i >= 0; i--) { 
	    points[j + 1] = yc + points[i--];
	    points[j] = xc + points[i];
	    j += 2;
	}
	xc = x1 + xrad; 
	j = 20;
	for(i = 0; i < 10; i++) { 
	    points[j++] = xc - points[i++];
	    points[j++] = yc + points[i];
	}
	yc = y2 + yrad;
	j = 30;
	for(i = 9; i >= 0; i--) { 
	    points[j + 1] = yc - points[i--];
	    points[j] = xc - points[i];
	    j += 2;
	}
	xc = x2 - xrad;
	j = 0;
	for(i = 0; i < 10; i++) { 
		points[j++] = xc + points[i++];
		points[j++] = yc - points[i];
	}
	points[40] = points[0];
	points[41] = points[1]; 

	if (gdp_code == 8) {
#if 0
		c_pline(vwk, n_steps + 1, colour, points - (n_steps + 1) * 2);
#else
		c_pline(vwk, 21, colour, points);
#endif
	} else {
#if 0
		filled_poly(vwk, points, 21, colour, pattern, points, mode, interior_style);
#else
		fill_poly(vwk, points, 21, colour, pattern, points, mode, interior_style);
#endif
		if (vwk->fill.perimeter)
#if 0
			c_pline(vwk, 21, colour, points - (n_steps + 1) * 2);
#else
			c_pline(vwk, 21, colour, points);
#endif
	}
}
	

#if 0
/* This is the fill pattern setup */
		case 2:
			if (fill_index < 8) {
			    patmsk = DITHRMSK;
			    patptr = &DITHER[fill_index * (patmsk + 1)];
			} else {
			    patmsk = OEMMSKPAT;
			    patptr = &OEMPAT[(fill_index - 8) * (patmsk + 1)]; 
			}
			break;
		case 3:
			if (fill_index < 6) {
			    patmsk = HAT_0_MSK;
			    patptr = &HATCH0[fill_index * (patmsk + 1)];
			} else {
			    patmsk = HAT_1_MSK;
			    patptr = &HATCH1[(fill_index - 6) * (patmsk + 1)];
			}
			break;
		case 4:
			patmsk = 0x000f;
			patptr = &UD_PATRN;
			break;
#endif


#if 0
ellipse

  ifne 0
;	move.w	#0,d0			; Background colour
;	swap	d0
	move.l	vwk_line_colour(a0),d0

	uses_d1

	movem.l	d2-d7/a3-a6,-(a7)
	move.l	ptsin(a1),a2

;	move.w	vwk_line_user_mask(a0),d5
;	move.w	vwk_line_type(a0),d1
;	cmp.w	#7,d1
;	beq	.userdef
;	lea	_line_types,a1
;	subq.w	#1,d1
;	add.w	d1,d1
;	move.w	0(a1,d1.w),d5
;.userdef:

	move.l	vwk_real_address(a0),a1
	move.l	wk_r_get_colour(a1),a1	; Index to real colour
	jsr	(a1)

	movem.w	4(a2),d2-d3
	tst.w	d2
	beq	.ellipse_end

	move.w	d2,d1
	mulu	d2,d1
	move.l	d1,d5		; d5 = a * a	(a2)
	move.w	d3,d1
	mulu	d3,d1
	move.l	d1,a6		; a6 = b * b	(b2)
;	sub.l	a3,a3		; a3 = 0	(b2x)
	move.l	d5,d6
	mulu	d3,d6
	neg.l	d6		; Was nothing (a5 now 0 - a2 * b   (b2x - a2 * b))
	move.l	d6,a5		; a5 = a2 * b	(a2y)
	move.l	d5,d6
	add.l	a5,d6		; Was	sub.l	a5,d6		; d6 = a2 - a2y	(dec1)
	move.l	a6,d7
	add.l	d7,d7		; d7 = b2 * 2	(dec2)
	move.l	d7,d3
	add.l	d6,d3
	add.l	a5,d3		; Was	sub.l	a5,d3		; d3 = dec2 + dec1 - a2y	(dec)
	add.l	d6,d6
	add.l	d6,d6		; d6 = dec1 * 4	(dec1)
	move.l	d7,d1
	add.l	d7,d7
	add.l	d1,d7		; d7 = dec2 * 3	(dec2)

	clr.w	d4
	swap	d4
	move.w	6(a2),d4	; d4 = x, y

	cmp.w	#1,vwk_mode(a0)
	bne	.nonsolid
	move.l	d0,a1		; Remember colours
	move.l	#0,-(a7)	; Get a memory block of any size (hopefully large)
	bsr	_allocate_block
	addq.l	#4,a7
	tst.l	d0
	bne	.table_ellipse
	move.l	a1,d0		; Restore colours
	
.nonsolid:			; Couldn't use the table routine
	clr.l	-(a7)
	move.l	a0,-(a7)

	move.w	vwk_mode(a0),-(a7)
	bsr	setup_plot
	addq.l	#2,a7

;	move.l	a7,a0			; a0 no longer -> VDI struct!

	bra	.xstep_end

.xstep_loop:
	movem.w	(a2),d1-d2

	add.w	d4,d2
	swap	d4
	add.w	d4,d1
	move.l	(a7),a0
	bsr	clip_point
	lbgt	.skip1,1
	move.l	a7,a0
	move	#1,ccr
	jsr	(a1)		; xc + x, yc + y
 label .skip1,1
	sub.w	d4,d1
	sub.w	d4,d1
	move.l	(a7),a0
	bsr	clip_point
	lbgt	.skip2,2
	move.l	a7,a0
	move	#1,ccr
	jsr	(a1)		; xc - x, yc + y
 label .skip2,2
	swap	d4
	sub.w	d4,d2
	sub.w	d4,d2
	move.l	(a7),a0
	bsr	clip_point
	lbgt	.skip3,3
	move.l	a7,a0
	move	#1,ccr
	jsr	(a1)		; xc - x, yc - y
 label .skip3,3
	swap	d4
	add.w	d4,d1
	add.w	d4,d1
	move.l	(a7),a0
	bsr	clip_point
	lbgt	.skip4,4
	move.l	a7,a0
	move	#1,ccr
	jsr	(a1)		; xc + x, yc - y
 label .skip4,4
	swap	d4

	tst.l	d3		; dec >= 0?
	bmi	.no_ystep
	add.l	d6,d3		; dec += dec1
	add.l	d5,d6
	add.l	d5,d6
	add.l	d5,d6
	add.l	d5,d6		; dec1 += 4 * a2
	subq.w	#1,d4		; y--
	add.l	d5,a5		; Was	sub.l	d5,a5		; a2y -= a2
.no_ystep:
	add.l	d7,d3		; dec += dec2
	add.l	a6,d7
	add.l	a6,d7
	add.l	a6,d7
	add.l	a6,d7		; dec2 += 4 * b2
	add.l	a6,a5		; Was	add.l	a6,a3

	add.l	#$10000,d4
.xstep_end
	cmp.l	#0,a5		; Was	cmp.l	a5,a3
	ble	.xstep_loop

	movem.w	4(a2),d2-d3
;	move.w	d2,d1
;	mulu	d2,d1
;	move.l	d1,d5		; d5 = a * a	(a2)
;	move.w	d3,d1
;	mulu	d3,d1
;	move.l	d1,a6		; a6 = b * b	(b2)
;	sub.l	a5,a5		; a5 = 0	(a2y)
	move.l	a6,d6
	mulu	d2,d6
	move.l	d6,a5		; Was	move.l	d6,a3		; a3 = b2 * a	(b2x)
	move.l	a6,d6
	sub.l	a5,d6		; Was	sub.l	a3,d6		; d6 = b2 - b2x	(dec1)
	move.l	d5,d7
	add.l	d7,d7		; d7 = a2 * 2	(dec2)
	move.l	d7,d3
	add.l	d6,d3
	sub.l	a5,d3		; Was	sub.l	a3,d3		; d3 = dec2 + dec1 - b2x	(dec)
	add.l	d6,d6
	add.l	d6,d6		; d6 = dec1 * 4	(dec1)
	move.l	d7,d1
	add.l	d7,d7
	add.l	d1,d7		; d7 = dec2 * 3	(dec2)

	move.w	4(a2),d4
	swap	d4		; d4 = x, y
	clr.w	d4

	bra	.ystep_end

.ystep_loop:
	movem.w	(a2),d1-d2

	add.w	d4,d2
	swap	d4
	add.w	d4,d1
	move.l	(a7),a0
	bsr	clip_point
	lbgt	.skip1,1
	move.l	a7,a0
	move	#1,ccr
	jsr	(a1)		; xc + x, yc + y
 label .skip1,1
	sub.w	d4,d1
	sub.w	d4,d1
	move.l	(a7),a0
	bsr	clip_point
	lbgt	.skip2,2
	move.l	a7,a0
	move	#1,ccr
	jsr	(a1)		; xc - x, yc + y
 label .skip2,2
	swap	d4
	sub.w	d4,d2
	sub.w	d4,d2
	move.l	(a7),a0
	bsr	clip_point
	lbgt	.skip3,3
	move.l	a7,a0
	move	#1,ccr
	jsr	(a1)		; xc - x, yc - y
 label .skip3,3
	swap	d4
	add.w	d4,d1
	add.w	d4,d1
	move.l	(a7),a0
	bsr	clip_point
	lbgt	.skip4,4
	move.l	a7,a0
	move	#1,ccr
	jsr	(a1)		; xc + x, yc - y
 label .skip4,4
	swap	d4

	tst.l	d3		; dec >= 0?
	bmi	.no_xstep
	add.l	d6,d3		; dec += dec1
	add.l	a6,d6
	add.l	a6,d6
	add.l	a6,d6
	add.l	a6,d6		; dec1 += 4 * b2
	sub.l	#$10000,d4	; x--
	sub.l	a6,a5		; b2x -= b2
.no_xstep:
	add.l	d7,d3		; dec += dec2
	add.l	d5,d7
	add.l	d5,d7
	add.l	d5,d7
	add.l	d5,d7		; dec2 += 4 * b2
	sub.l	d5,a5		; Was	add.l	d5,a5

	addq.w	#1,d4
.ystep_end
	cmp.l	#0,a5		; Was	cmp.l	a3,a5
	bgt	.ystep_loop	; Was	bls	.ystep_loop

	add.w	#8,a7
.ellipse_end:
	movem.l	(a7)+,d2-d7/a3-a6
	used_d1
	done_return

* When a table can be used
*
.table_ellipse:
	exg	a1,d0
	move.l	d0,-(a7)
	move.l	a1,-(a7)

	move.l	d5,d0
	add.l	d0,d0
	add.l	d0,d0			; 4 * a2
	move.l	a6,a3
	add.l	a3,a3
	add.l	a3,a3			; 4 * b2
	bra	.txstep_end

.txstep_loop:
	movem.w	(a2),d1-d2

	add.w	d4,d2
	swap	d4
	add.w	d4,d1
	bsr	clip_point
	lbgt	.skip1,1
	move.w	d1,(a1)+		; xc + x, yc + y
	move.w	d2,(a1)+
 label .skip1,1
	sub.w	d4,d1
	sub.w	d4,d1
	bsr	clip_point
	lbgt	.skip2,2
	move.w	d1,(a1)+		; xc - x, yc + y
	move.w	d2,(a1)+
 label .skip2,2
	swap	d4
	sub.w	d4,d2
	sub.w	d4,d2
	bsr	clip_point
	lbgt	.skip3,3
	move.w	d1,(a1)+		; xc - x, yc - y
	move.w	d2,(a1)+
 label .skip3,3
	swap	d4
	add.w	d4,d1
	add.w	d4,d1
	bsr	clip_point
	lbgt	.skip4,4
	move.w	d1,(a1)+		; xc + x, yc - y
	move.w	d2,(a1)+
 label .skip4,4
	swap	d4

	tst.l	d3		; dec >= 0?
	bmi	.tno_ystep
	add.l	d6,d3		; dec += dec1
	add.l	d0,d6		; dec1 += 4 * a2
	subq.w	#1,d4		; y--
	add.l	d5,a5		; Was	sub.l	d5,a5		; a2y -= a2
.tno_ystep:
	add.l	d7,d3		; dec += dec2
	add.l	a3,d7		; dec2 += 4 * b2
	add.l	a6,a5		; Was	add.l	a6,a3

	add.l	#$10000,d4
.txstep_end
	cmp.l	#0,a5		; Was	cmp.l	a5,a3
	ble	.txstep_loop

	movem.w	4(a2),d2-d3
	move.l	a6,d6
	mulu	d2,d6
	move.l	d6,a5		; Was	move.l	d6,a3		; a3 = b2 * a	(b2x)
	move.l	a6,d6
	sub.l	a5,d6		; Was	sub.l	a3,d6		; d6 = b2 - b2x	(dec1)
	move.l	d5,d7
	add.l	d7,d7		; d7 = a2 * 2	(dec2)
	move.l	d7,d3
	add.l	d6,d3
	sub.l	a5,d3		; Was	sub.l	a3,d3		; d3 = dec2 + dec1 - b2x	(dec)
	add.l	d6,d6
	add.l	d6,d6		; d6 = dec1 * 4	(dec1)
	move.l	d7,d1
	add.l	d7,d7
	add.l	d1,d7		; d7 = dec2 * 3	(dec2)

	move.w	4(a2),d4
	swap	d4		; d4 = x, y
	clr.w	d4

	bra	.tystep_end

.tystep_loop:
	movem.w	(a2),d1-d2

	add.w	d4,d2
	swap	d4
	add.w	d4,d1
	bsr	clip_point
	lbgt	.skip1,1
	move.w	d1,(a1)+	; xc + x, yc + y
	move.w	d2,(a1)+
 label .skip1,1
	sub.w	d4,d1
	sub.w	d4,d1
	bsr	clip_point
	lbgt	.skip2,2
	move.w	d1,(a1)+	; xc - x, yc + y
	move.w	d2,(a1)+
 label .skip2,2
	swap	d4
	sub.w	d4,d2
	sub.w	d4,d2
	bsr	clip_point
	lbgt	.skip3,3
	move.w	d1,(a1)+	; xc - x, yc - y
	move.w	d2,(a1)+
 label .skip3,3
	swap	d4
	add.w	d4,d1
	add.w	d4,d1
	bsr	clip_point
	lbgt	.skip4,4
	move.w	d1,(a1)+	; xc + x, yc - y
	move.w	d2,(a1)+
 label .skip4,4
	swap	d4

	tst.l	d3		; dec >= 0?
	bmi	.tno_xstep
	add.l	d6,d3		; dec += dec1
	add.l	a3,d6		; dec1 += 4 * b2
	sub.l	#$10000,d4	; x--
	sub.l	a6,a5		; b2x -= b2
.tno_xstep:
	add.l	d7,d3		; dec += dec2
	add.l	d0,d7		; dec2 += 4 * b2
	sub.l	d5,a5		; Was	add.l	d5,a5

	addq.w	#1,d4
.tystep_end
	cmp.l	#0,a5		; Was	cmp.l	a3,a5
	bgt	.tystep_loop	; Was	bls	.ystep_loop

	move.l	(a7)+,d1	; Table address
	move.l	(a7)+,d0	; Colours
	move.l	a1,d2
	sub.l	d1,d2
	lsr.l	#2,d2		; Number of points
	swap	d2
	clr.w	d2		; Coordinate mode

	move.l	d1,-(a7)	; For free_block below
	
	move.l	vwk_real_address(a0),a1
	move.l	wk_r_set_pixel(a1),a1
	clr.l	-(a7)
	move.l	a0,-(a7)
	move.l	a7,a0
	addq.l	#1,a0		; Table operation
	jsr	(a1)
	addq.l	#8,a7

	bsr	_free_block
	addq.l	#4,a7

	bra	.ellipse_end
  endc

#endif
