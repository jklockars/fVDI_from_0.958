*****
* FenixVDI line set/query functions
*
* Copyright 1997-2000, Johan Klockars 
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

transparent	equ	1		; Fall through?

	include	"vdi.inc"

*
* Macros
*
  ifne lattice
	include	"macros.dev"
  else
	include	"macros.tas"
  endc

	xdef	vsl_color,vsl_width,vsl_type,vsl_udsty,vsl_ends,vql_attributes

	xdef	lib_vsl_color,lib_vsl_width,lib_vsl_type,lib_vsl_udsty,lib_vsl_ends,lib_vql_attributes
	xdef	_lib_vsl_color,_lib_vsl_type
	xdef	v_bez_con


	text

	dc.b	0,0,"vsl_color",0
* vsl_color - Standard Trap function
* Todo: Get foreground colour?
* In:   a1      Parameter block
*       a0      VDI struct
vsl_color:
	move.l	intin(a1),a2
	move.w	(a2),d0
	move.l	vwk_real_address(a0),a2
	cmp.w	wk_screen_palette_size(a2),d0
	lblo	.ok,1
	moveq	#BLACK,d0
 label .ok,1
	move.w	d0,vwk_line_colour_foreground(a0)
	move.l	intout(a1),a2
	move.w	d0,(a2)
	done_return

* lib_vsl_color - Standard Library function
* Todo: ?
* In:	a1	Parameters   colour_set = lib_vsl_color(colour)
*	a0	VDI struct
_lib_vsl_color:
lib_vsl_color:
	move.w	(a1),d0
	move.l	vwk_real_address(a0),a2
	cmp.w	wk_screen_palette_size(a2),d0
	lblo	.ok,1
	moveq	#BLACK,d0
 label .ok,1
	move.w	d0,vwk_line_colour_foreground(a0)
	rts


	dc.b	0,0,"vsl_width",0
* vsl_width - Standard Trap function
* Todo: Get allowed widths.
* In:   a1      Parameter block
*       a0      VDI struct
vsl_width:
	move.l	ptsin(a1),a2
	move.w	(a2),d0

	ifne 0
	subq.w	#1,d0			; Allow odd widths
	and.w	#$00fe,d0		;   up to 255
	addq.w	#1,d0
	endc

	move.l	vwk_real_address(a0),a2

  ifne 1			; Desktop (7?) and Kandinsky (negative) uses strange numbers
	cmp.w	wk_drawing_line_wide_width_max(a2),d0
	lble	.ok1,2
	move.w	wk_drawing_line_wide_width_max(a2),d0
 label .ok1,2
	cmp.w	wk_drawing_line_wide_width_min(a2),d0
	lbge	.ok2,3
	move.w	wk_drawing_line_wide_width_min(a2),d0
 label .ok2,3
  endc

	tst.w   wk_drawing_line_wide_width_possibilities(a2)
	lbeq	.ok,1
	nop				; What if not continuous?
 label .ok,1
	move.w	d0,vwk_line_width(a0)
	move.l	ptsout(a1),a2
	move.w	d0,(a2)+
	move.w	#0,(a2)			; Why?
	done_return

* lib_vsl_width - Standard Library function
* Todo: ?
* In:	a1	Parameters   width_set = lib_vsl_width(width)
*	a0	VDI struct
lib_vsl_width:
	move.w	(a1),d0

  ifne 0
	subq.w	#1,d0			; Allow odd widths
	and.w	#$00fe,d0		;   up to 255
	addq.w	#1,d0
  endc

	move.l	vwk_real_address(a0),a2

  ifne 1
	cmp.w	wk_drawing_line_wide_width_max(a2),d0
	lble	.ok1,2
	move.w	wk_drawing_line_wide_width_max(a2),d0
 label .ok1,2
	cmp.w	wk_drawing_line_wide_width_min(a2),d0
	lbge	.ok2,3
	move.w	wk_drawing_line_wide_width_min(a2),d0
 label .ok2,3
  endc

	tst.w   wk_drawing_line_wide_width_possibilities(a2)
	lbeq	.ok,1
	nop				; What if not continuous?
 label .ok,1
	move.w	d0,vwk_line_width(a0)
	rts


	dc.b	0,"vsl_type",0
* vsl_type - Standard Trap function
* Todo: Get allowed line types.
* In:   a1      Parameter block
*       a0      VDI struct
vsl_type:
	move.l	intin(a1),a2
	move.w	(a2),d0
	lbeq	.not_ok,1
	move.l	vwk_real_address(a0),a2
	cmp.w	wk_drawing_line_types(a2),d0	; # line types
	lbls	.ok,2
 label .not_ok,1
	moveq	#1,d0			; Solid
 label .ok,2
	move.w	d0,vwk_line_type(a0)
	move.l	intout(a1),a2
	move.w	d0,(a2)
	done_return

* lib_vsl_type - Standard Library function
* Todo: ?
* In:	a1	Parameters   type_set = lib_vsl_type(type)
*	a0	VDI struct
_lib_vsl_type:
lib_vsl_type:
	move.w	(a1),d0
	lbeq	.not_ok,1
	move.l	vwk_real_address(a0),a2
	cmp.w	wk_drawing_line_types(a2),d0	; # line types
	lbls	.ok,2
 label .not_ok,1
	moveq	#1,d0			; Solid
 label .ok,2
	move.w	d0,vwk_line_type(a0)
	rts


	dc.b	0,0,"vsl_udsty",0
* vsl_udsty - Standard Trap function
* Todo: Get foreground colour?
* In:   a1      Parameter block
*       a0      VDI struct
vsl_udsty:
	move.l	intin(a1),a2
	move.w	(a2),vwk_line_user_mask(a0)
	done_return

* lib_vsl_udsty - Standard Library function
* Todo: ?
* In:	a1	Parameters   lib_vsl_udsty(pattern)
*	a0	VDI struct
lib_vsl_udsty:
	move.w	(a1),d0
	move.w	d0,vwk_line_user_mask(a0)
	rts


	dc.b	0,"vsl_ends",0
* vsl_ends - Standard Trap function
* Todo: -
* In:   a1      Parameter block
*       a0      VDI struct
vsl_ends:
	uses_d1
	move.l	vwk_real_address(a0),a2
	move.w	wk_drawing_line_wide_types_possible(a2),d1
	move.l	intin(a1),a2
	move.w	(a2)+,d0
	cmp.w	d1,d0			; # end styles
	lbls	.ok1,1
	moveq	#0,d0			; Squared
 label .ok1,1
	swap	d0
	move.w	(a2),d0
	cmp.w	d1,d0
	lbls	.ok2,2
	move.w	#0,d0			; Squared
 label .ok2,2
	move.l	d0,vwk_line_ends(a0)
	used_d1
	done_return

* lib_vsl_ends - Standard Library function
* Todo: ?
* In:	a1	Parameters   colour_set = lib_vsl_ends(begin, end)
*	a0	VDI struct
lib_vsl_ends:
	move.w	(a1)+,d0
	move.l	vwk_real_address(a0),a2
	move.w	wk_drawing_line_wide_types_possible(a2),d1
	cmp.w	d1,d0			; # end styles
	lbls	.ok1,1
	moveq	#0,d0			; Squared
 label .ok1,1
	swap	d0
	move.w	(a1),d0
	cmp.w	d1,d0
	lbls	.ok2,2
	move.w	#0,d0			; Squared
 label .ok2,2
	move.l	d0,vwk_line_ends(a0)
	rts


	dc.b	0,"vql_attributes",0
* vql_attributes - Standard Trap function
* Todo: -
* In:   a1      Parameter block
*       a0      VDI struct
vql_attributes:
	move.w	vwk_mode(a0),d0
	lea	vwk_line(a0),a0		; a0 no longer -> VDI struct!
	movem.l	intout(a1),a1-a2	; Get ptsout too
	move.w	(a0),(a1)+		; Type
	addq.l	#4,a0
	move.w	(a0)+,(a1)+		; Foreground
	move.w	d0,(a1)+		; Mode
	move.l	(a0)+,(a1)		; End styles
	move.w	(a0)+,(a2)+		; Width
	clr.w	(a2)
	done_return

* lib_vql_attributes - Standard Library function
* Todo: -
* In:   a1      Parameters   end_styles = lib_vql_attributes(settings)
*       a0      VDI struct
lib_vql_attributes:
	move.l	(a1),a1
	move.w	vwk_mode(a0),d0
	lea	vwk_line(a0),a0		; a0 no longer -> VDI struct!
	move.w	(a0),(a1)+		; Type
	addq.l	#4,a0
	move.w	(a0)+,(a1)+		; Foreground
	move.w	d0,(a1)+		; Mode
	move.l	(a0)+,d0		; End styles
	move.w	(a0),(a1)		; Width
	rts


	dc.b	0,0,"v_bez_con",0
* v_bez_con - Standard Trap function
* Todo: -
* In:   a1      Parameter block
*       a0      VDI struct
v_bez_con:
	move.l	control(a1),a2
	tst.w	L_ptsin(a2)
	beq		.v_bez_off
	move.w	#1,vwk_bezier_on(a0)
	move.w	#1,L_intout(a2)
	move.w	vwk_real_address(a0),a2
	move.w	wk_drawing_bezier_depth_scale_max(a2),d0
	move.w	d0,vwk_bezier_depth_scale(a0)
	move.w	wk_drawing_bezier_depth_max(a2),d0
	move.l	intout(a1),a2
	move.w	d0,0(a2)
.v_bez_end:
	done_return
.v_bez_off:
	move.w	#0,vwk_bezier_on(a0)
	bra	.v_bez_end

	end
