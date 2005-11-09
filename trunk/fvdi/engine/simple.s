*****
* fVDI miscellaneous functions
*
* $Id: simple.s,v 1.11 2005-11-09 23:06:41 johan Exp $
*
* Copyright 1997-2003, Johan Klockars 
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

transparent	equ	1		; Fall through?

	include	"vdi.inc"
	include	"macros.inc"

	xref	_v_opnwk,_v_opnvwk,_v_clsvwk,_v_clswk
	xref	_event

	xref	_vq_chcells,_v_exit_cur,_v_enter_cur,_v_curup,_v_curdown
	xref	_v_curright,_v_curleft,_v_curhome,_v_eeos,_v_eeol,_vs_curaddress
	xref	_v_curtext,_v_rvon,_v_rvoff,_vq_curaddress

	xref	_lib_vq_extnd

	xdef	v_opnwk,v_opnvwk,v_clsvwk,v_clswk
	xdef	vs_clip,vswr_mode,vq_extnd

	xdef	lib_vs_clip,lib_vswr_mode
	xdef	_lib_vs_clip

	xdef	vq_chcells,v_exit_cur,v_enter_cur,v_curup,v_curdown
	xdef	v_curright,v_curleft,v_curhome,v_eeos,v_eeol,vs_curaddress
	xdef	v_curtext,v_rvon,v_rvoff,vq_curaddress,v_dspcur


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

	uses_d1
	move.l	ptsin(a1),a2
	move.l	vwk_real_address(a0),a1

	move.w	0(a2),d0
	move.w	4(a2),d1
	cmp.w	d0,d1
	bge	.x_ordered
	exg	d0,d1
.x_ordered:

	tst.w	d0				; left
	bge	.left_ok
	moveq	#0,d0
.left_ok:
	move.w	d0,vwk_clip_rectangle_x1(a0)

	cmp.w	wk_screen_coordinates_max_x(a1),d1	; right
	ble	.right_ok
	move.w	wk_screen_coordinates_max_x(a1),d1
.right_ok:
	move.w	d1,vwk_clip_rectangle_x2(a0)

	move.w	2(a2),d0
	move.w	6(a2),d1
	cmp.w	d0,d1
	bge	.y_ordered
	exg	d0,d1
.y_ordered:

	tst.w	d0				; top
	bge	.top_ok
	moveq	#0,d0
.top_ok:
	move.w	d0,vwk_clip_rectangle_y1(a0)

	cmp.w	wk_screen_coordinates_max_y(a1),d1	; bottom
	ble	.bottom_ok
	move.w	wk_screen_coordinates_max_y(a1),d1
.bottom_ok:
	move.w	d1,vwk_clip_rectangle_y2(a0)
;	return

	used_d1
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
* Todo: Fix according to above!! Should allow for min_x/y coordinates as well
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
	moveq	#0,d0
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
	uses_d1
	movem.l	d2/a1,-(a7)
	
	move.l	ptsout(a1),a2
	move.l	a2,-(a7)
	move.l	intout(a1),a2
	move.l	a2,-(a7)
	move.l	intin(a1),a2
	move.w	(a2),d0
	ext.l	d0
	move.l	d0,-(a7)
	move.l	control(a1),a2
	move.w	subfunction(a2),d0
	ext.l	d0
	move.l	d0,-(a7)
	move.l	a0,-(a7)
	jsr	_lib_vq_extnd
	add.w	#20,a7

	movem.l	(a7)+,d2/a1
	tst.w	d0
	lbeq	.end,1
	move.l	control(a1),a2	; vq_scrninfo
	move.w	#0,L_ptsout(a2)
	move.w	#272,L_intout(a2)
	
 label .end,1
	used_d1
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
	pea	(a0)
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
	pea	(a0)
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
	pea	(a0)
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
	pea	(a0)
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
	pea	(a0)
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
	pea	(a0)
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
	pea	(a0)
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
	pea	(a0)
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
	pea	(a0)
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
	pea	(a0)
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
	pea	(a0)
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
	pea	(a0)
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
	pea	(a0)
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
	pea	(a0)
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
	pea	(a0)
	jsr	_vq_curaddress
	add.w	#12,a7
	movem.l	(a7)+,d2
	used_d1
	done_return

	dc.b	0,"v_dspcur",0
* v_dspcur - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
v_dspcur:
	uses_d1
	movem.l	d2,-(a7)
	move.l	ptsin(a1),a2

	move.l	(a2),-(a7)
	move.w	#6,-(a7)		; Forced absolute mouse movement
	move.w	#0,-(a7)
	jsr	_event
	addq.l	#8,a7
	
	movem.l	(a7)+,d2
	used_d1
	done_return

	end
