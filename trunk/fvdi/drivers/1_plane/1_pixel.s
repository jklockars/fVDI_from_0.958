*****
* Single plane pixel get/set
*
* $Id: 1_pixel.s,v 1.3 2005-04-23 18:57:52 johan Exp $
*
* Copyright 1997-2002, Johan Klockars 
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

both		equ	1	; Write in both FastRAM and on screen
longs		equ	1
get		equ	1
mul		equ	1	; Multiply rather than use table
shift		equ	1

	include		"pixelmac.inc"
	include		"vdi.inc"

	xdef		_write_pixel
	xdef		_read_pixel

	xref		get_colour_masks

	ifeq		shift
	xref		dot,lline,rline
	endc
	ifeq		mul
	xref		row
	endc


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
	mulu.w		d5,d2
	add.l		d2,a0

.no_shadow:
	move.l		#$80008000,d0
	move.w		d1,d7
	and.w		#$0f,d7
	lsr.l		d7,d0
	lsr.w		#4,d1
	lsl.w		#1,d1
	add.w		d1,a0
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
	
.normal:
	move.l		(a1),a1
	move.l		vwk_real_address(a1),a1

	ifne	mul
	move.l		wk_screen_mfdb_address(a1),a0
	endc

.xnormal:
	ifne	mul
	move.w		wk_screen_wrap(a1),d5
	mulu.w		d5,d2
	add.l		d2,a0
;	 ifne	both
;	move.l		wk_screen_shadow_address(a1),a4
;	add.l		d2,a4
;	 endc
	endc
	ifeq	mul
	lea		row(pc),a0
	move.l		(a0,d2.w*4),a0
	endc

	ifne	both
;	move.l		wk_screen_shadow_address(a1),a4
	move.l		wk_screen_shadow_address(a1),d7
	beq		.no_shadow
	move.l		d7,a4
	add.l		d2,a4
	endc

	move.l		#$80008000,d0
	move.w		d1,d7
	and.w		#$0f,d7
	lsr.l		d7,d0
	lsr.w		#4,d1
	lsl.w		#1,d1
	add.w		d1,a0
	ifne	both
	add.w		d1,a4
	endc
	move.l		d0,d2
	move.l		a2,d0
	and.l		d2,d0
	not.l		d2

	ifeq	both
	move.w		(a0),d3
	endc
	ifne	both
	move.w		(a4),d3
	endc
	and.w		d2,d3
	or.w		d0,d3
	ifeq	both
	move.w		d3,(a0)+
	endc
	ifne	both
	move.w		d3,(a4)+
	move.w		d3,(a0)+
	endc

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
	mulu.w		d5,d2
	add.l		d2,a0

.no_shadow:
	move.l		#$80008000,d0
	move.w		d1,d7
	and.w		#$0f,d7
	lsr.l		d7,d0
	lsr.w		#4,d1
	lsl.w		#1,d1
	add.w		d1,a0

	move.w		d0,d3
	and.w		(a0)+,d3

	add.l		d3,d3
	lsl.l		d7,d3
	swap		d3
	move.w		d3,d0

	rts

.normal:
	move.l		(a1),a1
	move.l		vwk_real_address(a1),a1

	ifne	mul
	move.l		wk_screen_mfdb_address(a1),a0
	endc

.xnormal:
	ifne	mul
	move.w		wk_screen_wrap(a1),d5
	mulu.w		d5,d2
	add.l		d2,a0
;	 ifne	both
;	move.l		wk_screen_shadow_address(a1),a4
;	add.l		d2,a4
;	 endc
	endc
	ifeq	mul
	lea		row(pc),a0
	move.l		(a0,d2.w*4),a0
	endc

	ifne	both
;	move.l		wk_screen_shadow_address(a1),a4
	move.l		wk_screen_shadow_address(a1),d7
	beq		.no_shadow
	move.l		d7,a4
	add.l		d2,a4
	endc

	move.l		#$80008000,d0
	move.w		d1,d7
	and.w		#$0f,d7
	lsr.l		d7,d0
	lsr.w		#4,d1
	lsl.w		#1,d1
	add.w		d1,a0
	ifne	both
	add.w		d1,a4
	endc

	ifeq	both
	move.w		d0,d3
	and.w		(a0)+,d3
	endc
	ifne	both
	move.w		d0,d3
	and.w		(a4)+,d3
	endc

	add.l		d3,d3
	lsl.l		d7,d3
	swap		d3
	move.w		d3,d0

	rts

	end
