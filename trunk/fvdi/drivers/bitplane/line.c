/*
 * abline - draw a line (general purpose)
 *
 * This routine draws a line between (_X1,_Y1) and (_X2,_Y2).
 * The line is modified by the LN_MASK and WRT_MODE variables.
 * This routine handles all 3 interleaved bitplanes video resolutions.
 *
 * Note that for line-drawing the background color is always 0 (i.e., there
 * is no user-settable background color).  This fact allows coding short-cuts
 * in the implementation of "replace" and "not" modes, resulting in faster
 * execution of their inner loops.
 *
 * This routines is more or less the one from the original VDI asm part.
 * I could not take bresenham, because pixels were set improperly in
 * use with the polygone filling part, did look ugly.  (MAD)
 *
 * input:
 *     X1, Y1, X2, Y2 = coordinates.
 *     num_planes     = number of video planes. (resolution)
 *     LN_MASK        = line mask. (for dashed/dotted lines)
 *     WRT_MODE       = writing mode:
 *                          0 => replace mode.
 *                          1 => or mode.
 *                          2 => xor mode.
 *                          3 => not mode.
 *
 * output:
 *     LN_MASK rotated to proper alignment with (X2,Y2).
 */

#include "fvdi.h"

#define WORD short
#define UWORD unsigned short
#define LONG long

extern void CDECL c_get_colours(Virtual *vwk, long colour, short *foreground, short* background);
extern long CDECL clip_line(Virtual *vwk, long *x1, long *y1, long *x2, long *y2);


long CDECL
c_line_draw(Virtual *vwk, long x1, long y1, long x2, long y2,
            long pattern, long colour, long mode)
{
    void *adr;                  /* Using void pointer is much faster */
    WORD dx;                    /* Width of rectangle around line */
    WORD dy;                    /* Height of rectangle around line */
    WORD xinc;                  /* Positive increase for each x step */
    WORD yinc;                  /* In/decrease for each y step */
    UWORD msk;
    int plane;
    UWORD linemask;             /* Linestyle bits */
    UWORD color;                /* Color index */

#if 0
    if (line->y1 == line->y2) {
        kprintf("Y = %d, MODE = %d.\n", line->y1, vwk->wrt_mode);
        horzline(X1, line);
        return;
    }
#endif

    /* Don't understand any table operations yet. */
    if ((long)vwk & 1)
      return -1;

    if (!clip_line(vwk, &x1, &y1, &x2, &y2))
      return 1;

    c_get_colours(vwk, colour, &color, (short *)&plane); /* Dummy background */
#if 0
    color = colour & 0xffff;
#endif
    linemask = pattern;                 /* To avoid compiler warning */

    /* Make x axis always going up */
    dy = y2 - y1;
    dx = x2 - x1;
    if (dx < 0) {
        /* if delta x < 0 then draw from point 2 to 1 */
        dx = -dx;
        dy = -dy;
        x1 = x2;
        y1 = y2;
    }

    /* Calculate increase values for x and y to add to actual address */
    yinc = vwk->real_address->screen.wrap;
    if (dy < 0) {
        dy = -dy;
        yinc = -yinc;
    }
    xinc = vwk->real_address->screen.mfdb.bitplanes << 1;

    adr = vwk->real_address->screen.mfdb.address;
    adr += (x1 / 16) * vwk->real_address->screen.mfdb.bitplanes * 2;
    adr += (LONG)y1 * vwk->real_address->screen.wrap;
    msk = 0x8000 >> (x1 & 0xf);         /* Initial bit position in WORD */

    for(plane = vwk->real_address->screen.mfdb.bitplanes - 1; plane >= 0; plane--) {
        void *addr;
        WORD  eps;              /* Epsilon */
        WORD  e1;               /* Epsilon 1 */
        WORD  e2;               /* Epsilon 2 */
        WORD  loopcnt;
        UWORD bit;

        /* Load values fresh for this bitplane */
        addr = adr;             /* Initial start address for changes */
        bit = msk;              /* Initial bit position in WORD */
        linemask = pattern;
	/* Skip back one step to make for easier check below */
        linemask = (linemask >> 1) | (linemask << 15);

        if (dx >= dy) {
            e1 = 2 * dy;
            eps = -dx;
            e2 = 2 * dx;

            switch(mode) {
            case 4:              /* Reverse transparent  */
                if (color & 0x0001) {
                    bit = ~bit;
                    for(loopcnt = dx; loopcnt >= 0; loopcnt--) {
                        linemask = (linemask >> 15) | (linemask << 1);     /* Get next bit of line style */
                        if (linemask & 0x8000)
                            *(WORD *)addr &= bit;
                        bit = (bit >> 1) | (bit << 15);
                        if (!(bit & 0x8000))
                            addr += xinc;
                        eps += e1;
                        if (eps >= 0) {
                            eps -= e2;
                            addr += yinc;       /* Increment y */
                        }
                    }
                } else {
                    for(loopcnt = dx; loopcnt >= 0; loopcnt--) {
                        linemask = (linemask >> 15) | (linemask << 1);     /* Get next bit of line style */
                        if (linemask & 0x8000)
                            *(WORD *)addr |= bit;
                        bit = (bit >> 1) | (bit << 15);
                        if (bit & 0x8000)
                            addr += xinc;
                        eps += e1;
                        if (eps >= 0) {
                            eps -= e2;
                            addr += yinc;       /* Increment y */
                        }
                    }
                }
                break;

            case 3:              /* xor */
                for(loopcnt = dx; loopcnt >= 0; loopcnt--) {
                    linemask = (linemask >> 15) | (linemask << 1);     /* Get next bit of line style */
                    if (linemask & 0x8000)
                        *(WORD *)addr ^= bit;
                    bit = (bit >> 1) | (bit << 15);
                    if (bit & 0x8000)
                        addr += xinc;
                    eps += e1;
                    if (eps >= 0) {
                        eps -= e2;
                        addr += yinc;       /* Increment y */
                    }
                }
                break;

            case 2:              /* or */
                if (color & 0x0001) {
                    for(loopcnt = dx; loopcnt >= 0; loopcnt--) {
		      linemask = (linemask >> 15) | (linemask << 1);     /* Get next bit of line style */
                        if (linemask & 0x8000)
                            *(WORD *)addr |= bit;
                        bit = (bit >> 1) | (bit << 15);
                        if (bit & 0x8000)
                            addr += xinc;
                        eps += e1;
                        if (eps >= 0) {
                            eps -= e2;
                            addr += yinc;       /* Increment y */
                        }
                    }
                } else {
                    bit = ~bit;
                    for(loopcnt = dx; loopcnt >= 0; loopcnt--) {
                        linemask = (linemask >> 15) | (linemask << 1);     /* Get next bit of line style */
                        if (linemask & 0x8000)
                            *(WORD *)addr &= bit;
                        bit = (bit >> 1) | (bit << 15);
                        if (!(bit & 0x8000))
                            addr += xinc;
                        eps += e1;
                        if (eps >= 0) {
                            eps -= e2;
                            addr += yinc;       /* Increment y */
                        }
                    }
                }
                break;

            case 1:              /* Replace */
                if (color & 0x0001) {
                    for(loopcnt = dx; loopcnt >= 0; loopcnt--) {
                        linemask = (linemask >> 15) | (linemask << 1);     /* Get next bit of line style */
                        if (linemask & 0x8000)
                            *(WORD *)addr |= bit;
                        else
                            *(WORD *)addr &= ~bit;
                        bit = (bit >> 1) | (bit << 15);
                        if (bit & 0x8000)
                            addr += xinc;
                        eps += e1;
                        if (eps >= 0) {
                            eps -= e2;
                            addr += yinc;       /* Increment y */
                        }
                    }
                }
                else {
                    bit = ~bit;
                    for(loopcnt = dx; loopcnt >= 0; loopcnt--) {
                        linemask = (linemask >> 15) | (linemask << 1);     /* Get next bit of line style */
                        *(WORD *)addr &= bit;
                        bit = (bit >> 1) | (bit << 15);
                        if (!(bit & 0x8000))
                            addr += xinc;
                        eps += e1;
                        if (eps >= 0) {
                            eps -= e2;
                            addr += yinc;       /* Increment y */
                        }
                    }
                }
            }
        } else {
            e1 = 2 * dx;
            eps = -dy;
            e2 = 2 * dy;

            switch (mode) {
            case 4:              /* Reverse transparent */
                if (color & 0x0001) {
                    bit = ~bit;
                    for(loopcnt = dy; loopcnt >= 0; loopcnt--) {
                        linemask = (linemask >> 15) | (linemask << 1);     /* Get next bit of line style */
                        if (linemask & 0x8000)
                            *(WORD *)addr &= bit;
                        addr += yinc;
                        eps += e1;
                        if (eps >= 0) {
                            eps -= e2;
                            bit = (bit >> 1) | (bit << 15);
                            if (!(bit & 0x8000))
                                addr += xinc;
                        }
                    }
                } else {
                    for(loopcnt = dy; loopcnt >= 0; loopcnt--) {
                        linemask = (linemask >> 15) | (linemask << 1);     /* Get next bit of line style */
                        if (linemask & 0x8000)
                            *(WORD *)addr |= bit;
                        addr += yinc;
                        eps += e1;
                        if (eps >= 0) {
                            eps -= e2;
                            bit = (bit >> 1) | (bit << 15);
                            if (bit & 0x8000)
                                addr += xinc;
                        }
                    }
                }
                break;

            case 3:              /* xor */
                for(loopcnt = dy; loopcnt >= 0; loopcnt--) {
                    linemask = (linemask >> 15) | (linemask << 1);     /* Get next bit of line style */
                    if (linemask & 0x8000)
                        *(WORD *)addr ^= bit;
                    addr += yinc;
                    eps += e1;
                    if (eps >= 0) {
                        eps -= e2;
                        bit = (bit >> 1) | (bit << 15);
                        if (bit & 0x8000)
                            addr += xinc;
                    }
                }
                break;

            case 2:              /* or */
                if (color & 0x0001) {
                    for(loopcnt = dy; loopcnt >= 0; loopcnt--) {
                        linemask = (linemask >> 15) | (linemask << 1);     /* Get next bit of line style */
                        if (linemask & 0x8000)
                            *(WORD *)addr |= bit;
                        addr += yinc;
                        eps += e1;
                        if (eps >= 0) {
                            eps -= e2;
                            bit = (bit >> 1) | (bit << 15);
                            if (bit & 0x8000)
                                addr += xinc;
                        }
                    }
                } else {
                    bit = ~bit;
                    for(loopcnt = dy; loopcnt >= 0; loopcnt--) {
                        linemask = (linemask >> 15) | (linemask << 1);     /* Get next bit of line style */
                        if (linemask & 0x8000)
                            *(WORD *)addr &= bit;
                        addr += yinc;
                        eps += e1;
                        if (eps >= 0) {
                            eps -= e2;
                            bit = (bit >> 1) | (bit << 15);
                            if (!(bit & 0x8000))
                                addr += xinc;
                        }
                    }
                }
                break;

            case 1:              /* Replace */
                if (color & 0x0001) {
                    for(loopcnt = dy; loopcnt >= 0; loopcnt--) {
                        linemask = (linemask >> 15) | (linemask << 1);     /* Get next bit of line style */
                        if (linemask & 0x8000)
                            *(WORD *)addr |= bit;
                        else
                            *(WORD *)addr &= ~bit;
                        addr += yinc;
                        eps += e1;
                        if (eps >= 0) {
                            eps -= e2;
                            bit = (bit >> 1) | (bit << 15);
                            if (bit & 0x8000)
                                addr += xinc;
                        }
                    }
                }
                else {
                    bit = ~bit;
                    for(loopcnt = dy; loopcnt >= 0; loopcnt--) {
                        linemask = (linemask >> 15) | (linemask << 1);     /* Get next bit of line style */
                        *(WORD *)addr &= bit;
                        addr += yinc;
                        eps += e1;
                        if (eps >= 0) {
                            eps -= e2;
                            bit = (bit >> 1) | (bit << 15);
                            if (!(bit & 0x8000))
                                addr += xinc;
                        }
                    }
                }
            }
        }
        adr += 2;
        color >>= 1;    /* Shift color index: next plane */
    }

    return 1;
}
