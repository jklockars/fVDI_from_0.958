*****
* fVDI text set/query functions
*
* $Id: text_sq.s,v 1.3 2002-07-01 22:24:40 johan Exp $
*
* Copyright 1997-2002, Johan Klockars 
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

transparent	equ	1		; Fall through?

SUB1		equ	0		; Subtract 1 from text width? (NVDI apparently doesn't)

	include	"vdi.inc"

*
* Macros
*
  ifne lattice
	include	"macros.dev"
  else
	include	"macros.tas"
  endc

	xdef	vst_color,vst_effects,vst_alignment,vst_rotation,vst_font
	xdef	vqt_name,vqt_font_info,vst_point,vst_height,vqt_attributes,vqt_extent
	xdef	vst_load_fonts,vst_unload_fonts,vqt_width
	xdef	vqt_f_extent,vqt_xfntinfo

	xdef	lib_vst_color,lib_vst_effects,lib_vst_alignment,lib_vst_rotation,lib_vst_font
	xdef	lib_vqt_name,lib_vqt_font_info,lib_vst_point,lib_vst_height,lib_vqt_attributes,lib_vqt_extent
	xdef	lib_vst_load_fonts,lib_vst_unload_fonts,lib_vqt_width,lib_vqt_xfntinfo
	xdef	_lib_vst_color,_lib_vst_font,_lib_vst_point

	xdef	_get_extent


	text

	dc.b	0,0,"vst_color",0
* vst_color - Standard Trap function
* Todo: -
* In:   a1      Parameter block
*       a0      VDI struct
vst_color:
	move.l	intin(a1),a2
	move.w	(a2),d0
	move.l	vwk_real_address(a0),a2
	cmp.w	wk_screen_palette_size(a2),d0
	blo	.ok
	moveq	#BLACK,d0
.ok:
	move.w	d0,vwk_text_colour_foreground(a0)
	move.l	intout(a1),a2
	move.w	d0,(a2)
	done_return

* lib_vst_color - Standard Library function
* Todo: -
* In:	a1	Parameters   colour_set = lib_vst_color(colour)
*	a0	VDI struct
_lib_vst_color:
lib_vst_color:
	move.w	(a1),d0
	move.l	vwk_real_address(a0),a2
	cmp.w	wk_screen_palette_size(a2),d0
	lblo	.ok,1
	moveq	#BLACK,d0
 label .ok,1
	move.w	d0,vwk_text_colour_foreground(a0)
	rts


	dc.b	0,0,"vst_effects",0
* vst_effects - Standard Trap function
* Todo: -
* In:   a1      Parameter block
*       a0      VDI struct
vst_effects:
	move.l	intin(a1),a2
	move.w	(a2),d0
	move.l	vwk_real_address(a0),a2
	and.w	wk_writing_effects(a2),d0
	move.w	d0,vwk_text_effects(a0)
	move.l	intout(a1),a2
	move.w	d0,(a2)
	done_return

* lib_vst_effects - Standard Library function
* Todo: -
* In:	a1	Parameters   effects_set = lib_vst_effects(effects)
*	a0	VDI struct
lib_vst_effects:
	move.w	(a1),d0
	move.l	vwk_real_address(a0),a2
	and.w	wk_writing_effects(a2),d0
	move.w	d0,vwk_text_effects(a0)
	rts


	dc.b	0,0,"vst_alignment",0
* vst_alignment - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vst_alignment:
	move.l	intin(a1),a2
	move.w	(a2)+,d0
	cmp.w	#2,d0		; # horizontal alignments (not from wk struct?)
	bls	.ok1
	moveq	#0,d0		; Left
.ok1:
	swap	d0
	move.w	(a2)+,d0
	cmp.w	#5,d0		; # vertical alignments (not from wk struct?)
	bls	.ok2
	move.w	#0,d0		; Baseline
.ok2:
	move.l	d0,vwk_text_alignment(a0)
	move.l	intout(a1),a2
	move.l	d0,(a2)
	done_return

* lib_vst_alignment - Standard Library function
* Todo: ?
* In:	a1	Parameters   lib_vst_alignment(halign, valign, &hresult, &vresult)
*	a0	VDI struct
lib_vst_alignment:
	move.w	(a1)+,d0
	cmp.w	#2,d0		; # horizontal alignments (not from wk struct?)
	lbls	.ok1,1
	moveq	#0,d0		; Left
 label .ok1,1
	swap	d0
	move.w	(a1)+,d0
	cmp.w	#5,d0		; # vertical alignments (not from wk struct?)
	lbls	.ok2,2
	move.w	#0,d0		; Baseline
 label .ok2,2
	move.l	d0,vwk_text_alignment(a0)
	move.l	(a1)+,a2
	swap	d0
	move.w	d0,(a2)
	move.l	(a1),a2
	swap	d0
	move.w	d0,(a2)
	rts


	dc.b	0,"vst_rotation",0
* vst_rotation - Standard Trap function
* Todo: Check if any angle is allowed.
* In:   a1      Parameter block
*       a0      VDI struct
vst_rotation:
	move.l	intin(a1),a2
	move.w	(a2),d0
	move.l	vwk_real_address(a0),a2
	tst.w	wk_writing_rotation_possible(a2)
	lbeq	.none,1
	cmp.w	#1,wk_writing_rotation_type(a2)
	lblo	.none,1
	bhi	.any
	add.w	#450,d0
	divu	#900,d0			; Only allow right angles
	cmp.w	#3,d0			; Should probably check font
	lbls	.ok,2
 label .none,1
	moveq	#0,d0
 label .ok,2
	mulu	#900,d0
.any:
	move.w	d0,vwk_text_rotation(a0)
	move.l	intout(a1),a2
	move.w	d0,(a2)
	done_return

* lib_vst_rotation - Standard Library function
* Todo: Check if any angle is allowed.
* In:	a1	Parameters   angle_set = lib_vst_rotation(angle)
*	a0	VDI struct
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


	dc.b	0,"vst_font",0
* vst_font - Standard Trap function
* Todo:	?
* In:   a1      Parameter block
*       a0      VDI struct
vst_font:
	uses_d1
	move.l	a1,-(a7)
	move.l	intin(a1),a1
	bsr	lib_vst_font
	move.l	(a7)+,a1
	move.l	intout(a1),a1
	move.w	d0,(a1)
	used_d1
	done_return

* lib_vst_font - Standard Library function
* Todo: Also look for correct size?
* In:	a1	Parameters   font_set = lib_vst_font(fontID)
*	a0	VDI struct
_lib_vst_font:
lib_vst_font:
	move.w	(a1),d0
	tst.w	d0
	lbne	.ok,1
	moveq	#1,d0
 label .ok,1
	move.l	vwk_text_current_font(a0),d1
	beq	.start
	move.l	d1,a2
	move.l	font_extra_first_size(a2),a2
	cmp.w	vwk_text_font(a0),d0
	beq	.same
	bhi	.search
.start:
	move.l	vwk_real_address(a0),a2
	move.l	wk_writing_first_font(a2),a2
.search:
	cmp.w	font_id(a2),d0
	bls	.maybe
	move.l	font_next(a2),a2
	move.l	a2,d1
	bne	.search
	bra	.not_found
.maybe:
	beq	.found
.not_found:
	moveq	#1,d0
	move.l	vwk_real_address(a0),a2
	move.l	wk_writing_first_font(a2),a2
.found:
	move.w	d0,vwk_text_font(a0)
	move.l	a2,vwk_text_current_font(a0)

	move.w	font_widest_character(a2),d1	; Setup character sizes in vwk
	swap	d1
	move.w	font_distance_top(a2),d1
	move.l	d1,vwk_text_character(a0)	; Character w/h in vwk
	swap	d1
	move.w	font_widest_cell(a2),d1
	swap	d1
	addq.w	#1,d1
	add.w	font_distance_bottom(a2),d1
	move.l	d1,vwk_text_cell(a0)		; Cell w/h in vwk

.same:
	rts


	dc.b	0,"vqt_name",0
* vqt_name - Standard Trap function
* Todo:	?
* In:   a1      Parameter block
*       a0      VDI struct
vqt_name:
	uses_d1
	move.l	intin(a1),a2
	move.w	(a2),d0
	move.l	intout(a1),a1
	pea	2(a1)
	move.w	d0,-(a7)
	move.l	a7,a1
	bsr	lib_vqt_name
	addq.l	#2,a7
	move.l	(a7)+,a1
	move.w	d0,-2(a1)
	used_d1
	done_return

* lib_vqt_name - Standard Library function
* Todo: ?
* In:	a1	Parameters   id = lib_vqt_name(number, name)
*	a0	VDI struct
lib_vqt_name:
	move.w	(a1),d0
	move.l	vwk_real_address(a0),a2
	tst.w	d0
	lbeq	.not_ok,1
	cmp.w	wk_writing_fonts(a2),d0
	lbls	.ok,2
 label .not_ok,1
	moveq	#1,d0
 label .ok,2
	subq.w	#1,d0
	move.l	wk_writing_first_font(a2),a2
	lbra	.loopend,4
 label .loop,3
	move.l	font_next(a2),a2
 label .loopend,4
	ldbra	d0,.loop,3

	move.l	2(a1),a1
	move.w	font_id(a2),a0
	lea	font_name(a2),a2
	moveq	#31,d1
	moveq	#0,d0
.name:
	move.b	(a2)+,d0
	move.w	d0,(a1)+
	dbra	d1,.name
	move.l	a0,d0
	rts


	dc.b	0,0,"vqt_font_info",0
* vqt_font_info - Standard Trap function
* Todo:	?
* In:   a1      Parameter block
*       a0      VDI struct
vqt_font_info:
	movem.l	intout(a1),a1-a2	; Get ptsout too
	move.l	vwk_text_current_font(a0),a0	; a0 no longer -> VDI struct!
	move.l	font_code(a0),(a1)
	move.w	font_widest_cell(a0),(a2)+
	move.w	font_distance_bottom(a0),(a2)+
	move.w	#0,(a2)+		; Temporary current spec. eff. change of width!
	move.w	font_distance_descent(a0),(a2)+
	move.w	#0,(a2)+		; Temporary current spec. eff. change to left!
	move.w	font_distance_half(a0),(a2)+
	move.w	#0,(a2)+		; Temporary current spec. eff. change to right!
	move.w	font_distance_ascent(a0),(a2)+
	move.w	#0,(a2)+
	move.w	font_distance_top(a0),(a2)+
	done_return

* lib_vqt_font_info - Standard Library function
* Todo: ?
* In:	a1	Parameters   lib_vqt_font_info(&minchar, &maxchar, distance, &maxwidth, effects)
*	a0	VDI struct
lib_vqt_font_info:
	move.l	vwk_text_current_font(a0),a0	; a0 no longer -> VDI struct!
	move.l	(a1)+,a2
	move.w	font_code_low(a0),(a2)
	move.l	(a1)+,a2
	move.w	font_code_high(a0),(a2)
	move.l	(a1)+,a2
	move.w	font_distance_bottom(a0),(a2)+
	move.w	font_distance_descent(a0),(a2)+
	move.w	font_distance_half(a0),(a2)+
	move.w	font_distance_ascent(a0),(a2)+
	move.w	font_distance_top(a0),(a2)
	move.l	(a1)+,a2
	move.w	font_widest_cell(a0),(a2)
	move.l	(a1)+,a2
	move.w	#0,(a2)+		; Temporary current spec. eff. change of width!
	move.w	#0,(a2)+		; Temporary current spec. eff. change to left!
	move.w	#0,(a2)			; Temporary current spec. eff. change to right!
	rts


	dc.b	0,"vqt_xfntinfo",0
* vqt_xfntinfo - Standard Trap function
* Todo:	?
* In:   a1      Parameter block
*       a0      VDI struct
vqt_xfntinfo:
	uses_d1
	move.l	a1,-(a7)
	move.l	intin(a1),a1
	bsr	lib_vqt_xfntinfo
	move.l	(a7)+,a1
	move.l	intin(a1),a2
	move.l	6(a2),a2
	addq.l	#4,a2
	move.l	intout(a1),a1
	move.l	(a2)+,(a1)+
	move.w	(a2)+,(a1)
	used_d1
	done_return

* lib_vqt_xfntinfo - Standard Library function
* Todo: More bits to test! Check size!
* In:	a1	Parameters   lib_vqt_xfntinfo(flag, id, index, info)
*	a0	VDI struct
lib_vqt_xfntinfo:
	move.l	d2,-(a7)
	move.l	vwk_real_address(a0),a2
	move.l	wk_writing_first_font(a2),a2

	move.w	2(a1),d0
	lbne	.id_ok,1
	move.w	4(a1),d1
	lbne	.index_ok,6

	moveq	#1,d0
	moveq	#1,d1
	move.l	vwk_text_current_font(a0),d2
	lbeq	.found,5
	move.w	font_id(a2),d0

 label .id_ok,1
	moveq	#0,d1
 label .search,2
	addq.w	#1,d1
	cmp.w	font_id(a2),d0
	lbls	.maybe,3
	move.l	font_next(a2),a2
	move.l	a2,d2
	lbne	.search,2
	lbra	.not_found,4
 label .maybe,3
	lbne	.not_found,4
	moveq	#1,d2
	lbra	.found,5

 label .index_ok,6
	move.w	d1,d0
 label .search2,7
	subq.w	#1,d1
	lbeq	.counted,8
	move.l	font_next(a2),a2
	move.l	a2,d2
	lbne	.search2,7
	lbra	.not_found,4
 label .counted,8
	move.w	d0,d1
	move.w	font_id(a2),d0
	moveq	#1,d2
	lbra	.found,5

 label .not_found,4
	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
 label .found,5
	swap	d2
	move.w	d0,d2
	move.w	(a1),d0
	move.l	6(a1),a1
	addq.l	#4,a1
	move.l	d2,(a1)+
	move.w	d1,(a1)+
	lbeq	.end,9

	move.l	a2,d2
	btst	#0,d0
	lbeq	.l10,10
	move.l	d2,a2
	lea	font_name(a2),a2
	swap	d0
	moveq	#16-1,d0
 label .loop1,11
	move.w	(a2)+,d1
	move.b	d1,(a1)+
	ldbra	d0,.loop1,11
	swap	d0
	move.b	#0,(a1)+
	add.w	#50-17,a1

 label .l10,10
	btst	#1,d0
	lbeq	.l12,12
	move.l	d2,a2
	lea	font_name(a2),a2
	swap	d0
	moveq	#8-1,d0
 label .loop2,13
	move.w	(a2)+,d1
	move.b	d1,(a1)+
	ldbra	d0,.loop2,13
	swap	d0
	move.b	#0,(a1)+
	add.w	#50-9,a1

 label .l12,12
	btst	#2,d0
	lbeq	.l14,14
	move.l	d2,a2
	lea	font_name(a2),a2
	add.w	#16,a2
	swap	d0
	moveq	#8-1,d0
 label .loop3,15
	move.w	(a2)+,d1
	move.b	d1,(a1)+
	ldbra	d0,.loop3,15
	swap	d0
	move.b	#0,(a1)+
	add.w	#50-9,a1

 label .l14,14

 label .end,9
	move.l	(a7)+,d2
	rts


	dc.b	0,"vqt_extent",0
* vqt_extent - Standard Trap function
* Todo:	The rest of the text modes
* In:   a1      Parameter block
*       a0      VDI struct
vqt_f_extent:					; Really more complicated
vqt_extent:
	uses_d1
	movem.l	d2-d4/a3-a4,-(a7)
	move.l	control(a1),a2
	move.w	6(a2),d0			; Number of characters
	move.l	ptsout(a1),a2
	move.l	intin(a1),a1
	move.l	vwk_text_current_font(a0),a4	; a0 no longer -> VDI struct!
	move.l	font_table_character(a4),a3
	move.w	font_code_low(a4),d3
	move.w	font_code_high(a4),d4

	moveq	#0,d2			; Width total
;	subq.w	#1,d0
	lbra	.no_char,2
 label .loop,1
	move.w	(a1)+,d1
	sub.w	d3,d1			; Negative numbers are higher
	cmp.w	d4,d1			;  than code_high
	lbhi	.no_char,2
	add.w	d1,d1
	add.w	2(a3,d1.w),d2
	sub.w	0(a3,d1.w),d2
 label .no_char,2
	ldbra	d0,.loop,1

	move.w	vwk_text_effects(a0),d0
	btst	#0,d0
	beq	.no_bold
	add.w	font_thickening(a4),d2
.no_bold:
	btst	#4,d0
	beq	.no_outline
	addq.w	#2,d2
.no_outline:
	btst	#2,d0
	beq	.no_italic
	move.w	font_skewing(a4),d1
	move.w	font_height(a4),d0
	subq.w	#1,d0
 label .loop2,3
	rol.w	#1,d1
	lbcc	.skip,4
	addq.w	#1,d2
 label .skip,4
	ldbra	d0,.loop2,3
.no_italic:

	move.l	#0,(a2)+
  ifne SUB1
	subq.w	#1,d2
  endc
	swap	d2
	move.l	d2,(a2)+
	move.w	font_height(a4),d2
  ifne SUB1
	subq.w	#1,d2
  endc
	move.l	d2,(a2)+
	ext.l	d2
	move.l	d2,(a2)+

	movem.l	(a7)+,d2-d4/a3-a4
	used_d1
	done_return

* lib_vqt_extent - Standard Library function
* Todo: ?
* In:	a1	Parameters   lib_vqt_extent(length, &string, points)
*	a0	VDI struct
_get_extent:
	move.l	4(a7),a0		; vwk as parameter
	lea	8(a7),a1
lib_vqt_extent:
	movem.l	d2-d4/a3-a4,-(a7)
	move.w	(a1),d0			; Number of characters
	move.l	6(a1),a2
	move.l	2(a1),a1
	move.l	vwk_text_current_font(a0),a0	; a0 no longer -> VDI struct!
	move.l	font_table_character(a0),a3
	move.w	font_code_low(a0),d3
	move.w	font_code_high(a0),d4

	moveq	#0,d2			; Width total
;	subq.w	#1,d0
	lbra	.no_char,2
 label .loop,1
	move.w	(a1)+,d1
	sub.w	d3,d1			; Negative numbers are higher
	cmp.w	d4,d1			;  than code_high
	lbhi	.no_char,2
	add.w	d1,d1
	add.w	2(a3,d1.w),d2
	sub.w	0(a3,d1.w),d2
 label .no_char,2
	ldbra	d0,.loop,1

	move.w	vwk_text_effects(a0),d0
	btst	#0,d0
	lbeq	.no_bold,5
	add.w	font_thickening(a4),d2
 label .no_bold,5
	btst	#4,d0
	lbeq	.no_outline,6
	addq.w	#2,d2
 label .no_outline,6
	btst	#2,d0
	lbeq	.no_italic,7
	move.w	font_skewing(a4),d1
	move.w	font_height(a4),d0
	subq.w	#1,d0
 label .loop2,3
	rol.w	#1,d1
	lbcc	.skip,4
	addq.w	#1,d2
 label .skip,4
	ldbra	d0,.loop2,3
 label .no_italic,7

	move.l	#0,(a2)+
  ifne SUB1
	subq.w	#1,d2
  endc
	swap	d2
	move.l	d2,(a2)+
	move.w	font_height(a0),d2
  ifne SUB1
	subq.w	#1,d2
  endc
	move.l	d2,(a2)+
	ext.l	d2
	move.l	d2,(a2)+

	movem.l	(a7)+,d2-d4/a3-a4
	rts


	dc.b	0,0,"vqt_width",0
* vqt_width - Standard Trap function
* Todo:	?
* In:   a1      Parameter block
*       a0      VDI struct
vqt_width:
	uses_d1
	movem.l	d2/a3,-(a7)
	move.l	intin(a1),a2
	move.w	(a2),d0			; Character to check
	move.l	ptsout(a1),a2
	move.l	intout(a1),a1
	move.l	vwk_text_current_font(a0),a0	; a0 no longer -> VDI struct!
	move.l	font_table_character(a0),a3
	move.w	font_code_low(a0),d1

	neg.w	d1
	add.w	d0,d1			; Negative numbers are higher
	cmp.w	font_code_high(a0),d1	;  than code_high
	bhi	.no_char
	move.w	d0,(a1)
	add.w	d1,d1
	move.w	2(a3,d1.w),d2
	sub.w	0(a3,d1.w),d2
	
	moveq	#0,d0
	move.w	d2,(a2)+
	move.w	d0,(a2)+
	move.w	font_flags(a0),d2
	and.w	#$0002,d2
	beq	.no_offset
	move.l	font_table_horizontal(a0),a3
	move.w	2(a3,d1.w),d2
	sub.w	0(a3,d1.w),d2
.no_offset:
	move.w	d2,(a2)+
	move.w	d0,(a2)+
	move.w	d0,(a2)+		; Right hand offset?
	move.w	d0,(a2)+
	
.end_vqt_width:	; .end:
	movem.l	(a7)+,d2/a3
	used_d1
	done_return

.no_char:
	move.w	#-1,(a1)
	bra	.end_vqt_width	; .end

* lib_vqt_width - Standard Library function
* Todo: ?
* In:	a1	Parameters   status = lib_vqt_width(char, &cellw, &left_offset, &right_offset)
*	a0	VDI struct
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


	dc.b	0,"vst_height",0
* vst_height - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vst_height:
	uses_d1
	move.l	ptsin(a1),a2
	move.w	2(a2),d0
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
	movem.l	ptsout(a1),a2
	move.w	font_widest_character(a3),d1
	move.w	d1,(a2)+
	swap	d1
	move.w	font_distance_top(a3),d1
;	addq.w	#1,d1
	move.w	d1,(a2)+			; Height in pixels
	move.l	d1,vwk_text_character(a0)	; Character w/h in vwk
	swap	d1
	move.w	font_widest_cell(a3),d1
	move.w	d1,(a2)+
	swap	d1
	addq.w	#1,d1
	add.w	font_distance_bottom(a3),d1
	move.w	d1,(a2)+			; Height in pixels
	move.l	d1,vwk_text_cell(a0)		; Cell w/h in vwk
	move.l	(a7)+,a3
	used_d1
	done_return

* lib_vst_height - Standard Library function
* Todo: ?
* In:	a1	Parameters   lib_vst_height(height, &charw, &charh, &cellw, &cellh)
*	a0	VDI struct
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


	dc.b	0,0,"vst_point",0
* vst_point - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vst_point:
	uses_d1
	move.l	a3,-(a7)
	move.l	intin(a1),a2
	move.w	(a2),d0
	move.l	vwk_text_current_font(a0),a2
	move.l	font_extra_first_size(a2),a2
	move.l	a2,a3
 label .search,1
	cmp.w	font_size(a2),d0
	lblo	.found,2
	move.l	a2,a3
	move.l	font_extra_next_size(a2),a2
	move.l	a2,d1
	lbne	.search,1
 label .found,2
	move.l	a3,vwk_text_current_font(a0)
	movem.l	intout(a1),a1-a2		; Get ptsout too
	move.w	font_widest_character(a3),d1
	move.w	d1,(a2)+
	swap	d1
	move.w	font_distance_top(a3),d1
;;	addq.w	#1,d1
	move.w	d1,(a2)+			; Height in pixels
	move.l	d1,vwk_text_character(a0)	; Character w/h in vwk
	swap	d1
	move.w	font_widest_cell(a3),d1
	move.w	d1,(a2)+
	swap	d1
;	addq.w	#1,d1
;	add.w	font_distance_bottom(a3),d1
	move.w	font_height(a3),d1
	move.w	d1,(a2)+			; Height in pixels
	move.l	d1,vwk_text_cell(a0)		; Cell w/h in vwk
	move.w	font_size(a3),(a1)
	move.l	(a7)+,a3
	used_d1
	done_return

* lib_vst_point - Standard Library function
* Todo: ?
* In:	a1	Parameters   point_set = lib_vst_point(height, &charw, &charh, &cellw, &cellh)
*	a0	VDI struct
_lib_vst_point:
lib_vst_point:
	move.w	(a1)+,d0
	move.l	a3,-(a7)
	move.l	vwk_text_current_font(a0),a2
	move.l	font_extra_first_size(a2),a2
	move.l	a2,a3
 label .search,1
	cmp.w	font_size(a2),d0
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
;;	addq.w	#1,d1
	move.l	(a1)+,a2
	move.w	d1,(a2)				; Height in pixels
	move.l	d1,vwk_text_character(a0)	; Character w/h in vwk
	swap	d1
	move.w	font_widest_cell(a3),d1
	move.l	(a1)+,a2
	move.w	d1,(a2)
	swap	d1
;	addq.w	#1,d1
;	add.w	font_distance_bottom(a3),d1
	move.w	font_height(a3),d1
	move.l	(a1),a2
	move.w	d1,(a2)				; Height in pixels
	move.l	d1,vwk_text_cell(a0)		; Cell w/h in vwk
	move.w	font_size(a3),d0
	move.l	(a7)+,a3
	rts


	dc.b	0,"vqt_attributes",0
* vqt_attributes - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vqt_attributes:
	move.w	vwk_mode(a0),d0
	lea	vwk_text(a0),a0		; a0 no longer -> VDI struct!
	movem.l	intout(a1),a1-a2	; Get ptsout too
	move.w	(a0),(a1)+		; Font
	addq.l	#4,a0
	move.l	(a0)+,(a1)+		; Foreground, rotation
	move.l	(a0)+,(a1)+		; Horizontal and vertical alignment
	move.w	d0,(a1)+		; Mode
	move.l	(a0)+,(a2)+		; Character height and width
	move.l	(a0)+,(a2)+		; Cell height and width
	done_return

* lib_vqt_attributes - Standard Library function
* Todo: ?
* In:	a1	Parameters   lib_vqt_attributes(settings)
*	a0	VDI struct
lib_vqt_attributes:
	move.l	(a1),a1
	move.w	vwk_mode(a0),d0
	lea	vwk_text(a0),a0		; a0 no longer -> VDI struct!
	move.w	(a0),(a1)+		; Font
	addq.l	#4,a0
	move.l	(a0)+,(a1)+		; Foreground, rotation
	move.l	(a0)+,(a1)+		; Horizontal and vertical alignment
	move.w	d0,(a1)+		; Mode
	move.l	(a0)+,(a1)+		; Character height and width
	move.l	(a0)+,(a1)+		; Cell height and width
	rts


	dc.b	0,"vst_load_fonts",0
* vst_load_fonts - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vst_load_fonts:
	move.l	intout(a1),a1
	move.l	vwk_real_address(a0),a2
;	move.w	wk_writing_fonts(a2),(a1)
;	move.w	#0,(a1)
	move.w	wk_writing_fonts(a2),d0
	subq.w	#1,d0
	move.w	d0,(a1)
	done_return

* lib_vst_load_fonts - Standard Library function
* Todo: ?
* In:	a1	Parameters   fonts_loaded = lib_vst_load_fonts(select)
*	a0	VDI struct
lib_vst_load_fonts:
	move.l	vwk_real_address(a0),a2
	move.w	wk_writing_fonts(a2),d0
	subq.w	#1,d0
	rts


	dc.b	0,"vst_unload_fonts",0
* vst_unload_fonts - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vst_unload_fonts:
	done_return

* lib_vst_unload_fonts - Standard Library function
* Todo: ?
* In:	a1	Parameters   lib_vst_unload_fonts(select)
*	a0	VDI struct
lib_vst_unload_fonts:
	rts

	end