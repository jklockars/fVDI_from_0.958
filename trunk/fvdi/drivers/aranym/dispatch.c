/*
 *	Drawing function dispatch
 */

#include "fvdi.h"


long ARAnyM_gfx = 0x71384e75L;    /* ARAnyM native graphics subrutines (M68K_EMUL_OP_VIDEO_CONTROL) */

#define ARAnyM(n) (((long CDECL (*)(long, ...))&ARAnyM_gfx)n)


static MFDB*
simplify(Virtual* vwk, MFDB* mfdb)
{
   vwk = (Virtual *)((long)vwk & ~1);
   if (!mfdb)
      return 0;
   else if (!mfdb->address)
      return 0;
   else if (mfdb->address == vwk->real_address->screen.mfdb.address)
      return 0;
#if 0
   else if (mfdb->address == old_screen)
      return 0;
#endif
   else
      return mfdb;
}


static long*
clipping(Virtual *vwk, long *rect)
{
   vwk = (Virtual *)((long)vwk & ~1);
   if (!vwk->clip.on)
      return 0;
   
   rect[0] = vwk->clip.rectangle.x1;
   rect[1] = vwk->clip.rectangle.y1;
   rect[2] = vwk->clip.rectangle.x2;
   rect[3] = vwk->clip.rectangle.y2;
   
   return rect;
}


long CDECL
c_read_pixel(Virtual *vwk, MFDB *mfdb, long x, long y)
{
   return ARAnyM((1, vwk, simplify(vwk, mfdb), x, y));
}


long CDECL
c_write_pixel(Virtual *vwk, MFDB *mfdb, long x, long y, long colour)
{
   return ARAnyM((2, vwk, simplify(vwk, mfdb), x, y, colour));
}


long CDECL
c_mouse_draw(Workstation *wk, long x, long y, Mouse *mouse)
{
   if ((long)mouse > 3)
      return ARAnyM((3, wk, x, y, &mouse->mask, &mouse->data, mouse->hotspot.x, mouse->hotspot.y, *(long *)&mouse->colour, mouse->type));
   else
      return ARAnyM((3, wk, x, y, (long)mouse));   /* Why is the cast needed for Lattice C? */
}


long CDECL
c_expand_area(Virtual *vwk, MFDB *src, long src_x, long src_y, MFDB *dst, long dst_x, long dst_y, long w, long h, long operation, long colour)
{
   return ARAnyM((4, vwk, src, src_x, src_y, simplify(vwk, dst), dst_x, dst_y, w, h, operation, colour));
}


long CDECL
c_fill_area(Virtual *vwk, long x, long y, long w, long h, short *pattern, long colour, long mode, long interior_style)
{
   return ARAnyM((5, vwk, x, y, w, h, pattern, colour, mode, interior_style));
}


long CDECL
c_blit_area(Virtual *vwk, MFDB *src, long src_x, long src_y, MFDB *dst, long dst_x, long dst_y, long w, long h, long operation)
{
   return ARAnyM((6, vwk, simplify(vwk, src), src_x, src_y, simplify(vwk, dst), dst_x, dst_y, w, h, operation));
}


long CDECL
c_line_draw(Virtual *vwk, long x1, long y1, long x2, long y2, long pattern, long colour, long mode)
{
   long rect[4];
   
   return ARAnyM((7, vwk, x1, y1, x2, y2, pattern, colour, mode, (long)clipping(vwk, rect)));   /* Why is the cast needed for Lattice C? */
}


long CDECL
c_fill_polygon(Virtual *vwk, short points[], long n, short index[], long moves, short *pattern, long colour, long mode, long interior_style)
{
   long rect[4];
   
   return ARAnyM((8, vwk, points, n, index, moves, pattern, colour, mode, interior_style, (long)clipping(vwk, rect)));   /* Why is the cast needed for Lattice C? */
}


long CDECL
c_set_colour_hook(long index, long red, long green, long blue)
{
   return ARAnyM((9, index, red, green, blue));
}


long CDECL
c_set_resolution(long width, long height, long depth, long frequency)
{
   return ARAnyM((10, width, height, depth, frequency));
}


long CDECL
c_get_videoramaddress()
{
   return ARAnyM((15));
}


long CDECL
c_debug_aranym(long n)
{
   return ARAnyM((20, n));
}
