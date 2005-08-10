*****
* fVDI colour functions
*
* $Id: colours.s,v 1.8 2005-08-10 10:08:00 johan Exp $
*
* Copyright 1997-2000, Johan Klockars 
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

transparent	equ	1		; Fall through?

	include	"vdi.inc"
	include	"macros.inc"

	xref	redirect
	xref	_lib_vs_color,_lib_vq_color,_lib_vs_fg_color,_lib_vs_bg_color
	xref	_lib_vq_fg_color,_lib_vq_bg_color

	xdef	vs_fg_color,vs_bg_color,vq_fg_color,vq_bg_color
	xdef	vs_x_color,vq_x_color
	xdef	vs_color,vq_color
	xdef	_set_palette


	text

* Intermediate function for calling driver from C code.
* Will go away when the driver interface is updated.
* void set_palette(Virtual *vwk, DrvPalette *palette_pars)
_set_palette:
	movem.l	d2/a2,-(a7)
	move.l	2*4+4(a7),a0
	move.l	2*4+8(a7),a1

	move.l	a1,-(a7)
	move.l	a0,-(a7)
	move.w	#$c0de,d0
	move.l	vwk_real_address(a0),a2
	move.l	wk_r_set_palette(a2),a2
	jsr	(a2)
	addq.l	#8,a7
	movem.l	(a7)+,d2/a2
	rts

	
	dc.b	0,0,"vs_fg_color",0
* vs_fg_color - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vs_fg_color:
	uses_d1
	movem.l	d2/a1,-(a7)

	move.l	intin(a1),a2
	pea	4(a2)
	move.l	(a2),-(a7)
	move.l	control(a1),a2
	move.w	subfunction(a2),d0
	ext.l	d0
	move.l	d0,-(a7)
	move.l	a0,-(a7)
	jsr	_lib_vs_fg_color
	add.w	#16,a7
	
	movem.l	(a7)+,d2/a1
	move.l	intout(a1),a2
	move.w	d0,(a2)
	used_d1
	done_return
	

	dc.b	0,0,"vs_bg_color",0
* vs_bg_color - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vs_bg_color:
	uses_d1
	movem.l	d2/a1,-(a7)

	move.l	intin(a1),a2
	pea	4(a2)
	move.l	(a2),-(a7)
	move.l	control(a1),a2
	move.w	subfunction(a2),d0
	ext.l	d0
	move.l	d0,-(a7)
	move.l	a0,-(a7)
	jsr	_lib_vs_bg_color
	add.w	#16,a7
	
	movem.l	(a7)+,d2/a1
	move.l	intout(a1),a2
	move.w	d0,(a2)
	used_d1
	done_return
	

	dc.b	0,"vs_x_color",0
* vs_x_color - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vs_x_color:
	moveq	#1,d0
	move.l	intout(a1),a2
	move.w	d0,(a2)
	done_return


	dc.b	0,"vq_x_color",0
* vq_x_color - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vq_x_color:
	move.l	intout(a1),a2
	move.l	#1,(a2)+		; RGB_SPACE
	move.w	#0,(a2)+		; Reserved
	move.l	#0,(a2)+		; RGB
	move.w	#0,(a2)+
	done_return


	dc.b	0,0,"vq_fg_color",0
* vq_fg_color - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vq_fg_color:
	uses_d1
	movem.l	d2/a1,-(a7)

	move.l	intout(a1),a2
	pea	4(a2)
	move.l	control(a1),a2
	move.w	subfunction(a2),d0
	ext.l	d0
	move.l	d0,-(a7)
	move.l	a0,-(a7)
	jsr	_lib_vq_fg_color
	add.w	#12,a7
	
	movem.l	(a7)+,d2/a1
	move.l	intout(a1),a2
	move.l	d0,(a2)
	used_d1
	done_return


	dc.b	0,0,"vq_bg_color",0
* vq_bg_color - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vq_bg_color:
	uses_d1
	movem.l	d2/a1,-(a7)

	move.l	intout(a1),a2
	pea	4(a2)
	move.l	control(a1),a2
	move.w	subfunction(a2),d0
	ext.l	d0
	move.l	d0,-(a7)
	move.l	a0,-(a7)
	jsr	_lib_vq_bg_color
	add.w	#12,a7
	
	movem.l	(a7)+,d2/a1
	move.l	intout(a1),a2
	move.l	d0,(a2)
	used_d1
	done_return


	dc.b	0,"vq_color",0
* vq_color - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vq_color:
	uses_d1
	movem.l	d2/a0-a1,-(a7)

	move.l	intout(a1),a2
	pea	2(a2)
	move.l	intin(a1),a2
	move.w	2(a2),d0
	ext.l	d0
	move.l	d0,-(a7)
	move.w	(a2),d0
	ext.l	d0
	move.l	d0,-(a7)
	move.l	a0,-(a7)
	jsr	_lib_vq_color
	add.w	#16,a7
	
	movem.l	(a7)+,d2/a0-a1
	move.l	intout(a1),a2
	move.w	d0,(a2)
	used_d1
	move.l	vwk_real_address(a0),a2
	move.l	wk_driver(a2),a2
	move.l	driver_device(a2),a2
	move.w	dev_format(a2),d0
	and.w	#2,d0
	beq	redirect	; Don't redirect for non-standard modes (and then only temporary) (needs a1)
	done_return
	

	dc.b	0,"vs_color",0
* vs_color - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vs_color:
	uses_d1
	movem.l	d2/a0-a1,-(a7)
	move.l	intin(a1),a2
	pea	2(a2)
	move.w	(a2),d0
	ext.l	d0
	move.l	d0,-(a7)
	move.l	a0,-(a7)
	jsr	_lib_vs_color
	add.w	#12,a7
	movem.l	(a7)+,d2/a0-a1
	used_d1
	move.l	vwk_real_address(a0),a2
	move.l	wk_driver(a2),a2
	move.l	driver_device(a2),a2
	move.w	dev_format(a2),d0
	and.w	#2,d0
	beq	redirect		; Don't redirect for non-standard modes (and then only temporary) (needs a1)
	done_return

	end
