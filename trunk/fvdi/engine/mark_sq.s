*****
* FenixVDI marker set/query functions
*
* Copyright 1997-2000, Johan Klockars 
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

transparent	equ	1		; Fall through?

	include	"vdi.inc"
	include	"macros.inc"

	xdef	vsm_color,vsm_height,vsm_type,vqm_attributes

	xdef	lib_vsm_color,lib_vsm_height,lib_vsm_type,lib_vqm_attributes
	xdef	_lib_vsm_color,_lib_vsm_type


	text

	dc.b	0,0,"vsm_color",0
* vsm_color - Standard Trap function
* Todo: Get foreground colour?
* In:   a1      Parameter block
*       a0      VDI struct
vsm_color:
	move.l	intin(a1),a2
	move.w	(a2),d0
	move.l	vwk_real_address(a0),a2
	cmp.w	wk_screen_palette_size(a2),d0
	lblo	.ok,1
	moveq	#BLACK,d0
 label .ok,1
	move.w	d0,vwk_marker_colour_foreground(a0)
	move.l	intout(a1),a2
	move.w	d0,(a2)
	done_return

* lib_vsm_color - Standard Library function
* Todo: ?
* In:	a1	Parameters   colour_set = lib_vsm_color(colour)
*	a0	VDI struct
_lib_vsm_color:
lib_vsm_color:
	move.w	(a1),d0
	move.l	vwk_real_address(a0),a2
	cmp.w	wk_screen_palette_size(a2),d0
	lblo	.ok,1
	moveq	#BLACK,d0
 label .ok,1
	move.w	d0,vwk_marker_colour_foreground(a0)
	rts


	dc.b	0,"vsm_height",0
* vsm_height - Standard Trap function
* Todo: Get allowed steps?
* In:   a1      Parameter block
*       a0      VDI struct
vsm_height:
	uses_d1
	move.l	a1,-(a7)
	move.l	intin(a1),a1
	bsr	lib_vsm_height
	move.l	(a7)+,a1
	move.l	ptsout(a1),a2
	move.l	d0,(a2)
	used_d1
	done_return

* lib_vsm_height - Standard Library function
* Todo: ?
* In:	a1	Parameters   height_set = lib_vsm_height(height)
*	a0	VDI struct
lib_vsm_height:
	move.w	(a1),d0
	ext.l	d0
	move.l	vwk_real_address(a0),a2
	tst.w	wk_drawing_marker_size_possibilities(a2)
	bne	.limited
	nop				; What if unlimited?
.limited:
	move.w	wk_drawing_marker_size_height_min(a2),d1
	lsr.w	#1,d1
	add.w	d1,d0			; Half the minimum height
	divu	wk_drawing_marker_size_height_min(a2),d0	; w = 15n, h = 11n
	lbne	.ok1,1
	moveq	#1,d0
 label .ok1,1
	cmp.w	wk_drawing_marker_size_possibilities(a2),d0
	lbls	.ok2,2
	moveq	#1,d0
 label .ok2,2
	move.w	d0,d1
	mulu	wk_drawing_marker_size_width_min(a2),d0
	swap	d0
	mulu	wk_drawing_marker_size_height_min(a2),d1
	move.w	d1,d0
	move.l	d0,vwk_marker_size(a0)
	rts


	dc.b	0,"vsm_type",0
* vsm_type - Standard Trap function
* Todo: Get allowed markers.
* In:   a1      Parameter block
*       a0      VDI struct
vsm_type:
	move.l	intin(a1),a2
	move.w	(a2),d0
	lbeq	.not_ok,1
	move.l	vwk_real_address(a0),a2
	cmp.w	wk_drawing_marker_types(a2),d0		; # markers
	lbls	.ok,2
 label .not_ok,1
	moveq	#3,d0			; Asterisk
 label .ok,2
	move.w	d0,vwk_marker_type(a0)
	move.l	intout(a1),a2
	move.w	d0,(a2)
	done_return

* lib_vsm_type - Standard Library function
* Todo: ?
* In:	a1	Parameters   type_set = lib_vsm_type(type)
*	a0	VDI struct
_lib_vsm_type:
lib_vsm_type:
	move.w	(a1),d0
	lbeq	.not_ok,1
	move.l	vwk_real_address(a0),a2
	cmp.w	wk_drawing_marker_types(a2),d0		; # markers
	lbls	.ok,2
 label .not_ok,1
	moveq	#3,d0			; Asterisk
 label .ok,2
	move.w	d0,vwk_marker_type(a0)
	rts


	dc.b	0,"vqm_attributes",0
* vqm_attributes - Standard Trap function
* Todo: -
* In:   a1      Parameter block
*       a0      VDI struct
vqm_attributes:
	move.w	vwk_mode(a0),d0
	lea	vwk_marker(a0),a0	; a0 no longer -> VDI struct!
	movem.l	intout(a1),a1-a2
	move.w	(a0),(a1)+		; Type
	addq.l	#4,a0
	move.w	(a0)+,(a1)+		; Foreground
	move.w	d0,(a1)			; Mode
	move.l	(a0),(a2)		; Width, height
	done_return

* lib_vqm_attributes - Standard Library function
* Todo: -
* In:   a1      Parameters   width = lib_vqm_attributes(settings)
*       a0      VDI struct
lib_vqm_attributes:
	move.l	(a1),a1
	move.w	vwk_mode(a0),d0
	lea	vwk_marker(a0),a0	; a0 no longer -> VDI struct!
	move.w	(a0),(a1)+		; Type
	addq.l	#4,a0
	move.w	(a0)+,(a1)+		; Foreground
	move.w	d0,(a1)+		; Mode
	move.w	(a0)+,d0		; Width
	move.w	(a0),(a1)		; Height
	rts

	end
