/* No debugging right now! */
#undef DEB
/*
 * fVDI text handling
 *
 * $Id: textlib.c,v 1.10 2006-02-23 09:36:09 johan Exp $
 *
 * Copyright 2005, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "fvdi.h"
#include "function.h"
#include "globals.h"

#define BLACK 1


void lib_vqt_extent(Virtual *vwk, long length, short *string, short *points);


#if 0
// This function is not allowed to use any kind of kerning and
// the character widths used are non-fractional (vqt_width)!
// Perhaps a version that takes a null-terminate C string?
// lib_v_gtext(x, y, string, length)
void lib_v_gtext(Virtual *vwk, long x, long y, short *string, long length)
{
    short points[8];
    int justification = 0;

    if (!length)
	return;

    if (vwk->text.alignment.horizontal) {
	lib_vqt_extent(vwk, length, string, points);
	justification = points[2];
#if 0
	justification++;    /* Was right coordinate, need width */
#endif
	if (vwk->text.alignment.horizontal != 2)
	    justification >>= 1;
    }

    x -= justification;

    // d1 = x,y
    // d0 = length
    // a0 = vwk
    // a1 = string
    // a2 = offset pointer (0)   No special offset table

#if 0
    move.l	vwk_real_address(a0),a3
    move.l	wk_r_text(a3),a3
    jsr	(a3)
#endif
}


// Currently not capable of dealing with colour at 20(a1)
// due to driver limitations.
void draw_text(Virtual *vwk, long x, long y,
               short *text, long length, long colour)
{
    // d1 = x,y
    // d0 = length
    // a0 = vwk
    // a1 = string
    // a2 = offset pointer (0)   No special offset table

#if 0
    move.l	vwk_real_address(a0),a3
    move.l	wk_r_text(a3),a3
    jsr	(a3)
#endif
}
#endif


#if 0
// This takes fractional character distances into account, unlike v_gtext.
v_ftext:
;	use_special_stack
	sub.l	#14,a7
	move.l	intin(a1),a2
	move.l	a2,4(a7)	; String
	move.l	control(a1),a2
	move.w	2(a2),d0
	move.w	6(a2),8(a7)	; Length
	beq	.v_ftext_end
	move.l	ptsin(a1),a2
	move.l	(a2)+,0(a7)	; x,y
	move.l	a2,10(a7)
	move.l	a7,a1
	cmp.w	#1,d0
	beq	.no_offset
	bsr	lib_v_ftext_offset
.v_ftext_end:
	add.l	#14,a7
	done_return

.no_offset:
	bsr	lib_v_gtext
	bra	.v_ftext_end

* lib_v_ftext_offset - Standard Library function
* Todo: ?
* In:	a1	Parameters (x, y, string, length, offset_table)
*	a0	VDI struct
lib_v_ftext_offset:
	uses_d1
	move.l	a3,-(a7)
	moveq	#0,d1
	tst.w	vwk_text_alignment_horizontal(a0)
	beq	.left_justified_f

	movem.l	d2-d6/a2-a4,-(a7)

	move.w	8(a1),d0		; Number of characters
	move.l	4(a1),a4
	move.l	10(a1),a2
	move.l	vwk_text_current_font(a0),a3
	move.w	font_code_low(a3),d3
	move.w	font_code_high(a3),d4
	move.l	font_table_character(a3),a3

	moveq	#0,d2			; Width total
	moveq	#0,d5			; max width
	lbra	.no_char,2
 label .loop,1
	move.w	(a4)+,d1
	sub.w	d3,d1			; Negative numbers are higher
	cmp.w	d4,d1			;  than code_high
	lbhi	.no_char,2
	add.w	d1,d1
	move.w	2(a3,d1.w),d6
	sub.w	0(a3,d1.w),d6
	add.w	(a2),d2
	add.w	d6,d2
	cmp.w	d2,d5
	lbhi	.no_char,2
	move.w	d2,d5			; Remember widest
 label .no_char,2
	addq.l	#4,a2
	ldbra	d0,.loop,1

	movem.l	(a7)+,d2-d6/a2-a4

	move.w	d2,d1
	cmp.w	#2,vwk_text_alignment_horizontal(a0)
	beq	.right_justified_f
	lsr.w	#1,d1 
.right_justified_f:
	neg.w	d1
	swap	d1
	clr.w	d1
.left_justified_f:

	add.l	0(a1),d1
	move.w	8(a1),d0
	move.l	10(a1),a2	; Offset table
	move.l	4(a1),a1
	move.l	vwk_real_address(a0),a3
	move.l	wk_r_text(a3),a3
	jsr	(a3)

	move.l	(a7)+,a3
	used_d1
	rts


	dc.b	0,0,"v_justified",0
* v_justified - Standard Trap function
* Todo: Actually do justification
* In:	a1	Parameter block
*	a0	VDI struct
v_justified:
;	use_special_stack
	sub.l	#16,a7
	move.l	intin(a1),a2
	move.l	(a2)+,12(a7)	; Flags
	move.l	a2,4(a7)	; String
	move.l	control(a1),a2
	move.w	6(a2),d0
	subq.w	#2,d0
	move.w	d0,8(a7)	; Length
	beq	.v_justified_end
	move.l	ptsin(a1),a2
	move.l	(a2)+,0(a7)	; x,y
	move.w	(a2),10(a7)	; x size
	move.l	a7,a1
	tst.l	12(a7)
	beq	.no_justification
	bsr	lib_v_justified
.v_justified_end:	; .end:
	add.l	#16,a7
	done_return

.no_justification:
	bsr	lib_v_gtext
	bra	.v_justified_end	; .end

// For vector fonts, requested width refers to sum of character widths.
// Extensions beyond that are not counted.
// lib_v_justified(x, y, string, length, x_size, word_space, char_space)
lib_v_justified:
	uses_d1
	movem.l	d2-d6/a3-a4,-(a7)

	move.l	#4*1024,-(a7)
	bsr	allocate_block
	addq.l	#4,a7
	tst.l	d0
	beq	.lib_v_justified_end
	move.l	d0,a2
	move.l	a2,-(a7)

	move.w	8(a1),d0		; Number of characters
	move.l	4(a1),a4		; String
	move.l	vwk_text_current_font(a0),a3
	move.w	font_code_low(a3),d3
	move.w	font_code_high(a3),d4
	move.l	font_table_character(a3),a3

	clr.l	(a2)+			; First character at 0,0
	moveq	#0,d2			; Width total
	moveq	#0,d5			; Spaces
	lbra	.no_char,2
 label .loop,1
	move.w	(a4)+,d1
	sub.w	d3,d1			; Negative numbers are higher
	cmp.w	d4,d1			;  than code_high
	lbhi	.no_char,2
	cmp.b	#' ',d1
	lbne	.skip,3
	addq.w	#1,d5
 label .skip,3
	add.w	d1,d1
	move.w	2(a3,d1.w),d6
	sub.w	0(a3,d1.w),d6
	add.w	d6,d2
	move.w	d6,(a2)+
	move.w	#0,(a2)+
 label .no_char,2
	ldbra	d0,.loop,1

	move.w	10(a1),d6
	sub.w	d2,d6
	beq	.updated
	ext.l	d6			; d6.l = amount to widen by (could be negative)

	move.w	8(a1),d4
	sub.w	d5,d4
	tst.w	12(a1)			; Any word spacing?
	beq	.spacing_done
	move.w	d5,d4
	tst.w	14(a1)
	beq	.spacing_done		; Any character spacing?
	move.w	8(a1),d4
.spacing_done:
	subq.w	#1,d4			; d4.w = number of positions to vary spacing at
	beq	.updated

	divs	d4,d6			; Ex 50 / 8 = 6 + 2/8	0/6/12/19/25/31/37/44/50
	move.l	d6,d2			;			2 4 6  80 2  4  6  80
	swap	d2
	ext.l	d2
	bpl	.positive
	neg.w	d2
.positive:
	moveq	#0,d3

	move.l	(a7),a2
	clr.l	(a2)+			; First character at 0,0
	move.w	8(a1),d0
	subq.w	#1,d0			; Skip first character
	move.l	4(a1),a4
	addq.l	#2,a4
	bra	.modify_loop_end
.modify_loop:
	move.w	(a2),d5
	move.w	(a4)+,d1
	cmp.b	#' ',d1
	bne	.not_space
	tst.w	14(a1)
	bra	.modify
.not_space:
	tst.w	12(a1)
.modify:
	beq	.modify_done
	add.w	d6,d5
.modify_done:
	add.w	d2,d3
	cmp.w	d4,d3
	blo	.x_done
	sub.w	d4,d3
	addq.w	#1,d5
	tst.l	d2
	bpl	.x_done
	subq.w	#2,d5
.x_done:
	move.w	d5,(a2)
	addq.l	#4,a2
.modify_loop_end:
	dbra	d0,.modify_loop

.updated:
	moveq	#0,d1
	tst.w	vwk_text_alignment_horizontal(a0)
	beq	.left_justified_j

	move.w	10(a1),d1
	cmp.w	#2,vwk_text_alignment_horizontal(a0)
	beq	.right_justified_j
	lsr.w	#1,d1 
.right_justified_j:
	neg.w	d1
	swap	d1
	clr.w	d1
.left_justified_j:

	add.l	0(a1),d1
	move.w	8(a1),d0
	move.l	4(a1),a1
	move.l	(a7),a2		; The character offset table
	move.l	vwk_real_address(a0),a3
	move.l	wk_r_text(a3),a3
	jsr	(a3)

	bsr	free_block	; Address still on stack
	addq.l	#4,a7

.lib_v_justified_end:		; .end:
	movem.l	(a7)+,d2-d6/a3-a4
	used_d1
	rts


	dc.b	0,"default_text",0
* _default_text - Buffer rendering or individual character mono-expand text routine
* Todo:	Add effects of effects to the size
* In:	a0	VDI struct
*	a1	Pointer to string
*	a2	Pointer to character offset table or zero
*	d0	String length
*	d1	Coordinates
* Call:	a0	VDI struct
*	a1	Parameters for lib_vrt_cpyfm
_default_text:
	movem.l	d3-d7/a3-a6,-(a7)

	moveq	#0,d4		; Offset extension of length
	move.l	a2,d3
	beq	.no_offsets
	move.l	a2,a3
	move.w	d0,d3
	subq.w	#1,d3
.test_offsets:
	move.l	(a3)+,d5
	beq	.no_offset_here
	tst.w	d5
	bne	.single_char
	swap	d5
	add.w	d5,d4
.no_offset_here:
	dbra	d3,.test_offsets
.no_offsets:

	move.l	d0,a3
	move.l	#0,-(a7)			; Get a memory block of any size (hopefully large)
	bsr	allocate_block
	addq.l	#4,a7
	tst.l	d0
	exg	d0,a3
	beq	.single_char
	move.l	a3,-(a7)			; For free_block below

	move.l	vwk_text_current_font(a0),a5	; Font structure
	move.w	font_height(a5),d5
	swap	d5

	btst	#3,font_flags(a5)		; Proportional?
	bne	.nonproportional

	movem.l	d0-d2/d4/a1/a3,-(a7)
	move.l	font_table_character(a5),a3
	move.w	font_code_low(a5),d2
	move.w	font_code_high(a5),d4

	moveq	#0,d3			; Width total
;	subq.w	#1,d0
	lbra	.no_char,2
 label .size_loop,1
	move.w	(a1)+,d1
	sub.w	d2,d1			; Negative numbers are higher
	cmp.w	d4,d1			;  than code_high
	lbhi	.no_char,2
	add.w	d1,d1
	add.w	2(a3,d1.w),d3
	sub.w	0(a3,d1.w),d3
 label .no_char,2
	ldbra	d0,.size_loop,1

	movem.l	(a7)+,d0-d2/d4/a1/a3
	bra	.width_done

.nonproportional:
	move.w	font_widest_cell(a5),d3
;	btst	#4,vwk_text_effects(a0)		; Outline?
;	beq	1$
;	addq.w	#2,d3
;1$:
	mulu	d0,d3

;	btst	#2,vwk_text_effects(a0)		; Italic?
;	beq	2$
;	add.w	font_offset_left(a5),d3
;	add.w	font_offset_right(a5),d3
;2$:
;	bra	.width_done

.width_done:

	tst.w	d4
	beq	.keep_width
	move.w	d4,d3
	add.w	font_widest_cell(a5),d3
.keep_width:
	move.w	d3,d5

	add.w	#15,d5
	lsr.w	#4,d5
	addq.w	#1,d5
	and.w	#$fffe,d5	; Even number of words wide
	add.w	d5,d5		; d5 - height, bytes wide

	movem.l	d1/d3,-(a7)
	movem.l	d5/a0/a3/a5,-(a7)

	tst.w	d4
	beq	.no_clear
	move.l	d5,d1
	swap	d1
	mulu	d5,d1
	lsr.w	#4,d1
	moveq	#0,d3
	moveq	#0,d4
	moveq	#0,d5
	moveq	#0,d6
 label .loop,1
	movem.l	d3-d6,(a3)
	add.w	#16,a3
	ldbra	d1,.loop,1
	movem.l	(a7),d5/a0/a3/a5
.no_clear:

	move.l	a1,a4				; String address
	sub.l	a1,a1				; Clip rectangle
	moveq	#5,d6				; Fake top line alignment
	moveq	#0,d3				; Coordinates
	moveq	#0,d4
	bsr	text_area
	move.l	4(a7),a0
	move.w	vwk_text_effects(a0),d0

	btst	#0,d0
	beq	.no_bold	
	movem.l	(a7),d5/a0/a3/a5
	move.l	a3,a0
	move.l	a3,a1
	move.w	d5,a6
	move.w	d5,d3
	lsr.w	#1,d3
	swap	d5
	move.w	d5,d7
	move.w	font_thickening(a5),d4
;	moveq	#1,d4
	bsr	bold
	move.l	4(a7),a0
	move.w	vwk_text_effects(a0),d0
.no_bold:

	btst	#3,d0
	beq	.no_underline
	movem.l	(a7),d5/a0/a3/a5
	move.w	d5,d0
	move.w	font_distance_top(a5),d1
	addq.w	#1,d1
	mulu	d0,d1
	add.l	d1,a3
	lsr.w	#2,d0
	subq.w	#1,d0
	move.w	font_underline(a5),d1
	subq.w	#1,d1
	moveq	#-1,d3
 label .loop1,1
	move.l	a3,a0
	move.w	d0,d2
 label .loop2,2
	move.l	d3,(a0)+
	ldbra	d2,.loop2,2
	add.w	d5,a3
	ldbra	d1,.loop1,1
	move.l	4(a7),a0
	move.w	vwk_text_effects(a0),d0
.no_underline:

	btst	#2,d0
	beq	.no_italic
	movem.l	(a7),d5/a0/a3/a5
	move.w	d5,d0
	move.w	font_distance_top(a5),d1
	add.w	font_distance_bottom(a5),d1
	addq.w	#1,d1
	mulu	d0,d1
	add.l	d1,a3
	lsr.w	#1,d0
	subq.w	#2,d0
	move.l	d5,d1
	swap	d1
	subq.w	#1,d1
	move.w	font_skewing(a5),d3
	moveq	#0,d4
 label .loop1b,1
	move.l	a3,a0
	move.w	d0,d2
 label .loop2b,2
	move.l	-4(a0),d6
	lsr.l	d4,d6
	move.w	d6,-(a0)
	ldbra	d2,.loop2b,2
	moveq	#0,d6
	move.w	-(a0),d6
	lsr.l	d4,d6
	move.w	d6,(a0)
	sub.w	d5,a3
	rol.w	#1,d3
	lbcc	.skip,3
	addq.w	#1,d4
 label .skip,3
	ldbra	d1,.loop1b,1
	move.l	4(a7),a0
	move.w	vwk_text_effects(a0),d0
.no_italic:

	btst	#4,d0
	beq	.no_outline
	movem.l	(a7),d5/a0/a3/a5
	move.l	a3,a0
	move.l	a3,a1
	move.w	d5,a6
	move.w	d5,d3
	swap	d5
	move.w	d5,d7
	mulu	d3,d5
	add.l	d5,a1
	move.l	a1,8(a7)		; New bitmap address
	lsr.w	#1,d3
	bsr	outline
	move.l	4(a7),a0
	move.w	vwk_text_effects(a0),d0
.no_outline:

	btst	#1,d0
	beq	.no_light
	movem.l	(a7),d5/a0/a3/a5
	move.w	d5,d0
	lsr.w	#2,d0
	subq.w	#1,d0
	move.l	d5,d1
	swap	d1
	subq.w	#1,d1
	move.w	font_lightening(a5),d2
	move.w	d2,d3
	swap	d3
	move.w	d2,d3
 label .loop1c,1
	move.l	a3,a0
	move.w	d0,d2
 label .loop2c,2
	and.l	d3,(a0)+
	ldbra	d2,.loop2c,2
	add.w	d5,a3
	rol.l	#1,d3
	ldbra	d1,.loop1c,1
.no_light:

	movem.l	(a7)+,d5/a0/a3/a5
	movem.l	(a7)+,d1/d3

	sub.l	#4,a7			; Create pens
;	move.w	vwk_text_colour(a0),0(a7)	; Foreground
;	move.w	#0,2(a7)		; Background
	move.l	vwk_text_colour(a0),d0
	swap	d0
	move.l	d0,0(a7)

	sub.l	#20,a7			; Create source MFDB for rendering buffer
	move.l	a3,mfdb_address(a7)	; Start address of buffer
	move.w	d5,d0
	lsr.w	#1,d0
	move.w	d0,mfdb_wdwidth(a7)	; Width in words
	lsl.w	#4,d0
	move.w	d0,mfdb_width(a7)	; Width in pixels
	swap	d5
	move.w	d5,mfdb_height(a7)	; Height in pixels
	move.w	#1,mfdb_standard(a7)	; Standard format
	move.w	#1,mfdb_bitplanes(a7)	; Monochrome

	sub.l	#16,a7			; Create points
	move.l	#0,0(a7)		; Source left, top
	subq.w	#1,d3
	move.w	d3,4(a7)		; Source right
	subq.w	#1,d5
	move.w	d5,6(a7)		; Source bottom

;	move.l	d1,d3

	move.l	vwk_text_current_font(a0),a5	; Font structure
	move.w	vwk_text_alignment_vertical(a0),d4
	add.w	d4,d4
	add.w	font_extra_distance(a5,d4.w),d1
	move.l	d1,8(a7)		; Destination left,top
	add.w	d5,d1
	move.w	d1,14(a7)		; Destination bottom
	swap	d1
	add.w	d3,d1
	move.w	d1,12(a7)		; Destination right

	sub.l	#18,a7			; Create parameters
	move.w	vwk_mode(a0),0(a7)	; Mode
	lea	18(a7),a3
	move.l	a3,2(a7)		; Points
	lea	16(a3),a3
	move.l	a3,6(a7)		; Source MFDB
	move.l	#0,10(a7)		; Screen as destination
	lea	20(a3),a3
	move.l	a3,14(a7)		; Pens

	move.l	a7,a1
	bsr	lib_vrt_cpyfm

	add.l	#18+16+20+4,a7

	bsr	free_block
	addq.l	#4,a7

	movem.l	(a7)+,d3-d7/a3-a6
	rts


.single_char:
;	movem.l	d3-d7/a3-a6,-(a7)
	sub.l	#4,a7			; Create pens
;	move.w	vwk_text_colour(a0),0(a7)	; Foreground
;	move.w	#0,2(a7)		; Background
	move.l	vwk_text_colour(a0),d5
	swap	d5
	move.l	d5,0(a7)

	sub.l	#20,a7			; Create source MFDB for font
	move.l	vwk_text_current_font(a0),a5	; Font structure
	move.l	font_data(a5),d5
	move.l	d5,mfdb_address(a7)	; Start address of font
	move.w	font_width(a5),d5
	lsr.w	#1,d5
	move.w	d5,mfdb_wdwidth(a7)	; Width in words
	lsl.w	#4,d5
	move.w	d5,mfdb_width(a7)	; Width in pixels
	move.w	font_height(a5),mfdb_height(a7)	; Height in pixels
	move.w	#1,mfdb_standard(a7)	; Standard format
	move.w	#1,mfdb_bitplanes(a7)	; Monochrome

	sub.l	#16,a7			; Create points
	move.w	#0,2(a7)		; Source top
	move.w	font_height(a5),d5
	subq.w	#1,d5
	move.w	d5,6(a7)		; Source bottom

	move.l	d1,d3

	move.w	vwk_text_alignment_vertical(a0),d4
	add.w	d4,d4
	add.w	font_extra_distance(a5,d4.w),d3
	move.l	d3,8(a7)		; Destination left,top
	add.w	d5,d3
	move.w	d3,14(a7)		; Destination bottom

	swap	d3
	subq.w	#1,d3			; Previous character s right edge,
	move.w	d3,12(a7)		;   current width is added below

	sub.l	#18,a7			; Create parameters
	move.w	vwk_mode(a0),0(a7)	; Mode
	lea	18(a7),a3
	move.l	a3,2(a7)		; Points
	lea	16(a3),a3
	move.l	a3,6(a7)		; Source MFDB
	move.l	#0,10(a7)		; Screen as destination
	lea	20(a3),a3
	move.l	a3,14(a7)		; Pens

	move.l	a1,a3			; String
	move.w	d0,d3			; Length
	move.w	font_code_low(a5),d1	; First character

	move.l	font_table_character(a5),a5	; Character offset table

	move.l	a2,d0
	bne	.loopend_offset
	bra	.loopend

.loop_char:			; .loop:
	move.w	(a3)+,d4	; Get character
	sub.w	d1,d4
	add.w	d4,d4
	move.w	0(a5,d4.w),d0
	move.w	d0,0+18(a7)	; Character source left coordinate
	move.w	2(a5,d4.w),d5
	move.w	d5,4+18(a7)	; Character source right coordinate
	sub.w	d0,d5		; Character width
	add.w	d5,12+18(a7)	; Character destination right coordinate
	
	move.l	a7,a1
	bsr	lib_vrt_cpyfm

	add.w	d5,8+18(a7)	; Increase destination x coordinate

.loopend:
	dbra	d3,.loop_char

.default_text_end:
	add.l	#18+16+20+4,a7
	movem.l	(a7)+,d3-d7/a3-a6
	rts

.loop_offset:			; .loop:
	move.w	(a3)+,d4	; Get character
	sub.w	d1,d4
	add.w	d4,d4
	move.w	0(a5,d4.w),d0
	move.w	d0,0+18(a7)	; Character source left coordinate
	move.w	2(a5,d4.w),d5
	move.w	d5,4+18(a7)	; Character source right coordinate
	sub.w	d0,d5		; Character width
	add.w	8+18(a7),d5
	subq.w	#1,d5
	move.w	d5,12+18(a7)	; Character destination right coordinate
	
	move.l	a7,a1
	move.l	a2,-(a7)
	bsr	lib_vrt_cpyfm
	move.l	(a7)+,a2

.loopend_offset:
	move.w	(a2)+,d4
	add.w	d4,8+18(a7)	; Increase destination x/y coordinates
	move.w	(a2)+,d4
	add.w	d4,10+18(a7)

	dbra	d3,.loop_offset
	bra	.default_text_end


* In:	a0	area1
*	a1	area2
*	a6	wrap
*	d4	fattening
*	d3	words
*	d7	lines
bold:
...
	rts


* In:	a0	area1
*	a1	area2
*	a6	wrap
*	d3	words
*	d7	lines
outline:
...
	rts

dscale:
...
	rts

uscale:
...
	rts






SUB1		equ	0		; Subtract 1 from text width? (NVDI apparently does not)
#endif

// colour_set = lib_vst_color(colour)
long lib_vst_color(Virtual *vwk, unsigned long colour)
{
  if (colour >= vwk->real_address->screen.palette.size)
    colour = BLACK;
  vwk->text.colour.foreground = colour;

  return colour;
}

// effects_set = lib_vst_effects(effects)
long lib_vst_effects(Virtual *vwk, long effects)
{
    effects &= vwk->real_address->writing.effects;
    vwk->text.effects = effects;

    return effects;
}

// lib_vst_alignment(halign, valign, &hresult, &vresult)
void lib_vst_alignment(Virtual *vwk, unsigned long halign, unsigned long valign,
                       short *hresult, short *vresult)
{
    if (halign > 2)    /* Not from wk struct? */
	halign = 0;    /* Left */
    if (valign > 5)    /* Not from wk struct? */
	valign = 0;    /* Baseline */

    *hresult = vwk->text.alignment.horizontal = halign;
    *vresult = vwk->text.alignment.vertical   = valign;
}

#if 0
// Todo: Check if any angle is allowed.
// angle_set = lib_vst_rotation(angle)
lib_vst_rotation:
	move.w	(a1),d0
	move.l	vwk_real_address(a0),a2
	tst.w	wk_writing_rotation_possible(a2)
	lbeq	.none,1
	cmp.w	#1,wk_writing_rotation_type(a2)
	lblo	.none,1
	lbhi	.any,3
	add.w	#450,d0
	divu	#900,d0			; Only allow right angles
	cmp.w	#3,d0			; Should probably check font
	lbls	.ok,2
 label .none,1
	moveq	#0,d0
 label .ok,2
	mulu	#900,d0
 label .any,3
	move.w	d0,vwk_text_rotation(a0)
	rts
#endif

// Todo: Also look for correct size?
// font_set = lib_vst_font(fontID)
int lib_vst_font(Virtual *vwk, long fontID)
{
    Fontheader *font;
    char buf[10];
    short dummy, size;

    if (!fontID)
	fontID = 1;

#if DEB
    puts("lib_vst_font ");
    ltoa(buf, fontID, 10);
    puts(buf);
    puts("\x0d\x0a");
#endif

    font = vwk->real_address->writing.first_font;
    if (vwk->text.current_font) {
	if (vwk->text.font == fontID)
	    return fontID;
	if (vwk->text.font > fontID)
	    font = font->extra.first_size;
    }


    do {
#if DEB
	puts("  loop\x0d\x0a");
	ltoa(buf, font->id, 10);
	puts(buf);
	puts("\x0d\x0a");
#endif
	if (fontID <= font->id)
	    break;
	font = font->next;
    } while (font);

    if (!font || (font->id != fontID)) {
#if DEB
	puts("  set first\x0d\x0a");
#endif
	fontID = 1;
	font = vwk->real_address->writing.first_font;
    }

    if (vwk->text.current_font)
        size = vwk->text.current_font->size;
    else
        size = 10;

    vwk->text.font = fontID;
    vwk->text.current_font = font;

    /* Choose the right size */
    lib_vst_point(vwk, size, &dummy, &dummy, &dummy, &dummy);

    return fontID;
}


// Apparently extended since NVDI 3.00 (add 33rd word, bitmap/vector flag)
// Perhaps a version that returns the name as 32 bytes rather than 32 words?
// id = lib_vqt_name(number, name)
long lib_vqt_name(Virtual *vwk, long number, short *name)
{
    int i;
    Fontheader *font;
    unsigned char *font_name;

    if (!number || ((unsigned long)number > vwk->real_address->writing.fonts))
	number = 1;

    font = vwk->real_address->writing.first_font;
    for(number -= 2; number >= 0; number--)
	font = font->next;

    font_name = font->name;
    for(i = 31; i >= 0; i--)
	*name++ = *font_name++;

    if (font->flags & 0x4000)
	*name = 1;   /* Vector font! */
    else
	*name = 0;

    return font->id;
}



// lib_vqt_font_info(&minchar, &maxchar, distance, &maxwidth, effects)
void lib_vqt_font_info(Virtual *vwk, short *minchar, short *maxchar,
                       short *distance, short *maxwidth, short *effects)
{
    *minchar    = vwk->text.current_font->code.low;
    *maxchar    = vwk->text.current_font->code.high;
    distance[0] = vwk->text.current_font->distance.bottom;
    distance[1] = vwk->text.current_font->distance.descent;
    distance[2] = vwk->text.current_font->distance.half;
    distance[3] = vwk->text.current_font->distance.ascent;
    distance[4] = vwk->text.current_font->distance.top;
    *maxwidth   = vwk->text.current_font->widest.cell;
    effects[0]  = 0;   /* Temporary current spec. eff. change of width! */
    effects[1]  = 0;   /* Temporary current spec. eff. change to left! */
    effects[2]  = 0;   /* Temporary current spec. eff. change to right! */
}


void lib_vqt_xfntinfo(Virtual *vwk, long flags, long id, long index,
                      XFNT_INFO *info)
{
  int i;
  Fontheader *font;

  font = vwk->real_address->writing.first_font;

  if (index) {
    for(i = index - 1; i >= 0; i--) {
      if (!(font = font->next))
	break;
    }
    if (font) {
      id = font->id;
    }
  } else if (id) {
    index = 1;
  } else {
    if (!vwk->text.current_font)
      vwk->text.current_font = font;
    id = vwk->text.current_font->id;
    index = 1;
  }

  while (font && (id > font->id)) {
    font = font->next;
    index++;
  }
  if (font && (id != font->id))
    font = 0;

  if (!font) {
    info->format = 0;
    info->id     = 0;
    info->index  = 0;
    return;
  }

  info->id     = id;
  info->index  = index;

  if ((font->flags & 0x8000) && external_xfntinfo) {
    set_stack_call(vdi_stack_top, vdi_stack_size,
                   external_xfntinfo,
                   vwk, font, flags, info);
    return;
  }

  info->format = 1;

  if (flags & 0x01) {
    for(i = 0; i < 32; i++) {
      info->font_name[i] = font->name[i];
    }
    info->font_name[i] = 0;
  }

  /* Dummy text */
  if (flags & 0x02) {
    strncpy(info->family_name, "Century 725 Italic BT",
            sizeof(info->family_name));
  }

  /* Dummy text */
  if (flags & 0x04) {
    strncpy(info->style_name, "Italic",
            sizeof(info->style_name));
  }

  if (flags & 0x08) {
      info->file_name1[0] = 0;
  }

  if (flags & 0x10) {
    info->file_name2[0] = 0;
  }

  if (flags & 0x20) {
    info->file_name3[0] = 0;
  }

  /* 0x100 is without enlargement, 0x200 with */
  if (flags & 0x300) {
    i = 0;
    font = font->extra.first_size;
    while (font) {
      info->pt_sizes[i] = font->size;
      i++;
      font = font->extra.next_size;
    }
    info->pt_cnt = i;
  }
}


/* Which one of these is really correct? */
#if 0
void lib_vqt_fontheader(Virtual *vwk, VQT_FHDR *fhdr, const char *filename)
#else
void lib_vqt_fontheader(Virtual *vwk, VQT_FHDR *fhdr)
#endif
{
  int i;
  Fontheader *font;

  /* Strings should not have NUL termination if max size. */
  /* Normally 1000 ORUs per Em square (width of 'M'), but header says. */
  /* 6 byte transformation parameters contain:
   *  short y offset (ORUs)
   *  short x scaling (units of 1/4096)
   *  short y scaling (units of 1/4096)
   */

  /* Is this correct? */
  font = vwk->text.current_font;

  if ((font->flags & 0x8000) && external_fontheader) {
    set_stack_call(vdi_stack_top, vdi_stack_size,
                   external_fontheader,
                   vwk, font, fhdr, 0);
    return;
  }

  memcpy(fhdr->fh_fmver, "D1.0\x0d\x0a\0\0", 8);  /* Format identifier */
  fhdr->fh_fntsz = 0;     /* Font file size */
  fhdr->fh_fbfsz = 0;     /* Minimum font buffer size (non-image data) */
  fhdr->fh_cbfsz = 0;     /* Minimum character buffer size (largest char) */
  fhdr->fh_hedsz = sizeof(VQT_FHDR);  /* Header size */
  fhdr->fh_fntid = 0;     /* Font ID (Bitstream) */
  fhdr->fh_sfvnr = 0;     /* Font version number */
  for(i = 0; i < 32; i++) {   /* Font full name (vqt_name) */
    fhdr->fh_fntnm[i] = font->name[i];
  }
  fhdr->fh_fntnm[i] = 0;
  fhdr->fh_mdate[0] = 0;  /* Manufacturing date (DD Mon YY) */
  fhdr->fh_laynm[0] = 0;  /* Character set name, vendor ID, character set ID */
  /* Last two is char set, usually the second two characters in font filename
   *   Bitstream International Character Set = '00'
   * Two before that is manufacturer, usually first two chars in font filename
   *   Bitstream fonts use 'BX'
   */
  fhdr->fh_cpyrt[0] = 0;  /* Copyright notice */
  fhdr->fh_nchrl = 0;     /* Number of character indices in character set */
  fhdr->fh_nchrf = 0;     /* Total number of character indices in font */
  fhdr->fh_fchrf = 0;     /* Index of first character */
  fhdr->fh_nktks = 0;     /* Number of kerning tracks */
  fhdr->fh_nkprs = 0;     /* Number of kerning pairs */
  fhdr->fh_flags = 0;     /* Font flags, bit 0 - extended mode */
  /* Extended mode is for fonts that require higher quality of rendering,
   * such as chess pieces. Otherwise compact, the default.
   */
  fhdr->fh_cflgs = 1;     /* Classification flags */
  /* bit 0 - Italic
   * bit 1 - Monospace
   * bit 2 - Serif
   * bit 3 - Display
   */
  fhdr->fh_famcl = 0;     /* Family classification */
  /* 0 - Don't care
   * 1 - Serif
   * 2 - Sans serif
   * 3 - Monospace
   * 4 - Script
   * 5 - Decorative
   */
  fhdr->fh_frmcl = 0x68;  /* Font form classification */
  /* 0x_4 - Condensed
   * 0x_5 - (Reserved for 3/4 condensed)
   * 0x_6 - Semi-condensed
   * 0x_7 - (Reserved for 1/4 condensed)
   * 0x_8 - Normal
   * 0x_9 - (Reserved for 3/4 expanded)
   * 0x_a - Semi-expanded
   * 0x_b - (Reserved for 1/4 expanded)
   * 0x_c - Expanded
   * 0x1_ - Thin
   * 0x2_ - Ultralight
   * 0x3_ - Extralight
   * 0x4_ - Light
   * 0x5_ - Book
   * 0x6_ - Normal
   * 0x7_ - Medium
   * 0x8_ - Semibold
   * 0x9_ - Demibold
   * 0xa_ - Bold
   * 0xb_ - Extrabold
   * 0xc_ - Ultrabold
   * 0xd_ - Heavy
   * 0xe_ - Black
   */
  /* Dummy text */
  strncpy(fhdr->fh_sfntn, "Century725BT-Italic",  /* Short font name */
          sizeof(fhdr->fh_sfntn));
  /* Abbreviation of Postscript equivalent font name */
  strncpy(fhdr->fh_sfacn, "Century 725 BT",  /* Short face name */
          sizeof(fhdr->fh_sfacn));
  /* Abbreviation of the typeface family name */
  strncpy(fhdr->fh_fntfm, "Italic",   /* Font form (as above), style */
          sizeof(fhdr->fh_fntfm));
  fhdr->fh_itang = 0;     /* Italic angle */
  /* Skew in 1/256 of degrees clockwise, if italic font */
  fhdr->fh_orupm = 2048;  /* ORUs per Em */
  /* Outline Resolution Units */

  /* There's actually a bunch of more values, but they are not
   * in the struct definition, so skip them
   */
}


// lib_vqt_extent(length, &string, points)
//_get_extent:
//	move.l	4(a7),a0		; vwk as parameter
//	lea	8(a7),a1
void lib_vqt_extent(Virtual *vwk, long length, short *string, short *points)
{
    short ch, width;
    unsigned short low, high;
    short *char_tab;

    /* Some other method should be used for this! */
    if (vwk->text.current_font->flags < 0) {
	/* Handle differently? This is not really allowed at all! */
	if (!external_vqt_extent)
	    return;
	width = set_stack_call(vdi_stack_top, vdi_stack_size,
			       external_vqt_extent,
			       vwk->text.current_font, string, length, 0);
    } else {

	char_tab = vwk->text.current_font->table.character;
	low      = vwk->text.current_font->code.low;
	high     = vwk->text.current_font->code.high;
	width   = 0;
	
	for(length-- ;length >= 0; length--) {
	    ch = *string++ - low;
	    /* Negative numbers are very high as unsigned */
	    if ((unsigned short)ch <= high)
		width += char_tab[ch + 1] - char_tab[ch];
	}
	
	if (vwk->text.effects & 0x01)
	    width += vwk->text.current_font->thickening;
	
	if (vwk->text.effects & 0x10)   /* Outline */
	    width += 2;
	
	if (!(vwk->text.effects & 0x04)) {
	    unsigned short skewing = vwk->text.current_font->skewing;
	    short height = vwk->text.current_font->height;
	    for(height--; height >= 0; height--) {
		skewing = (skewing << 1) | (skewing >> 15);
		width += skewing & 1;
	    }
	}
    }

#ifndef SUB1
    points[0] = 0;
    points[1] = 0;
    points[2] = width;
    points[3] = 0;
    points[4] = width;
    points[5] = vwk->text.current_font->height;
    points[6] = 0;
    points[7] = vwk->text.current_font->height;
#else
    points[0] = 0;
    points[1] = 0;
    points[2] = width - 1;
    points[3] = 0;
    points[4] = width - 1;
    points[5] = vwk->text.current_font->height - 1;
    points[6] = 0;
    points[7] = vwk->text.current_font->height - 1;
#endif
}


#if 0
// status = lib_vqt_width(char, &cellw, &left_offset, &right_offset)
lib_vqt_width:
	movem.l	d2,-(a7)
	move.w	(a1)+,d0		; Character to check
	move.l	vwk_text_current_font(a0),a0	; a0 no longer -> VDI struct!
	move.l	font_table_character(a0),a2
	move.w	font_code_low(a0),d1

	neg.w	d1
	add.w	d0,d1			; Negative numbers are higher
	cmp.w	font_code_high(a0),d1	;  than code_high
	lbhi	.no_char,3
	add.w	d1,d1
	move.w	2(a2,d1.w),d2
	sub.w	0(a2,d1.w),d2
	move.l	(a1)+,a2
	move.w	d2,(a2)
	
	move.w	font_flags(a0),d2
	and.w	#$0002,d2
	lbeq	.no_offset,1
	move.l	font_table_horizontal(a0),a2
	move.w	2(a2,d1.w),d2
	sub.w	0(a2,d1.w),d2
 label .no_offset,1
	move.l	(a1)+,a2
	move.w	d2,(a2)
	move.l	(a1),a2
	move.w	#0,(a2)			; Right hand offset?
	
 label .end,2
	movem.l	(a7)+,d2
	rts

 label .no_char,3
	moveq	#-1,d0
	lbra	.end,2


// lib_vst_height(height, &charw, &charh, &cellw, &cellh)
lib_vst_height:
	move.w	(a1)+,d0
	move.l	a3,-(a7)
	move.l	vwk_text_current_font(a0),a2
	move.l	font_extra_first_size(a2),a2
	move.l	a2,a3
 label .search,1
	move.w	font_distance_top(a2),d1
	cmp.w	d1,d0
	lblo	.found,2
	move.l	a2,a3
	move.l	font_extra_next_size(a2),a2
	move.l	a2,d1
	lbne	.search,1
 label .found,2
	move.l	a3,vwk_text_current_font(a0)
	move.w	font_widest_character(a3),d1
	move.l	(a1)+,a2
	move.w	d1,(a2)
	swap	d1
	move.w	font_distance_top(a3),d1
;	addq.w	#1,d1
	move.l	(a1)+,a2
	move.w	d1,(a2)				; Height in pixels
	move.l	d1,vwk_text_character(a0)	; Character w/h in vwk
	swap	d1
	move.w	font_widest_cell(a3),d1
	move.l	(a1)+,a2
	move.w	d1,(a2)
	swap	d1
	addq.w	#1,d1
	add.w	font_distance_bottom(a3),d1
	move.l	(a1),a2
	move.w	d1,(a2)				; Height in pixels
	move.l	d1,vwk_text_cell(a0)		; Cell w/h in vwk
	move.l	(a7)+,a3
	rts
#endif


// point_set = lib_vst_point(height, &charw, &charh, &cellw, &cellh)
int lib_vst_point(Virtual *vwk, long height, short *charw, short *charh,
                   short *cellw, short *cellh)
{
    Fontheader *font;

    /* Some other method should be used for this! */
#if DEB
    puts("lib_vst_point\x0d\x0a");
#endif
    if (vwk->text.current_font->flags < 0) {
#if DEB
	puts("  vector\x0d\x0a");
#endif
	/* Handle differently? This is not really allowed at all! */
	if (!external_vst_point)
	    return 0;
#if DEB
	puts("  vector ok\x0d\x0a");
#endif
	font = set_stack_call_p(vdi_stack_top, vdi_stack_size,
				external_vst_point,
				vwk, height, sizes, 0);
#if DEB
	puts("  vector found\x0d\x0a");
#endif
    } else {
#if DEB
	char buf[10];
	ltoa(buf, height, 10);
	puts(buf);
	puts(" ");
#endif

	font = vwk->text.current_font->extra.first_size;

	while (font->extra.next_size && (font->extra.next_size->size <= height)) {
	    font = font->extra.next_size;
	}
#if DEB
	ltoa(buf, font->height, 10);
	puts(buf);
	puts("\x0d\x0a");
#endif
    }

    vwk->text.current_font = font;
    *charw = vwk->text.character.width  = font->widest.character;
    *charh = vwk->text.character.height = font->distance.top;
    *cellw = vwk->text.cell.width       = font->widest.cell;
    *cellh = vwk->text.cell.height      = font->height;

    return font->size;
}


// lib_vqt_attributes(settings)
void lib_vqt_attributes(Virtual *vwk, short *settings)
{
    settings[0] = vwk->text.font;
    settings[1] = vwk->text.colour.foreground;
    settings[2] = vwk->text.rotation;
    settings[3] = vwk->text.alignment.horizontal;
    settings[4] = vwk->text.alignment.vertical;
    settings[5] = vwk->mode;
    settings[6] = vwk->text.character.width;
    settings[7] = vwk->text.character.height;
    settings[8] = vwk->text.cell.width;
    settings[9] = vwk->text.cell.height;
}

// fonts_loaded = lib_vst_load_fonts(select)
long lib_vst_load_fonts(Virtual *vwk, long select)
{
    return vwk->real_address->writing.fonts - 1;
}

// lib_vst_unload_fonts(select)
void lib_vst_unload_fonts(Virtual *vwk, long select)
{
}
