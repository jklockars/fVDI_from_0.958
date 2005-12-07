/*
 * Bitplane fill routines
 *
 * $Id: fill.c,v 1.4 2005-12-07 06:52:40 johan Exp $
 *
 * Copyright 2005, Johan Klockars 
 * Copyright 2002 The EmuTOS development team
 *
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "fvdi.h"
#include "relocate.h"

#define BOOL int
#define TRUE 1
#define FALSE 0
#define BYTE char
#define UBYTE unsigned char
#define WORD short
#define UWORD unsigned short
#define LONG long
#define ULONG unsigned long


extern Access *access;
static long dbg = 0;

extern void CDECL
c_get_colours(Virtual *vwk, long colour, short *foreground, short* background);


#define GetMemW(addr) ((ULONG)*(UWORD *)(addr))
#define SetMemW(addr, val) *(UWORD *)(addr) = val


/*
 * draw_rect - draw one or more horizontal lines
 *
 * Rewritten in C instead of assembler this routine is now looking
 * very complicated. Since in assembler you can pass parameters in
 * registers, you have an easy game to be both, fast and flexible.
 *
 * In C we need to double the code a bit, like loops and such things.
 * So, the following code works as follows:
 *
 * 1.  Fill variables with all values for masks, fringes etc.
 * 2.  Decide, if we have the situation of writing just one WORD
 * 3.  In any case we choose the writing mode now
 * 4.  Then loop through the y axis (just one pass for horzline)
 * 5.  Inner loop through the planes
 * 6a. If just one WORD, mask out the bits and process the action
 * 6b. else mask out left fringe, write WORDS, process the right fringe
 */

void draw_rect(Virtual *vwk, long x1, long y1, long w, long h, short *patternptr,
               UWORD fillcolor, long mode, long interior_style)
{
    UWORD leftmask, rightmask, patmsk;
    UWORD *addr;
    int x2 = x1 + w - 1;
    int y2 = y1 + h - 1;
    int dx, y, yinc;
    int patadd;               /* advance for multiplane patterns */
    int leftpart;
    int rightpart;
    int planes;
    UWORD pattern, bits, help;

    mode -= 1;

    /* Get the pattern with which the line is to be drawn. */
    planes = vwk->real_address->screen.mfdb.bitplanes;
    dx = x2 - x1 - 16;      /* Width of line - one WORD */
    addr = vwk->real_address->screen.mfdb.address;
    addr += (x1 / 16) * vwk->real_address->screen.mfdb.bitplanes;
    addr += (LONG)y1 * vwk->real_address->screen.wrap / 2;
#if 0
    patmsk = vwk->patmsk;                   /* Which pattern to start with */
    patadd = vwk->multifill ? 16 : 0;       /* Multi plane pattern offset */
#else
    patmsk = 0x0f;
    patadd = 0;
#endif
    leftpart  = x1 & 0xf;               /* Left fringe */
    rightpart = x2 & 0xf;               /* Right fringe */
    leftmask  = ~(0xffff >> leftpart);  /* Origin for not left fringe lookup */
    rightmask =  0x7fff >> rightpart;   /* Origin for right fringe lookup */
    yinc = vwk->real_address->screen.wrap / 2 - planes;

#if 0
    {
      char buf[10];
      access->funcs.ltoa(buf, x1, 10);
      access->funcs.puts(buf);
      access->funcs.puts(",");
      access->funcs.ltoa(buf, y1, 10);
      access->funcs.puts(buf);
      access->funcs.puts(" ");
      access->funcs.ltoa(buf, x2, 10);
      access->funcs.puts(buf);
      access->funcs.puts(",");
      access->funcs.ltoa(buf, y2, 10);
      access->funcs.puts(buf);
      access->funcs.puts(" ");
      access->funcs.ltoa(buf, (long)addr, 16);
      access->funcs.puts(buf);
      access->funcs.puts(" ");
      access->funcs.ltoa(buf, mode, 10);
      access->funcs.puts(buf);
      access->funcs.puts(" ");
      access->funcs.ltoa(buf, fillcolor, 10);
      access->funcs.puts(buf);
      access->funcs.puts("\x0a\x0d");
    }
#endif

    /* Check if we have to process just one single WORD on screen */
    if (dx + leftpart < 0) {
        switch (mode) {
        case 3:  /* nor */
            for(y = y1; y <= y2; y++ ) {
                int plane;
                int patind = patmsk & y;        /* Pattern to start with */
                UWORD color = fillcolor;

                /* Init adress counter */
                for(plane = planes - 1; plane >= 0; plane--) {
                    /* Load values fresh for this bitplane */
                    pattern = patternptr[patind];
                        /* Isolate the necessary pixels */
                        bits = *addr;
                        help = bits;

                    if (color & 0x0001) {
                        bits |= ~pattern;       /* Complement of mask with source */
                    } else {
                        bits &= pattern;        /* Complement of mask with source */
                    }
                        help ^= bits;           /* Isolate changed bits */
                        help &= leftmask | rightmask;   /* Isolate changed bits outside of fringe */
                        bits ^= help;           /* Restore them to original states */
                        *addr = bits;
                    addr++;                     /* Next plane */
                    patind += patadd;
                    color >>= 1;
                }
                addr += yinc;                   /* Next scanline */
            }
            break;

        case 2:  /* xor */
            for(y = y1; y <= y2; y++) {
                int plane;
                int patind = patmsk & y;        /* Pattern to start with */

                for(plane = planes - 1; plane >= 0; plane--) {
                    /* Load values fresh for this bitplane */
                    pattern = patternptr[patind];

                    /* Isolate the necessary pixels */
                    bits = *addr;
                    help = bits;
                    bits ^= pattern;
                    help ^= bits;        /* xor result with source - now have pattern */
                    help &= leftmask | rightmask;       /* Isolate bits */
                    bits ^= help;       /* Restore states of bits outside of fringe */
                    *addr = bits;
                    addr++;            /* Next plane */
                    patind += patadd;
                }
                addr += yinc;           /* Next scanline */
            }
            break;

        case 1:  /* or */
            for(y = y1; y <= y2; y++) {
                int plane;
                int patind = patmsk & y;        /* Pattern to start with */
                UWORD color = fillcolor;

                for(plane = planes - 1; plane >= 0; plane--) {
                    /* Load values fresh for this bitplane */
                    pattern = patternptr[patind];
                        /* Isolate the necessary pixels */
                        bits = *addr;
                        help = bits;

                    if (color & 0x0001) {
                        bits |= pattern;        /* Complement of mask with source */
                        help ^= bits;           /* Isolate changed bits */
                        help &= ~(leftmask | rightmask);        /* Isolate bits */
                    } else {
                        bits &= ~pattern;        /* Complement of mask with source */
                        help ^= bits;           /* Isolate changed bits */
                        help &= leftmask | rightmask;   /* Isolate bits */
                    }
                        bits ^= help;           /* Restore them to original states */
                        *addr = bits;
                    addr++;               /* Next plane */
                    patind += patadd;
                    color >>= 1;
                }
                addr += yinc;           /* Next scanline */
            }
            break;

        default: /* Replace */
            for(y = y1; y <= y2; y++) {
                int patind = patmsk & y;        /* Pattern to start with */
                int plane;
                UWORD color = fillcolor;

                for(plane = planes - 1; plane >= 0; plane--) {
                    /* Load values fresh for this bitplane */
                    bits;
                    pattern = 0;

                    /* Isolate the necessary pixels */
                    bits = *addr;
                    if (color & 0x0001) {
                        pattern = patternptr[patind];

                    bits ^= pattern;
                    bits &= leftmask | rightmask; /* Isolate the bits outside the fringe */
                    bits ^= pattern;    /* Restore the bits outside the fringe */
		    } else {
                    bits &= leftmask | rightmask; /* Isolate the bits outside the fringe */
		    }
                    *addr = bits;
                    addr++;             /* Next plane */
                    patind += patadd;
                    color >>= 1;
                }
                addr += yinc;           /* Next scanline */
            }
        }

    }
    /* Write the left fringe, then some whole WORDs, then the right fringe */
    else {
        leftpart = 16 - leftpart;       /* Precalculate delta for the left */

        switch (mode) {
        case 3:  /* nor */
            for(y = y1; y <= y2; y++) {
                int plane;
                int patind = patmsk & y;        /* Pattern to start with */
                UWORD color = fillcolor;

                /* Init adress counter */
                for(plane = planes - 1; plane >= 0; plane--) {
                    /* Load values fresh for this bitplane */
                    UWORD *adr = addr;
                    int pixels = dx;
                    pattern = patternptr[patind];

                    if (color & 0x0001) {
                        int bw;
                        pattern = ~pattern;
                        /* Check if the line is completely contained within one WORD */

                        /* Draw the left fringe */
                        if (leftmask) {
                            bits = *adr;
                            help = bits;
                            bits |= pattern;    /* Complement of mask with source */
                            help ^= bits;       /* Isolate changed bits */
                            help &= leftmask;   /* Isolate changed bits outside of fringe */
                            bits ^= help;       /* Restore them to original states */
                            *adr = bits;

                            adr += planes;
                            pixels -= leftpart;
                        }
                        /* Full bytes */
                        for(bw = pixels >> 4; bw >= 0; bw--) {
                            *adr |= pattern;
                            adr += planes;
                        }
                        /* Draw the right fringe */
                        if (~rightmask) {
                            bits = *adr;
                            help = bits;
                            bits |= pattern;    /* Complement of mask with source */
                            help ^= bits;       /* Isolate changed bits */
                            help &= rightmask;  /* Isolate changed bits outside of fringe */
                            bits ^= help;       /* Restore them to original states */
                            *adr = bits;
                        }
                        pattern = ~pattern;
                    } else {
                        int bw;

                        /* Draw the left fringe */
                        if (leftmask) {
                            bits = *adr;
                            help = bits;
                            bits &= pattern;    /* Complement of mask with source */
                            help ^= bits;       /* Isolate changed bits */
                            help &= leftmask;   /* Isolate changed bits outside of fringe */
                            bits ^= help;       /* Restore them to original states */
                            *adr = bits;

                            adr += planes;
                            pixels -= leftpart;
                        }
                        /* Full bytes */
                        for(bw = pixels >> 4; bw >= 0; bw--) {
                            *adr &= pattern;
                            adr += planes;
                        }
                        /* Draw the right fringe */
                        if (~rightmask) {
                            bits = *adr;
                            help = bits;
                            bits &= pattern;    /* Complement of mask with source */
                            help ^= bits;       /* Isolate changed bits */
                            help &= rightmask;  /* Isolate changed bits outside of fringe */
                            bits ^= help;       /* Restore them to original states */
                            *adr = bits;
                        }
                    }
                    addr++;                     /* Next plane */
                    patind += patadd;
                    color >>= 1;
                }
                addr += yinc;           /* Next scanline */
            }
            break;

        case 2:  /* xor */
            for(y = y1; y <= y2; y++ ) {
                int plane;
                int patind = patmsk & y;        /* Pattern to start with */

                for(plane = planes - 1; plane >= 0; plane--) {
                    /* Load values fresh for this bitplane */
                    UWORD *adr = addr;
                    int pixels = dx;
                    int bw;
                    pattern = patternptr[patind];

                    /* Draw the left fringe */
                    if (leftmask) {
                        bits = *adr;
                        help = bits;
                        bits ^= pattern;        /* xor the pattern with the source */
                        help ^= bits;           /* xor result with source - now have pattern */
                        help &= leftmask;       /* Isolate changed bits outside of fringe */
                        bits ^= help;           /* Restore states of bits outside of fringe */
                        *adr = bits;

                        adr += planes;
                        pixels -= leftpart;
                    }
                    /* Full bytes */
                    for(bw = pixels >> 4; bw >= 0; bw--) {
                        *adr ^= pattern;
                        adr += planes;
                    }
                    /* Draw the right fringe */
                    if (~rightmask) {
                        bits = *adr;
                        help = bits;
                        bits ^= pattern;    /* xor the pattern with the source */
                        help ^= bits;               /* xor result with source - now have pattern */
                        help &= rightmask;    /* Isolate changed bits outside of fringe */
                        bits ^= help;               /* Restore states of bits outside of fringe */
                        *adr = bits;
                    }
                    addr++;                  /* Next plane */
                    patind += patadd;
                }
                addr += yinc;           /* Next scanline */
            }
            break;

        case 1:  /* or */
            for(y = y1; y <= y2; y++) {
                int plane;
                int patind = patmsk & y;        /* Pattern to start with */
                UWORD color = fillcolor;

                for(plane = planes - 1; plane >= 0; plane--) {
                    /* Load values fresh for this bitplane */
                    UWORD *adr = addr;
                    int pixels = dx;
                    pattern = patternptr[patind];

                    if (color & 0x0001) {
                        int bw;

                        /* Draw the left fringe */
                        if (leftmask) {
                            bits = *adr;
                            help = bits;
                            bits |= pattern;    /* Complement of mask with source */
                            help ^= bits;       /* Isolate changed bits */
                            help &= ~leftmask;  /* Isolate changed bits outside of fringe */
                            bits ^= help;       /* Restore them to original states */
                            *adr = bits;

                            adr += planes;
                            pixels -= leftpart;
                        }
                        /* Full bytes */
                        for(bw = pixels >> 4; bw >= 0; bw--) {
                            *adr |= pattern;
                            adr += planes;
                        }
                        /* Draw the right fringe */
                        if (~rightmask) {
                            bits = *adr;
                            help = bits;
                            bits |= pattern;    /* Complement of mask with source */
                            help ^= bits;       /* Isolate changed bits */
                            help &= ~rightmask; /* Isolate changed bits outside of fringe */
                            bits ^= help;       /* restore them to original states */
                            *adr = bits;
                        }
                    } else {
                        int bw;

                        pattern = ~pattern;

                        /* Draw the left fringe */
                        if (leftmask) {
                            bits = *adr;
                            help = bits;
                            bits &= pattern;    /* Complement of mask with source */
                            help ^= bits;       /* Isolate changed bits */
                            help &= leftmask;   /* Isolate changed bits outside of fringe */
                            bits ^= help;       /* Restore them to original states */
                            *adr = bits;

                            adr += planes;
                            pixels -= leftpart;
                        }
                        /* Full bytes */
                        for(bw = pixels >> 4; bw >= 0; bw--) {
                            *adr &= pattern;
                            adr += planes;
                        }
                        /* Draw the right fringe */
                        if (~rightmask) {
                            bits = *adr;
                            help = bits;
                            bits &= pattern;    /* Complement of mask with source */
                            help ^= bits;       /* Isolate changed bits */
                            help &= rightmask;  /* Isolate changed bits outside of fringe */
                            bits ^= help;       /* Restore them to original states */
                            *adr = bits;
                        }
                        pattern = ~pattern;
                    }
                    addr++;                     /* Next plane */
                    patind += patadd;
                    color >>= 1;
                }
                addr += yinc;           /* Next scanline */
            }
            break;

        default: /* Replace */
            for(y = y1; y <= y2; y++ ) {
                int patind = patmsk & y;        /* Ppattern to start with */
                int plane;
                UWORD color = fillcolor;

                for(plane = planes - 1; plane >= 0; plane--) {
                    /* Load values fresh for this bitplane */
                    int bw;
                    UWORD *adr = addr;
                    int pixels = dx;
                    pattern = 0;

                    if (color & 0x0001)
                        pattern = patternptr[patind];

                    /* Draw the left fringe */
                    if (leftmask) {
                        bits = *adr;
                        bits ^= pattern;        /* xor the pattern with the source */
                        bits &= leftmask;       /* Isolate the bits outside the fringe */
                        bits ^= pattern;        /* Restore the bits outside the fringe */
                        *adr = bits;

                        adr += planes;
                        pixels -= leftpart;
                    }
                    /* Full WORDs */
                    for(bw = pixels >> 4; bw >= 0; bw--) {
                        *adr = pattern;
                        adr += planes;
                    }
                    /* Draw the right fringe */
                    if (~rightmask) {
                        bits = *adr;
                        bits ^= pattern;        /* xor the pattern with the source */
                        bits &= rightmask;      /* isolate the bits outside the fringe */
                        bits ^= pattern;        /* Restore the bits outside the fringe */
                        *adr = bits;
                    }
                    addr++;                     /* Next plane */
                    patind += patadd;
                    color >>= 1;
                }
                addr += yinc;           /* Next scanline */
            }
        }
    }
}


long CDECL
c_fill_area(Virtual *vwk, long x, long y, long w, long h, short *pattern,
            long colour, long mode, long interior_style)
{
  MFDB src;
  short foreground;
  short background;

  /* Don't understand any table operations yet */
  if ((long)vwk & 1)
    return -1;

    c_get_colours((Virtual *)((long)vwk & ~1), colour, &foreground, &background);

#if 0
  src.address   = pattern;
  src.width     = 16;
  src.height    = 16;
  src.wdwidth   = 1;
  src.standard  = 0;
  src.bitplanes = 1;

  /* Always fill with black, white or XOR, for the time being */
  if (mode == 3)
    c_blit_area(vwk, 0L, 0L, 0L, 0L, x, y, w, h, 12L);  /* XOR */
  else if (colour == 1)
    c_blit_area(vwk, 0L, 0L, 0L, 0L, x, y, w, h, 15L);  /* Black */
  else
    c_blit_area(vwk, 0L, 0L, 0L, 0L, x, y, w, h, 0L);   /* White */
#else
  draw_rect(vwk, x, y, w, h, pattern, foreground, mode, interior_style);
#endif

  return 1;
}


#if 0
long CDECL
c_write_pixel(Virtual *vwk, MFDB *mfdb, long x, long y, long colour)
{
  MFDB src;
  short pixel;

  /* Don't understand any table operations yet. */
  if ((long)vwk & 1)
    return -1;

  pixel = 0x8000;
  src.address   = &pixel;
  src.width     = 16;
  src.height    = 1;
  src.wdwidth   = 1;
  src.standard  = 0;
  src.bitplanes = 1;

  /* Blit a transparent (could as well have been replace mode) pixel */
  c_expand_area(vwk, &src, 0L, 0L, mfdb, x, y, 1L, 1L, 2L, colour);

  return 1;
}
#endif
