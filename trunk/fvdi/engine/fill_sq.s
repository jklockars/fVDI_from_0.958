*****
* fVDI fill set/query functions
*
* Copyright 1997-2000, Johan Klockars 
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

;lattice		equ	1		; 1 - Assemble for DevPac/Lattice

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

	xref	_malloc

	xdef	vsf_color,vsf_interior,vsf_style,vsf_perimeter,vsf_udpat,vqf_attributes

	xdef	lib_vsf_color,lib_vsf_interior,lib_vsf_style,lib_vsf_perimeter,lib_vsf_udpat,lib_vqf_attributes
	xdef	_lib_vsf_color,_lib_vsf_interior,_lib_vsf_style


	text

	dc.b	0,0,"vsf_color",0
* vsf_color - Standard Trap function
* Todo: Get foreground colour?
* In:   a1      Parameter block
*       a0      VDI struct
vsf_color:
	move.l	intin(a1),a2
	move.w	(a2),d0
	move.l	vwk_real_address(a0),a2
	cmp.w	wk_screen_palette_size(a2),d0
;	blo	1$	; .ok
	lblo	.ok,1
	moveq	#BLACK,d0
;1$:			; .ok:
 label .ok,1
	move.w	d0,vwk_fill_colour_foreground(a0)
	move.l	intout(a1),a2
	move.w	d0,(a2)
	done_return

* lib_vsf_color - Standard Library function
* Todo: ?
* In:	a1	Parameters   colour_set = lib_vsf_color(colour)
*	a0	VDI struct
_lib_vsf_color:
lib_vsf_color:
	move.w	(a1),d0
	move.l	vwk_real_address(a0),a2
	cmp.w	wk_screen_palette_size(a2),d0
;	blo	1$	; .ok
	lblo	.ok,1
	moveq	#BLACK,d0
;1$:			; .ok:
 label .ok,1
	move.w	d0,vwk_fill_colour_foreground(a0)
	rts


	dc.b	0,"vsf_interior",0
* vsf_interior - Standard Trap function
* Todo: Get allowed types.
* In:   a1      Parameter block
*       a0      VDI struct
vsf_interior:
	move.l	intin(a1),a2
	move.w	(a2),d0
	cmp.w	#4,d0			; # fill types (not from wk struct?)
;	bls	1$	; .ok
	lbls	.ok,1
	moveq	#0,d0			; Hollow
;1$:			; .ok:
 label .ok,1
	move.w	d0,vwk_fill_interior(a0)
	move.l	intout(a1),a2
	move.w	d0,(a2)
	done_return

* lib_vsf_interior - Standard Library function
* Todo: ?
* In:	a1	Parameters   pattern_set = lib_vsf_interior(pattern)
*	a0	VDI struct
_lib_vsf_interior:
lib_vsf_interior:
	move.w	(a1),d0
	cmp.w	#4,d0			; # fill types (not from wk struct?)
;	bls	1$	; .ok
	lbls	.ok,1
	moveq	#0,d0			; Hollow
;1$:			; .ok:
 label .ok,1
	move.w	d0,vwk_fill_interior(a0)
	rts


	dc.b	0,0,"vsf_style",0
* vsf_style - Standard Trap function
* Todo: Get allowed styles.
* In:   a1      Parameter block
*       a0      VDI struct
vsf_style:
	move.l	intin(a1),a2
	move.w	(a2),d0
;	beq	1$	; .not_ok
	lbeq	.not_ok,1
	cmp.w	#24,d0			; # fill types (not from wk struct?)
;	bls	2$	; .ok		;   really 12/24 pattern/hatch
	lbls	.ok,2			;   really 12/24 pattern/hatch
;1$:			; .not_ok:
 label .not_ok,1
	moveq	#1,d0			; First
;2$:			; .ok:
 label .ok,2
	move.w	d0,vwk_fill_style(a0)
	move.l	intout(a1),a2
	move.w	d0,(a2)
	done_return

* lib_vsf_style - Standard Library function
* Todo: ?
* In:	a1	Parameters   index_set = lib_vsf_style(index)
*	a0	VDI struct
_lib_vsf_style:
lib_vsf_style:
	move.w	(a1),d0
;	beq	1$	; .not_ok
	lbeq	.not_ok,1
	cmp.w	#24,d0			; # fill types (not from wk struct?)
;	bls	2$	; .ok		;   really 12/24 pattern/hatch
	lbls	.ok,2			;   really 12/24 pattern/hatch
;1$:			; .not_ok:
 label .not_ok,1
	moveq	#1,d0			; First
;2$:			; .ok:
 label .ok,2
	move.w	d0,vwk_fill_style(a0)
	rts


	dc.b	0,0,"vsf_perimeter",0
* vsf_perimeter - Standard Trap function
* Todo: -
* In:   a1      Parameter block
*       a0      VDI struct
vsf_perimeter:
	move.l	intin(a1),a2
	move.w	(a2),d0
	cmp.w	#1,d0			; Only on/off (not from wk struct?)
;	bls	1$	; .ok
	lbls	.ok,1
	moveq	#0,d0			; Off
;1$:			; .ok:
 label .ok,1
	move.w	d0,vwk_fill_perimeter(a0)
	move.l	intout(a1),a2
	move.w	d0,(a2)
	done_return

* lib_vsf_perimeter - Standard Library function
* Todo: ?
* In:	a1	Parameters   vis_set = lib_vsf_perimeter(vis_flag)
*	a0	VDI struct
lib_vsf_perimeter:
	move.w	(a1),d0
	cmp.w	#1,d0			; Only on/off (not from wk struct?)
;	bls	1$	; .ok
	lbls	.ok,1
	moveq	#0,d0			; Off
;1$:			; .ok:
 label .ok,1
	move.w	d0,vwk_fill_perimeter(a0)
	rts


	dc.b	0,0,"vsf_udpat",0
* vsf_udpat - Standard Trap function
* Todo: -
* In:   a1      Parameter block
*       a0      VDI struct
vsf_udpat:
	uses_d1
	move.l	control(a1),a2
	move.w	6(a2),-(a7)
	move.l	intin(a1),-(a7)
	move.l	a7,a1
	bsr	lib_vsf_udpat
	addq.l	#6,a7
	used_d1
	done_return

* lib_vsf_udpat - Standard Library function
* Todo: ?
* In:	a1	Parameters   lib_vsf_udpat(pat_dat, planes)
*	a0	VDI struct
lib_vsf_udpat:
	move.l	(a1)+,a2
	move.w	(a1),d1			; Number of words
	moveq	#0,d0
	move.l	a0,a1
	add.l	#vwk_struct_size,a1
	cmp.w	#16,d1
	ble	.single_plane		; Actually only n*16 allowed
	move.l	vwk_fill_user_pattern_extra(a0),d0
	bne	.allocated

	movem.l	d1-d2/a0/a2,-(a7)
	ext.l	d1
	add.w	d1,d1
;	move.l	d1,-(a7)
;	move.w	#$48,-(a7)		; Malloc
;	trap	#1
;	addq.l	#6,a7
	move.l	#3,-(a7)
	move.l	d1,-(a7)
	bsr	_malloc
	addq.l	#8,a7
	movem.l	(a7)+,d1-d2/a0/a2
	tst.l	d0
	beq	.error_vsf_updat	; .error

.allocated:
	move.l	d0,a1
	moveq	#1,d0
.single_plane:
	move.w	d0,vwk_fill_user_multiplane(a0)
	move.l	a1,vwk_fill_user_pattern_in_use(a0)
	subq.w	#1,d1
;1$:			; .loop:
 label .loop,1
	move.w	(a2)+,(a1)+
;	dbra	d1,1$	; .loop
	ldbra	d1,.loop,1
	rts

.error_vsf_updat:	; .error:
	move.l	a0,a1
	add.l	#vwk_struct_size,a1
	moveq	#16,d1			; Well, what else is there?
	moveq	#0,d0
	bra	.single_plane


	dc.b	0,"vqf_attributes",0
* vqf_attributes - Standard Trap function
* Todo: -
* In:   a1      Parameter block
*       a0      VDI struct
vqf_attributes:
	move.w	vwk_mode(a0),d0
	lea	vwk_fill(a0),a0		; a0 no longer -> VDI struct!
	move.l	intout(a1),a2
	move.w	(a0),(a2)+		; Interior
	addq.w	#4,a0
	move.l	(a0)+,(a2)+		; Foreground, style
	move.w	d0,(a2)+		; Mode
	move.w	(a0)+,(a2)+		; Perimeter
	done_return

* lib_vqf_attributes - Standard Library function
* Todo: -
* In:   a1      Parameters   outlining = lib_vqf_attributes(settings)
*       a0      VDI struct
lib_vqf_attributes:
	move.l	(a1),a1
	move.w	vwk_mode(a0),d0
	lea	vwk_fill(a0),a0		; a0 no longer -> VDI struct!
	move.w	(a0),(a1)+		; Interior
	addq.l	#4,a0
	move.l	(a0)+,(a1)+		; Foreground, style
	move.w	d0,(a1)+		; Mode
	move.w	(a0),d0			; Perimeter
	rts

	end
