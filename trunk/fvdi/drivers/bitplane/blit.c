/*
 * Bitplane blit routines
 *
 * $Id: blit.c,v 1.6 2005-12-07 08:53:55 johan Exp $
 *
 * Copyright 2005, Johan Klockars 
 * Copyright 2003 The EmuTOS development team
 * Copyright 2002 Joachim Hoenig (blitter)
 *
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "fvdi.h"
#include "relocate.h"

#define DBG_BLIT 0      /* See, what happens (a bit) */

#define BOOL int
#define TRUE 1
#define FALSE 0
#define BYTE char
#define UBYTE unsigned char
#define WORD short
#define UWORD unsigned short
#define LONG long
#define ULONG unsigned long


/* flag:1 SOURCE and PATTERN   flag:0 SOURCE only */
#define PAT_FLAG        16


extern Access *access;
static long dbg = 0;

extern void CDECL
c_get_colours(Virtual *vwk, long colour, short *foreground, short* background);
extern void CDECL
x_get_colours(Workstation *wk, long colour, short *foreground, short* background);
extern long CDECL clip_line(Virtual *vwk, long *x1, long *y1, long *x2, long *y2);


/* Passes parameters to bitblt */
struct blit_frame {
    WORD b_wd;          // Width of block in pixels
    WORD b_ht;          // Height of block in pixels
    WORD plane_ct;      // Number of consequitive planes to blt
    UWORD fg_col;       // Foreground color (logic op table index:hi bit)
    UWORD bg_col;       // Background color (logic op table index:lo bit)
    UBYTE op_tab[4];    // Logic ops for all fore and background combos
    WORD s_xmin;        // Minimum X: source
    WORD s_ymin;        // Minimum Y: source
    UWORD *s_form;      // Source form base address
    WORD s_nxwd;        // Offset to next word in line  (in bytes)
    WORD s_nxln;        // Offset to next line in plane (in bytes)
    WORD s_nxpl;        // Offset to next plane from start of current plane
    WORD d_xmin;        // Minimum X: destination
    WORD d_ymin;        // Minimum Y: destination
    UWORD *d_form;      // Destination form base address
    WORD d_nxwd;        // Offset to next word in line  (in bytes)
    WORD d_nxln;        // Offset to next line in plane (in bytes)
    WORD d_nxpl;        // Offset to next plane from start of current plane
    UWORD *p_addr;      // Address of pattern buffer   (0:no pattern)
    WORD p_nxln;        // Offset to next line in pattern  (in bytes)
    WORD p_nxpl;        // Offset to next plane in pattern (in bytes)
    WORD p_mask;        // Pattern index mask

    /* These frame parameters are internally set */
    WORD p_indx;        // Initial pattern index
    UWORD *s_addr;      // Initial source address
    WORD s_xmax;        // Maximum X: source
    WORD s_ymax;        // Maximum Y: source
    UWORD *d_addr;      // Initial destination address
    WORD d_xmax;        // Maximum X: destination
    WORD d_ymax;        // Maximum Y: destination
    WORD inner_ct;      // Blt inner loop initial count
    WORD dst_wr;        // Destination form wrap (in bytes)
    WORD src_wr;        // Source form wrap (in bytes)
};


#define FXSR    0x80
#define NFSR    0x40
#define SKEW    0x0f

#define GetMemW(addr) ((ULONG)*(UWORD *)(addr))
#define SetMemW(addr, val) *(UWORD *)(addr) = val


/* Blitter registers */
typedef struct blit blit;
struct blit {
    WORD  src_x_inc, src_y_inc;
    ULONG src_addr;
    WORD  end_1, end_2, end_3;
    WORD  dst_x_inc, dst_y_inc;
    ULONG dst_addr;
    UWORD x_cnt, y_cnt;
    BYTE  hop, op, status, skew;
    //BYTE  ready;
};



#if 0
static inline void
do_blit(blit * blt)
{
    ULONG   blt_src_in;
    UWORD   blt_src_out, blt_dst_in, blt_dst_out, mask_out;
    int  xc, yc, last, first;

#if DBG_BLIT
    kprintf ("bitblt: Start\n");
    kprintf ("HALFT[] 0x%04x-%04x-%04x-%04x\n", (UWORD) blt->halftone[0], blt->h
alftone[1], blt->halftone[2], blt->halftone[3]);
    kprintf ("X COUNT 0x%04x\n", (UWORD) blt->x_cnt);
    kprintf ("Y COUNT 0x%04x\n", (UWORD) blt->y_cnt);
    kprintf ("X S INC 0x%04x\n", (UWORD) blt->src_x_inc);
    kprintf ("Y S INC 0x%04x\n", (UWORD) blt->src_y_inc);
    kprintf ("X D INC 0x%04x\n", (UWORD) blt->dst_x_inc);
    kprintf ("Y D INC 0x%04x\n", (UWORD) blt->dst_y_inc);
    kprintf ("ENDMASK 0x%04x-%04x-%04x\n", (UWORD) blt->end_1, (UWORD) blt->end_
2, (UWORD) blt->end_3);
    kprintf ("S_ADDR  0x%08lx\n", blt->src_addr);
    kprintf ("D_ADDR  0x%08lx\n", blt->dst_addr);
    kprintf ("HOP=%01d, OP=%02d\n", blt->hop & 0x3, blt->op & 0xf);
    kprintf ("HOPline=%02d\n", blt->status & 0xf);
    kprintf ("NFSR=%d, FXSR=%d, SKEW=%02d\n", (blt->skew & NFSR) != 0,
                                              (blt->skew & FXSR) != 0,
                                              (blt->skew & SKEW));
#endif
    if (blt->x_cnt == 0) blt->x_cnt = 65535;
    if (blt->y_cnt == 0) blt->y_cnt = 65535;

    xc = 0;
    yc = blt->y_cnt;
    while (yc-- > 0) {
        xc = blt->x_cnt;
        first = 1;
        blt_src_in = 0;
        /* next line to get rid of obnoxious compiler warnings */
        blt_src_out = blt_dst_out = 0;
        while (xc-- > 0) {
            last = (xc == 0);
            /* read source into blt_src_in */
            if (blt->src_x_inc >= 0) {
                if (first && (blt->hop & FXSR)) {
                    blt_src_in = GetMemW (blt->src_addr);
                    blt->src_addr += blt->src_x_inc;
                }
                blt_src_in <<= 16;

                if (last && (blt->hop & NFSR)) {
                    blt->src_addr -= blt->src_x_inc;
                } else {
                    blt_src_in |= GetMemW (blt->src_addr);
                    if (!last) {
                        blt->src_addr += blt->src_x_inc;
                    }
                }
            } else {
                if (first &&  (blt->hop & FXSR)) {
                    blt_src_in = GetMemW (blt->src_addr);
                    blt->src_addr +=blt->src_x_inc;
                } else {
                    blt_src_in >>= 16;
                }
                if (last && (blt->hop & NFSR)) {
                    blt->src_addr -= blt->src_x_inc;
                } else {
                    blt_src_in |= (GetMemW (blt->src_addr) << 16);
                    if (!last) {
                        blt->src_addr += blt->src_x_inc;
                    }
                }
            }
            /* shift blt->skew times into blt_src_out */
            blt_src_out = blt_src_in >> (blt->skew & SKEW);

            /* read destination into blt_dst_in */
            blt_dst_in = GetMemW (blt->dst_addr);
            /* op into blt_dst_out */
            switch (blt->op & 0xf) {
            case 0:
                blt_dst_out = 0;
                break;
            case 1:
                blt_dst_out = blt_src_out & blt_dst_in;
                break;
            case 2:
                blt_dst_out = blt_src_out & ~blt_dst_in;
                break;
            case 3:
                blt_dst_out = blt_src_out;
                break;
            case 4:
                blt_dst_out = ~blt_src_out & blt_dst_in;
                break;
            case 5:
                blt_dst_out = blt_dst_in;
                break;
            case 6:
                blt_dst_out = blt_src_out ^ blt_dst_in;
                break;
            case 7:
                blt_dst_out = blt_src_out | blt_dst_in;
                break;
            case 8:
                blt_dst_out = ~blt_src_out & ~blt_dst_in;
                break;
            case 9:
                blt_dst_out = ~blt_src_out ^ blt_dst_in;
                break;
            case 0xa:
                blt_dst_out = ~blt_dst_in;
                break;
            case 0xb:
                blt_dst_out = blt_src_out | ~blt_dst_in;
                break;
            case 0xc:
                blt_dst_out = ~blt_src_out;
                break;
            case 0xd:
                blt_dst_out = ~blt_src_out | blt_dst_in;
                break;
            case 0xe:
                blt_dst_out = ~blt_src_out | ~blt_dst_in;
                break;
            case 0xf:
                blt_dst_out = 0xffff;
                break;
            }

            /* and endmask */
            if (first) {
                mask_out = (blt_dst_out & blt->end_1) | (blt_dst_in & ~blt->end_1);
            } else if (last) {
                mask_out = (blt_dst_out & blt->end_3) | (blt_dst_in & ~blt->end_3);
            } else {
                mask_out = (blt_dst_out & blt->end_2) | (blt_dst_in & ~blt->end_2);
            }
            SetMemW (blt->dst_addr, mask_out);
            if (!last) {
                blt->dst_addr += blt->dst_x_inc;
            }
            first = 0;
        }
        blt->status = (blt->status + ((blt->dst_y_inc >= 0) ? 1 : 15)) & 0xef;
        blt->src_addr += blt->src_y_inc;
        blt->dst_addr += blt->dst_y_inc;
    }
    /* blt->status &= ~BUSY; */
    blt->y_cnt = 0;
}

#else
#if 0
static void
do_blit_0(blit *blt)
{
    int xc, yc, x_cnt;
    ULONG dst_addr  = blt->dst_addr;
    ULONG dst_x_inc = blt->dst_x_inc;
    ULONG dst_y_inc = blt->dst_y_inc;
    WORD end_1 = blt->end_1;
    WORD end_2;
    WORD end_3 = blt->end_3;

#if 1
    if (blt->x_cnt == 0)
       blt->x_cnt = 65535;
    if (blt->y_cnt == 0)
       blt->y_cnt = 65535;
#endif

    x_cnt = blt->x_cnt;
    yc = blt->y_cnt - 1;

    end_1 = ~end_1;
    end_2 = (long)yc >> 16;    // Force compiler to use 0 in a register!
    if (x_cnt > 1) {
	end_3 = ~end_3;
	x_cnt -= 3;
	for(; yc >= 0; yc--) {
	    *(UWORD *)dst_addr &= end_1;
	    dst_addr += dst_x_inc;
		
#if 1
	    for(xc = x_cnt; xc >= 0; xc--) {
		SetMemW(dst_addr, end_2);
		dst_addr += dst_x_inc;
	    }
#else
	    if (x_cnt >= 0) {
     xc = ((x_cnt + 1 + 7) >> 3) - 1;
     switch ((x_cnt + 1) & 0x07) {
     case 0:
         do {
		SetMemW(dst_addr, end_2);
		dst_addr += dst_x_inc;
     case 7:
		SetMemW(dst_addr, end_2);
		dst_addr += dst_x_inc;
     case 6:
		SetMemW(dst_addr, end_2);
		dst_addr += dst_x_inc;
     case 5:
		SetMemW(dst_addr, end_2);
		dst_addr += dst_x_inc;
     case 4:
		SetMemW(dst_addr, end_2);
		dst_addr += dst_x_inc;
     case 3:
		SetMemW(dst_addr, end_2);
		dst_addr += dst_x_inc;
     case 2:
		SetMemW(dst_addr, end_2);
		dst_addr += dst_x_inc;
     case 1:
		SetMemW(dst_addr, end_2);
		dst_addr += dst_x_inc;
	 } while (--xc >= 0);
     }
}
#endif
	    
	    *(UWORD *)dst_addr &= end_3;	
	    dst_addr += dst_y_inc;
	}
    } else {
	for(; yc >= 0; yc--) {
	    *(UWORD *)dst_addr &= end_1;
	    dst_addr += dst_y_inc;
	}
    }
}

static void
do_blit_15(blit * blt)
{
    int xc, yc, x_cnt;
    ULONG dst_addr  = blt->dst_addr;
    ULONG dst_x_inc = blt->dst_x_inc;
    ULONG dst_y_inc = blt->dst_y_inc;
    WORD end_1 = blt->end_1;
    WORD end_2;
    WORD end_3 = blt->end_3;

#if 1
    if (blt->x_cnt == 0)
       blt->x_cnt = 65535;
    if (blt->y_cnt == 0)
       blt->y_cnt = 65535;
#endif

    x_cnt = blt->x_cnt;
    yc = blt->y_cnt - 1;

    end_2 = ~((long)yc >> 16);    // Force compiler to use 0xffff in a register!

    if (x_cnt > 1) {
	x_cnt -= 3;
	for(; yc >= 0; yc--) {
	    *(UWORD *)dst_addr |= end_1;
	    dst_addr += dst_x_inc;
		
	    for(xc = x_cnt; xc >= 0; xc--) {
		SetMemW(dst_addr, end_2);
		dst_addr += dst_x_inc;
	    }
	    
	    *(UWORD *)dst_addr |= end_3;
	    dst_addr += dst_y_inc;
	}
    } else {
	for(; yc >= 0; yc--) {
	    *(UWORD *)dst_addr |= end_1;
	    dst_addr += dst_y_inc;
	}
    }
}
#endif


static void
do_blit_short_32(blit *blt)
{
    ULONG blt_src_in;
    UWORD blt_src_out, blt_dst_in;
    int yc;
    ULONG skew;
    ULONG dst_addr, src_addr;
    LONG src_x_inc, src_y_inc, dst_y_inc;
    WORD end_1;
    void *my_op;
    static void *ops[] = {
	&&op0, &&op1, &&op2, &&op3, &&op4, &&op5, &&op6, &&op7,
	&&op8, &&op9, &&opa, &&opb, &&opc, &&opd, &&ope, &&opf
    };

    skew      = blt->skew;
    dst_addr  = blt->dst_addr;
    src_addr  = blt->src_addr;
    src_y_inc = blt->src_y_inc;
    dst_y_inc = blt->dst_y_inc;
    src_x_inc = blt->src_x_inc;
    end_1     = blt->end_1;

    yc = blt->y_cnt;
#if 1
    if (yc == 0)
	yc = 65535;
#endif

    if (blt->op < 2)
	end_1 = ~end_1;
    my_op = ops[blt->op];

#if 1
    do {
	blt_src_in = GetMemW(src_addr);
	blt_src_in <<= 16;
	src_addr += src_x_inc;
	blt_src_in |= GetMemW(src_addr);
	if (src_x_inc < 0)
	    blt_src_in = (blt_src_in << 16) | (blt_src_in >> 16);
#else
    if (src_x_inc < 0) {
        src_addr += src_x_inc;
        src_x_inc = -src_x_inc;
    }
    src_y_inc += src_x_inc;
    do {
	blt_src_in = GetMemW(src_addr);
	blt_src_in <<= 16;
	blt_src_in |= GetMemW(src_addr + src_x_inc);
#endif

	/* Shift blt->skew times into blt_src_out */
	blt_src_out = blt_src_in >> skew;

	goto *my_op;

op0:
	*(UWORD *)dst_addr &= end_1;
	goto op_end;

op1:
	*(UWORD *)dst_addr &= blt_src_out | end_1;
	goto op_end;

op2:
	blt_src_out ^= end_1;
op8:
	blt_dst_in = GetMemW(dst_addr);
	blt_src_out = ((blt_src_out & end_1) | blt_dst_in) ^ end_1;
	SetMemW(dst_addr, blt_src_out);
	goto op_end;

op3:
	blt_dst_in = GetMemW(dst_addr);
	blt_src_out = ((blt_src_out ^ blt_dst_in) & end_1) ^ blt_dst_in;
	SetMemW(dst_addr, blt_src_out);
	goto op_end;

op4:
	*(UWORD *)dst_addr &= ~(blt_src_out & end_1);
	goto op_end;

op5:
	goto op_end;

op9:
	blt_src_out ^= end_1;
op6:
	*(UWORD *)dst_addr ^= (blt_src_out & end_1);
	goto op_end;

opd:
	blt_src_out ^= end_1;
op7:
	*(UWORD *)dst_addr |= (blt_src_out & end_1);
	goto op_end;

opa:
	*(UWORD *)dst_addr ^= end_1;
	goto op_end;

ope:
	blt_src_out ^= end_1;
opb:
	blt_dst_in = GetMemW(dst_addr) ^ end_1;
	blt_src_out = (blt_src_out & end_1) | blt_dst_in;
	SetMemW(dst_addr, blt_src_out);
	goto op_end;

opc:
	blt_dst_in = GetMemW(dst_addr) | end_1;
	blt_src_out = (blt_src_out & end_1) ^ blt_dst_in;
	SetMemW(dst_addr, blt_src_out);
	goto op_end;

opf:
	*(UWORD *)dst_addr |= end_1;
	goto op_end;
op_end:

	src_addr += src_y_inc;
	dst_addr += dst_y_inc;
    } while (--yc > 0);
}



static void
do_blit_short(blit *blt)
{
    ULONG blt_src_in;
    UWORD blt_src_out, blt_dst_in;
    int yc;
    ULONG skew;
    ULONG dst_addr, src_addr;
#if 1
    LONG src_x_inc;
#endif
    LONG src_y_inc, dst_y_inc;
    WORD end_1;
    void *my_op;
    static void *ops[] = {
	&&op0, &&op1, &&op2, &&op3, &&op4, &&op5, &&op6, &&op7,
	&&op8, &&op9, &&opa, &&opb, &&opc, &&opd, &&ope, &&opf
    };

    if (blt->hop & FXSR) {
	do_blit_short_32(blt);
	return;
    }

    skew      = blt->skew;
    dst_addr  = blt->dst_addr;
    src_addr  = blt->src_addr;
    src_y_inc = blt->src_y_inc;
    dst_y_inc = blt->dst_y_inc;
#if 1
    src_x_inc = blt->src_x_inc;
#endif
    end_1     = blt->end_1;

    yc = blt->y_cnt;
#if 1
    if (yc == 0)
	yc = 65535;
#endif

    if (blt->op < 2)
	end_1 = ~end_1;
    my_op = ops[blt->op];

    do {
	blt_src_in = GetMemW(src_addr);
#if 0
	if (src_x_inc < 0)
	    blt_src_in = (blt_src_in << 16) | (blt_src_in >> 16);
#else
	blt_src_in = (blt_src_in << 16) | blt_src_in;
#endif

	/* Shift blt->skew times into blt_src_out */
	blt_src_out = blt_src_in >> skew;

	goto *my_op;

op0:
	*(UWORD *)dst_addr &= end_1;
	goto op_end;

op1:
	*(UWORD *)dst_addr &= blt_src_out | end_1;
	goto op_end;

op2:
	blt_src_out ^= end_1;
op8:
	blt_dst_in = GetMemW(dst_addr);
	blt_src_out = ((blt_src_out & end_1) | blt_dst_in) ^ end_1;
	SetMemW(dst_addr, blt_src_out);
	goto op_end;

op3:
	blt_dst_in = GetMemW(dst_addr);
	blt_src_out = ((blt_src_out ^ blt_dst_in) & end_1) ^ blt_dst_in;
	SetMemW(dst_addr, blt_src_out);
	goto op_end;

op4:
	*(UWORD *)dst_addr &= ~(blt_src_out & end_1);
	goto op_end;

op5:
	goto op_end;

op9:
	blt_src_out ^= end_1;
op6:
	*(UWORD *)dst_addr ^= (blt_src_out & end_1);
	goto op_end;

opd:
	blt_src_out ^= end_1;
op7:
	*(UWORD *)dst_addr |= (blt_src_out & end_1);
	goto op_end;

opa:
	*(UWORD *)dst_addr ^= end_1;
	goto op_end;

ope:
	blt_src_out ^= end_1;
opb:
	blt_dst_in = GetMemW(dst_addr) ^ end_1;
	blt_src_out = (blt_src_out & end_1) | blt_dst_in;
	SetMemW(dst_addr, blt_src_out);
	goto op_end;

opc:
	blt_dst_in = GetMemW(dst_addr) | end_1;
	blt_src_out = (blt_src_out & end_1) ^ blt_dst_in;
	SetMemW(dst_addr, blt_src_out);
	goto op_end;

opf:
	*(UWORD *)dst_addr |= end_1;
	goto op_end;
op_end:

	src_addr += src_y_inc;
	dst_addr += dst_y_inc;
    } while (--yc > 0);
}




static void
do_blit(blit *blt)
{
    ULONG   blt_src_in;
    UWORD   blt_src_out, blt_dst_in, blt_dst_out;
    int  xc, yc;
    ULONG skew;
    ULONG dst_addr, src_addr;
    LONG src_x_inc, dst_x_inc, src_y_inc, dst_y_inc;
    ULONG end_1;
    void *my_op, *my_op1, *my_op2, *my_op3;
    static void *ops1[] = {
	&&op1_0, &&op1_1, &&op1_2, &&op1_3, &&op1_4, &&op1_5, &&op1_6, &&op1_7,
	&&op1_8, &&op1_9, &&op1_a, &&op1_b, &&op1_c, &&op1_d, &&op1_e, &&op1_f
    };
    static void *ops2[] = {
	&&op2_0, &&op2_1, &&op2_2, &&op2_end, &&op2_4, &&op2_5, &&op2_6, &&op2_7,
	&&op2_8, &&op2_9, &&op2_a, &&op2_b, &&op2_c, &&op2_d, &&op2_e, &&op2_f
    };
    static void *ops3[] = {
	&&op3_0, &&op3_1, &&op3_2, &&op3_3, &&op3_4, &&op3_5, &&op3_6, &&op3_7,
	&&op3_8, &&op3_9, &&op3_a, &&op3_b, &&op3_c, &&op3_d, &&op3_e, &&op3_f
    };

#if DBG_BLIT
    kprintf ("bitblt: Start\n");
    kprintf ("X COUNT 0x%04x\n", (UWORD)blt->x_cnt);
    kprintf ("Y COUNT 0x%04x\n", (UWORD)blt->y_cnt);
    kprintf ("X S INC 0x%04x\n", (UWORD)src_x_inc);
    kprintf ("Y S INC 0x%04x\n", (UWORD)blt->src_y_inc);
    kprintf ("X D INC 0x%04x\n", (UWORD)blt->dst_x_inc);
    kprintf ("Y D INC 0x%04x\n", (UWORD)blt->dst_y_inc);
    kprintf ("ENDMASK 0x%04x-%04x-%04x\n", (UWORD)end_1,
             (UWORD)blt->end_2, (UWORD)blt->end_3);
    kprintf ("S_ADDR  0x%08lx\n", src_addr);
    kprintf ("D_ADDR  0x%08lx\n", dst_addr);
    kprintf ("HOP=%01d, OP=%02d\n", blt->hop & 0x3, blt->op);
    kprintf ("HOPline=%02d\n", blt->status & 0xf);
    kprintf ("NFSR=%d, FXSR=%d, SKEW=%02d\n", (blt->skew & NFSR) != 0,
                                              (blt->skew & FXSR) != 0,
                                              (blt->skew & SKEW));
#endif

    if (blt->x_cnt == 1) {
	do_blit_short(blt);
	return;
    }

#if 0
    if (blt->op == 7 && dbg) {
      dbg = 2;
      //        blt->op = 3;
    }
#endif

    skew      = blt->skew;
    dst_addr  = blt->dst_addr;
    src_addr  = blt->src_addr;
    src_y_inc = blt->src_y_inc;
    dst_y_inc = blt->dst_y_inc;
    src_x_inc = blt->src_x_inc;
    dst_x_inc = blt->dst_x_inc;
    end_1     = ((ULONG)blt->end_3 << 16) | (UWORD)blt->end_1;

    if (blt->op < 2)
	end_1 = ~end_1;
    my_op1 = ops1[blt->op];
    my_op2 = ops2[blt->op];
    my_op3 = ops3[blt->op];

    yc = blt->y_cnt;
#if 1
    if (blt->x_cnt == 0)
       blt->x_cnt = 65535;
    if (yc == 0)
       yc = 65535;
#endif

    if (dbg) {
      char buf[10];
      access->funcs.ltoa(buf, end_1, 16);
      access->funcs.puts(buf);
      access->funcs.puts(" (");
      access->funcs.ltoa(buf, (long)blt->op, 16);
      access->funcs.puts(buf);
      access->funcs.puts(")\x0a\x0d");
    }

    do {
        xc = blt->x_cnt;
        blt_src_in = 0;
        /* Next line to get rid of obnoxious compiler warnings */
        blt_src_out = blt_dst_out = 0;

	xc--;
	/* Read source into blt_src_in */
	if (blt->hop & FXSR) {
	    blt_src_in = GetMemW(src_addr) << 16;
	    src_addr += src_x_inc;
	}
	blt_src_in |= GetMemW(src_addr);
	src_addr += src_x_inc;
	if (src_x_inc < 0)
	    blt_src_in = (blt_src_in << 16) | (blt_src_in >> 16);

	/* Shift blt->skew times into blt_src_out */
	blt_src_out = blt_src_in >> skew;

	goto *my_op1;
op1_0:
	*(UWORD *)dst_addr &= end_1;
	goto op1_end;

op1_1:
	*(UWORD *)dst_addr &= blt_src_out | end_1;
	goto op1_end;

op1_2:
	blt_src_out ^= end_1;
op1_8:
	blt_dst_in = GetMemW(dst_addr);
	blt_src_out = ((blt_src_out & end_1) | blt_dst_in) ^ end_1;
	SetMemW(dst_addr, blt_src_out);
	goto op1_end;

op1_3:
	blt_dst_in = GetMemW(dst_addr);
	blt_src_out = ((blt_src_out ^ blt_dst_in) & end_1) ^ blt_dst_in;
	SetMemW(dst_addr, blt_src_out);
	goto op1_end;

op1_4:
	*(UWORD *)dst_addr &= ~(blt_src_out & end_1);
	goto op1_end;

op1_5:
	goto op1_end;

op1_9:
	blt_src_out ^= end_1;
op1_6:
	*(UWORD *)dst_addr ^= (blt_src_out & end_1);
	goto op1_end;

op1_d:
	blt_src_out ^= end_1;
op1_7:
	*(UWORD *)dst_addr |= (blt_src_out & end_1);
	goto op1_end;

op1_a:
	*(UWORD *)dst_addr ^= end_1;
	goto op1_end;

op1_e:
	blt_src_out ^= end_1;
op1_b:
	blt_dst_in = GetMemW(dst_addr) ^ end_1;
	blt_src_out = (blt_src_out & end_1) | blt_dst_in;
	SetMemW(dst_addr, blt_src_out);
	goto op1_end;

op1_c:
	blt_dst_in = GetMemW(dst_addr) | end_1;
	blt_src_out = (blt_src_out & end_1) ^ blt_dst_in;
	SetMemW(dst_addr, blt_src_out);
	goto op1_end;

op1_f:
	*(UWORD *)dst_addr |= end_1;
	goto op1_end;
op1_end:

	if (xc) {
	    dst_addr += dst_x_inc;
	}

	my_op = my_op2;
        while (--xc > 0) {
            if (src_x_inc >= 0) {
                blt_src_in <<= 16;
                blt_src_in |= GetMemW(src_addr);
            } else {
                blt_src_in >>= 16;
                blt_src_in |= (GetMemW(src_addr) << 16);
            }
            src_addr += src_x_inc;

            /* Shift blt->skew times into blt_src_out */
            blt_src_out = blt_src_in >> skew;

	    goto *my_op;
op2_0:
	    blt_src_out = 0;
	    goto op2_end;

op2_4:
	    blt_src_out = ~blt_src_out;
op2_1:
	    blt_dst_in = GetMemW(dst_addr);
	    blt_src_out = blt_src_out & blt_dst_in;
	    goto op2_end;

op2_2:
	    blt_dst_in = GetMemW(dst_addr);
	    blt_src_out = blt_src_out & ~blt_dst_in;
	    goto op2_end;

op2_3:
	    goto op2_end;

op2_5:
	    blt_dst_in = GetMemW(dst_addr);
	    blt_src_out = blt_dst_in;
	    goto op2_end;

op2_9:
	    blt_src_out = ~blt_src_out;
op2_6:
	    blt_dst_in = GetMemW(dst_addr);
	    blt_src_out = blt_src_out ^ blt_dst_in;
	    goto op2_end;

op2_d:
	    blt_src_out = ~blt_src_out;
op2_7:
	    blt_dst_in = GetMemW(dst_addr);
	    blt_src_out = blt_src_out | blt_dst_in;
	    goto op2_end;

op2_8:
	    blt_dst_in = GetMemW(dst_addr);
	    blt_src_out = ~(blt_src_out | blt_dst_in);
	    goto op2_end;

op2_a:
	    blt_dst_in = GetMemW(dst_addr);
	    blt_src_out = ~blt_dst_in;
	    goto op2_end;

op2_e:
	    blt_src_out = ~blt_src_out;
op2_b:
	    blt_dst_in = GetMemW(dst_addr);
	    blt_src_out = blt_src_out | ~blt_dst_in;
	    goto op2_end;

op2_c:
	    blt_src_out = ~blt_src_out;
	    goto op2_end;

op2_f:
	    blt_src_out = 0xffff;
	    goto op2_end;
op2_end:

            SetMemW(dst_addr, blt_src_out);
	    dst_addr += dst_x_inc;
        }

	/* Read source into blt_src_in */
	if (src_x_inc >= 0) {
	  blt_src_in <<= 16;
	  if (blt->hop & NFSR) {
	    src_addr -= src_x_inc;
	  } else {
	    blt_src_in |= GetMemW(src_addr);
	  }
	} else {
	  blt_src_in >>= 16;
	  if (blt->hop & NFSR) {
	    src_addr -= src_x_inc;
	  } else {
	    blt_src_in |= (GetMemW(src_addr) << 16);
	  }
	}

	/* Shift blt->skew times into blt_src_out */
	blt_src_out = blt_src_in >> skew;

	end_1 = (end_1 >> 16) | (end_1 << 16);
	goto *my_op3;
op3_0:
	*(UWORD *)dst_addr &= end_1;
	goto op3_end;

op3_1:
	*(UWORD *)dst_addr &= blt_src_out | end_1;
	goto op3_end;

op3_2:
	blt_src_out ^= end_1;
op3_8:
	blt_dst_in = GetMemW(dst_addr);
	blt_src_out = ((blt_src_out & end_1) | blt_dst_in) ^ end_1;
	SetMemW(dst_addr, blt_src_out);
	goto op3_end;

op3_3:
	blt_dst_in = GetMemW(dst_addr);
	blt_src_out = ((blt_src_out ^ blt_dst_in) & end_1) ^ blt_dst_in;
	SetMemW(dst_addr, blt_src_out);
	goto op3_end;

op3_4:
	*(UWORD *)dst_addr &= ~(blt_src_out & end_1);
	goto op3_end;

op3_5:
	goto op3_end;

op3_9:
	blt_src_out ^= end_1;
op3_6:
	*(UWORD *)dst_addr ^= (blt_src_out & end_1);
	goto op3_end;

op3_d:
	blt_src_out ^= end_1;
op3_7:
	*(UWORD *)dst_addr |= (blt_src_out & end_1);
	goto op3_end;

op3_a:
	*(UWORD *)dst_addr ^= end_1;
	goto op3_end;

op3_e:
	blt_src_out ^= end_1;
op3_b:
	blt_dst_in = GetMemW(dst_addr) ^ end_1;
	blt_src_out = (blt_src_out & end_1) | blt_dst_in;
	SetMemW(dst_addr, blt_src_out);
	goto op3_end;

op3_c:
	blt_dst_in = GetMemW(dst_addr) | end_1;
	blt_src_out = (blt_src_out & end_1) ^ blt_dst_in;
	SetMemW(dst_addr, blt_src_out);
	goto op3_end;

op3_f:
	*(UWORD *)dst_addr |= end_1;
	goto op3_end;
op3_end:
	end_1 = (end_1 >> 16) | (end_1 << 16);

        src_addr += src_y_inc;
        dst_addr += dst_y_inc;
    } while (--yc > 0);
}
#endif


/*
 * endmask data
 *
 * a bit means:
 *
 *   0: Destination
 *   1: Source <<< Invert right end mask data >>>
 */

/* TiTLE: BLiT_iT */

/* PuRPoSE: */
/* Transfer a rectangular block of pixels located at an */
/* arbitrary X,Y position in the source memory form to */
/* another arbitrary X,Y position in the destination memory */
/* form using replace mode (boolean operator 3). */
/* The source and destination rectangles should not overlap. */

void
bit_blt(struct blit_frame *info)
{
    WORD plane;
    UWORD s_xmin, s_xmax;
    UWORD d_xmin, d_xmax;
    UWORD lendmask, rendmask;
    WORD skew, skew_idx;
    WORD s_span, s_xmin_off, s_xmax_off;
    WORD d_span, d_xmin_off, d_xmax_off;
    ULONG s_addr, d_addr;
    blit blitter;

    blit *blt = &blitter;

    /* Setting of skew flags */

    /* QUALIFIERS   ACTIONS   BITBLT DIRECTION: LEFT -> RIGHT */

    /* equal Sx&F> */
    /* spans Dx&F FXSR NFSR */

    /* 0     0     0    1 |..ssssssssssssss|ssssssssssssss..|   */
    /*   |......dddddddddd|dddddddddddddddd|dd..............|   */

    /* 0     1      1  0 */
    /*   |......ssssssssss|ssssssssssssssss|ss..............|   */
    /*   |..dddddddddddddd|dddddddddddddd..|   */

    /* 1     0     0    0 |..ssssssssssssss|ssssssssssssss..|   */
    /*   |...ddddddddddddd|ddddddddddddddd.|   */

    /* 1     1     1    1 |...sssssssssssss|sssssssssssssss.|   */
    /*   |..dddddddddddddd|dddddddddddddd..|   */

#define mSkewFXSR    0x80
#define mSkewNFSR    0x40

    const UBYTE skew_flags[8] = {
        mSkewNFSR,              /* Source span < Destination span */
        mSkewFXSR,              /* Source span > Destination span */
        0,                      /* Spans equal Shift Source right */
        mSkewNFSR+mSkewFXSR,    /* Spans equal Shift Source left */

        /* When Destination span is but a single word ... */
        0,                      /* Implies a Source span of no words */
        mSkewFXSR,              /* Source span of two words */
        0,                      /* Skew flags aren't set if Source and */
        0                       /* destination spans are both one word */
    };

    /* Calculate Xmax coordinates from Xmin coordinates and width */
    s_xmin = info->s_xmin;               /* d0<- src Xmin */
    s_xmax = s_xmin + info->b_wd - 1;    /* d1<- src Xmax=src Xmin+width-1 */
    d_xmin = info->d_xmin;               /* d2<- dst Xmin */
    d_xmax = d_xmin + info->b_wd - 1;    /* d3<- dst Xmax=dstXmin+width-1 */

    /* Skew value is (destination Xmin mod 16 - source Xmin mod 16) */
    /* && 0x000F.  Three discriminators are used to determine the */
    /* states of FXSR and NFSR flags: */

    /* bit 0     0: Source Xmin mod 16 =< Destination Xmin mod 16 */
    /*           1: Source Xmin mod 16 >  Destination Xmin mod 16 */

    /* bit 1     0: SrcXmax/16-SrcXmin/16 <> DstXmax/16-DstXmin/16 */
    /*                       Source span      Destination span */
    /*           1: SrcXmax/16-SrcXmin/16 == DstXmax/16-DstXmin/16 */

    /* bit 2     0: multiple word Destination span */
    /*           1: single word Destination span */

    /* These flags form an offset into a skew flag table yielding */
    /* correct FXSR and NFSR flag states for the given source and */
    /* destination alignments */

    skew_idx = 0x0000;                  /* Default */

    s_xmin_off = s_xmin >> 4;           /* d0<- word offset to src Xmin */
    s_xmax_off = s_xmax >> 4;           /* d1<- word offset to src Xmax */
    s_span = s_xmax_off - s_xmin_off;   /* d1<- Src span - 1 */

    d_xmin_off = d_xmin >> 4;           /* d2<- word offset to dst Xmin */
    d_xmax_off = d_xmax >> 4;           /* d3<- word offset to dst Xmax */
    d_span = d_xmax_off - d_xmin_off;   /* d3<- dst span - 1 */

    /* The last discriminator is the */
    if (d_span == s_span) {             /* equality of src and dst spans */
        skew_idx |= 0x0002;             /* Equal spans */
    }

    /* Number of words in dst line */
    blt->x_cnt = d_span + 1;            /* Set value in BLiTTER */

    /* Endmasks derived from source Xmin mod 16 and source Xmax mod 16 */
    lendmask =   0xffff >> (d_xmin % 16);
    rendmask = ~(0x7fff >> (d_xmax % 16));

    /* Does destination just span a single word? */
    if (!d_span) {
        /* Merge both end masks into Endmask1. */
        lendmask &= rendmask;           /* Single word end mask */
#if 0
	rendmask = lendmask;
#endif
        skew_idx |= 0x0004;             /* Single word dst */
        /* The other end masks will be ignored by the BLiTTER */
    }

    /* Calculate starting addresses */
    s_addr = (ULONG)info->s_form +
             (ULONG)info->s_ymin * (ULONG)info->s_nxln +
             (ULONG)s_xmin_off   * (ULONG)info->s_nxwd;
    d_addr = (ULONG)info->d_form +
             (ULONG)info->d_ymin * (ULONG)info->d_nxln +
             (ULONG)d_xmin_off   * (ULONG)info->d_nxwd;

#if 0
      if (info->b_ht != 1)
    {
      char buf[10];
      access->funcs.ltoa(buf, (long)s_addr, 16);
      access->funcs.puts(buf);
      access->funcs.puts("->");
      access->funcs.ltoa(buf, (long)d_addr, 16);
      access->funcs.puts(buf);
      access->funcs.puts(" : ");
      access->funcs.ltoa(buf, (long)info->s_nxwd, 10);
      access->funcs.puts(buf);
      access->funcs.puts(" , ");
      access->funcs.ltoa(buf, (long)info->d_nxwd, 10);
      access->funcs.puts(buf);
      access->funcs.puts(" ; ");
      access->funcs.ltoa(buf, (long)(info->s_nxln - info->s_nxwd * s_span), 10);
      access->funcs.puts(buf);
      access->funcs.puts(" , ");
      access->funcs.ltoa(buf, (long)(info->d_nxln - info->d_nxwd * d_span), 10);
      access->funcs.puts(buf);
      access->funcs.puts(" * ");
      access->funcs.ltoa(buf, (long)info->b_ht, 10);
      access->funcs.puts(buf);
      access->funcs.puts("\x0a\x0d");
    }
#endif

#if 1
    if (s_addr < d_addr) {
#else
      //    if ((info->s_form == info->d_form) && (s_addr < d_addr)) {
      //    if ((info->s_form != info->d_form) || (s_addr < d_addr)) {
    if (1) {
#endif
        /* Start from lower right corner, so add width+length */
        s_addr = (ULONG)info->s_form + 
                 (ULONG)info->s_ymax * (ULONG)info->s_nxln +
                 (ULONG)s_xmax_off   * (ULONG)info->s_nxwd;
        d_addr = (ULONG)info->d_form +
                 (ULONG)info->d_ymax * (ULONG)info->d_nxln +
                 (ULONG)d_xmax_off   * (ULONG)info->d_nxwd;

        /* Offset between consecutive words in planes */
        blt->src_x_inc = -info->s_nxwd;
        blt->dst_x_inc = -info->d_nxwd;

        /* Offset from last word of a line to first word of next one */
        blt->src_y_inc = -(info->s_nxln - info->s_nxwd * s_span);
        blt->dst_y_inc = -(info->d_nxln - info->d_nxwd * d_span);

        blt->end_1 = rendmask;          /* Left end mask */
        blt->end_2 = 0xFFFF;            /* Center mask */
        blt->end_3 = lendmask;          /* Right end mask */

        /* We start at maximum, d7<- Dst Xmax mod16 - Src Xmax mod16 */
        skew = (d_xmax & 0x0f) - (s_xmax & 0x0f);
        if (skew >= 0 )
            skew_idx |= 0x0001;         /* d6[bit0]<- Alignment flag */
    } else {
#if 0
      if (info->b_ht != 1)
    {
      access->funcs.puts("******* Top to bottom **********\x0a\x0d");
    }
#endif
        /* Offset between consecutive words in planes */
        blt->src_x_inc = info->s_nxwd;
        blt->dst_x_inc = info->d_nxwd;

        /* Offset from last word of a line to first word of next one */
        blt->src_y_inc = info->s_nxln - info->s_nxwd * s_span;
        blt->dst_y_inc = info->d_nxln - info->d_nxwd * d_span;

        blt->end_1 = lendmask;          /* Left end mask */
        blt->end_2 = 0xFFFF;            /* Center mask */
        blt->end_3 = rendmask;          /* Right end mask */

        /* d7<- Dst Xmin mod16 - Src Xmin mod16 */
        skew = (d_xmin & 0x0f) - (s_xmin & 0x0f);
        if (skew < 0)
            skew_idx |= 0x0001;         /* Alignment flag */

#if 0
    {
      char buf[10];
      access->funcs.ltoa(buf, (long)s_addr, 16);
      access->funcs.puts(buf);
      access->funcs.puts("->");
      access->funcs.ltoa(buf, (long)d_addr, 16);
      access->funcs.puts(buf);
      access->funcs.puts(" : ");
      access->funcs.ltoa(buf, (long)lendmask, 16);
      access->funcs.puts(buf);
      access->funcs.puts(" , ");
      access->funcs.ltoa(buf, (long)rendmask, 16);
      access->funcs.puts(buf);
      access->funcs.puts("\x0a\x0d");
    }
#endif

    }

#if 0
      if (info->b_ht != 1)
    {
      char buf[10];
      access->funcs.ltoa(buf, (long)s_addr, 16);
      access->funcs.puts(buf);
      access->funcs.puts("->");
      access->funcs.ltoa(buf, (long)d_addr, 16);
      access->funcs.puts(buf);
      access->funcs.puts("\x0a\x0d");
      access->funcs.puts("\x0a\x0d");
    }
#endif

    /*
     * The low nibble of the difference in Source and Destination alignment
     * is the skew value.  Use the skew flag index to reference FXSR and
     * NFSR states in skew flag table.
     */
    blt->skew = skew & 0x0f;
    blt->hop  = skew_flags[skew_idx];

    for(plane = info->plane_ct - 1; plane >= 0; plane--) {
        int op_tabidx;

        blt->src_addr = s_addr;         /* Load Source pointer to this plane */
        blt->dst_addr = d_addr;         /* Load Dest ptr to this plane */
        blt->y_cnt    = info->b_ht;     /* Load the line count */

        /* Calculate operation for actual plane */
        op_tabidx  = ((info->fg_col >> plane) & 0x0001) << 1;
        op_tabidx |=  (info->bg_col >> plane) & 0x0001;
        blt->op = info->op_tab[op_tabidx] & 0x000f;

        do_blit(blt);

        s_addr += info->s_nxpl;         /* a0-> start of next src plane */
        d_addr += info->d_nxpl;         /* a1-> start of next dst plane */
    }
#if 0
    while(info->b_ht == 114);
#endif
}


#if 0
/*
 * If bit 5 of mode is set, use pattern with blit
 */
/* This needs more thought (and to be actually implemented in the blit) */
static void
setup_pattern(Virtual *vwk, struct blit_frame *info)
{
    /* Multi-plane pattern? */
    info->p_nxpl = 0;           /* Next plane pattern offset default. */
    if (vwk->fill.user.multiplane) {
        info->p_nxpl = 32;      /* Yes, next plane pat offset = 32. */
    }
    info->p_addr = vwk->fill.user.pattern.in_use; /* Get pattern pointer */
    info->p_nxln = 2;           /* Offset to next line in pattern */
    info->p_mask = 0xf;         /* Pattern index mask */
}
#endif


/*
 * Fill the info structure with MFDB values
 */
static BOOL
setup_info(Workstation *wk, struct blit_frame *info, MFDB *src, MFDB *dst,
           int src_x, int src_y, int dst_x, int dst_y, int w, int h)
{
    /* Setup plane info for source MFDB */
    if (src && src->address && (src->address != wk->screen.mfdb.address)) {
        /* For a positive source address */
        info->s_form = src->address;
        info->s_nxwd = src->bitplanes * 2;
        info->s_nxln = src->wdwidth * info->s_nxwd;
    }
    else {
        /* Source form is screen */
        info->s_form = (UWORD *)wk->screen.mfdb.address;
        info->s_nxwd = wk->screen.mfdb.bitplanes * 2;
        info->s_nxln = wk->screen.wrap;
    }

    /* Setup plane info for destination MFDB */
    if (dst && dst->address && (dst->address != wk->screen.mfdb.address)) {
        /* For a positive address */
        info->d_form   = dst->address;
        info->plane_ct = dst->bitplanes;
        info->d_nxwd   = dst->bitplanes * 2;
        info->d_nxln   = dst->wdwidth * info->d_nxwd;
    }
    else {
        /* Destination form is screen */
        info->d_form   = (UWORD *)wk->screen.mfdb.address;
        info->plane_ct = wk->screen.mfdb.bitplanes;
        info->d_nxwd   = wk->screen.mfdb.bitplanes * 2;
        info->d_nxln   = wk->screen.wrap;
    }

    /* Source */
    info->s_xmin = src_x;
    info->s_ymin = src_y;
    info->s_xmax = src_x + w - 1;
    info->s_ymax = src_y + h - 1;

    /* Width and height of block in pixels */
    info->b_wd = w;
    info->b_ht = h;

    /* Destination */
    info->d_xmin = dst_x;
    info->d_ymin = dst_y;
    info->d_xmax = dst_x + w - 1;
    info->d_ymax = dst_y + h - 1;

    info->s_nxpl = 2;           /* Next plane offset (source) */
    info->d_nxpl = 2;           /* Next plane offset (destination) */

    /* Only 8, 4, 2, and 1 planes are valid (destination) */
    return info->plane_ct & ~0x000f;
}


/*
 * copy raster opaque
 *
 * This function copies a rectangular raster area from source form to
 * destination form using the logic operation specified by the application.
 */
long CDECL
c_blit_area(Virtual *vwk, MFDB *src, long src_x, long src_y, MFDB *dst,
            long dst_x, long dst_y, long w, long h, long operation)
{
    struct blit_frame info;     /* Holds some internal info for bit_blt */

#if 0
    /* If mode is made up of more than the first 5 bits */
    if (operation & ~0x001f)
        return 1;                       /* Mode is invalid */
#else
    operation &= 0x0f;
#endif

    /* Check the pattern flag (bit 5) and revert to log op # */
    info.p_addr = 0;                    /* Clear pattern pointer */
#if 0
    if (operation & PAT_FLAG) {
        operation &= ~PAT_FLAG;         /* Set bit to 0! */
        setup_pattern(vwk, &info);      /* Fill in pattern related stuff */
    }
#endif

    /* If true, the plane count is invalid or clipping took all! */
    if (setup_info(vwk->real_address, &info, src, dst, src_x, src_y, dst_x, dst_y, w, h))
        return 1;

    /* Planes of source and destination must be equal in number */
    if (info.s_nxwd != info.d_nxwd)
        return 1;

    info.op_tab[0] = operation;     /* fg:0 bg:0 */
    info.bg_col = 0;                /* bg:0 & fg:0 => only first OP_TAB */
    info.fg_col = 0;                /* entry will be referenced */

    bit_blt(&info);

    return 1;
}


long CDECL
x_blit_area(Workstation *wk, MFDB *src, long src_x, long src_y, MFDB *dst,
            long dst_x, long dst_y, long w, long h, long operation)
{
    struct blit_frame info;     /* Holds some internal info for bit_blt */

    /* If mode is made up of more than the first 5 bits */
#if 0
    if (operation & ~0x001f)
        return 1;                       /* Mode is invalid */
#else
    operation &= 0x0f;
#endif

    /* Check the pattern flag (bit 5) and revert to log op # */
    info.p_addr = 0;                    /* Clear pattern pointer */

    /* If true, the plane count is invalid or clipping took all! */
    if (setup_info(wk, &info, src, dst, src_x, src_y, dst_x, dst_y, w, h))
        return 1;

    /* Planes of source and destination must be equal in number */
    if (info.s_nxwd != info.d_nxwd)
        return 1;

    info.op_tab[0] = operation;     /* fg:0 bg:0 */
    info.bg_col = 0;                /* bg:0 & fg:0 => only first OP_TAB */
    info.fg_col = 0;                /* entry will be referenced */

    bit_blt(&info);

    return 1;
}


/*
 * vdi_vrt_cpyfm - copy raster transparent
 *
 * This function copies a monochrome raster area from source form to a
 * color area. A writing mode and color indices for both 0's and 1's
 * are specified in the INTIN array.
 */

long CDECL
c_expand_area(Virtual *vwk, MFDB *src, long src_x, long src_y,
              MFDB *dst, long dst_x, long dst_y, long w, long h,
              long operation, long colour)
{
    struct blit_frame info;     /* Holds some internal info for bit_blt */
    short foreground;
    short background;

    c_get_colours((Virtual *)((long)vwk & ~1), colour, &foreground, &background);

#if 0
    /* With this active, no texts disappear on icons */
    foreground = 1;
    background = 0;
#endif

#if 0
    /* If mode is made up of more than the first 5 bits */
    if (operation & ~0x001f)
        return;                 /* Mode is invalid */
#else
    operation = ((operation - 1) & 0x03) + 1;
#endif

    /* Check the pattern flag (bit 5) and revert to log op # */
    info.p_addr = 0;            /* Get pattern pointer*/
#if 0
    if (operation & PAT_FLAG) {
        operation &= ~PAT_FLAG;      /* Set bit to 0! */
        setup_pattern(vwk, &info);   /* Fill in pattern related stuff */
    }
#endif

#if 0
    /* Try to get everything to display! */
    if (dst_x && !(dst_x % 16))
 #if 0
        dst_x -= 1;
 #else
        dbg = 1;
 #endif
#endif
#if 0
    if ((w != 1) || (h != 1))
    {
      char buf[10];
      access->funcs.ltoa(buf, src_x, 10);
      access->funcs.puts(buf);
      access->funcs.puts(",");
      access->funcs.ltoa(buf, src_y, 10);
      access->funcs.puts(buf);
      access->funcs.puts(" ");
      access->funcs.ltoa(buf, dst_x, 10);
      access->funcs.puts(buf);
      access->funcs.puts(",");
      access->funcs.ltoa(buf, dst_y, 10);
      access->funcs.puts(buf);
      access->funcs.puts(" ");
      access->funcs.ltoa(buf, w, 10);
      access->funcs.puts(buf);
      access->funcs.puts(",");
      access->funcs.ltoa(buf, h, 10);
      access->funcs.puts(buf);
      access->funcs.puts("\x0a\x0d");
    }
#endif

    /* If true, the plane count is invalid or clipping took all! */
    if (setup_info(vwk->real_address, &info, src, dst, src_x, src_y, dst_x, dst_y, w, h))
        return 1;

    /*
     * COPY RASTER TRANSPARENT - copies a monochrome raster area
     * from source form to a color area. A writing mode and color
     * indices for both 0's and 1's are specified in the INTIN array.
     */

    /* Is source area one plane? */
    if (info.s_nxwd != 2)
        return 1;    /* Source must be mono plane */

    info.s_nxpl = 0;            /* Use only one plane of source */

    switch(operation) {
    case 2:
        info.op_tab[0] = 04;    /* fg:0 bg:0  D' <- [not S] and D */
        info.op_tab[2] = 07;    /* fg:1 bg:0  D' <- S or D */
        info.fg_col = foreground;       /* We're only interested in one color */
        info.bg_col = 0;                /* Save the color of interest */
        break;

    case 1:
        /* CHECK: bug, that colors are reversed? */
        info.op_tab[0] = 00;    /* fg:0 bg:0  D' <- 0 */
        info.op_tab[1] = 12;    /* fg:0 bg:1  D' <- not S */
        info.op_tab[2] = 03;    /* fg:1 bg:0  D' <- S */
        info.op_tab[3] = 15;    /* fg:1 bg:1  D' <- 1 */
        info.bg_col = background;      /* Save fore and background colors */
        info.fg_col = foreground;
        break;

    case 3:
        info.op_tab[0] = 06;    /* fg:0 bg:0  D' <- S xor D */
        info.bg_col = 0;
        info.fg_col = 0;
        break;

    case 4:
        info.op_tab[0] = 01;    /* fg:0 bg:0  D' <- S and D */
        info.op_tab[1] = 13;    /* fg:0 bg:1  D' <- [not S] or D */
        info.fg_col = 0;                /* We're only interested in one color */
        info.bg_col = background;       /* Save the color of interest */
        break;

    default:
        return 1;                   /* Unsupported mode */
    }

    bit_blt(&info);

    return 1;
}


long CDECL
x_expand_area(Workstation *wk, MFDB *src, long src_x, long src_y,
              MFDB *dst, long dst_x, long dst_y, long w, long h,
              long operation, long colour)
{
    struct blit_frame info;     /* Holds some internal info for bit_blt */
    short foreground;
    short background;

#if 1
    x_get_colours(wk, colour, &foreground, &background);
#else
    foreground = colour & 0xffff;
    background = (colour >> 16) & 0xffff;
#endif

#if 0
    /* If mode is made up of more than the first 5 bits */
    if (operation & ~0x001f)
        return;                 /* Mode is invalid */
#else
    operation = ((operation - 1) & 0x03) + 1;
#endif

    /* Check the pattern flag (bit 5) and revert to log op # */
    info.p_addr = 0;            /* Get pattern pointer*/

    /* If true, the plane count is invalid or clipping took all! */
    if (setup_info(wk, &info, src, dst, src_x, src_y, dst_x, dst_y, w, h))
        return 1;

    /*
     * COPY RASTER TRANSPARENT - copies a monochrome raster area
     * from source form to a color area. A writing mode and color
     * indices for both 0's and 1's are specified in the INTIN array.
     */

    /* Is source area one plane? */
    if (info.s_nxwd != 2)
        return 1;    /* Source must be mono plane */

    info.s_nxpl = 0;            /* Use only one plane of source */

    switch(operation) {
    case 2:
        info.op_tab[0] = 04;    /* fg:0 bg:0  D' <- [not S] and D */
        info.op_tab[2] = 07;    /* fg:1 bg:0  D' <- S or D */
        info.fg_col = foreground;       /* We're only interested in one color */
        info.bg_col = 0;                /* Save the color of interest */
        break;

    case 1:
        /* CHECK: bug, that colors are reversed? */
        info.op_tab[0] = 00;    /* fg:0 bg:0  D' <- 0 */
        info.op_tab[1] = 12;    /* fg:0 bg:1  D' <- not S */
        info.op_tab[2] = 03;    /* fg:1 bg:0  D' <- S */
        info.op_tab[3] = 15;    /* fg:1 bg:1  D' <- 1 */
        info.bg_col = background;      /* Save fore and background colors */
        info.fg_col = foreground;
        break;

    case 3:
        info.op_tab[0] = 06;    /* fg:0 bg:0  D' <- S xor D */
        info.bg_col = 0;
        info.fg_col = 0;
        break;

    case 4:
        info.op_tab[0] = 01;    /* fg:0 bg:0  D' <- S and D */
        info.op_tab[1] = 13;    /* fg:0 bg:1  D' <- [not S] or D */
        info.fg_col = 0;                /* We're only interested in one color */
        info.bg_col = background;       /* Save the color of interest */
        break;

    default:
        return 1;                     /* Unsupported mode */
    }

    bit_blt(&info);

    return 1;
}


long CDECL
c_read_pixel(Virtual *vwk, MFDB *mfdb, long x, long y)
{
  MFDB dst;
  short pixel[8], *ptr;
  int colour, i;

  dst.address   = pixel;
  dst.width     = 16;
  dst.height    = 1;
  dst.wdwidth   = 1;
  dst.standard  = 0;
  dst.bitplanes = mfdb->bitplanes;

  /* Fetch one pixel in D=S mode */
  c_blit_area(vwk, mfdb, x, y, &dst, 15, 0, 1, 1, 3);

  colour = 0;
  ptr = pixel;
  for(i = mfdb->bitplanes - 1; i >= 0; --i) {
    colour *= 2;
    colour += *ptr++ & 1;
  }

  return colour;
}


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


long CDECL
c_mouse_draw(Workstation *wk, long x, long y, Mouse *mouse)
{
    static short mask[16], data[16];
    static short saved[16 * 8];
    static short old_x = 0;
    static short old_y = 0;
    int i;
    MFDB src, dst;

    src.width     = dst.width     = 16;
    src.height    = dst.height    = 16;
    src.wdwidth   = dst.wdwidth   = 1;
    src.standard  = dst.standard  = 0;
//    src.bitplanes = dst.bitplanes = 1;

    switch((long)mouse) {
    case 0:   /* Move */
        /* Restore old background */
	src.address = saved;
	src.bitplanes = wk->screen.mfdb.bitplanes;
	x_blit_area(wk, &src, 0, 0, 0, old_x, old_y, 16, 16, 3);
	/* Save new background */
	dst.address = saved;
	dst.bitplanes = wk->screen.mfdb.bitplanes;
	x_blit_area(wk, 0, x, y, &dst, 0, 0, 16, 16, 3);
	old_x = x;
	old_y = y;
	/* Draw mask */
	src.address = mask;
	src.bitplanes = 1;
	x_expand_area(wk, &src, 0, 0, 0, x, y, 16, 16, 2, 1);
	/* Draw shape */
	src.address = data;
	src.bitplanes = 1;
	x_expand_area(wk, &src, 0, 0, 0, x, y, 16, 16, 2, 0);
	break;
    case 1:
	break;
    case 2:   /* Hide */
        /* Restore old background */
	src.address = saved;
	src.bitplanes = wk->screen.mfdb.bitplanes;
	x_blit_area(wk, &src, 0, 0, 0, old_x, old_y, 16, 16, 3);
	break;
    case 3:   /* Show */
	/* Save new background */
	dst.address = saved;
	dst.bitplanes = wk->screen.mfdb.bitplanes;
	x_blit_area(wk, 0, x, y, &dst, 0, 0, 16, 16, 3);
	old_x = x;
	old_y = y;
	/* Draw mask */
	src.address = mask;
	src.bitplanes = 1;
	x_expand_area(wk, &src, 0, 0, 0, x, y, 16, 16, 2, 1);
	/* Draw shape */
	src.address = data;
	src.bitplanes = 1;
	x_expand_area(wk, &src, 0, 0, 0, x, y, 16, 16, 2, 0);
	break;
    case 4:
    case 5:
    case 6:
    case 7:
        break;
    default:  /* New mouse shape */
	for(i = 0; i < 16; i++) {
	    mask[i] = mouse->mask[i];
	    data[i] = mouse->data[i];
	}
	break;
    }

    return 0; 
}
