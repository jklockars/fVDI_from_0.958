*-------------------------------------------------------*
*	Draw in single plane modes			*	
*-------------------------------------------------------*

	include		"vdi.inc"

	xdef		_write_pixel
	xdef		_read_pixel

	xref		get_colour_masks


	text

*----------
* Set pixel
*----------
* In:	a1	VDI struct, destination MFDB (odd address marks table operation)
*	d0	colour
*	d1	x or table address
*	d2	y or table length (high) and type (0 - coordinates)
* XXX:	?
_write_pixel:
	move.l		a1,d3
	bclr		#0,d3
	bne		.unknown_write
	move.l		d3,a1

	bsr		get_colour_masks

	bsr		addressing

	move.l		d0,d2
	move.l		a2,d0
	and.l		d2,d0
	not.l		d2

	move.w		(a0),d3
	and.w		d2,d3
	or.w		d0,d3
	move.w		d3,(a0)+

	moveq		#1,d0
	rts

.unknown_write:
	moveq		#0,d0
	rts


*----------
* Get pixel
*----------
* In:	a1	VDI struct, source MFDB
*	d1	x
*	d2	y
_read_pixel:
	bsr		addressing

	move.w		d0,d3
	and.w		(a0)+,d3

	add.l		d3,d3
	lsl.l		d7,d3
	swap		d3
	move.w		d3,d0

	rts



addressing:
	move.l		4(a1),d0
	beq		.normal

	move.l		d0,a0
	move.l		mfdb_address(a0),d0
	beq		.normal
	move.w		mfdb_wdwidth(a0),d5
	move.w		mfdb_bitplanes(a0),d7
	move.l		(a1),a1
	move.l		vwk_real_address(a1),a1
	move.l		wk_screen_mfdb_address(a1),a0
	cmp.l		a0,d0
	beq		.xnormal

	move.l		d0,a0
	add.w		d5,d5
	mulu.w		d7,d5
.do_op:
	mulu.w		d5,d2
	add.l		d2,a0

	move.l		#$80008000,d0
	move.w		d1,d7
	and.w		#$0f,d7
	lsr.l		d7,d0
	lsr.w		#4,d1
	lsl.w		#1,d1
	add.w		d1,a0
	rts
	
.normal:
	move.l		(a1),a1
	move.l		vwk_real_address(a1),a1

	move.l		wk_screen_mfdb_address(a1),a0

.xnormal:
	move.w		wk_screen_wrap(a1),d5
	bra		.do_op


	end
