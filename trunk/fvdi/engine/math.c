/*
 * fVDI integer sin/cos/sqrt code
 *
 * The sin/cos part is almost unmodified from code with an
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
**                                                                      **
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

/*
 * Integer sine and cosine functions.
 */


#define HALFPI	900 
#define PI	1800
#define TWOPI	3600

/* Sines of angles 1 - 90 degrees normalized between 0 and 32767. */

static short sin_tbl[92] = {    
		    0,   572,  1144,  1716,  2286,  2856,  3425,  3993, 
		 4560,  5126,  5690,  6252,  6813,  7371,  7927,  8481, 
		 9032,  9580, 10126, 10668, 11207, 11743, 12275, 12803,
		13328, 13848, 14364, 14876, 15383, 15886, 16383, 16876,
		17364, 17846, 18323, 18794, 19260, 19720, 20173, 20621,
		21062, 21497, 21925, 22347, 22762, 23170, 23571, 23964,
		24351, 24730, 25101, 25465, 25821, 26169, 26509, 26841,
		27165, 27481, 27788, 28087, 28377, 28659, 28932, 29196,
		29451, 29697, 29934, 30162, 30381, 30591, 30791, 30982,
		31163, 31335, 31498, 31650, 31794, 31927, 32051, 32165,
		32269, 32364, 32448, 32523, 32587, 32642, 32687, 32722,
		32747, 32762, 32767, 32767 
		};
/*
 * Returns integer sin between -32767 and 32767.
 * Uses integer lookup table sintable^[].
 * Expects angle in tenths of degree 0 - 3600.
 * Assumes positive angles only.
 */
short Isin(short angle) 
{
	short  direction;		/* When checking quadrants, must keep track of direction in array. */
	short  index, remainder, tmpsin;	/* Holder for sin. */
	short  quadrant;		/* 0-3 = 1st, 2nd, 3rd, 4th. */

	while (angle > 3600) 
		angle -= 3600;
	quadrant = angle / HALFPI;
	direction = 1;

	switch (quadrant) {
	case 0:
		break;
	case 1:
		angle = PI - angle;
		break;
	case 2:
		angle -= PI;
		break;
	case 3:
		angle = TWOPI - angle;
		break;
	case 4:
		angle -= TWOPI;
		break;
	}

	index = angle / 10;
	remainder = angle % 10;
	tmpsin = sin_tbl[index];
	if (remainder != 0)   /* Add interpolation. */
		tmpsin += ((sin_tbl[index + 1] - tmpsin) * remainder) / 10;
		if (quadrant > 1) 
			tmpsin = -tmpsin;

	return tmpsin;
}

/*
 * Return integer cos between -32767 and 32767.
 */
short Icos(short angle) 
{
	angle = angle + HALFPI;
	if (angle > TWOPI)
		angle -= TWOPI;

	return Isin(angle);
}


#if 0
/* Quick integer square root using binomial theorem (from Dr. Dobbs journal) */
static ulong_t isqrt(ulong_t N)
{
        unsigned long l2, u, v, u2, n;

        if (2 > N) {
                return N;
        }
        u = N;
        l2 = 0;
        /* 1/2 * log_2 N = highest bit in the result */
        while ((u >>= 2)) {
                l2++;
        }
        u = 1L << l2;
        v = u;
        u2 = u << l2;
        while (l2--) {
                v >>= 1;
                n = (u + u + v) << l2;
                n += u2;
                if (n <= N) {
                        u += v;
                        u2 = n;
                }
        }
        return u;
}
#endif

#if 1
short isqrt(unsigned long x)
{
   unsigned long s1, s2;

   if (x < 2)
      return x;

   s1 = x;
   s2 = 2;
   do {
      s1 /= 2;
      s2 *= 2;
   } while (s1 > s2);

   s2 = (s1 + (s2 / 2)) / 2;
   
   do {
      s1 = s2;
      s2 = (x / s1 + s1) / 2;
   } while (s1 > s2);
   
   return (short)s1;
}
#endif
