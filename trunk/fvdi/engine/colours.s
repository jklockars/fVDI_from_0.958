*****
* fVDI colour functions
*
* $Id: colours.s,v 1.5 2005-07-26 21:40:08 johan Exp $
*
* Copyright 1997-2000, Johan Klockars 
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

transparent	equ	1		; Fall through?

neg_pal_n	equ	9		; Number of negative palette entries

	include	"vdi.inc"
	include	"macros.inc"

	xref	_malloc,_free
	xref	redirect

	xdef	vs_fg_color,vs_bg_color,vq_fg_color,vq_bg_color
	xdef	vs_x_color,vq_x_color
	xdef	vs_color,vq_color

;	xdef	lib_vsf_color,lib_vsf_interior,lib_vsf_style,lib_vsf_perimeter,lib_vsf_udpat,lib_vqf_attributes
	xdef	lib_vs_color


	text

	dc.b	0,0,"vs_fg_color",0
* vs_fg_color - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vs_fg_color:
	uses_d1
	move.l	a3,-(a7)
	move.l	intin(a1),a2
	move.l	0(a2),d0		; Colour space, don't check for now
	move.l	control(a1),a2
	moveq	#-1,d0
	move.w	subfunction(a2),d1
	cmp.w	#4,d1
	lbhi	.end,2
	add.w	d1,d1
	lea	fg_offset,a2
	move.w	0(a2,d1),d0
	neg.w	d1
	subq.w	#2,d1			; Index -2/-4/...
	move.w	d1,0(a0,d0)

	movem.l	a1,-(a7)
	moveq	#1,d0			; One colour to set up
	swap	d0
	move.w	d1,d0
	move.l	intin(a1),a1
	addq.l	#4,a1
	addq.l	#1,a1			; Odd for new style entries
	move.l	vwk_palette(a0),d1
	lbne	.exists,1

	move.l	d0,a3
	movem.l	d2/a0-a2,-(a7)
	move.l	#3,-(a7)
	move.l	#neg_pal_n*colour_struct_size,-(a7)	; Only negative indices
	jsr	_malloc
	addq.l	#8,a7
	movem.l	(a7)+,d2/a0-a2

	move.l	d0,d1
	moveq	#-1,d0
	tst.l	d1
	lbeq	.end,2
	add.l	#neg_pal_n*colour_struct_size+1,d1
	move.l	d1,vwk_palette(a0)
	move.l	a3,d0

 label .exists,1
	and.w	#$fffe,d1
	move.l	vwk_real_address(a0),a2
	move.l	wk_r_set_palette(a2),a3
	move.l	d1,a2
	jsr	(a3)
	movem.l	(a7)+,a1

	moveq	#1,d0
 label .end,2
	move.l	intout(a1),a2
	move.w	d0,(a2)
	move.l	(a7)+,a3
	used_d1
	done_return


	dc.b	0,0,"vs_bg_color",0
* vs_bg_color - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vs_bg_color:
	uses_d1
	move.l	a3,-(a7)
	move.l	intin(a1),a2
	move.l	0(a2),d0		; Colour space, don't check for now
	move.l	control(a1),a2
	moveq	#-1,d0
	move.w	subfunction(a2),d1
	cmp.w	#4,d1
	lbhi	.end,2
	add.w	d1,d1
	lea	bg_offset,a2
	move.w	0(a2,d1),d0
	neg.w	d1
	subq.w	#3,d1			; Index -3/-5/...
	move.w	d1,0(a0,d0)

	movem.l	a1,-(a7)
	moveq	#1,d0			; One colour to set up
	swap	d0
	move.w	d1,d0
	move.l	intin(a1),a1
	addq.l	#4,a1
	addq.l	#1,a1			; Odd for new style entries
	move.l	vwk_palette(a0),d1
	lbne	.exists,1

	move.l	d0,a3
	movem.l	d2/a0-a2,-(a7)
	move.l	#3,-(a7)
	move.l	#neg_pal_n*colour_struct_size,-(a7)	; Only negative inidices
	jsr	_malloc
	addq.l	#8,a7
	movem.l	(a7)+,d2/a0-a2

	move.l	d0,d1
	moveq	#-1,d0
	tst.l	d1
	lbeq	.end,2
	add.l	#neg_pal_n*colour_struct_size+1,d1
	move.l	d1,vwk_palette(a0)
	move.l	a3,d0

 label .exists,1
	and.w	#$fffe,d1
	move.l	vwk_real_address(a0),a2
	move.l	wk_r_set_palette(a2),a3
	move.l	d1,a2
	jsr	(a3)
	movem.l	(a7)+,a1

	moveq	#1,d0
 label .end,2
	move.l	intout(a1),a2
	move.w	d0,(a2)
	move.l	(a7)+,a3
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
	move.l	control(a1),a2
	move.w	subfunction(a2),d1
	cmp.w	#4,d1
	lbhi	.error,3
	add.w	d1,d1
	lea	fg_offset,a2
	move.w	0(a2,d1),d1
	move.w	0(a0,d1),d0

	move.l	vwk_real_address(a0),a2
	mulu	#colour_struct_size,d0
	move.l	vwk_palette(a0),d2
	lbne	.local_palette,1
	move.l	wk_screen_palette_colours(a2),d2
 label .local_palette,1
	bclr	#0,d2
	move.l	d2,a0			; a0 no longer -> VDI
	lbne	.neg_palette,4
 label .normal_palette,2
	add.w	d0,a0

	move.l	intout(a1),a2
	move.l	#1,(a2)+		; RGB_SPACE
	move.w	#0,(a2)+		; Reserved
	move.l	(a0)+,(a2)+		; RGB
	move.w	(a0)+,(a2)+
	done_return

 label .error,3
	move.l	intout(a1),a2
	move.w	#-1,(a2)
	done_return

 label .neg_palette,4		; Sometimes only the negative palette is local
	tst.w	d0
	lbmi	.normal_palette,2
	move.l	wk_screen_palette_colours(a2),a0
	lbra	.normal_palette,2
	

	dc.b	0,0,"vq_bg_color",0
* vq_bg_color - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vq_bg_color:
	move.l	control(a1),a2
	move.w	subfunction(a2),d1
	cmp.w	#4,d1
	lbhi	.error,3
	add.w	d1,d1
	lea	bg_offset,a2
	move.w	0(a2,d1),d1
	move.w	0(a0,d1),d0

	move.l	vwk_real_address(a0),a2
	mulu	#colour_struct_size,d0
	move.l	vwk_palette(a0),d2
	lbne	.local_palette,1
	move.l	wk_screen_palette_colours(a2),d2
 label .local_palette,1
	bclr	#0,d2
	move.l	d2,a0			; a0 no longer -> VDI
	lbne	.neg_palette,4		; Only part local?
 label .normal_palette,2
	add.w	d0,a0

	move.l	intout(a1),a2
	move.l	#1,(a2)+		; RGB_SPACE
	move.w	#0,(a2)+		; Reserved
	move.l	(a0)+,(a2)+		; RGB
	move.w	(a0)+,(a2)+
	done_return

 label .error,3
	move.l	intout(a1),a2
	move.w	#-1,(a2)
	done_return

 label .neg_palette,4			; Sometimes only the negative palette is local
	tst.w	d0
	lbmi	.normal_palette,2
	move.l	wk_screen_palette_colours(a2),a0
	lbra	.normal_palette,2


	dc.b	0,"vq_color",0
* vq_color - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vq_color:
	uses_d1
	movem.l	d2/a0-a1,-(a7)
	move.l	intin(a1),a2
	movem.w	(a2),d0-d1		; Both colour index and inquiry mode
	move.l	intout(a1),a2

	move.l	vwk_real_address(a0),a1
	move.w	wk_screen_palette_size(a1),d2
	cmp.w	d2,d0
	bcc	.invalid_index	;bhs
	move.w	d0,(a2)+

	move.l	wk_driver(a1),a1
	move.l	driver_device(a1),a1
	move.w	dev_clut(a1),d2
	move.l	vwk_real_address(a0),a1
	cmp.w	#1,d2			; Hardware CLUT? (used to test look_up_table)
	bne	.colour_ok		; No VDI->TOS conversion for true colour
	cmp.w	#16,d0
	blo	.lookup			; 0-15
	cmp.w	#255,d0
	bne	.colour_ok		; 16-254
	moveq	#15,d0
	bra	.colour_ok
.lookup:
	move.l	a2,d2
	lea	tos_colours,a2
	move.b	0(a2,d0),d0
	move.l	d2,a2
  ifne 1
	bpl	.colour_ok	; Value for VDI colour 1 differs (marked -1)
	move.w	wk_screen_palette_size(a1),d0
	subq.w	#1,d0
  endc
  ifne 0
	move.w	wk_screen_palette_size(a1),d2
	subq.w	#1,d2
	and.w	d2,d0		; Takes care of VDI colour 1 -> pal_size-1
  endc
.colour_ok:
	
	mulu	#colour_struct_size,d0
	move.l	vwk_palette(a0),d2
	bne	.local_palette
	move.l	wk_screen_palette_colours(a1),d2
.local_palette:
	bclr	#0,d2
	move.l	d2,a0			; a0 no longer -> VDI
	bne	.neg_palette		; Only part local?
.normal_palette:
	lea	colour_vdi(a0,d0.w),a0

	tst.w	d1
	beq	.requested_values
	add.w	#colour_hw-colour_vdi,a0
.requested_values:	
	move.l	(a0)+,(a2)+
	move.w	(a0)+,(a2)+

.end_vq_color:		; .end:
	movem.l	(a7)+,d2/a0-a1
	used_d1
	move.l	vwk_real_address(a0),a2
	move.l	wk_driver(a2),a2
	move.l	driver_device(a2),a2
	move.w	dev_format(a2),d0
	and.w	#2,d0
	beq	redirect		; Don't redirect for non-standard modes (and then only temporary) (needs a1)
	done_return

.neg_palette:				; Sometimes only the negative palette is local
	tst.w	d0			; d0 really should never be negative here
	bmi	.normal_palette
	move.l	wk_screen_palette_colours(a1),a0
	bra	.normal_palette

.invalid_index:
	move.w	#-1,(a2)
	bra	.end_vq_color	; .end


	dc.b	0,"vs_color",0
* vs_color - Standard Trap function
* Todo: ?
* In:   a1      Parameter block
*       a0      VDI struct
vs_color:
	uses_d1
	movem.l	a0-a1,-(a7)
	move.l	intin(a1),a2
	pea	2(a2)
	move.w	(a2),-(a7)
	move.l	a7,a1
	bsr	lib_vs_color
	addq.l	#6,a7
	movem.l	(a7)+,a0-a1
	used_d1
	move.l	vwk_real_address(a0),a2
	move.l	wk_driver(a2),a2
	move.l	driver_device(a2),a2
	move.w	dev_format(a2),d0
	and.w	#2,d0
	beq	redirect		; Don't redirect for non-standard modes (and then only temporary) (needs a1)
	done_return

 ifne 0
.error_vs_color:	; .error:
	move.l	a1,d1			; If no memory for local palette,
	bra	.error_bypass		;   modify in global (BAD!)
 endc

* lib_vs_color - Standard Library function
* Todo: ?
* In:	a1	Parameters   lib_vs_color(pen, rgb)
*	a0	VDI struct
lib_vs_color:
	movem.l	a3,-(a7)

	moveq	#1,d0			; One colour to set up
	swap	d0
	move.w	(a1)+,d0
	move.l	(a1),a1
	move.l	vwk_real_address(a0),a2
	cmp.w	wk_screen_palette_size(a2),d0
	bcc	.end_vs_color	; bhs	.end

	move.l	vwk_palette(a0),d1
	bne	.local_palette_vs_color	; .local_palette
	move.l	wk_driver(a2),a2
	move.l	driver_device(a2),a2
	move.w	dev_clut(a2),d1
	move.l	vwk_real_address(a0),a2
	cmp.w	#1,d1			; Hardware CLUT? (used to test look_up_table)
	bne	.allocate_palette
	move.l	wk_screen_palette_colours(a2),d1
	bra	.palette_ok		; .local_palette	; Well, it's actually a common global

.allocate_palette:
	movem.l	d0/a1,-(a7)

	move.w	wk_screen_palette_size(a2),d0
	add.w	#neg_pal_n,d0		; Make room for negative indices
	mulu	#colour_struct_size,d0	; Bytes for local colour index table

	movem.l	d2/a0/a2,-(a7)
	move.l	#3,-(a7)
	move.l	d0,-(a7)
	jsr	_malloc
	addq.l	#8,a7
	movem.l	(a7)+,d2/a0/a2

	move.l	wk_screen_palette_colours(a2),a1
	tst.l	d0
	beq	.error_vs_color	; .error
;	move.l	d0,d1
	add.l	#neg_pal_n*colour_struct_size,d0	; Point to index 0
	move.l	d0,vwk_palette(a0)
	move.w	wk_screen_palette_size(a2),d1
;	add.w	#neg_pal_n,d1		; Don't forget the negative indices
	subq.w	#1,d1
	move.l	d0,a3			; Make a local copy of the default palette
.loop2:
	move.l	(a1)+,(a3)+		; Assume Colour is 16 bytes large
	move.l	(a1)+,(a3)+
	move.l	(a1)+,(a3)+
	move.l	(a1)+,(a3)+
	dbra	d1,.loop2

	move.l	d0,d1
.error_bypass:
	movem.l	(a7)+,d0/a1

.palette_ok:	; .local_palette:
	move.l	wk_r_set_palette(a2),a3
	move.l	d1,a2
	jsr	(a3)

.end_vs_color:		; .end:
	movem.l	(a7)+,a3
	rts

.local_palette_vs_color:
	bclr	#0,d1
	beq	.palette_ok
	move.l	wk_driver(a2),a2
	move.l	driver_device(a2),a2
	move.w	dev_clut(a2),d1
	move.l	vwk_real_address(a0),a2
	cmp.w	#1,d1			; Hardware CLUT? (used to test look_up_table)
	bne	.no_hardware_clut
	move.l	wk_screen_palette_colours(a2),d1
	bra	.palette_ok		; .local_palette	; Well, it's actually a common global

.no_hardware_clut:
	movem.l	d0/a1,-(a7)

	move.w	wk_screen_palette_size(a2),d0
	add.w	#neg_pal_n,d0		; Make room for negative indices
	mulu	#colour_struct_size,d0	; Bytes for local colour index table

	movem.l	d1-d2/a0/a2,-(a7)
	move.l	#3,-(a7)
	move.l	d0,-(a7)
;	bsr	_malloc
	jsr	_malloc
	addq.l	#8,a7
	movem.l	(a7)+,d1-d2/a0/a2

	move.l	wk_screen_palette_colours(a2),a1
	tst.l	d0
	beq	.error_vs_color	; .error

	move.l	d0,a3
	sub.l	#neg_pal_n*colour_struct_size,d1
	move.l	d1,a1
	moveq	#neg_pal_n-1,d0
 label .loop3,1
	move.l	(a1)+,(a3)+		; Assume Colour is 16 bytes large
	move.l	(a1)+,(a3)+
	move.l	(a1)+,(a3)+
	move.l	(a1)+,(a3)+
	ldbra	d0,.loop3,1

	movem.l	d2/a0/a2,-(a7)
	move.l	d1,-(a7)
	jsr	_free
	addq.l	#4,a7
	movem.l	(a7)+,d2/a0/a2

	move.l	a3,vwk_palette(a0)
	move.l	wk_screen_palette_colours(a2),a1
	move.w	wk_screen_palette_size(a2),d1
	subq.w	#1,d1
 label .loop4,2
	move.l	(a1)+,(a3)+		; Assume Colour is 16 bytes large
	move.l	(a1)+,(a3)+
	move.l	(a1)+,(a3)+
	move.l	(a1)+,(a3)+
	ldbra	d1,.loop4,2

	move.l	vwk_palette(a0),d1
	movem.l	(a7)+,d0/a1

	bra	.palette_ok

.error_vs_color:	; .error:
	move.l	a1,d1			; If no memory for local palette,
	bra	.error_bypass		;   modify in global (BAD!)


	data

tos_colours:
	dc.b	0,-1,1,2,4,6,3,5,7,8,9,10,12,14,11,13

fg_offset:
	dc.w	vwk_text_colour_foreground,vwk_fill_colour_foreground,vwk_line_colour_foreground
	dc.w	vwk_marker_colour_foreground,-1		; The last will be for bitmaps

bg_offset:
	dc.w	vwk_text_colour_background,vwk_fill_colour_background,vwk_line_colour_background
	dc.w	vwk_marker_colour_background,-1		; The last will be for bitmaps

	end
