/*
 * palette.c - Palette handling routines
 *
 * Copyright 2005, Johan Klockars
 *
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */


#include "fvdi.h"


long CDECL
x_get_colour(Workstation *wk, long colour)
{
  static signed char tos_colours[] = {0,-1,1,2,4,6,3,5,7,8,9,10,12,14,11,13};
  int ret;

  colour &= 0x00ff;
  if (colour == 255)
    ret = 15;
  else if (colour >= 16)
    ret = colour;
  else {
    ret = tos_colours[colour];
    if (ret < 0)
      ret = wk->screen.palette.size - 1;
  }

  return ret;
}


long CDECL
c_get_colour(Virtual *vwk, long colour)
{
  return x_get_colour(vwk->real_address, colour);
}


void CDECL
c_get_colours(Virtual *vwk, long colour, short *foreground, short* background)
{
  *foreground = x_get_colour(vwk->real_address, colour & 0xffff);
  *background = x_get_colour(vwk->real_address, (colour >> 16) & 0xffff);
}


void CDECL
x_get_colours(Workstation *wk, long colour, short *foreground, short* background)
{
  *foreground = x_get_colour(wk, colour & 0xffff);
  *background = x_get_colour(wk, (colour >> 16) & 0xffff);
}


void CDECL
c_set_colours(Virtual *vwk, long start, long entries, unsigned short *requested,
 Colour palette[])
{
}
