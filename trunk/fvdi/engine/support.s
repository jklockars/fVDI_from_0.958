*****
* fVDI support routines
*
* $Id: support.s,v 1.4 2002-07-01 22:27:18 johan Exp $
*
* Copyright 1997-2002, Johan Klockars 
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

	xref	_old_wk_handle
	xref	_malloc
	xref	_cpu
	xref	_stand_alone

	xdef	remove_xbra,_remove_xbra
	xdef	set_cookie,_set_cookie
	xdef	get_protected_l,set_protected_l
	xdef	_get_protected_l,_set_protected_l
	xdef	_flip_words,_flip_longs
	xdef	redirect,redirect_d0
	xdef	call_other,_call_other
	xdef	_initialize_palette
	xdef	initialize_pool,_initialize_pool
	xdef	allocate_block,_allocate_block
	xdef	free_block,_free_block
	xdef	cache_flush,_cache_flush


	text

	dc.b	0,"flip_words",0
* flip_words(short *addr, long n)
* Byte swap a number of consecutive words
_flip_words:
	move.l	4(a7),a0
	move.l	8(a7),d0
	bra	.loopend
.loop:
	move.w	(a0),d1
	rol.w	#8,d1
	move.w	d1,(a0)+
.loopend:
	dbra	d0,.loop
	rts


* flip_longs(long *addr, long n)
* Byte swap a number of consecutive longs
_flip_longs:
	move.l	4(a7),a0
	move.l	8(a7),d0
	bra	.loopend_l
.loop_l:
	move.l	(a0),d1
	rol.w	#8,d1
	swap	d1
	rol.w	#8,d1
	move.l	d1,(a0)+
.loopend_l:
	dbra	d0,.loop_l
	rts


	dc.b	0,0,"remove_xbra",0
* int remove_xbra(long vector, long xbra_id)
* Follows an XBRA chain and removes a link if found
* Returns 0 if XBRA id not found
remove_xbra:
_remove_xbra:
	movem.l	a0-a1,-(a7)
	move.l	(4+2*4)(a7),a1
	move.l	a1,-(a7)
	bsr	get_protected_l		; Probably an exception vector
	addq.l	#4,a7
	move.l	d0,a0
	move.l	(8+2*4)(a7),d0
.xbra_search:
	cmp.l	#'XBRA',-12(a0)
	bne	.xbra_not
	cmp.l	-8(a0),d0
	beq	.xbra_found
	lea	-4(a0),a1
	move.l	(a1),a0
	bra	.xbra_search
.xbra_found:
	move.l	-4(a0),-(a7)
	move.l	a1,-(a7)
	bsr	set_protected_l		; Might well be an exception vector
	addq.l	#8,a7
	bra	.xbra_end
.xbra_not:
	moveq	#0,d0
.xbra_end:
	movem.l	(a7)+,a0-a1
	rts


	dc.b	0,"set_cookie",0
* int set_cookie(char *name, long value)
* Set a cookie value
* Replace if there already is one
* Create/expand jar if needed
* Returns != 0 if an old cookie was replaced
set_cookie:
_set_cookie:
	movem.l	d1-d3/d6-d7/a0-a2,-(a7)
	move.l	#$5a0,-(a7)		; _p_cookies
	bsr	get_protected_l
	addq.l	#4,a7
	move.l	d0,a1
	tst.l	d0
	beq	.full
	move.l	d0,a0
	moveq	#0,d0

	move.l	(4+8*4)(a7),a2		; Cookie name
	move.b	(a2)+,d2
	lsl.w	#8,d2
	move.b	(a2)+,d2
	swap	d2
	move.b	(a2)+,d2
	lsl.w	#8,d2
	move.b	(a2)+,d2
	move.l	d2,d3
.search:
	move.l	(a0),d1
	beq	.no_more
	addq.w	#1,d0
	cmp.l	d2,d1
	beq	.end_found
	addq.l	#8,a0
	bra	.search

.no_more:
	move.l	4(a0),d1
; Must make sure there is room for the final count!  [010109]
	move.l	d1,d7
	subq.l	#1,d7
	cmp.l	d0,d7
;	cmp.l	d0,d1
	beq	.full
	move.l	#0,8(a0)
	move.l	d1,12(a0)
	bra	.end_ok

.full:
	move.l	d0,d7
	addq.l	#8,d0
	move.l	d0,d6
	lsl.l	#3,d0
	move.l	d0,-(a7)
	move.w	#$48,-(a7)
	trap	#1
	addq.l	#6,a7
;	tst.l	d0
;	beq	.end_bad

	move.l	d0,-(a7)
	move.l	#$5a0,-(a7)	; _p_cookies
	bsr	set_protected_l
	addq.l	#8,a7

	move.l	d0,a0
	tst.l	d7
	beq	.no_copy
	subq.w	#1,d7
.copy:
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+	
	dbra	d7,.copy

.no_copy:
	move.l	#0,8(a0)
	move.l	d6,12(a0)

.end_ok:
	moveq	#0,d0
.end_found:
	move.l	d3,(a0)			; Cookie name
	move.l	(8+8*4)(a7),4(a0)	; Cookie value
	movem.l	(a7)+,d1-d3/d6-d7/a0-a2
	rts


	dc.b	0,0,"get_protected",0
* long get_protected_l(long *addr)
* Get a long value from low (protected) memory
get_protected_l:
_get_protected_l:
	movem.l	d1-d2/a0-a2,-(a7)
	move.l	#0,-(a7)
	move.w	#$20,-(a7)	; Super
	trap	#1
	addq.l	#6,a7
	move.l	(4+5*4)(a7),a0
	move.l	(a0),-(a7)
	move.l	d0,-(a7)
	move.w	#$20,-(a7)	; Super
	trap	#1
	addq.l	#6,a7
	move.l	(a7)+,d0
	movem.l	(a7)+,d1-d2/a0-a2
	rts


	dc.b	0,0,"set_protected",0
* set_protected_l(long *addr, long value)
* Set a long value in low (protected) memory
set_protected_l:
_set_protected_l:
	movem.l	d0-d2/a0-a2,-(a7)
	move.l	#0,-(a7)
	move.w	#$20,-(a7)	; Super
	trap	#1
	addq.l	#6,a7
	move.l	(4+6*4)(a7),a0
	move.l	(8+6*4)(a7),(a0)
	move.l	d0,-(a7)
	move.w	#$20,-(a7)	; Super
	trap	#1
	addq.l	#6,a7
	movem.l	(a7)+,d0-d2/a0-a2
	rts


	dc.b	0,"redirect",0
* redirect    - Remaps VDI call to default physical workstation
* redirect_d0 =      -   "   -     specified handle
* Todo:	?
* In:	a1	Parameter block   **** Should perhaps put this in d1? ****
redirect:
	move.w	_old_wk_handle,d0
redirect_d0:
	move.l	control(a1),a0
	cmp.w	handle(a0),d0		; Already correct?
	bne	call
	move.l	a1,d1			; That's where the VDI wants it
	return
call:
	tst.w	_stand_alone
	bne	.no_redirect
	bsr	call_other
.no_redirect:
	real_return


	dc.b	0,"call_other",0
* call_other - Do a VDI call to the previous VDI instead
* Todo:	?
* In:	a1	Parameter block   **** Should perhaps put this in d1? ****
*	d0	Handle of the other workstation
* Out:	d0	Possibly new handle created
_call_other:
	move.l	4(a7),a1
	move.l	8(a7),d0
call_other:
	move.l	a1,d1			; That's where the VDI wants it
;	move.l	a0,-(a7)		; Remember workstation struct
	move.l	control(a1),a0
	move.w	handle(a0),-(a7)	; Remember original handle
	move.w	d0,handle(a0)		; Point to handle from above (normally default physical workstation)
	move.w	#$ffff,-(a7)		; Mark old stack
	moveq	#$73,d0

* Pretend the call was from this code
* We need to check various values when the normal VDI returns

	move.w	#$88,-(a7)		; In case we're on >='020
	pea	.vdi_ret(pc)
	move.w	sr,-(a7)
	move.l	vdi_address(pc),-(a7)
	rts

.vdi_ret:
	cmp.w	#$ffff,(a7)+
	beq	.was_020
	addq.l	#2,a7			; Skip $88 if not >='020
.was_020:

	move.l	control(a1),a0
	move.w	handle(a0),d0
	move.w	(a7)+,handle(a0)
;	move.l	(a7)+,a0		; Workstation struct
	rts


	dc.b		0,"initialize_palette",0
* initialize_palette(Virtual *vwk, long start, long n, short requested[][3], Colour palette[])
* Set palette colours
_initialize_palette:
	movem.l		d0-d7/a0-a6,-(sp)	; Overkill

	move.l		15*4+4(a7),a0
	move.l		15*4+8(a7),d1
	move.l		15*4+12(a7),d0
	swap		d0
	move.w		d1,d0
	move.l		15*4+16(a7),a1
	move.l		15*4+20(a7),a2

	move.l		vwk_real_address(a0),a3
	move.l		wk_r_set_palette(a3),a3
	jsr		(a3)

	movem.l		(sp)+,d0-d7/a0-a6
	rts


	dc.b		0,0,"initialize_pool",0
* initialize_pool(long size, long n)
* Initialize an internal memory pool
initialize_pool:
_initialize_pool:
	moveq	#0,d0
	move.l	8(a7),d1
	beq	.error
	move.l	4(a7),d0
;	mulu	d1,d0
	move.l	d0,a0
	neg.l	d0
.mul_loop:			; Should only be a couple of loops
	add.l	a0,d0
	dbra	d1,.mul_loop
	move.l	#3,-(a7)
	move.l	d0,-(a7)
	bsr	_malloc
	addq.l	#8,a7
	tst.l	d0
	beq	.error
	move.l	d0,a0
	move.l	4(a7),d1
	move.l	d1,block_size
	move.l	8(a7),d0
	subq.w	#1,d0
	sub.l	a1,a1
.block_loop:
	move.l	a0,block_chain
	move.l	a1,(a0)
	move.l	a0,a1
	add.l	d1,a0
	dbra	d0,.block_loop
	moveq	#1,d0
.error:
	rts


	dc.b	0,"allocate_block",0
* long allocate_block(long size)
* Allocate a block from the internal memory pool
allocate_block:
_allocate_block:
	move.l	a0,-(a7)
	move.l	4+4(a7),d0
	cmp.l	block_size,d0
	bhi	.no_block
	move.l	block_chain,d0
	beq	.no_block
	move.l	d0,a0
	move.l	(a0),block_chain
	move.l	block_size,(a0)		; Make size info available
.allocate_end:
	move.l	(a7)+,a0
	rts

.no_block:
	moveq	#0,d0
	bra	.allocate_end


	dc.b	0,"free_block",0
* free_block(void *addr)
* Free a block and return it to the internal memory pool
free_block:
_free_block:
	move.l	a0,-(a7)
	move.l	4+4(a7),a0
	move.l	block_chain,(a0)
	move.l	a0,block_chain
	move.l	(a7)+,a0
	rts


	dc.b	0,0,"cache_flush",0
* cache_flush(void)
* Flush both caches
cache_flush:
_cache_flush:
	movem.l	d0-d1,-(a7)
	move.l	_cpu,d0
	cmp.w	#20,d0
	blo	.cache_end
	cmp.w	#30,d0
	beq	.is_030

  ifne lattice
	cpusha	bc		; This is an '040 or '060 ($f4f8
  else
	dc.w	$f4f8
  endc
.cache_end:
	movem.l	(a7)+,d0-d1
	rts

.is_030:
	movec	cacr,d0
	move.l	d0,d1
	or.w	#$808,d1
	movec	d1,cacr
	movec	d0,cacr
	bra	.cache_end


	data

block_size:	dc.l	10*1024
block_chain:	ds.l	1

	end
