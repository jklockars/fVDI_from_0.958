	xdef	_setjmp
	xdef	_longjmp

	xref	_int_is_short
	
	text

_setjmp:
	move.l	4(sp),a0		; Address of jmp_buf[]
	move.l	(sp),(a0)		; Save return address
	movem.l	d2-d7/a2-a7,4(a0)	; Save registers d2-d7/a2-a7
	clr.l	d0
	rts


_longjmp:
	move.l	4(sp),a0		; Address of jmp_buf[]
	move.w	8(sp),d0		; Value to return
	ext.l	d0
	tst.w	_int_is_short		; Really compiled with -mshort?
	bne	.value_fetched
	move.l	8(sp),d0		; Value to return
.value_fetched:
	tst.l	d0
	jne	.value_ok		; Zero is not allowed
	moveq	#1,d0
.value_ok:
	movem.l	4(a0),d2-d7/a2-a7	; Restore saved reggies
	move.l	(a0),(sp)		;  and the saved return address
	rts

	end
