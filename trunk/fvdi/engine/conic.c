/*
 * fVDI circle/ellipse/pie/arc code
 *
 * $Id: conic.c,v 1.2 2002-07-10 22:12:25 johan Exp $
 *
 * This is extracted and modified from code with an
 * original copyright as follows.
 * Johan Klockars, 1999
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


/*
 * External functions called
 */

extern void c_pline(Virtual *vwk, long num_pts, long colour, short *points);
extern void filled_poly(Virtual *vwk, short *p, long n, long colour, short *pattern, short *points, long mode, long interior_style);
extern void fill_poly(Virtual *vwk, short *p, long n, long colour, short *pattern, short *points, long mode, long interior_style);

extern short Isin(short angle);
extern short Icos(short angle);


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


#if 0
int clc_nsteps(int xrad, int yrad)
{
	int n_steps;

	if (xrad > yrad)
		n_steps = xrad;
	else
		n_steps = yrad;

	n_steps = n_steps >> 2;

	if (n_steps < 16)
		n_steps = 16;
	else if (n_steps > MAX_ARC_CT)
		n_steps = MAX_ARC_CT;
	
	return n_steps;
}


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
void gdp_rbox()
{
	short i, j, rdeltax, rdeltay;

	arb_corner(PTSIN, LLUR);
	X1 = PTSIN[0];
	Y1 = PTSIN[1];
	X2 = PTSIN[2];
	Y2 = PTSIN[3];
	rdeltax = (X2 - X1) / 2;
	rdeltay = (Y1 - Y2) / 2;
	xrad = xres >> 6;
	if (xrad > rdeltax)
	    xrad = rdeltax;
	yrad = SMUL_DIV( xrad, xsize, ysize );
	if (yrad > rdeltay)
	    yrad = rdeltay;
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
	xc = X2 - xrad;
	yc = Y1 - yrad;
	j = 10;
	for(i = 9; i >= 0; i--) { 
	    PTSIN[j + 1] = yc + PTSIN[i--];
	    PTSIN[j] = xc + PTSIN[i];
	    j += 2;
	}
	xc = X1 + xrad; 
	j = 20;
	for(i = 0; i < 10; i++) { 
	    PTSIN[j++] = xc - PTSIN[i++];
	    PTSIN[j++] = yc + PTSIN[i];
	}
	yc = Y2 + yrad;
	j = 30;
	for(i = 9; i >= 0; i--) { 
	    PTSIN[j + 1] = yc - PTSIN[i--];
	    PTSIN[j] = xc - PTSIN[i];
	    j += 2;
	}
	xc = X2 - xrad;
	j = 0;
	for(i = 0; i < 10; i++) { 
	    PTSIN[j++] = xc + PTSIN[i++];
	    PTSIN[j++] = yc - PTSIN[i];
	}
	PTSIN[40] = PTSIN[0];
	PTSIN[41] = PTSIN[1]; 
	iptscnt = 21;
    	if (gdp_code == 8) {
		LN_MASK = LINE_STYL[line_index];
		FG_BP_1 = line_color;

		if (line_width == 1) {
			pline();
		} else
			wline();
	} else
		plygn();

	return;
}
#endif
	

#if 0
v_gdp()
{
	short i, ltmp_end, rtmp_end;
	    case 7: /* GDP Rounded Box */
	      ltmp_end = line_beg;
	      line_beg = SQUARED;
	      rtmp_end = line_end;
	      line_end = SQUARED;
	      gdp_rbox();
	      line_beg = ltmp_end;
	      line_end = rtmp_end; 	
	      break;

}    	
#endif


#if 0
/* This is the fill pattern setup */
		case 2:
			if ( fill_index < 8 )
			{
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
