*-------------------------------------------------------*
*	Draw in single plane modes			*	
*-------------------------------------------------------*
**********
*
* Actual drawing routines
*
**********

;left:
	label	left

	tst.w		d7		; Colour number
	jump	beq,l_0			; Go away if not 1s

	swap		d0
.loop1:
	swap		d0
	move.l		(a0),d7
	lsl.l		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
	swap		d7
	and.w		d6,d7
	not.w		d6
;	move.l		a2,d4
;	and.w		d4,d7

	RandorW

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
;	move.l		a2,d4
;	and.w		d4,d7

	nopW

	dbra		d6,.loop2
.loop2_end:

	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	moveq		#-1,d6
	lsr.w		d2,d6
;	move.l		a2,d4
;	and.w		d4,d7
	not.w		d6
	and.w		d6,d7
	not.w		d6		; Not needed before RandorW

	RandorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc

	add.w		a2,a0


	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts

;l_0
	label	l_0
	swap		d0
.loop1:
	swap		d0
	move.l		(a0),d7
	lsl.l		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
	swap		d7

	not.w		d7		; new

	and.w		d6,d7
	not.w		d6
;	move.l		a2,d4
;	and.w		d4,d7

	RandorW

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7

	not.w		d7		; new

;	move.l		a2,d4
;	and.w		d4,d7

	nopW

	dbra		d6,.loop2
.loop2_end:

	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	moveq		#-1,d6
	lsr.w		d2,d6

	not.w		d7		; new

	not.w		d6
	and.w		d6,d7
	not.w		d6		; Not needed before RandorW

;	move.l		a2,d4
;	and.w		d4,d7

	RandorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc

	add.w		a2,a0

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts

;right:
	label	right
;	add.l		#$20000,d5
	addq.l		#2,a2
	neg.w		d1

	tst.w		d7		; Colour number
	jump	beq,r_0			; Go away if not 1s

	swap		d0
.loop1:
	swap		d0
	move.w		(a0),d7
	lsr.w		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
;	swap		d7
	and.w		d6,d7
	not.w		d6
;	move.l		a2,d4
;	and.w		d4,d7

	RandorW

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
;	addq.l		#2,a0
	move.l		(a0),d7
	lsr.l		d1,d7
;	swap		d7
;	move.l		a2,d4
;	and.w		d4,d7

	nopW

	addq.l		#2,a0
	dbra		d6,.loop2
.loop2_end:

;	addq.l		#2,a0
	move.l		(a0),d7
	lsr.l		d1,d7
;	swap		d7
;	move.l		a2,d4
;	and.w		d4,d7
	moveq		#-1,d6
	lsr.w		d2,d6
;	move.l		a2,d4
;	and.w		d4,d7
	not.w		d6
	and.w		d6,d7
	not.w		d6		; Not needed before RandorW

	RandorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc

	add.w		a2,a0

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts

;r_0
	label	r_0
	swap		d0
.loop1:
	swap		d0
	move.w		(a0),d7
	lsr.w		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
;	swap		d7

	not.w		d7		; new

	and.w		d6,d7
	not.w		d6
;	move.l		a2,d4
;	and.w		d4,d7

	RandorW

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
;	addq.l		#2,a0
	move.l		(a0),d7
	lsr.l		d1,d7
;	swap		d7

	not.w		d7		; new

;	move.l		a2,d4
;	and.w		d4,d7

	nopW

	addq.l		#2,a0
	dbra		d6,.loop2
.loop2_end:

;	addq.l		#2,a0
	move.l		(a0),d7
	lsr.l		d1,d7
;	swap		d7

	not.w		d7		; new

;	move.l		a2,d4
;	and.w		d4,d7
	moveq		#-1,d6
	lsr.w		d2,d6
;	move.l		a2,d4
;	and.w		d4,d7
	not.w		d6
	and.w		d6,d7
	not.w		d6		; Not needed before RandorW

	RandorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc

	add.w		a2,a0

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts


;single:
	label	single

	swap		d0
	moveq		#-1,d6
	lsr.w		d3,d6
	move.w		#-1,d4
	lsr.w		d2,d4
	not.w		d4
	and.w		d4,d6


	sub.w		d3,d1			; d1 - shift length
	jump	blt,sright

	move.w		d6,d3
	not.w		d6

	tst.w		d7		; Colour number
	jump	beq,s_0			; Go away if not 1s

.loop1:
	move.l		(a0),d7
	lsl.l		d1,d7

	swap		d7
	and.w		d3,d7

	RandorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc

	add.w		a2,a0

	dbra		d0,.loop1
;	subq.w		#1,d0
;	bpl		.loop1

	moveq		#1,d0
	rts

;s_0:
	label	s_0
.loop1:
	move.l		(a0),d7
	lsl.l		d1,d7

	swap		d7

	not.w		d7		; new

	and.w		d3,d7

	RandorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc

	add.w		a2,a0

	dbra		d0,.loop1
;	subq.w		#1,d0
;	bpl		.loop1

	moveq		#1,d0
	rts

;sright:
	label	sright
	neg.w		d1

	move.w		d6,d3
	not.w		d6

	tst.w		d7		; Colour number
	jump	beq,sr_0		; Go away if not 1s

.loop1:
	move.w		(a0),d7
	lsr.w		d1,d7

;;;	swap		d7
	and.w		d3,d7

	RandorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc

	add.w		a2,a0

	dbra		d0,.loop1
;	subq.w		#1,d0
;	bpl		.loop1

	moveq		#1,d0
	rts

;sr_0:
	label	sr_0
.loop1:
	move.w		(a0),d7
	lsr.w		d1,d7

;;;	swap		d7

	not.w		d7		; new

	and.w		d3,d7

	RandorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc

	add.w		a2,a0

	dbra		d0,.loop1
;	subq.w		#1,d0
;	bpl		.loop1

	moveq		#1,d0
	rts


;left_transp:
	label	left_transp

	tst.w		d7
	jump	beq,l_transp_0		; Go away if not 1s

	swap		d0
.loop1:
	swap		d0
	move.l		(a0),d7
	lsl.l		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
	swap		d7
	and.w		d6,d7

	RorW

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7

	RorW

	dbra		d6,.loop2
.loop2_end:

	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	moveq		#-1,d6
	lsr.w		d2,d6
	not.w		d6
	and.w		d6,d7

	RorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts

;l_transp_0:
	label	l_transp_0
	swap		d0
.loop1:
	swap		d0
	move.l		(a0),d7
	lsl.l		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
	swap		d7
	and.w		d6,d7
	not.w		d7			; new

	RandW

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	not.w		d7			; new

	RandW

	dbra		d6,.loop2
.loop2_end:

	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	moveq		#-1,d6
	lsr.w		d2,d6
	not.w		d7
	or.w		d6,d7

	RandWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts

;right_transp:
	label	right_transp

;	add.l		#$20000,d5
	addq.l		#2,a2
	neg.w		d1

	tst.w		d7
	jump	beq,r_transp_0		; Go away if not 1s

	swap		d0
.loop1:
	swap		d0
	move.w		(a0),d7
	lsr.w		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
	and.w		d6,d7

	RorW

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	move.l		(a0),d7
	lsr.l		d1,d7

	RorW

	addq.l		#2,a0
	dbra		d6,.loop2
.loop2_end:

	move.l		(a0),d7
	lsr.l		d1,d7
	moveq		#-1,d6
	lsr.w		d2,d6
	not.w		d6
	and.w		d6,d7

	RorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts

;r_transp_0:
	label	r_transp_0
	swap		d0
.loop1:
	swap		d0
	move.w		(a0),d7
	lsr.w		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
	and.w		d6,d7
	not.w		d7

	RandW

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	move.l		(a0),d7
	lsr.l		d1,d7
	not.w		d7

	RandW

	addq.l		#2,a0
	dbra		d6,.loop2
.loop2_end:

	move.l		(a0),d7
	lsr.l		d1,d7
	moveq		#-1,d6
	lsr.w		d2,d6
	not.w		d7
	or.w		d6,d7

	RandWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts

;single_transp:
	label	single_transp

	swap		d0
	moveq		#-1,d6
	lsr.w		d3,d6
	move.w		#-1,d4
	lsr.w		d2,d4
	not.w		d4
	and.w		d4,d6

	sub.w		d3,d1			; d1 - shift length
	jump	blt,sright_transp

	tst.w		d7
	jump	beq,s_transp_0		; Go away if not 1s
.loop1:
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7

	and.w		d6,d7

	RorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	dbra		d0,.loop1

	moveq		#1,d0
	rts

;s_transp_0:
	label	s_transp_0
	not.w		d6
.loop1:
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7

	not.w		d7
	or.w		d6,d7

	RandWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	dbra		d0,.loop1

	moveq		#1,d0
	rts

;sright_transp:
	label	sright_transp
	neg.w		d1

	tst.w		d7
	jump	beq,sr_transp_0		; Go away if not 1s
.loop1:
	move.w		(a0),d7
	lsr.w		d1,d7

	and.w		d6,d7

	RorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	dbra		d0,.loop1

	moveq		#1,d0
	rts

;sr_transp_0:
	label	sr_transp_0
	not.w		d6
.loop1:
	move.w		(a0),d7
	lsr.w		d1,d7

	not.w		d7
	or.w		d6,d7

	RandWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	dbra		d0,.loop1

	moveq		#1,d0
	rts


;left_xor:
	label	left_xor
	swap		d0
.loop1:
	swap		d0
	move.l		(a0),d7
	lsl.l		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
	swap		d7
	and.w		d6,d7
	not.w		d6

	ReorW

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7

	ReorW

	dbra		d6,.loop2
.loop2_end:

	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	moveq		#-1,d6
	lsr.w		d2,d6
	not.w		d6
	and.w		d6,d7

	ReorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts

;right_xor:
	label	right_xor
;	add.l		#$20000,d5
	addq.l		#2,a2
	neg.w		d1
	swap		d0
.loop1:
	swap		d0
	move.w		(a0),d7
	lsr.w		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
	and.w		d6,d7
	not.w		d6

	ReorW

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	move.l		(a0),d7
	lsr.l		d1,d7

	ReorW

	addq.l		#2,a0
	dbra		d6,.loop2
.loop2_end:

	move.l		(a0),d7
	lsr.l		d1,d7
	moveq		#-1,d6
	lsr.w		d2,d6
	not.w		d6
	and.w		d6,d7

	ReorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts

;single_xor:
	label	single_xor
	sub.w		d3,d1			; d1 - shift length
	jump	blt,sright_xor

	swap		d0
	moveq		#-1,d6
	lsr.w		d3,d6
	move.w		#-1,d4
	lsr.w		d2,d4
	not.w		d4
	and.w		d4,d6
.loop1:
	move.l		(a0),d7
	lsl.l		d1,d7

	swap		d7
	and.w		d6,d7

	ReorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	dbra		d0,.loop1

	moveq		#1,d0
	rts

;sright_xor:
	label	sright_xor
	neg.w		d1
	swap		d0

	moveq		#-1,d6
	lsr.w		d3,d6
	move.w		#-1,d4
	lsr.w		d2,d4
	not.w		d4
	and.w		d4,d6
.loop1:
	move.w		(a0),d7
	lsr.w		d1,d7

	and.w		d6,d7

	ReorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	dbra		d0,.loop1

	moveq		#1,d0
	rts


;left_revtransp:
	label	left_revtransp

	tst.w		d7
	jump	beq,l_revtransp_0		; Go away if not 1s

	swap		d0
.loop1:
	swap		d0
	move.l		(a0),d7
	lsl.l		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
	swap		d7
	not.w		d7
	and.w		d6,d7

	RorW

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	not.w		d7

	RorW

	dbra		d6,.loop2
.loop2_end:

	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	not.w		d7
	moveq		#-1,d6
	lsr.w		d2,d6
	not.w		d6
	and.w		d6,d7

	RorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts

;l_revtransp_0:
	label	l_revtransp_0
	swap		d0
.loop1:
	swap		d0
	move.l		(a0),d7
	lsl.l		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
	swap		d7
	not.w		d7
	and.w		d6,d7
	not.w		d7			; new

	RandW

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
;	not.w		d7			; new

	RandW

	dbra		d6,.loop2
.loop2_end:

	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	moveq		#-1,d6
	lsr.w		d2,d6
;	not.w		d7
	or.w		d6,d7

	RandWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts

;right_revtransp:
	label	right_revtransp
;	add.l		#$20000,d5
	addq.l		#2,a2
	neg.w		d1

	tst.w		d7
	jump	beq,r_revtransp_0		; Go away if not 1s

	swap		d0
.loop1:
	swap		d0
	move.w		(a0),d7
	lsr.w		d1,d7
	not.w		d7
	moveq		#-1,d6
	lsr.w		d3,d6
	and.w		d6,d7

	RorW

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	move.l		(a0),d7
	lsr.l		d1,d7
	not.w		d7

	RorW

	addq.l		#2,a0
	dbra		d6,.loop2
.loop2_end:

	move.l		(a0),d7
	lsr.l		d1,d7
	not.w		d7
	moveq		#-1,d6
	lsr.w		d2,d6
	not.w		d6
	and.w		d6,d7

	RorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts

;r_revtransp_0:
	label	r_revtransp_0
	swap		d0
.loop1:
	swap		d0
	move.w		(a0),d7
	lsr.w		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
	not.w		d7
	and.w		d6,d7
	not.w		d7

	RandW

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	move.l		(a0),d7
	lsr.l		d1,d7
;	not.w		d7

	RandW

	addq.l		#2,a0
	dbra		d6,.loop2
.loop2_end:

	move.l		(a0),d7
	lsr.l		d1,d7
	moveq		#-1,d6
	lsr.w		d2,d6
;	not.w		d7
	or.w		d6,d7

	RandWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts

;single_revtransp:
	label	single_revtransp
	swap		d0
	moveq		#-1,d6
	lsr.w		d3,d6
	move.w		#-1,d4
	lsr.w		d2,d4
	not.w		d4
	and.w		d4,d6

	sub.w		d3,d1			; d1 - shift length
	jump	blt,sright_revtransp

	tst.w		d7
	jump	beq,s_revtransp_0		; Go away if not 1s
.loop1:
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7

	not.w		d7
	and.w		d6,d7

	RorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	dbra		d0,.loop1

	moveq		#1,d0
	rts

;s_revtransp_0:
	label	s_revtransp_0
	not.w		d6
.loop1:
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7

;	not.w		d7
	or.w		d6,d7

	RandWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	dbra		d0,.loop1

	moveq		#1,d0
	rts

;sright_revtransp:
	label	sright_revtransp
	neg.w		d1

	tst.w		d7
	jump	beq,sr_revtransp_0		; Go away if not 1s
.loop1:
	move.w		(a0),d7
	lsr.w		d1,d7

	not.w		d7
	and.w		d6,d7

	RorWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	dbra		d0,.loop1

	moveq		#1,d0
	rts

;sr_revtransp_0:
	label	sr_revtransp_0
	not.w		d6
.loop1:
	move.w		(a0),d7
	lsr.w		d1,d7

;	not.w		d7
	or.w		d6,d7

	RandWl

	add.w		a3,a1
	ifne	both
	add.w		a3,a4
	endc
	add.w		a2,a0

	dbra		d0,.loop1

	moveq		#1,d0
	rts

