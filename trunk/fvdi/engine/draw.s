*****
* fVDI drawing functions
*
* $Id: draw.s,v 1.7 2004-10-17 21:44:11 johan Exp $
*
* Copyright 1997-2003, Johan Klockars 
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

transparent	equ	1		; Fall through?

;max_arc_count	equ	256

	include	"vdi.inc"
	include	"macros.inc"

	xref	clip_point,clip_line,clip_rect
	xref	setup_plot,tos_colour
	xref	_line_types
	xref	lib_vqt_extent,lib_vrt_cpyfm
	xref	allocate_block,free_block
	xref	_pattern_ptrs
	xref	_filled_poly,_filled_poly_m,_clc_arc,_wide_line,_calc_bez
	xref	_arc_split,_arc_min,_arc_max
	xref	_lib_v_bez,_rounded_box

	xdef	v_pline,v_circle,v_arc,v_ellipse,v_ellarc,v_pie,v_ellpie
	xdef	v_pmarker
	xdef	v_fillarea
	xdef	lib_v_pline
	xdef	_lib_v_pline
	xdef	v_rbox,v_rfbox

	xdef	_default_line
	xdef	_fill_poly,_hline,_fill_spans
	xdef	_c_pline
	xdef	_v_bez_accel


	text

* c_pline(vwk, numpts, colour, points)
_c_pline:
	move.l	a2,-(a7)
	subq.l	#6,a7
	move.l	6+4+4(a7),a0
	move.l	6+4+8(a7),d0
	move.w	d0,0(a7)
	move.l	6+4+12(a7),d0
	move.l	6+4+16(a7),2(a7)
	move.l	a7,a1
	bsr	c_v_pline
	addq.l	#6,a7
	move.l	(a7)+,a2
	rts


	dc.b	0,0,"v_pline",0
* v_pline - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_pline:
	uses_d1
	move.l	control(a1),a2
	move.w	L_intin(a2),d1
	beq	.normal
	tst.w	vwk_bezier_on(a0)
	bne	v_bez
	cmp.w	#13,subfunction(a2)
	beq	v_bez
.normal:
	subq.l	#6,a7
	move.w	L_ptsin(a2),0(a7)
	move.l	ptsin(a1),2(a7)		; List of coordinates

	move.l	a7,a1
	bsr	lib_v_pline
	addq.l	#6,a7
	used_d1
	done_return			; Should be real_return


v_bez:
	sub.w	#22,a7
	move.w	L_ptsin(a2),0(a7)
	move.l	ptsin(a1),2(a7)
	move.l	intin(a1),6(a7)
	move.l	ptsout(a1),a2
	move.l	a2,10(a7)
	move.l	intout(a1),a2
	move.l	a2,14(a7)
	addq.l	#2,a2
	move.l	a2,18(a7)

	move.l	control(a1),a2
	move.w	#2,L_ptsout(a2)
	move.w	#6,L_intout(a2)

	move.l	a7,a1
	bsr	lib_v_bez
	add.w	#22,a7
	used_d1
	done_return

* v_bez_accel((long)vwk + 1, points, 
*             (num_points << 16) | 1, *par->totmoves, xmov,
*             pattern, vwk->line.colour, vwk->mode);
_v_bez_accel:
	movem.l	d2-d6,-(a7)
	move.l	5*4+4(a7),d0
	and.w	#$fffe,d0
	move.l	d0,a0
	move.l	5*4+8(a7),d1
	move.l	5*4+12(a7),d2
	move.l	5*4+16(a7),d3
	move.l	5*4+20(a7),d4
	move.l	5*4+24(a7),d5
	moveq	#0,d6
	move.w	vwk_mode(a0),d6
	move.l	d0,a0
	move.l	vwk_real_address(a0),a1
	move.l	wk_r_line(a1),d0
	move.l	d0,a1
	move.l	5*4+28(a7),d0
	addq.l	#1,a0
	jsr	(a1)
	movem.l	(a7)+,d2-d6
	rts	

* lib_v_bez - Standard Library function
* Todo: ?
* In:	a1	Parameters  lib_v_bez(num_pts, points, bezarr, extent, totpoints, totmoves)
*	a0	VDI struct
lib_v_bez:
  ifeq 0
	movem.l	d0-d2/a2,-(a7)
	move.l	a1,-(a7)
	move.l	a0,-(a7)
	jsr	_lib_v_bez
	move.l	(a7)+,a0
	move.l	(a7)+,a1
	movem.l	(a7)+,d0-d2/a2
	rts
  endc
; Removed old code


* lib_v_pline - Standard Library function
* Todo: ?
* In:	a1	Parameters  lib_v_pline(num_pts, points)
*	a0	VDI struct
_lib_v_pline:
	move.l	4(a7),a0
	move.l	8(a7),a1
lib_v_pline:
;	use_special_stack
;	move.w	#0,d0			; Background colour
;	swap	d0
	move.l	vwk_line_colour(a0),d0
c_v_pline:

	cmp.w	#1,vwk_line_width(a0)
	bhi	.wide_line

.no_wide:
	movem.l	d2-d6,-(a7)
	move.w	(a1)+,d6
	cmp.w	#2,d6
	blt	.end_v_pline	; .end		; No coordinates?  (-1 in Kandinsky)
	move.l	(a1),a2			; List of coordinates
  ifne 1
	bgt	.poly_line

	movem.w	(a2),d1-d4		; Optimized check for single lines outside clip rectangle (test 000618)
;	cmp.w	d1,d3
;	bne	.diff_x
	move.w	d2,d5			; Same x -> sort y
	move.w	d4,d6
	cmp.w	d5,d6
	bge	.ord_y
	exg	d5,d6
.ord_y:
	cmp.w	vwk_clip_rectangle_y2(a0),d5
	bgt	.end_v_pline
	cmp.w	vwk_clip_rectangle_y1(a0),d6
	blt	.end_v_pline
;	bra	.single_line

.diff_x:
;	cmp.w	d2,d4
;	bne	.diff_y
	move.w	d1,d5			; Same y -> sort x
	move.w	d3,d6
	cmp.w	d5,d6
	bge	.ord_x
	exg	d5,d6
.ord_x:
	cmp.w	vwk_clip_rectangle_x2(a0),d5
	bgt	.end_v_pline
	cmp.w	vwk_clip_rectangle_x1(a0),d6
	blt	.end_v_pline
.diff_y:

.single_line:
	move.w	vwk_line_user_mask(a0),d5
	move.w	vwk_line_type(a0),d6
	cmp.w	#7,d6
	beq	.userdef_s
	lea	_line_types,a1
	subq.w	#1,d6
	add.w	d6,d6
	move.w	0(a1,d6.w),d5
.userdef_s:

	move.l	vwk_real_address(a0),a1
	move.l	wk_r_line(a1),d6
	move.l	d6,a1
	moveq	#0,d6
	move.w	vwk_mode(a0),d6

	jsr	(a1)

	movem.l	(a7)+,d2-d6
	rts


.poly_line:
  endc
	move.w	vwk_line_user_mask(a0),d5
	move.w	vwk_line_type(a0),d1
	cmp.w	#7,d1
	beq	.userdef
	lea	_line_types,a1
	subq.w	#1,d1
	add.w	d1,d1
	move.w	0(a1,d1.w),d5
.userdef:

	move.l	vwk_real_address(a0),a1
	move.l	wk_r_line(a1),d1
	move.l	d1,a1

  ifne 0
	subq.w	#1,d6
	bra	.loop_end
.loop:
	movem.w	(a2),d1-d4
	bsr	clip_line
	bvs	.no_draw
	move.l	d6,-(a7)
	moveq	#0,d6
	move.w	vwk_mode(a0),d6
	jsr	(a1)
	move.l	(a7)+,d6
.no_draw:
	addq.l	#4,a2
.loop_end:
	dbra	d6,.loop

;	bra	.end_v_pline
  else
	move.l	a2,d1
	move.w	d6,d2
	swap	d2
	clr.w	d2
	moveq	#0,d6
	move.w	vwk_mode(a0),d6
	addq.l	#1,a0
	jsr	(a1)
  endc

.end_v_pline:		; .end
	movem.l	(a7)+,d2-d6
	rts


.wide_line:
	move.l	d0,d1
	clr.l	-(a7)
	bsr	allocate_block
	addq.l	#4,a7
	tst.l	d0
	beq	.no_wide

	move.l	d2,-(a7)

	moveq	#0,d2
	move.w	vwk_mode(a0),d2
	move.l	d2,-(a7)

	move.l	d0,-(a7)	; For free_block below (and _wide_line call)
	move.l	d1,-(a7)
	moveq	#0,d0
	move.w	0(a1),d0
	move.l	d0,-(a7)
	move.l	2(a1),-(a7)
	move.l	a0,-(a7)
	jsr	_wide_line
	add.w	#16,a7

	bsr	free_block
	addq.l	#8,a7

	move.l	(a7)+,d2
	rts


	dc.b	0,"default_line",0
* _default_line - Pixel by pixel line routine
* In:	a0	VDI struct (odd address marks table operation)
*	d0	Colour
*	d1	x1 or table address
*	d2	y1 or table length (high) and type (0 - coordinate pairs, 1 - pairs+moves)
*	d3	x2 or move point count
*	d4	y2 or move index address
*	d5.w	Pattern
*	d6	Logic operation
* Call:	a0	VDI struct, 0 (destination MFDB)
*	d1-d2.w	Coordinates
*	a3-a4	Set/get pixel
_default_line:
	movem.l	d6-d7/a1/a3-a4,-(a7)

	move.w	a0,d7
	and.w	#1,d7
	sub.w	d7,a0

	move.l	vwk_real_address(a0),a1
	move.l	wk_r_get_colour(a1),a1	; Index to real colour
	jsr	(a1)

	clr.l	-(a7)			; No MFDB => draw on screen
	move.l	a0,-(a7)

	move.w	d6,-(a7)
	bsr	setup_plot		; Setup pixel plot functions (a1/a3/a4)
	addq.l	#2,a7

	tst.w	d7
	bne	.multiline

	bsr	clip_line
	bvs	.skip_draw

	move.l	a7,a0			; a0 no longer -> VDI struct!

	bsr	.draw
.skip_draw:

	move.l	(a7),a0
	addq.l	#8,a7

	movem.l	(a7)+,d6-d7/a1/a3-a4
	rts

.draw:
	move.l	#$00010001,d7		; d7 = y-step, x-step

	sub.w	d1,d3			; d3 = dx
	bge	.ok1
	neg.w	d3
	neg.w	d7	
.ok1:
	sub.w	d2,d4			; d4 = dy
	bge	.ok2
	neg.w	d4
	swap	d7
	neg.w	d7
	swap	d7
.ok2:
	and.l	#$ffff,d5
	cmp.w	d3,d4
	bls	.xmajor
	or.l	#$80000000,d5
	exg	d3,d4
.xmajor:
	add.w	d4,d4			; d4 = incrE = 2dy
	move.w	d4,d6
	sub.w	d3,d6			; d6 = lines, d = 2dy - dx
	swap	d4
	move.w	d6,d4
	sub.w	d3,d4			; d4 = incrE, incrNE = 2(dy - dx)

	rol.w	#1,d5
	jsr	(a1)

	swap	d1
	move.w	d2,d1
	swap	d1			; d1 = y, x
	bra	.loop_end1

.loop1:
	tst.w	d6
	bgt	.both
	swap	d4
	add.w	d4,d6
	swap	d4
	tst.l	d5
	bmi	.ymajor
	add.w	d7,d1
	bra	.plot
.ymajor:
	swap	d7
	swap	d1
	add.w	d7,d1
	swap	d7
	swap	d1
	bra	.plot
.both:
	add.w	d4,d6
;	add.l	d7,d1
	add.w	d7,d1
	swap	d7
	swap	d1
	add.w	d7,d1
	swap	d7
	swap	d1
.plot:
	move.l	d1,d2
	swap	d2
	rol.w	#1,d5
	jsr	(a1)

.loop_end1:
	dbra	d3,.loop1
	rts

.multiline:				; Transform multiline to single ones
	cmp.w	#1,d2
	bhi	.line_done		; Only coordinate pairs and pairs+marks available so far
	beq	.use_marks
	moveq	#0,d3			; Move count
.use_marks:
	swap	d3
	move.w	#1,d3			; Current index in high word
	swap	d3
	movem.l	d0/d2/d3/d5/a0/a5-a6,-(a7)
	move.l	d1,a5			; Table address
	move.l	d4,a6			; Move index address
	tst.w	d3			;  may not be set
	beq	.no_start_move
	add.w	d3,a6
	add.w	d3,a6
	subq.l	#2,a6
	cmp.w	#-4,(a6)
	bne	.no_start_movex
	subq.l	#2,a6
	sub.w	#1,d3
.no_start_movex:
	cmp.w	#-2,(a6)
	bne	.no_start_move
	subq.l	#2,a6
	sub.w	#1,d3
.no_start_move:
	bra	.line_loop_end
.line_loop:
	movem.w	(a5),d1-d4
	move.l	7*4(a7),a0
	bsr	clip_line
	bvs	.no_draw
	move.l	0(a7),d6		; Colour
	move.l	3*4(a7),d5		; Pattern
;	move.l	xxx(a7),d0		; Logic operation
	lea	7*4(a7),a0
	bsr	.draw
.no_draw:
	move.l	2*4(a7),d3
	tst.w	d3
	beq	.no_marks
	swap	d3
	addq.w	#1,d3
	move.w	d3,d4
	add.w	d4,d4
	subq.w	#4,d4
	cmp.w	(a6),d4
	bne	.no_move
	subq.l	#2,a6
	addq.w	#1,d3
	swap	d3
	subq.w	#1,d3
	swap	d3
	addq.l	#4,a5
	subq.w	#1,1*4(a7)
.no_move:
	swap	d3
	move.l	d3,2*4(a7)
.no_marks:
	addq.l	#4,a5
.line_loop_end:
	subq.w	#1,1*4(a7)
	bgt	.line_loop
	movem.l	(a7)+,d0/d2/d3/d5/a0/a5-a6
.line_done:
	move.l	(a7),a0
	addq.l	#8,a7

	movem.l	(a7)+,d6-d7/a1/a3-a4
	rts


*
* Various
*

	dc.b	0,"v_circle",0
* v_circle - Standard Trap function
* Todo: -
* In:   a1      Parameter block
*       a0      VDI struct
v_circle:
;	use_special_stack
	move.l	ptsin(a1),a2
	move.w	8(a2),4(a2)
	move.w	8(a2),6(a2)
	bra	v_ellipse


	dc.b	0,0,"v_arc",0
* v_arc - Standard Trap function
* Todo: -
* In:   a1      Parameter block
*       a0      VDI struct
v_arc:
;	use_special_stack
	move.l	ptsin(a1),a2
	move.w	12(a2),4(a2)
	move.w	12(a2),6(a2)
	bra	v_ellarc


	dc.b	0,0,"v_pie",0
* v_pie - Standard Trap function
* Todo: -
* In:   a1      Parameter block
*       a0      VDI struct
v_pie:
;	use_special_stack
	move.l	ptsin(a1),a2
	move.w	12(a2),4(a2)
	move.w	12(a2),6(a2)
	bra	v_ellpie


* calc_nsteps
* In:	d3	x-radius
*	d4	y-radius
* Out:	d0	Number of steps
calc_nsteps:
	moveq	#0,d0
	move.w	d3,d0
	cmp.w	d3,d4
	bls	.x_large
	move.w	d4,d0
.x_large:
;	lsr.w	#2,d0
;	cmp.w	#16,d0
	mulu	_arc_split,d0
	clr.w	d0
	swap	d0
	cmp.w	_arc_min,d0
	bcc	.not_small
;	moveq	#16,d0
	move.w	_arc_min,d0
.not_small:
;	cmp.w	#max_arc_count,d0
	cmp.w	_arc_max,d0
	bls	.not_large
;	move.w	#max_arc_count,d0
	move.w	_arc_max,d0
.not_large:
	rts

* col_pat
* In:	a0	VDI struct
* Out:	d0	Colours
*	d5	Pattern
*	a2	 - " -
col_pat:
;	move.w	#0,d0			; Background colour
;	swap	d0
	move.l	vwk_fill_colour(a0),d0

	move.w	vwk_fill_interior(a0),d5

	tst.w	d5
	bne	.solid
	swap	d0			; Hollow, so background colour
.solid:

	cmp.w	#4,d5
	bne	.no_user
	move.l	vwk_fill_user_pattern_in_use(a0),a2
	bra	.got_pattern
.no_user:
	add.w	d5,d5
	add.w	d5,d5
	lea	_pattern_ptrs,a2
	move.l	0(a2,d5.w),a2
	and.w	#$08,d5			; Check former bit 1 (interior 2 or 3)
	beq	.got_pattern
	move.w	vwk_fill_style(a0),d5
	subq.w	#1,d5
	lsl.w	#5,d5			; Add style index
	add.w	d5,a2
.got_pattern:
	move.l	a2,d5

	rts


	dc.b	0,"v_ellarc",0
* v_ellarc - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_ellarc:
	uses_d1

	movem.l	d2-d7,-(a7)
	move.l	ptsin(a1),a2
	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3
	moveq	#0,d4
	movem.w	0(a2),d1-d4
;	tst.w	d3
;	beq	.ellarc_end

	clr.l	-(a7)
	bsr	allocate_block
	addq.l	#4,a7
	tst.l	d0
	beq	.ellarc_end

	clr.l	-(a7)		; Dummy interior/style since not filled
	moveq	#0,d5
	move.w	vwk_mode(a0),d5
	move.l	d5,-(a7)

	move.l	d0,-(a7)	; For free_block below (and _clc_arc call)

;	bsr	col_pat
;	move.l	d5,-(a7)
;	move.l	d0,-(a7)
	clr.l	-(a7)		; No pattern!
	move.l	vwk_line_colour(a0),d0
	move.l	d0,-(a7)

	moveq	#0,d5
	moveq	#0,d6
	move.l	intin(a1),a2
	movem.w	0(a2),d5-d6
	move.l	d6,d7
	sub.w	d5,d7
	bpl	.not_negative
	add.w	#3600,d7
.not_negative:

;	if (xfm_mode < 2)	/* If xform != raster then flip */
;		yrad = yres - yrad;

	bsr	calc_nsteps

	mulu	d7,d0
	divu	#3600,d0
	ext.l	d0
	beq	.ellarc_finish

	move.l	d0,-(a7)	; n_steps
	move.l	d7,-(a7)
	move.l	d6,-(a7)
	move.l	d5,-(a7)
	move.l	d4,-(a7)
	move.l	d3,-(a7)
	move.l	d2,-(a7)
	move.l	d1,-(a7)
	move.l	#6,-(a7)	; ellarc
	move.l	a0,-(a7)
	jsr	_clc_arc	; vwk, gdb_code, xc, yc, xrad, yrad, beg_ang, end_ang, del_ang, n_steps, colour, pattern, points, mode, interior_style

	add.w	#10*4,a7

.ellarc_finish:
	addq.l	#2*4,a7

	bsr	free_block
	add.w	#3*4,a7

.ellarc_end:
	movem.l	(a7)+,d2-d7
	used_d1
	done_return


	dc.b	0,"v_ellpie",0
* v_ellpie - Standard Trap function
* Todo: -
* In:   a1      Parameter block
*       a0      VDI struct
v_ellpie:
	uses_d1

	movem.l	d2-d7,-(a7)
	move.l	ptsin(a1),a2
	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3
	moveq	#0,d4
	movem.w	0(a2),d1-d4
;	tst.w	d3
;	beq	.ellpie_end

	clr.l	-(a7)
	bsr	allocate_block
	addq.l	#4,a7
	tst.l	d0
	beq	.ellpie_end

	move.w	vwk_fill_interior(a0),d5
	swap	d5
	move.w	vwk_fill_style(a0),d5
	move.l	d5,-(a7)
	moveq	#0,d5
	move.w	vwk_mode(a0),d5
	move.l	d5,-(a7)

	move.l	d0,-(a7)	; For free_block below (and _clc_arc call)

	bsr	col_pat
	move.l	d5,-(a7)
	move.l	d0,-(a7)

	moveq	#0,d5
	moveq	#0,d6
	move.l	intin(a1),a2
	movem.w	0(a2),d5-d6
	move.l	d6,d7
	sub.w	d5,d7
	lbpl	.not_negative,1
	add.w	#3600,d7
 label .not_negative,1

;	if (xfm_mode < 2)	/* If xform != raster then flip */
;		yrad = yres - yrad;

	bsr	calc_nsteps

	mulu	d7,d0
	divu	#3600,d0
	ext.l	d0
	beq	.ellpie_finish

	move.l	d0,-(a7)	; n_steps
	move.l	d7,-(a7)
	move.l	d6,-(a7)
	move.l	d5,-(a7)
	move.l	d4,-(a7)
	move.l	d3,-(a7)
	move.l	d2,-(a7)
	move.l	d1,-(a7)
	move.l	#7,-(a7)	; ellpie
	move.l	a0,-(a7)
	jsr	_clc_arc	; vwk, gdb_code, xc, yc, xrad, yrad, beg_ang, end_ang, del_ang, n_steps, colour, pattern, points, mode, interior_style

	add.w	#10*4,a7

.ellpie_finish:
	addq.l	#2*4,a7

	bsr	free_block
	add.w	#3*4,a7

.ellpie_end:
	movem.l	(a7)+,d2-d7
	used_d1
	done_return


	dc.b	0,0,"v_ellipse",0
* v_ellipse - Standard Trap function
* Todo: -
* In:   a1      Parameter block
*       a0      VDI struct
v_ellipse:
;	use_special_stack
  ifne 1
	uses_d1

	movem.l	d2-d5,-(a7)
	move.l	ptsin(a1),a2

	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3
	moveq	#0,d4
	movem.w	0(a2),d1-d4
;	tst.w	d3				; Not sure this should be here
;	beq	.ellipse_end

	clr.l	-(a7)
	bsr	allocate_block
	addq.l	#4,a7
	tst.l	d0
	beq	.ellipse_end

	move.w	vwk_fill_interior(a0),d5
	swap	d5
	move.w	vwk_fill_style(a0),d5
	move.l	d5,-(a7)
	moveq	#0,d5
	move.w	vwk_mode(a0),d5
	move.l	d5,-(a7)

	move.l	d0,-(a7)	; For free_block below (and _clc_arc call)

	bsr	col_pat
	move.l	d5,-(a7)
	move.l	d0,-(a7)

  ifne 0
	...
;	if (xfm_mode < 2) /* if xform != raster then flip */
;		yrad = yres - yrad;	
  endc

	bsr	calc_nsteps
	move.l	d0,-(a7)	; n_steps
	move.l	#3600,-(a7)
	move.l	#0,-(a7)
	move.l	#0,-(a7)
	move.l	d4,-(a7)
	move.l	d3,-(a7)
	move.l	d2,-(a7)
	move.l	d1,-(a7)
	move.l	#5,-(a7)	; ellipse
	move.l	a0,-(a7)
	jsr	_clc_arc	; vwk, gdb_code, xc, yc, xrad, yrad, 0, 0, 3600, n_steps, colour, pattern, points, mode, interior_style

	add.w	#12*4,a7

	bsr	free_block
	add.w	#3*4,a7

.ellipse_end:
	movem.l	(a7)+,d2-d5
	used_d1
	done_return
  endc
; Removed old code


	dc.b	0,"v_rbox",0
* v_rbox - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_rbox:
	uses_d1
	subq.l	#4,a7
	move.l	ptsin(a1),0(a7)		; Edges

	move.l	a7,a1
	bsr	lib_v_rbox
	addq.l	#4,a7
	used_d1
	done_return			; Should be real_return

* lib_v_rbox - Standard library function
* Todo: ?
* In:	a1	Parameters  lib_v_rbox(points)
*	a0	VDI struct
lib_v_rbox:
	movem.l	d2-d5,-(a7)

	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3
	moveq	#0,d4
	move.l	0(a1),a1
	movem.w	0(a1),d1-d4

	cmp.w	d1,d3
	bge	.rb_xorder
	exg	d1,d3
.rb_xorder:
	cmp.w	d2,d4
	bge	.rb_yorder
	exg	d2,d4
.rb_yorder:

	clr.l	-(a7)
	bsr	allocate_block
	addq.l	#4,a7
	tst.l	d0
	beq	.rbox_end

	move.w	vwk_fill_interior(a0),d5
	swap	d5
	move.w	vwk_fill_style(a0),d5
	move.l	d5,-(a7)
	moveq	#0,d5
	move.w	vwk_mode(a0),d5
	move.l	d5,-(a7)

	move.l	d0,-(a7)	; For free_block below (and _clc_arc call)

	bsr	col_pat
	move.l	d5,-(a7)
	move.l	d0,-(a7)
	move.l	d4,-(a7)
	move.l	d3,-(a7)
	move.l	d2,-(a7)
	move.l	d1,-(a7)
	move.l	#8,-(a7)	; rbox
	move.l	a0,-(a7)
	jsr	_rounded_box	; vwk, gdb_code, x1, y1, x2, y2, colour, pattern, points, mode, interior_style

	add.w	#8*4,a7

	bsr	free_block
	add.w	#3*4,a7

.rbox_end:
	movem.l	(a7)+,d2-d5
	rts


	dc.b	0,0,"v_rfbox",0
* v_rfbox - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_rfbox:
	uses_d1
	subq.l	#4,a7
	move.l	ptsin(a1),0(a7)		; Edges

	move.l	a7,a1
	bsr	lib_v_rfbox
	addq.l	#4,a7
	used_d1
	done_return			; Should be real_return

* lib_v_rfbox - Standard library function
* Todo: ?
* In:	a1	Parameters  lib_v_rfbox(points)
*	a0	VDI struct
lib_v_rfbox:
	movem.l	d2-d5,-(a7)

	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3
	moveq	#0,d4
	move.l	0(a1),a1
	movem.w	0(a1),d1-d4

	cmp.w	d1,d3
	bge	.rfb_xorder
	exg	d1,d3
.rfb_xorder:
	cmp.w	d2,d4
	bge	.rfb_yorder
	exg	d2,d4
.rfb_yorder:

	clr.l	-(a7)
	bsr	allocate_block
	addq.l	#4,a7
	tst.l	d0
	beq	.rfbox_end

	move.w	vwk_fill_interior(a0),d5
	swap	d5
	move.w	vwk_fill_style(a0),d5
	move.l	d5,-(a7)
	moveq	#0,d5
	move.w	vwk_mode(a0),d5
	move.l	d5,-(a7)

	move.l	d0,-(a7)	; For free_block below (and _clc_arc call)

	bsr	col_pat
	move.l	d5,-(a7)
	move.l	d0,-(a7)
	move.l	d4,-(a7)
	move.l	d3,-(a7)
	move.l	d2,-(a7)
	move.l	d1,-(a7)
	move.l	#9,-(a7)	; rbox
	move.l	a0,-(a7)
	jsr	_rounded_box	; vwk, gdb_code, x1, y1, x2, y2, colour, pattern, points, mode, interior_style

	add.w	#8*4,a7

	bsr	free_block
	add.w	#3*4,a7

.rfbox_end:
	movem.l	(a7)+,d2-d5
	rts


	dc.b	0,0,"v_pmarker",0
* v_pmarker - Standard Trap function
* Todo: All the other types, multiple markers
* In:   a1      Parameter block
*       a0      VDI struct
v_pmarker:
;	use_special_stack
;	move.w	#0,d0			; Background colour
;	swap	d0
	move.l	vwk_marker_colour(a0),d0

	uses_d1
	movem.l	d2-d6,-(a7)
	move.l	control(a1),a2
	move.w	2(a2),d6
	beq	.end_v_pmarker	; .end		; No coordinates?
	move.l	ptsin(a1),a2		; List of coordinates

;	move.w	vwk_line_user_mask(a0),d5
;	move.w	vwk_line_type(a0),d1
;	cmp.w	#7,d1
;	beq	.userdef
;	lea	_line_types,a1
;	subq.w	#1,d1
;	add.w	d1,d1
;	move.w	0(a1,d1.w),d5
;.userdef:
	move.w	#$ffff,d5

	move.l	vwk_real_address(a0),a1
	move.l	wk_r_line(a1),d1

	move.l	d1,a1

;	subq.w	#1,d6
;	bra	.loop_end
;.loop:
;	movem.w	(a2),d1-d4
;	bsr	clip_line
;	bvs	.no_draw
;	move.l	d6,-(a7)
;	moveq	#0,d6
;	move.w	vwk_mode(a0),d6
;	jsr	(a1)
;	move.l	(a7)+,d6
;.no_draw:
;	addq.l	#4,a2
;.loop_end:
;	dbra	d6,.loop
;
	movem.w	(a2),d1-d2	; Only a single dot for now
	move.w	d1,d3
	move.w	d2,d4
	moveq	#0,d6
	move.w	vwk_mode(a0),d6
	jsr	(a1)

.end_v_pmarker:		; .end
	movem.l	(a7)+,d2-d6
	used_d1
	done_return			; Should be real_return


	dc.b	0,"v_fillarea",0
* v_fillarea - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_fillarea:
;	use_special_stack
	uses_d1
	move.l	control(a1),a2

	move.w	L_intin(a2),d1
	lbeq	.normal,1
	tst.w	vwk_bezier_on(a0)
	bne	v_bez_fill
	cmp.w	#13,subfunction(a2)
	beq	v_bez_fill
 label .normal,1

	subq.l	#6,a7
	move.w	L_ptsin(a2),0(a7)
	move.l	ptsin(a1),2(a7)		; List of coordinates

	move.l	a7,a1
	bsr	lib_v_fillarea
	addq.l	#6,a7
	used_d1
	done_return			; Should be real_return


v_bez_fill:
	sub.w	#22,a7
	move.w	L_ptsin(a2),0(a7)
	move.l	ptsin(a1),2(a7)
	move.l	intin(a1),6(a7)
	move.l	ptsout(a1),a2
	move.l	a2,10(a7)
	move.l	intout(a1),a2
	move.l	a2,14(a7)
	addq.l	#2,a2
	move.l	a2,18(a7)

	move.l	control(a1),a2
	move.w	#2,L_ptsout(a2)
	move.w	#6,L_intout(a2)

	move.l	a7,a1
	bsr	lib_v_bez_fill
	add.w	#22,a7
	used_d1
	done_return


* In:	d0	Number of points
*	a0	Points
* Out:	d0	ymin/xmin
*	d1	ymax/xmax
bezier_size:
	movem.l	d2/d4,-(a7)
	move.l	#$7fff7fff,d2
	move.l	#$80008000,d4
	bra	.minmax_end
.minmax_loop:
	swap	d0
	move.w	#1,d0
.minmax_inner:
	move.w	(a0)+,d1
	cmp.w	d1,d2
	blt	.not_min
	move.w	d1,d2
.not_min:
	cmp.w	d1,d4
	bgt	.not_max
	move.w	d1,d4
.not_max:
	swap	d2
	swap	d4
	dbra	d0,.minmax_inner
	swap	d0
.minmax_end:
	dbra	d0,.minmax_loop
	move.l	d2,d0
	move.l	d4,d1
	movem.l	(a7)+,d2/d4
	rts


* lib_v_bez_fill - Standard Library function
* Todo: ?
* In:	a1	Parameters  lib_v_bez_fill(num_pts, points, bezarr, extent, totpoints, totmoves)
*	a0	VDI struct
lib_v_bez_fill:
	sub.w	#10,a7
	move.l	a7,a2
	movem.l	a0-a1,-(a7)
	move.l	a2,-(a7)
	move.l	18(a1),-(a7)
	pea	2(a2)
	move.l	a0,d0
	add.w	#vwk_clip_rectangle,d0
	move.l	d0,2(a2)
	pea	6(a2)
	moveq	#0,d0
	move.w	0(a1),d0
	move.l	d0,-(a7)	; marks = num_pts   ?
	move.l	d0,-(a7)
	move.w	vwk_bezier_depth_scale(a0),d0
;	move.w	#0,d0
	or.w	#$100,d0	; Close loops
	move.l	d0,-(a7)
	move.l	2(a1),-(a7)
	move.l	6(a1),-(a7)
	bra	.bezf_loop_end
.bezf_loop:
	jsr	_calc_bez	; (ch *marks, sh *points, sh flags, sh maxpnt, sh maxin, sh **xmov, sh **xpts, sh *pnt_mv_cnt, sh *x_used)
	tst.l	d0
	bge	.done_f
	tst.w	9*4+2*4(a7)		; xused?
	beq	.normal_fill
	addq.w	#1,8+2(a7)
.bezf_loop_end:
	move.l	9*4(a7),a0
	move.l	a0,d0			; Restore clip rectangle pointer
	add.w	#vwk_clip_rectangle,d0
	move.l	24(a7),a2
	move.l	d0,(a2)
	move.l	vwk_real_address(a0),a2
	move.w	8+2(a7),d0
	and.w	#$ff,d0		; Mask off loop flag
	cmp.w	wk_drawing_bezier_depth_scale_min(a2),d0
;	cmp.w	#9,8+2(a7)
	ble	.bezf_loop

	add.w	#9*4,a7		; Should we ever get here?
	movem.l	(a7),a0-a1
	moveq	#0,d0
	move.w	d0,0(a7)	; No allocated memory etc
	move.l	d0,4(a7)
	move.l	d0,8(a7)
	move.w	0(a1),d0
	lea	2(a1),a0
	bra	.finish_up_f

.done_f:
	add.w	#9*4,a7
;	move.l	0(a7),a0
	movem.l	0(a7),a0-a1
	subq.l	#6,a7
	move.w	d0,0(a7)

	movem.l	d2-d7,-(a7)
	move.w	d0,d6
	move.l	6*4+6+2*4+2(a7),a2		; Points
;	move.w	6*4+6+2*4+0(a7),d3		; Move point count
	move.l	18(a1),a1
	moveq	#0,d3
	move.w	(a1),d3
	move.l	6*4+6+2*4+6(a7),d4		; Move indices

	tst.w	d6
	ble	.no_poly		; No coordinates?  (-1 in Kandinsky)

	move.l	vwk_real_address(a0),a1
	move.l	wk_r_fillpoly(a1),d0
	beq	.no_accel_poly
	move.l	d0,a1
	move.l	a2,d1
	move.w	d6,d2
	exg	d3,d4
	bsr	col_pat		; d0 - colours, d5 - pattern

	move.w	vwk_fill_interior(a0),d7
	swap	d7
	move.w	vwk_fill_style(a0),d7
	move.l	d6,-(a7)
	moveq	#0,d6
	move.w	vwk_mode(a0),d6

	jsr	(a1)

	move.l	(a7)+,d6

	move.l	d1,a2
	bra	.no_poly

.no_accel_poly:
	move.l	#0,-(a7)	; Get a memory block of any size (hopefully large)
	bsr	allocate_block
	addq.l	#4,a7
	tst.l	d0
	beq	.no_poly

	tst.w	d3
	beq	.no_jumps

	move.w	vwk_fill_interior(a0),d7
	swap	d7
	move.w	vwk_fill_style(a0),d7
	move.l	d7,-(a7)
	moveq	#0,d7
	move.w	vwk_mode(a0),d7
	move.l	d7,-(a7)

	move.l	d3,-(a7)
	move.l	d4,-(a7)

	move.l	d0,-(a7)

	move.l	a2,-(a7)
	bsr	col_pat
	move.l	(a7)+,a2
	move.l	d5,-(a7)	; Pattern
	move.l	d0,-(a7)	; Colours

	ext.l	d6
	move.l	d6,-(a7)
	move.l	a2,-(a7)
	move.l	a0,-(a7)
	jsr	_filled_poly_m
	add.w	#20,a7

	bsr	free_block
	addq.l	#4,a7
	add.w	#16,a7
.no_poly:		; .end

	bra	.end_bez_draw_f		; Should check for outline

	move.l	vwk_line_colour(a0),d0
	cmp.w	#1,vwk_line_width(a0)
	bhi	.wide_bez_f			; Do wide lines too!!!

.no_wide_bez_f:
	move.w	vwk_line_user_mask(a0),d5
	move.w	vwk_line_type(a0),d1
	cmp.w	#7,d1
	beq	.bez_userdef_f
	lea	_line_types,a1
	subq.w	#1,d1
	add.w	d1,d1
	move.w	0(a1,d1.w),d5
.bez_userdef_f:

	move.l	vwk_real_address(a0),a1
	move.l	wk_r_line(a1),d1
	move.l	d1,a1

	addq.l	#1,a0
	move.l	a2,d1
	move.w	d6,d2
	swap	d2
	move.w	#1,d2			; Should be 1 for move handling
	move.w	#0,d6
	move.w	vwk_mode(a0),d6
	jsr	(a1)

.end_bez_draw_f:	; .end
	movem.l	(a7)+,d2-d7

	move.w	0(a7),d0
	addq.l	#6,a7
	move.l	2*4+2(a7),a0

.finish_up_f:
	move.l	d0,a2
	bsr	bezier_size
	movem.l	(a7)+,a0-a1
	move.l	10(a1),a0
	move.l	d0,(a0)+
	move.l	d1,(a0)
	move.l	14(a1),a0
	move.w	a2,(a0)
	move.l	18(a1),a0
	move.w	0(a7),(a0)
	move.l	2(a7),d0
	beq	.no_free_f
	move.l	d0,-(a7)
	bsr	free_block
	addq.l	#4,a7
.no_free_f:
	add.w	#10,a7
	rts

.no_jumps:
	move.w	vwk_fill_interior(a0),d7
	swap	d7
	move.w	vwk_fill_style(a0),d7
	move.l	d7,-(a7)
	moveq	#0,d7
	move.w	vwk_mode(a0),d7
	move.l	d7,-(a7)

	move.l	d0,-(a7)

	move.l	a2,-(a7)
	bsr	col_pat
	move.l	(a7)+,a2
	move.l	d5,-(a7)	; Pattern
	move.l	d0,-(a7)	; Colours

	ext.l	d6
	move.l	d6,-(a7)
	move.l	a2,-(a7)
	move.l	a0,-(a7)
	jsr	_filled_poly
	add.w	#28,a7

	bsr	free_block
	addq.l	#4,a7
	bra	.no_poly
		
.normal_fill:
	add.w	#9*4,a7
	movem.l	(a7),a0-a1
	bsr	lib_v_fillarea
	movem.l	(a7),a0-a1
	moveq	#0,d0
	move.w	d0,2*4+0(a7)	; No allocated memory etc
	move.l	d0,2*4+2(a7)
	move.l	d0,2*4+6(a7)
	move.w	0(a1),d0
	move.l	2(a1),a0
	bra	.finish_up_f

.wide_bez_f:
	move.l	d0,d1
	clr.l	-(a7)
	bsr	allocate_block
	addq.l	#4,a7
	tst.l	d0
	beq	.no_wide_bez_f

	moveq	#0,d2
	move.w	vwk_mode(a0),d2
	move.l	d2,-(a7)

	move.l	d0,-(a7)	; For free_block below (and _wide_line call)
	move.l	d1,-(a7)
	ext.l	d6
	move.l	d6,-(a7)
	move.l	a2,-(a7)
	move.l	a0,-(a7)
	jsr	_wide_line
	add.w	#16,a7

	bsr	free_block
	addq.l	#8,a7

	bra	.end_bez_draw_f


* lib_v_fillarea - Standard Library function
* Todo: ?
* In:	a1	Parameters  lib_v_fillarea(num_pts, points)
*	a0	VDI struct
lib_v_fillarea:
	movem.l	d2-d7,-(a7)
	move.w	(a1)+,d6
	ble	.end_lib_v_fillarea	; .end		; No coordinates?  (-1 in Kandinsky)

	move.l	vwk_real_address(a0),a2
	move.l	wk_r_fillpoly(a2),d0
	lbeq	.no_accel_poly,1
	move.l	(a1)+,d1
	move.l	d0,a1
	move.w	d6,d2
	moveq	#0,d3
	moveq	#0,d4
	bsr	col_pat		; d0 - colours, d5 - pattern

	move.w	vwk_fill_interior(a0),d7
	swap	d7
	move.w	vwk_fill_style(a0),d7
	moveq	#0,d6
	move.w	vwk_mode(a0),d6

	jsr	(a1)
	bra	.end_lib_v_fillarea

 label .no_accel_poly,1
	move.l	#0,-(a7)	; Get a memory block of any size (hopefully large)
	bsr	allocate_block
	addq.l	#4,a7
	tst.l	d0
	beq	.end_lib_v_fillarea

	move.w	vwk_fill_interior(a0),d7
	swap	d7
	move.w	vwk_fill_style(a0),d7
	move.l	d7,-(a7)
	moveq	#0,d7
	move.w	vwk_mode(a0),d7
	move.l	d7,-(a7)

	move.l	d0,-(a7)

	bsr	col_pat
	move.l	d5,-(a7)	; Pattern
	move.l	d0,-(a7)	; Colours

	ext.l	d6
	move.l	d6,-(a7)
	move.l	(a1),-(a7)
	move.l	a0,-(a7)
	jsr	_filled_poly
	add.w	#28,a7

	bsr	free_block
	addq.l	#4,a7

.end_lib_v_fillarea:		; .end
	movem.l	(a7)+,d2-d7
	rts


	dc.b	0,0,"fill_poly",0
* fill_poly(Virtual *vwk, short *p, int n, int colour, short *pattern, short *points, long mode, long interior_style);
*
_fill_poly:
	move.l	12(a7),d1
	ble	.end_fill_poly	; .end		; No coordinates?

	move.l	4(a7),a0
	move.l	vwk_real_address(a0),a1
	move.l	wk_r_fillpoly(a1),d0
	beq	.do_c_poly
	movem.l	d2-d7,-(a7)
	move.l	d1,d2
	move.l	6*4+8(a7),d1
	move.l	d0,a1
	moveq	#0,d3
	moveq	#0,d4
	move.l	6*4+16(a7),d0
	move.l	6*4+20(a7),d5
	move.l	6*4+28(a7),d6
	move.l	6*4+32(a7),d7
	jsr	(a1)
	movem.l	(a7)+,d2-d7

.end_fill_poly:		; .end
	rts

.do_c_poly:
	jmp	_filled_poly


	dc.b	0,0,"hline",0
* hline(Virtual *vwk, long x1, long y1, long y2, long colour, short *pattern, long mode, long interior_style)
*
_hline:
	movem.l	d2-d7/a2-a6,-(a7)

	move.l	11*4+4+0(a7),a0
	move.l	11*4+4+16(a7),d0
	move.l	11*4+4+4(a7),d1
	move.l	11*4+4+8(a7),d2
	move.l	11*4+4+12(a7),d3
	move.l	d2,d4

	bsr	clip_rect
	blt	.end			; Empty rectangle?

	move.l	vwk_real_address(a0),a2
	move.l	wk_r_fill(a2),a1

	move.l	11*4+4+20(a7),d5

	move.l	11*4+4+24(a7),d6
	move.l	11*4+4+28(a7),d7

	jsr	(a1)

.end:
	movem.l	(a7)+,d2-d7/a2-a6
	rts


	dc.b	0,"fill_spans",0
* fill_spans(Virtual *vwk, short *spans, long n, long colour, short *pattern, long mode, long interior_style)
*
_fill_spans:
	movem.l	d2-d7/a2-a6,-(a7)

	move.l	11*4+4+0(a7),a0
	move.l	11*4+4+12(a7),d0
	move.l	11*4+4+4(a7),d1
	move.l	11*4+4+8(a7),d2
	swap	d2
	clr.w	d2
	moveq	#0,d3
	moveq	#0,d4

	move.l	vwk_real_address(a0),a2
	addq.l	#1,a0
	move.l	wk_r_fill(a2),a1

	move.l	11*4+4+16(a7),d5

	move.l	11*4+4+20(a7),d6
	move.l	11*4+4+24(a7),d7

	jsr	(a1)

	movem.l	(a7)+,d2-d7/a2-a6
	rts

	end
