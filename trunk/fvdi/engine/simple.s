*****
* fVDI miscellaneous functions
*
* $Id: simple.s,v 1.4 2002-07-03 21:31:26 johan Exp $
*
* Copyright 1997-2002, Johan Klockars 
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

transparent	equ	1		; Fall through?

neg_pal_n	equ	9		; Number of negative palette entries

HANDLES		equ	32		; Max number of handles

	include	"vdi.inc"

*
* Macros
*
  ifne lattice
	include	"macros.dev"
  else
	include	"macros.tas"
  endc

	xref	_v_opnwk,_v_opnvwk,_v_clsvwk,_v_clswk
	xref	_set_protected_l
	xref	_old_gdos

	xref	_vq_chcells,_v_exit_cur,_v_enter_cur,_v_curup,_v_curdown
	xref	_v_curright,_v_curleft,_v_curhome,_v_eeos,_v_eeol,_vs_curaddress
	xref	_v_curtext,_v_rvon,_v_rvoff,_vq_curaddress

	xdef	v_opnwk,v_opnvwk,v_clsvwk,v_clswk
	xdef	vs_clip,vswr_mode,vq_extnd
	xdef	_opnvwk_values

	xdef	lib_vs_clip,lib_vswr_mode
	xdef	_lib_vs_clip

	xdef	vq_chcells,v_exit_cur,v_enter_cur,v_curup,v_curdown
	xdef	v_curright,v_curleft,v_curhome,v_eeos,v_eeol,vs_curaddress
	xdef	v_curtext,v_rvon,v_rvoff,vq_curaddress


	text

	dc.b	0,0,"v_opnwk",0
* v_opnwk - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_opnwk:
	uses_d1
	move.l	d2,-(a7)

	move.l	a1,-(a7)
	jsr	_v_opnwk
	addq.l	#4,a7
	
	move.l	(a7)+,d2
	used_d1
	
	real_return


	dc.b	0,"v_opnvwk",0
* v_opnvwk - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_opnvwk:
	uses_d1
	move.l	d2,-(a7)
	
	move.l	a1,-(a7)
	move.l	a0,-(a7)
	jsr	_v_opnvwk
	addq.l	#8,a7
	
	move.l	(a7)+,d2
	used_d1
	
	real_return


	dc.b	0,"v_clsvwk",0
* v_clsvwk - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_clsvwk:
	uses_d1
	move.l	d2,-(a7)
	
	move.l	a1,-(a7)
	move.l	a0,-(a7)
	jsr	_v_clsvwk
	addq.l	#8,a7
	
	move.l	(a7)+,d2
	used_d1
	
	done_return


	dc.b	0,0,"v_clswk",0
* v_clswk - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_clswk:
	uses_d1
	move.l	d2,-(a7)
	
	move.l	a1,-(a7)
	move.l	a0,-(a7)
	jsr	_v_clswk
	addq.l	#8,a7
	
	move.l	(a7)+,d2
	used_d1
	
	done_return


	dc.b	0,0,"vs_clip",0
* vs_clip - Standard Trap function
* Todo: Should allow for min_x/y coordinates as well
* In:   a1      Parameter block
*       a0      VDI struct
vs_clip:
	move.l	intin(a1),a2
	move.w	(a2),vwk_clip_on(a0)
	beq	.no_clip				; Not sure this is a good idea
	move.l	ptsin(a1),a2
;	move.l	(a2)+,vwk_clip_rectangle_x1(a0)		; left/top
;	move.l	(a2)+,vwk_clip_rectangle_x2(a0)		; right/bottom
	move.l	vwk_real_address(a0),a1
	move.w	(a2)+,d0				; left
	bge	.left_ok
	moveq	#0,d0
.left_ok:
	move.w	d0,vwk_clip_rectangle_x1(a0)
	move.w	(a2)+,d0				; top
	bge	.top_ok
	moveq	#0,d0
.top_ok:
	move.w	d0,vwk_clip_rectangle_y1(a0)
	move.w	(a2)+,d0
	cmp.w	wk_screen_coordinates_max_x(a1),d0	; right
	ble	.right_ok
	move.w	wk_screen_coordinates_max_x(a1),d0
.right_ok:
	move.w	d0,vwk_clip_rectangle_x2(a0)
	move.w	(a2)+,d0
	cmp.w	wk_screen_coordinates_max_y(a1),d0	; bottom
	ble	.bottom_ok
	move.w	wk_screen_coordinates_max_y(a1),d0
.bottom_ok:
	move.w	d0,vwk_clip_rectangle_y2(a0)
;	return
	done_return

.no_clip:
	move.l	vwk_real_address(a0),a1
	moveq	#0,d0
	move.w	d0,vwk_clip_rectangle_x1(a0)
	move.w	d0,vwk_clip_rectangle_y1(a0)
	move.w	wk_screen_coordinates_max_x(a1),d0
	move.w	d0,vwk_clip_rectangle_x2(a0)
	move.w	wk_screen_coordinates_max_y(a1),d0
	move.w	d0,vwk_clip_rectangle_y2(a0)
;	return
	done_return


* lib_vs_clip - Standard library function
* Todo: Should allow for min_x/y coordinates as well
* In:   a1      Parameters   lib_vs_clip(clip_flag, points)
*       a0      VDI struct
_lib_vs_clip:
lib_vs_clip:
	move.w	(a1)+,vwk_clip_on(a0)
	lbeq	.no_clip,5				; Not sure this is a good idea
	move.l	(a1),a2
	move.l	vwk_real_address(a0),a1
	move.w	(a2)+,d0				; left
	lbge	.left_ok,1
	moveq	#0,d0
 label .left_ok,1
	move.w	d0,vwk_clip_rectangle_x1(a0)
	move.w	(a2)+,d0				; top
	lbge	.top_ok,2
 label .top_ok,2
	move.w	d0,vwk_clip_rectangle_y1(a0)
	move.w	(a2)+,d0
	cmp.w	wk_screen_coordinates_max_x(a1),d0	; right
	lble	.right_ok,3
	move.w	wk_screen_coordinates_max_x(a1),d0
 label .right_ok,3
	move.w	d0,vwk_clip_rectangle_x2(a0)
	move.w	(a2)+,d0
	cmp.w	wk_screen_coordinates_max_y(a1),d0	; bottom
	lble	.bottom_ok,4
	move.w	wk_screen_coordinates_max_y(a1),d0
 label .bottom_ok,4
	move.w	d0,vwk_clip_rectangle_y2(a0)
	rts

 label .no_clip,5
	move.l	vwk_real_address(a0),a1
	moveq	#0,d0
	move.w	d0,vwk_clip_rectangle_x1(a0)
	move.w	d0,vwk_clip_rectangle_y1(a0)
	move.w	wk_screen_coordinates_max_x(a1),d0
	move.w	d0,vwk_clip_rectangle_x2(a0)
	move.w	wk_screen_coordinates_max_y(a1),d0
	move.w	d0,vwk_clip_rectangle_y2(a0)
	rts


*
* Various
*

	dc.b	0,0,"vswr_mode",0
* vswr_mode - Standard Trap function
* Todo: Double colour mode?
* In:   a1      Parameter block
*       a0      VDI struct
vswr_mode:
	move.l	intin(a1),a2
	move.w	(a2),d0
	lbeq	.not_ok,1
	move.l	vwk_real_address(a0),a2
	cmp.w	wk_drawing_writing_modes(a2),d0		; # write modes
	lbls	.ok,2
 label .not_ok,1
	moveq	#1,d0			; Replace
 label .ok,2
	move.w	d0,vwk_mode(a0)
	move.l	intout(a1),a2
	move.w	d0,(a2)
;	return
	done_return

* lib_vswr_mode - Standard Library function
* Todo: ?
* In:	a1	Parameters   mode_set = lib_vswr_mode(mode)
*	a0	VDI struct
lib_vswr_mode:
	move.w	(a1),d0
	lbeq	.not_ok,1
	move.l	vwk_real_address(a0),a2
	cmp.w	wk_drawing_writing_modes(a2),d0		; # write modes
	lbls	.ok,2
 label .not_ok,1
	moveq	#1,d0			; Replace
 label .ok,2
	move.w	d0,vwk_mode(a0)
	rts


	dc.b	0,"vq_extnd",0
* vq_extnd - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vq_extnd:
	move.l	intin(a1),a2
	move.w	(a2),d0
	move.l	intout(a1),a2
	bne	.not_opnvwk
	bsr	opnvwk_values
	bra	.end_vq_extnd	; .end

.not_opnvwk:
	cmp.w	#2,d0
	bne	.normal_extended
	move.l	control(a1),a2
	cmp.w	#1,10(a2)
	beq	vq_scrninfo
	move.l	intout(a1),a2
.normal_extended:
	move.l	a1,d0
	move.l	vwk_real_address(a0),a1
	move.w	wk_screen_type(a1),(a2)+
	move.w	wk_screen_bkg_colours(a1),(a2)+
	move.w	wk_writing_effects(a1),(a2)+	
	move.w	wk_raster_scaling(a1),(a2)+
	move.w	wk_screen_mfdb_bitplanes(a1),(a2)+
	move.w	wk_screen_look_up_table(a1),(a2)+
	move.w	wk_raster_performance(a1),(a2)+
	move.w	wk_drawing_flood_fill(a1),(a2)+
	move.w	wk_writing_rotation_type(a1),(a2)+
	move.w	wk_drawing_writing_modes(a1),(a2)+
	move.w	wk_various_input_type(a1),(a2)+
	move.w	wk_writing_justification(a1),(a2)+
	move.w	wk_various_inking(a1),(a2)+
	move.w	wk_drawing_rubber_banding(a1),(a2)+
	move.w	wk_various_max_ptsin(a1),(a2)+
	move.w	wk_various_max_intin(a1),(a2)+
	move.w	wk_various_buttons(a1),(a2)+
	move.w	wk_drawing_line_wide_types_possible(a1),(a2)+
	move.w	wk_drawing_line_wide_writing_modes(a1),(a2)+
	move.w	vwk_clip_on(a0),(a2)+
	move.l	d0,a1

	move.w	#0,(a2)+		; No pixel sizes in the next few places

	moveq	#27-20-1,d0
 label .loop1,1
	move.w	#0,(a2)+
	ldbra	d0,.loop1,1

	move.w	#2,(a2)+		; Beziers!

	moveq	#29-28-1,d0
 label .loop2,2
	move.w	#0,(a2)+
	ldbra	d0,.loop2,2

	move.w	#0,(a2)+		; 1 - bitmap scale, 2 - new raster functions

	move.w	#0,(a2)+

	move.w	#1,(a2)+		; New style colour routines (at least some of them)

	moveq	#39-32-1,d0
 label .loop3,3
	move.w	#0,(a2)+
	ldbra	d0,.loop3,3

	move.w	#0,(a2)+		; Unusable left border
	move.w	#0,(a2)+		;          upper
	move.w	#0,(a2)+		;          right
	move.w	#0,(a2)+		;          lower

	move.w	#0,(a2)+

	move.l	ptsout(a1),a1
	move.w	vwk_clip_rectangle_x1,(a1)+
	move.w	vwk_clip_rectangle_y1,(a1)+
	move.w	vwk_clip_rectangle_x2,(a1)+
	move.w	vwk_clip_rectangle_y2,(a1)+

	moveq	#11-3-1,d0
 label .loop4,4
	move.w	#0,(a1)+
	ldbra	d0,.loop4,4

.end_vq_extnd:		; .end:
;	return
	done_return

_opnvwk_values:
	move.l	a2,-(a7)
	move.l	4+4(a7),a0
	move.l	4+8(a7),a1
	move.l	intout(a1),a2
	bsr	opnvwk_values
	move.l	(a7)+,a2
	rts

opnvwk_values:
	uses_d1
	movem.l	d2/a3,-(a7)
	move.l	ptsout(a1),a1
	move.l	vwk_real_address(a0),a0
	move.l	wk_screen_coordinates_max_x(a0),(a2)+
	move.w	wk_screen_coordinates_course(a0),(a2)+
	move.l	wk_screen_pixel_width(a0),(a2)+
	move.w	wk_writing_size_possibilities(a0),(a2)+
	move.w	wk_drawing_line_types(a0),(a2)+
	move.w	wk_drawing_line_wide_width_possibilities(a0),(a2)+
	move.w	wk_drawing_marker_types(a0),(a2)+
	move.w	wk_drawing_marker_size_possibilities(a0),(a2)+
;	move.w	wk_writing_fonts(a0),(a2)+
	move.w	#1,(a2)+
	move.l	wk_drawing_fill_patterns(a0),(a2)+
	move.w	wk_screen_palette_size(a0),(a2)+
	move.w	wk_drawing_primitives_supported(a0),(a2)+

	lea	10*2(a2),a3
	move.l	wk_drawing_primitives_attributes(a0),d0
	moveq	#9,d1
 label .loop,1
	move.w	d0,d2
	and.w	#$0007,d2
	lbeq	.not_implemented,2
	subq.w	#1,d2
	move.w	d2,(a3)+
	moveq	#10,d2
	sub.w	d1,d2
	move.w	d2,-(2+10*2)(a3)
 label .not_implemented,2
	lsr.l	#3,d0
	ldbeq	d1,.loop,1
	lea	2*10*2(a2),a2
	cmp.l	a2,a3
	beq	.no_marker
	move.w	#-1,-(2+10*2)(a3)
.no_marker:

	move.w	wk_screen_colour(a0),(a2)+
	move.w	wk_writing_rotation_possible(a0),(a2)+
	move.w	wk_drawing_fill_possible(a0),(a2)+
	move.w	wk_drawing_cellarray_available(a0),(a2)+
	move.w	wk_screen_palette_possibilities(a0),(a2)+
	move.l	wk_various_cursor_movement(a0),(a2)+	; Also number_entry
	move.l	wk_various_selection(a0),(a2)+		; Also typing
	move.w	wk_various_workstation_type(a0),(a2)+

	move.w	wk_writing_size_width_min(a0),(a1)+
	move.w	wk_writing_size_height_min(a0),(a1)+
	move.w	wk_writing_size_width_max(a0),(a1)+
	move.w	wk_writing_size_height_max(a0),(a1)+
	move.w	wk_drawing_line_wide_width_min(a0),(a1)+
	move.w	#0,(a1)+
	move.w	wk_drawing_line_wide_width_max(a0),(a1)+
	move.w	#0,(a1)+
	move.w	wk_drawing_marker_size_width_min(a0),(a1)+
	move.w	wk_drawing_marker_size_height_min(a0),(a1)+
	move.w	wk_drawing_marker_size_width_max(a0),(a1)+
	move.w	wk_drawing_marker_size_height_max(a0),(a1)+

	movem.l	(a7)+,d2/a3
	used_d1
	rts

vq_scrninfo:
	move.w	#0,4(a2)		; # ptsout
	move.w	#272,8(a2)		; # intout
	move.l	intout(a1),a2

	move.l	vwk_real_address(a0),a0		; a0 no longer -> VDI struct
	move.l	wk_driver(a0),a1
	move.l	driver_device(a1),a1

	move.w	#271,d0
 label .loop,1
	move.w	(a1)+,(a2)+
	ldbra	d0,.loop,1
	sub.w	#271*2+2,a2

	move.l	wk_screen_mfdb_address(a0),dev_address(a2)
	move.w	wk_screen_wrap(a0),dev_byte_width(a2)

	done_return


	dc.b	0,"vq_chcells",0
* vq_chcells - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vq_chcells:
	uses_d1
	movem.l	d2,-(a7)
	move.l	intout(a1),a2
	pea	2(a2)
	pea	0(a2)
	pea	(a1)
	jsr	_vq_chcells
	add.w	#12,a7
	movem.l	(a7)+,d2
	used_d1
	done_return


	dc.b	0,"v_exit_cur",0
* v_exit_cur - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_exit_cur:
	uses_d1
	movem.l	d2,-(a7)
	pea	(a1)
	jsr	_v_exit_cur
	addq.w	#4,a7
	movem.l	(a7)+,d2
	used_d1
	done_return


	dc.b	0,0,"v_enter_cur",0
* v_enter_cur - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_enter_cur:
	uses_d1
	movem.l	d2,-(a7)
	pea	(a1)
	jsr	_v_enter_cur
	addq.w	#4,a7
	movem.l	(a7)+,d2
	used_d1
	done_return


	dc.b	0,0,"v_curup",0
* v_curup - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_curup:
	uses_d1
	movem.l	d2,-(a7)
	pea	(a1)
	jsr	_v_curup
	addq.w	#4,a7
	movem.l	(a7)+,d2
	used_d1
	done_return


	dc.b	0,0,"v_curdown",0
* v_curdown - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_curdown:
	uses_d1
	movem.l	d2,-(a7)
	pea	(a1)
	jsr	_v_curdown
	addq.w	#4,a7
	movem.l	(a7)+,d2
	used_d1
	done_return


	dc.b	0,"v_curright",0
* v_curright - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_curright:
	uses_d1
	movem.l	d2,-(a7)
	pea	(a1)
	jsr	_v_curright
	addq.w	#4,a7
	movem.l	(a7)+,d2
	used_d1
	done_return


	dc.b	0,0,"v_curleft",0
* v_curleft - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_curleft:
	uses_d1
	movem.l	d2,-(a7)
	pea	(a1)
	jsr	_v_curleft
	addq.w	#4,a7
	movem.l	(a7)+,d2
	used_d1
	done_return


	dc.b	0,0,"v_curhome",0
* v_curhome - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_curhome:
	uses_d1
	movem.l	d2,-(a7)
	pea	(a1)
	jsr	_v_curhome
	addq.w	#4,a7
	movem.l	(a7)+,d2
	used_d1
	done_return


	dc.b	0,"v_eeos",0
* v_eeos - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_eeos:
	uses_d1
	movem.l	d2,-(a7)
	pea	(a1)
	jsr	_v_eeos
	addq.w	#4,a7
	movem.l	(a7)+,d2
	used_d1
	done_return


	dc.b	0,"v_eeol",0
* v_eeol - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_eeol:
	uses_d1
	movem.l	d2,-(a7)
	pea	(a1)
	jsr	_v_eeol
	addq.w	#4,a7
	movem.l	(a7)+,d2
	used_d1
	done_return


	dc.b	0,0,"vs_curaddress",0
* vs_curaddress - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vs_curaddress:
	uses_d1
	movem.l	d2,-(a7)
	move.l	intin(a1),a2
	move.w	2(a2),d0
	ext.l	d0
	move.l	d0,-(a7)
	move.w	0(a2),d0
	ext.l	d0
	move.l	d0,-(a7)
	pea	(a1)
	jsr	_vs_curaddress
	add.w	#12,a7
	movem.l	(a7)+,d2
	used_d1
	done_return


	dc.b	0,0,"v_curtext",0
* v_curtext - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_curtext:
	uses_d1
	movem.l	d2,-(a7)
	move.l	control(a1),a2
	move.w	L_intin(a2),d0
	ext.l	d0
	move.l	d0,-(a7)
	move.l	intin(a1),a2
	pea	(a2)
	pea	(a1)
	jsr	_v_curtext
	add.w	#12,a7
	movem.l	(a7)+,d2
	used_d1
	done_return


	dc.b	0,"v_rvon",0
* v_rvon - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_rvon:
	uses_d1
	movem.l	d2,-(a7)
	pea	(a1)
	jsr	_v_rvon
	addq.w	#4,a7
	movem.l	(a7)+,d2
	used_d1
	done_return


	dc.b	0,0,"v_rvoff",0
* v_rvoff - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_rvoff:
	uses_d1
	movem.l	d2,-(a7)
	pea	(a1)
	jsr	_v_rvoff
	addq.w	#4,a7
	movem.l	(a7)+,d2
	used_d1
	done_return


	dc.b	0,0,"vq_curaddress",0
* vq_curaddress - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vq_curaddress:
	uses_d1
	movem.l	d2,-(a7)
	move.l	intout(a1),a2
	pea	2(a2)
	pea	0(a2)
	pea	(a1)
	jsr	_vq_curaddress
	add.w	#12,a7
	movem.l	(a7)+,d2
	used_d1
	done_return

	end
