/*
 * fVDI wide line code
 *
 * $Id: line.c,v 1.2 2002-07-10 22:12:25 johan Exp $
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

#define MAX_L_WIDTH	32
#define X_ASPECT 1
#define Y_ASPECT 1

#define ABS(x) ((x) >= 0 ? (x) : -(x))

#define ARROWED 1

#if 1
#define SMUL_DIV(x,y,z)	((short)(((short)(x)*(long)((short)(y)))/(short)(z)))
#else
int SMUL_DIV(int, int, int);   //   d0d1d0d2
#pragma inline d0 = SMUL_DIV(d0, d1, d2) { "c1c181c2"; }
#endif


extern short solid;

void do_arrow(Virtual *vwk, short *pts, int numpts, int colour, short *points, long mode);

extern void fill_poly(Virtual *vwk, short *p, long n, long colour, short *pattern,
                      short *points, long mode, long interior_style);
extern short isqrt(unsigned long x);

int wide_setup(Virtual *vwk, int width, short *q_circle)
{
	int i, j, x, y, d, low, high;
	int xsize, ysize;
	int num_qc_lines;

	/* Limit the requested line width to a reasonable value. */

	if (width < 1)
		width = 1;
	else if (width > MAX_L_WIDTH)
		width = MAX_L_WIDTH;

	/* Make the line width an odd number (one less, if even). */

	width = (width - 1) | 1;

	/* Set the line width internals and the return parameters.  
	 * Return if the line width is being set to one.
	 */
#if 0
	if ((line_qw = width) == 1)
#else
	if (width == 1)
#endif
		return 0;

	/* Initialize the circle DDA.  "y" is set to the radius. */

	x = 0;
	y = (width + 1) / 2;
	d = 3 - 2 * y;

#if Y_ASPECT >= X_ASPECT
	for(i = 0; i < MAX_L_WIDTH; i++) {
		q_circle[i] = 0 ;
	}
#else
	for(i = 0; i < ((MAX_L_WIDTH * X_ASPECT / Y_ASPECT)) / 2 + 1); i++) {
		 q_circle[i] = 0 ;
	}
#endif

	/* Do an octant, starting at north.  
	 * The values for the next octant (clockwise) will
	 * be filled by transposing x and y.
	 */
	while (x < y) {
		q_circle[y] = x;
		q_circle[x] = y;

		if (d < 0) {
			d = d + (4 * x) + 6;
		} else {
			d = d + (4 * (x - y)) + 10;
			y--;
		}
		x++;
	}

	if (x == y)
		q_circle[x] = x;

	 /* Calculate the number of vertical pixels required. */

	xsize = vwk->real_address->screen.pixel.width;
	ysize = vwk->real_address->screen.pixel.height;
	num_qc_lines = (width * xsize / ysize) / 2 + 1;

	/* Fake a pixel averaging when converting to 
	 * non-1:1 aspect ratio.
	 */

#if Y_ASPECT >= X_ASPECT
	low = 0;
	for(i = 0; i < num_qc_lines; i++) {
		high = ((2 * i + 1) * ysize / xsize) / 2;
		d = 0;

		for (j = low; j <= high; j++) {
			d += q_circle[j];
		}

		q_circle[i] = d / (high - low + 1);
		low = high + 1;
	} 
#else
	for(i = num_qc_lines - 1; i >= 0; i--) {
		q_circle[i] = q_circle[(2 * i * ysize / xsize + 1) / 2];
	}
#endif

	return num_qc_lines;
}


void quad_xform(int quad, int x, int y, int *tx, int *ty)
{
	if (quad & 2)
		*tx = -x;		/* 2, 3 */
	else
		*tx = x;		/* 1, 4 */

	if (quad > 2)
		*ty = -y;		/* 3, 4 */
	else
		*ty = y;		/* 1, 2 */
}


void perp_off(int *vx, int *vy, short *q_circle, int num_qc_lines)
{
	int x, y, u, v, quad, magnitude, min_val, x_val, y_val;

	/* Mirror transform the vector so that it is in the first quadrant. */
	if (*vx >= 0)
		quad = (*vy >= 0) ? 1 : 4;
	else
		quad = (*vy >= 0) ? 2 : 3;

	quad_xform(quad, *vx, *vy, &x, &y);

	/* Traverse the circle in a dda-like manner and find the coordinate pair
	 * (u, v) such that the magnitude of (u*y - v*x) is minimized.  In case of
	 * a tie, choose the value which causes (u - v) to be minimized.  If not
	 * possible, do something.
	 */
	min_val = 32767;
	x_val = u = q_circle[0];		/* x_val/y_val new here */
	y_val = v = 0;
	while (1) {
		/* Check for new minimum, same minimum, or finished. */
		if (((magnitude = ABS(u * y - v * x)) < min_val ) ||
		    ((magnitude == min_val) && (ABS(x_val - y_val) > ABS(u - v)))) {
			min_val = magnitude;
			x_val = u;
			y_val = v;
		} else
			break;

		/* Step to the next pixel. */
		if (v == num_qc_lines - 1) {
			if (u == 1)
				break;
			else
				u--;
		} else {
			if (q_circle[v + 1] >= u - 1) {
				v++;
				u = q_circle[v];
			} else {
				u--;
			}
		}
	}

	/* Transform the solution according to the quadrant. */
	quad_xform(quad, x_val, y_val, vx, vy);
}


void wide_line(Virtual *vwk, short *pts, long numpts, long colour, short *points, long mode)
{
	int i, j, k;
	int wx1, wy1, wx2, wy2, vx, vy;
#if 0
#if Y_ASPECT >= X_ASPECT
	short q_circle[MAX_L_WIDTH];
#else
	short q_circle[(MAX_L_WIDTH * X_ASPECT / Y_ASPECT) / 2 + 1];
#endif
#else
	short *q_circle;
#endif
	int num_qc_lines;
	int xsize, ysize;

	/* Don't attempt wide lining on a degenerate polyline. */
	if (numpts < 2)
		return;

	q_circle = points;
#if Y_ASPECT >= X_ASPECT
	points += MAX_L_WIDTH;
#else
	points += (MAX_L_WIDTH * X_ASPECT / Y_ASPECT) / 2 + 1;
#endif

	num_qc_lines = wide_setup(vwk, vwk->line.width, q_circle);

	/* If the ends are arrowed, output them. */
	if ((vwk->line.ends.beginning | vwk->line.ends.end) & ARROWED)
		do_arrow(vwk, pts, numpts, colour, points, mode);

	/* Initialize the starting point for the loop. */
	j = 0;
	wx1 = pts[j++];
	wy1 = pts[j++];

#if 0
	/* If the end style for the first point is not squared, output a circle. */
	if (vwk->line.ends.beginning != SQUARED)
		do_circ(wx1, wy1);
#endif

	/* Loop over the number of points passed in. */
	for(i = 1; i < numpts; i++) {
		/* Get the ending point for the line segment and the vector from the
		 * start to the end of the segment.
		 */
		wx2 = pts[j++];
		wy2 = pts[j++];   

		vx = wx2 - wx1;
		vy = wy2 - wy1;

		/* Ignore lines of zero length. */
		if ((vx == 0) && (vy == 0))
			continue;

		/* Calculate offsets to fatten the line.  If the line segment is
		 * horizontal or vertical, do it the simple way.
		 */
		if (vx == 0) {
			vx = q_circle[0];
			vy = 0;
		} else if (vy == 0) {
			vx = 0;
			vy = num_qc_lines - 1;
		} else {
			/* Find the offsets in x and y for a point perpendicular to the line
			 * segment at the appropriate distance.
			 */

			xsize = vwk->real_address->screen.pixel.width;
			ysize = vwk->real_address->screen.pixel.height;

			k = SMUL_DIV(-vy, ysize, xsize);
			vy = SMUL_DIV(vx, xsize, ysize);
			vx = k;
			perp_off(&vx, &vy, q_circle, num_qc_lines);
		}

		/* Prepare the points parameters for the polygon call. */
		points[0] = wx1 + vx;
		points[1] = wy1 + vy;
		points[2] = wx1 - vx;
		points[3] = wy1 - vy;
		points[4] = wx2 - vx;
		points[5] = wy2 - vy;
		points[6] = wx2 + vx;
		points[7] = wy2 + vy;
		fill_poly(vwk, points, 4, colour, &solid, &points[8], mode, 0x00010000L);

#if 0
		/* If the terminal point of the line segment is an internal joint,
		 * or the end style is not squared, output a filled circle.
		 */
		if ((vwk->line.ends.end != SQUARED) || (i < numpts - 1))
			do_circ(wx2, wy2);
#endif

		/* The line segment end point becomes the starting point for the next
		 * line segment.
		 */
		wx1 = wx2;
		wy1 = wy2;
	}
}


#if 0
v_pline()
{
	SHORT pln_sts;

	if (HIDE_CNT == 0) {
		pln_sts = 1;
		HIDE_CUR();
	} else
		pln_sts = 0;	
	LN_MASK = LINE_STYL[line_index];
	FG_BP_1 = line_color;

	if (line_width == 1) {
		pline();
		if ((line_beg | line_end ) & ARROWED)
			do_arrow(vwk, pts, numpts, colour, points, mode);
	} else
		wline();
	if (pln_sts == 1)
		DIS_CUR();
}


v_pmarker()
{
#define MARKSEGMAX 5

  extern short m_dot[], m_plus[], m_star[], m_square[], m_cross[], m_dmnd[];

  static short *markhead[] = {m_dot, m_plus, m_star, m_square, m_cross, m_dmnd};

  WORD i, j, k, num_lines, num_vert, x_center, y_center;
  WORD sav_index, sav_color, sav_width, sav_beg, sav_end, sav_clip;
  short *mark_ptr;

  /* Save the current polyline attributes which will be used. */
  sav_index = line_index;
  sav_color = line_color;
  sav_width = line_width;
  sav_beg = line_beg;
  sav_end = line_end;
  sav_clip = CLIP;

  /* Set the appropriate polyline attributes. */
  line_index = 0;
  line_color = mark_color;
  line_width = 1;
  line_beg = 0;
  line_end = 0;
  CLIP = 1;

  /* Copy the PTSIN values which will be overwritten by the polyline arrays. */
  num_vert = 2 * CONTRL[1];
  for(i = 0; i < 2 * MARKSEGMAX; i++)
    PTSOUT[i] = PTSIN[i];

  /* Loop over the number of points. */
  for(i = 0; i < num_vert; i++) {
    /* Get the (x, y) position for the marker. */
    if (i < 2 * MARKSEGMAX) {
      x_center = PTSOUT[i++];
      y_center = PTSOUT[i];
    }  /* End if:  grab from PTSOUT. */
    else {
      x_center = PTSIN[i++];
      y_center = PTSIN[i];
    }  /* End else:  grab from PTSIN. */

    /* Get the pointer to the appropriate marker type definition. */
    mark_ptr = markhead[mark_index];
    num_lines = *mark_ptr++;

    /* Loop over the number of polylines which define the marker. */
    for(j = 0; j < num_lines; j++) {
      /* How many points?  Get them. */
      CONTRL[1] = *mark_ptr++;
      for(k = 0; k < 2 * CONTRL[1]; ) {
        PTSIN[k++] = x_center + mark_scale * (*mark_ptr++);
        PTSIN[k++] = y_center + mark_scale * (*mark_ptr++);
      }  /* End for:  extract points. */

      /* Output the polyline. */
      v_pline();
    }  /* End for:  over the number of polylines defining the marker. */
  }  /* End for:  over marker points. */

  /* Restore the current polyline attributes. */
  line_index = sav_index;
  line_color = sav_color;
  line_width = sav_width;
  line_beg = sav_beg;
  line_end = sav_end;
  CLIP = sav_clip;
}


void pline(void)
{
	short i, j;

	j = 0;
	LSTLIN = FALSE;
	for(i = CONTRL[1] - 1; i > 0; i--) {
		if (i == 1)	      
			LSTLIN = TRUE;
		X1 = PTSIN[j++];
		Y1 = PTSIN[j++];	
        	X2 = PTSIN[j];
		Y2 = PTSIN[j+1];
		if (!CLIP || clip_line())
			ABLINE();
	}				
}


short clip_line(void)
{
	short deltax, deltay, x1y1_clip_flag, x2y2_clip_flag, line_clip_flag;
	short *x, *y;

	while ((x1y1_clip_flag = code(X1, Y1)) | (x2y2_clip_flag = code(X2, Y2))) {
		if ((x1y1_clip_flag & x2y2_clip_flag))
			return FALSE ;
		if (x1y1_clip_flag) {
			line_clip_flag = x1y1_clip_flag;
			x = &X1;
			y = &Y1;
		} else {
			line_clip_flag = x2y2_clip_flag;
			x = &X2;
			y = &Y2;
		}
		deltax = X2 - X1; deltay = Y2 - Y1;
		if (line_clip_flag & 1) {		/* Left? */
			*y = Y1 + SMUL_DIV(deltay, (XMN_CLIP - X1), deltax);
			*x = XMN_CLIP;
		} else if (line_clip_flag & 2) {	/* Right? */
			*y = Y1 + SMUL_DIV(deltay, (XMX_CLIP - X1), deltax);
			*x = XMX_CLIP;
		} else if (line_clip_flag & 4) {	/* Top? */
			*x = X1 + SMUL_DIV(deltax, (YMN_CLIP - Y1), deltay);
			*y = YMN_CLIP;
		} else if (line_clip_flag & 8) {	/* Bottom? */
			*x = X1 + SMUL_DIV(deltax, (YMX_CLIP-Y1), deltay);
			*y = YMX_CLIP;
		}
	}
	return TRUE;		/* Segment now cliped  */
}


short code(short x, short y)
{
	short clip_flag;

	clip_flag = 0;
	if (x < XMN_CLIP)
		clip_flag = 1;
	else if (x > XMX_CLIP)
		clip_flag = 2;
	if ( y < YMN_CLIP )
		clip_flag += 4;
	else if (y > YMX_CLIP)
		clip_flag += 8;
	return clip_flag;
}
#endif


void arrow(Virtual *vwk, short *xy, short inc, int numpts, int colour, short *points, long mode)
{
	short i, arrow_len, arrow_wid, line_len;
	short *xybeg;
	short dx, dy;
	short base_x, base_y, ht_x, ht_y;
	long arrow_len2, line_len2;
	int xsize, ysize;

	xsize = vwk->real_address->screen.pixel.width;
	ysize = vwk->real_address->screen.pixel.height;

	/* Set up the arrow-head length and width as a function of line width. */
	if (vwk->line.width == 1)
		arrow_len = 8;
	else
		arrow_len = 3 * vwk->line.width - 1;
	arrow_len2 = arrow_len * arrow_len;
	arrow_wid = arrow_len / 2;

	/* Initialize the beginning pointer. */
	xybeg = xy;

	/* Find the first point which is not so close to the end point that it
	 * will be obscured by the arrowhead.
	 */
	for(i = 1; i < numpts; i++) {
		/* Find the deltas between the next point and the end point.
		 * Transform to a space such that the aspect ratio is uniform
		 * and the x axis distance is preserved.
		 */
		xybeg += inc;
		dx = *xy - *xybeg;
		dy = SMUL_DIV(*(xy + 1) - *(xybeg + 1), ysize, xsize);

		/* Get the length of the vector connecting the point with the end point.
		 * If the vector is of sufficient length, the search is over.
		 */
#if 0
		if ((line_len = vec_len(ABS(dx), ABS(dy))) >= arrow_len)
#else
		line_len2 = (long)dx * dx + (long)dy * dy;
		if (line_len2 >= arrow_len2)
#endif
			break;
	}

	/* If the longest vector is insufficiently long, don't draw an arrow. */
#if 0
	if (line_len < arrow_len)
#else
	if (line_len2 < arrow_len2)
#endif
		return;

	line_len = isqrt(line_len2);
	
	/* Rotate the arrow-head height and base vectors.
	 * Perform calculations in 1000x space.
	 */
	ht_x = SMUL_DIV(arrow_len, SMUL_DIV(dx, 1000, line_len), 1000);
	ht_y = SMUL_DIV(arrow_len, SMUL_DIV(dy, 1000, line_len), 1000);
	base_x = SMUL_DIV(arrow_wid, SMUL_DIV(dy, -1000, line_len), 1000);
	base_y = SMUL_DIV(arrow_wid, SMUL_DIV(dx, 1000, line_len), 1000);

	/* Transform the y offsets back to the correct aspect ratio space. */
	ht_y = SMUL_DIV(ht_y, xsize, ysize);
	base_y = SMUL_DIV(base_y, xsize, ysize);

	/* Build a polygon to send to plygn.  Build into a local array
	 * first since xy will probably be pointing to the PTSIN array.
	 */
	points[0] = *xy + base_x - ht_x;
	points[1] = *(xy + 1) + base_y - ht_y;
	points[2] = *xy - base_x - ht_x;
	points[3] = *(xy + 1) - base_y - ht_y;
	points[4] = *xy;
	points[5] = *(xy + 1);
	fill_poly(vwk, points, 3, colour, &solid, &points[6], mode, 0x00010000L);

	/* Adjust the end point and all points skipped. */
	*xy -= ht_x;
	*(xy + 1) -= ht_y;
	while ((xybeg -= inc) != xy) {
		*xybeg = *xy;
		*(xybeg + 1) = *(xy + 1);
	}
}


void do_arrow(Virtual *vwk, short *pts, int numpts, int colour, short *points, long mode)
{
	short x_start, y_start, new_x_start, new_y_start;

	/* Function "arrow" will alter the end of the line segment.
	 * Save the starting point of the polyline in case two calls to "arrow"
	 * are necessary.
	 */
	new_x_start = x_start = pts[0];
	new_y_start = y_start = pts[1];

	if (vwk->line.ends.beginning & ARROWED) {
		arrow(vwk, &pts[0], 2, numpts, colour, points, mode);
		new_x_start = pts[0];
		new_y_start = pts[1];
	}

	if (vwk->line.ends.end & ARROWED) {
		pts[0] = x_start;
		pts[1] = y_start;
		arrow(vwk, &pts[2 * numpts - 2], -2, numpts, colour, points, mode);
		pts[0] = new_x_start;
		pts[1] = new_y_start;
	}
}


#if 0
void do_circ(short cx, short cy)
{
	short k;

	/* Only perform the act if the circle has radius. */
	if (num_qc_lines > 0) {
		/* Do the horizontal line through the center of the circle. */
		X1 = cx - q_circle[0];
		X2 = cx + q_circle[0];
		Y1 = Y2 = cy;
		if (clip_line())
			ABLINE();

		/* Do the upper and lower semi-circles. */
		for(k = 1; k < num_qc_lines; k++) {
			/* Upper semi-circle. */
			X1 = cx - q_circle[k];
			X2 = cx + q_circle[k];
			Y1 = Y2 = cy - k;
			if (clip_line())
				ABLINE();

			/* Lower semi-circle. */
			X1 = cx - q_circle[k];
			X2 = cx + q_circle[k];
			Y1 = Y2 = cy + k;
			if (clip_line())
				ABLINE();
		}
	}
}
#endif
