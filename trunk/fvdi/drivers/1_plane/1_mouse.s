	include	"vdi.inc"
	include	"types.inc"

	xref	_me

	xdef	mouse_draw,_mouse_draw
	xdef	_mouse_pos


	text

	dc.b	"mouse_draw"
*	d0.w	x
*	d1.w	y
*	d2	0 - move shown  1 - move hidden  2 - hide  3 - show  >3 - change shape (pointer to mouse struct)
_mouse_draw:
mouse_draw:
	sub.w	hotspot_x,d0
	bpl	.x_positive
	move.w	#0,d0
.x_positive:
	sub.w	hotspot_y,d1
	bpl	.y_positive
	moveq	#0,d1
.y_positive:
	swap	d1
	move.w	d0,d1
	move.l	d1,_mouse_pos
	
	tas	here_already
	bne	.in_use

	swap	d0

	cmp.l	#3,d2
	bhi	.shape_setup
	btst	#4,d0
	beq	.skip_shape_setup
	move.l	shape_data,d2
.shape_setup:
	move.l	d2,a0				; Points to mouse structure
	move.l	mouse_hotspot(a0),hotspot	; Both coordinates

	lea	mouse_data(a0),a1
	move.l	a1,cursor
	lea	mouse_mask(a0),a1
	move.l	a1,mask
	bra	.finish
.skip_shape_setup:

	cmp.w	#0,d2
	beq	.move
	cmp.w	#1,d2
	beq	.finish
	cmp.w	#2,d2
	beq	.restore
	cmp.w	#3,d2
	beq	.draw
.restore:
	bsr	restore_mouse
	bra	.finish
.move:
	bsr	restore_mouse
.draw:
	bsr	draw_cursor

.finish:
	moveq	#0,d0
	clr.b	here_already
.end:
	rts

.in_use:
	cmp.l	#3,d2
	bhi	.wanted_shape
	bset	d0,d2
	or.l	#$ffff0000,d0
	bra	.end

.wanted_shape:
	or.l	#$ffff0010,d0
	move.l	d2,shape_data
	bra	.end



* Restore old mouse background
restore_mouse
	move.w	back_count,d0
	bmi	.end_restore
	move.w	#-1,back_count		; Mark as restored
	move.l	_me,a1
	move.l	driver_default_vwk(a1),a1
	move.l	vwk_real_address(a1),a1
	move.w	wk_screen_wrap(a1),d1
	move.l	back_address,a1
	lea	back_buffer,a0

.1_loop:
	move.l	(a0)+,(a1)
	add.w	d1,a1			; d6 is lineoffset
	dbf	d0,.1_loop
.end_restore:
	rts		


* This routine both draws a new cursor and
* moves the background to the backbuffer
*
* In:	d0.w	cursor x-pos
*	d1.w	cursor y-pos
*
* Set d5 to lineoffset
* d6 is a mask used to mask off the cursor on the right side 
*    it is 1 where the cursor should be seen
draw_cursor:
	movem.l	d3-d7/a2-a5,-(a7)
	move.l	_mouse_pos,d1
	moveq	#0,d0
	move.w	d1,d0
	swap	d1
	move.l	_me,a1
	move.l	driver_default_vwk(a1),a1
	move.l	vwk_real_address(a1),a1
	move.w	wk_screen_wrap(a1),d5
	move.l	wk_screen_mfdb_address(a1),a3
	move.w	wk_screen_mfdb_height(a1),d4
	sub.w	d1,d4
	cmp.w	#16,d4
	bls	.at_bottom
	moveq	#16,d4
.at_bottom:
	subq.w	#1,d4
	move.w	d4,back_count
	moveq	#-1,d6			; Temporary!
;	move.w	d6,d7
;	not.w	d7
;	move.l	d7,a5
	move.l	#0,a5
	mulu	d5,d1
	move.w	d5,a4
	lea	back_buffer,a0
	move.l	mask,a1
	move.l	cursor,a2

.1_routine:
	move.w	d0,d3
	and.w	#$000f,d3
	sub.w	#16,d3
	neg.w	d3		; d3 - left shifts
	and.w	#$fff0,d0
	lsr.w	#3,d0	; d0 - offset to pixel group
	add.l	d0,d1
	add.l	d1,a3
	move.l	a3,back_address
	sub.w	#4,a4
.1_yloop:
	move.l	a5,d1	
	or.w	(a1)+,d1	; a1 points to mask
	moveq	#0,d2
	move.w	(a2)+,d2	; a2 points to cursor
	and.w	d6,d2
	lsl.l	d3,d1
	not.l	d1
	lsl.l	d3,d2

	move.l	(a3),d5
	move.l	d5,(a0)+
	and.l	d1,d5
	or.l	d2,d5
	move.l	d5,(a3)+

	add.w	a4,a3
	dbf	d4,.1_yloop

	movem.l	(a7)+,d3-d7/a2-a5
	rts


	data
hotspot:
hotspot_x:	dc.w	0
hotspot_y:	dc.w	0

here_already:	dc.b	0,0

mask:		dc.l	back_buffer
cursor:		dc.l	back_buffer

back_count:	dc.w	-1	; Number of lines to replace (-1 at startup)


	bss
_mouse_pos:	ds.l	1
shape_data:	ds.l	1

back_address:	ds.l	1 	; Address at which to replace background
back_buffer:	ds.w	16*16	; Maximal backbuffer needed for mouse
				; 2*8*16 words are needed for 8 bpp modes
				; 16*16, words for 16 bpps modes

	end
