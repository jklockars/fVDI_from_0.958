/*
 * palette.c - Palette handling routines
 *
 * Copyright 2005, Johan Klockars
 *
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */


#include "fvdi.h"


void CDECL
c_get_colour(Virtual *vwk, long colour, short *foreground, short* background)
{
  *foreground = colour & 0xffff;
  *background = (colour >> 16) & 0xffff;
}


void CDECL
c_set_colours(Virtual *vwk, long start, long entries, unsigned short *requested,
 Colour palette[])
{
}
