*-------------------------------------------------------*
*	Draw in single plane modes			*	
*-------------------------------------------------------*

**********
*
* Actual drawing routines
*
* Replace mode
*
**********

;none:
	label	none
	swap		d0
.loop1:
	swap		d0
;	move.l		(a0),d7
	move.w		(a0)+,d7
;	lsl.l		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
;	swap		d7
	and.w		d6,d7
	not.w		d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	and.w		d6,d4
	or.w		d7,d4
	ifeq	both
	move.w		d4,(a1)+
	endc
	ifne	both
	move.w		d4,(a4)+
	move.w		d4,(a1)+
	endc

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	lsr.w		#1,d6
	bcc		.no_word
	move.w		(a0)+,d7
	ifeq	both
        move.w		d7,(a1)+
	endc
	ifne	both
        move.w		d7,(a4)+
        move.w		d7,(a1)+
	endc
	bra		.loop_start
.no_word:
	subq.w		#1,d6
.loop2:
;	addq.l		#2,a0
;	move.l		(a0),d7
	move.l		(a0)+,d7
;	lsl.l		d1,d7
;	swap		d7
	ifeq	both
;	move.w		d7,(a1)+
	move.l		d7,(a1)+
	endc
	ifne	both
;	move.w		d7,(a4)+
	move.l		d7,(a4)+
;	move.w		d7,(a1)+
	move.l		d7,(a1)+
	endc
.loop_start:
	dbra		d6,.loop2
.loop2_end:

;	addq.l		#2,a0
;	move.l		(a0),d7
	move.w		(a0),d7
;	lsl.l		d1,d7
;	swap		d7
	moveq		#-1,d6
	lsr.w		d2,d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	and.w		d6,d4
	not.w		d6
	and.w		d6,d7
	or.w		d7,d4
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc

	add.w		d5,a1
	ifne	both
	add.w		d5,a4
	endc
	swap		d5
	add.w		d5,a0
	swap		d5

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts


;left:
	label	left
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
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	and.w		d6,d4
	or.w		d7,d4
	ifeq	both
	move.w		d4,(a1)+
	endc
	ifne	both
	move.w		d4,(a4)+
	move.w		d4,(a1)+
	endc

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	ifeq	both
	move.w		d7,(a1)+
	endc
	ifne	both
	move.w		d7,(a4)+
	move.w		d7,(a1)+
	endc
	dbra		d6,.loop2
.loop2_end:

	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	moveq		#-1,d6
	lsr.w		d2,d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	and.w		d6,d4
	not.w		d6
	and.w		d6,d7
	or.w		d7,d4
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc

	add.w		d5,a1
	ifne	both
	add.w		d5,a4
	endc
	swap		d5
	add.w		d5,a0
	swap		d5

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts


;right:
	label	right
	add.l		#$20000,d5
	neg.w		d1
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
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	and.w		d6,d4
	or.w		d7,d4
	ifeq	both
	move.w		d4,(a1)+
	endc
	ifne	both
	move.w		d4,(a4)+
	move.w		d4,(a1)+
	endc

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
;	addq.l		#2,a0
	move.l		(a0),d7
	lsr.l		d1,d7
;	swap		d7
	ifeq	both
	move.w		d7,(a1)+
	endc
	ifne	both
	move.w		d7,(a4)+
	move.w		d7,(a1)+
	endc
	addq.l		#2,a0
	dbra		d6,.loop2
.loop2_end:

;	addq.l		#2,a0
	move.l		(a0),d7
	lsr.l		d1,d7
;	swap		d7
	moveq		#-1,d6
	lsr.w		d2,d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	and.w		d6,d4
	not.w		d6
	and.w		d6,d7
	or.w		d7,d4
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc

	add.w		d5,a1
	ifne	both
	add.w		d5,a4
	endc
	swap		d5
	add.w		d5,a0
	swap		d5

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts

	dc.b		"pan_back"

;pan_backwards:
	label	pan_backwards
	addq.l		#2,a7			; skip op check for now ********

	move.w		d0,d7
	add.w		d7,d7
	addq.w		#2,d7
	add.w		d7,a0
	add.w		d7,a1
	ifne	both
	add.w		d7,a4
	endc
	add.w		d7,d7
	add.w		d7,d5
	swap		d5
	add.w		d7,d5
	swap		d5
	sub.w		d3,d1			; d1 - shift length
	jump	blt,pright

	swap		d0
.loop1:
	swap		d0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	moveq		#-1,d6
	lsr.w		d2,d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	and.w		d6,d4
	not.w		d6
	and.w		d6,d7
	or.w		d7,d4
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc
	subq.l		#2,a1
	ifne	both
	subq.l		#2,a4
	endc

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	subq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	ifeq	both
	move.w		d7,(a1)
	endc
	ifne	both
	move.w		d7,(a4)
	move.w		d7,(a1)
	endc
	subq.l		#2,a1
	ifne	both
	subq.l		#2,a4
	endc
	dbra		d6,.loop2
.loop2_end:

	subq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
	swap		d7
	and.w		d6,d7
	not.w		d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	and.w		d6,d4
	or.w		d7,d4
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc

	add.w		d5,a1
	ifne	both
	add.w		d5,a4
	endc
	swap		d5
	add.w		d5,a0
	swap		d5

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts

;pright:
	label	pright
	subq.l		#2,a0
	sub.l		#$20000,d5
	neg.w		d1
	swap		d0
.loop1:
	swap		d0
	move.l		(a0),d7
	lsr.l		d1,d7
	moveq		#-1,d6
	lsr.w		d2,d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	and.w		d6,d4
	not.w		d6
	and.w		d6,d7
	or.w		d7,d4
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc
	subq.l		#2,a0
	subq.l		#2,a1
	ifne	both
	subq.l		#2,a4
	endc

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	move.l		(a0),d7
	lsr.l		d1,d7
	ifeq	both
	move.w		d7,(a1)
	endc
	ifne	both
	move.w		d7,(a4)
	move.w		d7,(a1)
	endc
	subq.l		#2,a0
	subq.l		#2,a1
	ifne	both
	subq.l		#2,a4
	endc
	dbra		d6,.loop2
.loop2_end:

	addq.l		#2,a0
	move.w		(a0),d7
	lsr.w		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
;	swap		d7
	and.w		d6,d7
	not.w		d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	and.w		d6,d4
	or.w		d7,d4
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc

	add.w		d5,a1
	ifne	both
	add.w		d5,a4
	endc
	swap		d5
	add.w		d5,a0
	swap		d5

	swap		d0
	dbra		d0,.loop1

	moveq		#1,d0
	rts


;single:
	label	single
	addq.l		#4,a7

	move.w		(a7)+,d7
	cmp.w		#3,d7
	jump	bne,single_op

	sub.w		d3,d1			; d1 - shift length
	jump	blt,sright

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
	not.w		d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	and.w		d6,d4
	not.w		d6
	or.w		d7,d4
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc

	add.w		d5,a1
	ifne	both
	add.w		d5,a4
	endc
	swap		d5
	add.w		d5,a0
	swap		d5

	dbra		d0,.loop1

	moveq		#1,d0
	rts

;sright:
	label	sright
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

;	swap		d7
	and.w		d6,d7
	not.w		d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	and.w		d6,d4
	not.w		d6
	or.w		d7,d4
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc

	add.w		d5,a1
	ifne	both
	add.w		d5,a4
	endc
	swap		d5
	add.w		d5,a0
	swap		d5

	dbra		d0,.loop1

	moveq		#1,d0
	rts


**********
*
* Actual drawing routines
*
* All other operations
*
**********

;none_op:
	label	none_op
	swap		d0
.loop1:
	swap		d0
;	move.l		(a0),d7
	move.w		(a0)+,d7
;	lsl.l		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
;	swap		d7
;	and.w		d6,d7
	not.w		d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
;	and.w		d6,d4
;	or.w		d7,d4
	jsr		(a6)
	ifeq	both
	move.w		d4,(a1)+
	endc
	ifne	both
	move.w		d4,(a4)+
	move.w		d4,(a1)+
	endc

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	lsr.w		#1,d6
	bcc		.no_word
	move.w		(a0)+,d7
	ifeq	both
	move.w		(a1),d4			; new
	endc
	ifne	both
	move.w		(a4),d4			; new
	endc
	jsr		10(a6)
	ifeq	both
        move.w		d7,(a1)+
	endc
	ifne	both
        move.w		d7,(a4)+
        move.w		d7,(a1)+
	endc
	bra		.loop_start
.no_word:
	subq.w		#1,d6
.loop2:
;	addq.l		#2,a0
;	move.l		(a0),d7
	move.l		(a0)+,d7
;	lsl.l		d1,d7
;	swap		d7
	ifeq	both
;	move.w		(a1),d4
	move.l		(a1),d4
	endc
	ifne	both
;	move.w		(a4),d4
	move.l		(a4),d4
	endc
	jsr		10(a6)
	swap		d4
	swap		d7
	jsr		10(a6)
	swap		d4
	swap		d7
	ifeq	both
;	move.w		d7,(a1)+
	move.l		d7,(a1)+
	endc
	ifne	both
;	move.w		d7,(a4)+
	move.l		d7,(a4)+
;	move.w		d7,(a1)+
	move.l		d7,(a1)+
	endc
.loop_start:
	dbra		d6,.loop2
.loop2_end:

;	addq.l		#2,a0
;	move.l		(a0),d7
	move.w		(a0),d7
;	lsl.l		d1,d7
;	swap		d7
	moveq		#-1,d6
	lsr.w		d2,d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
;	and.w		d6,d4
;	not.w		d6
;	and.w		d6,d7
;	or.w		d7,d4
	jsr		(a6)
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc

	add.w		d5,a1
	ifne	both
	add.w		d5,a4
	endc
	swap		d5
	add.w		d5,a0
	swap		d5

	swap		d0
	dbra		d0,.loop1

	move.l		(a7)+,a6
	moveq		#1,d0
	rts


;left_op:
	label	left_op
	swap		d0
.loop1:
	swap		d0
	move.l		(a0),d7
	lsl.l		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
	swap		d7
;	and.w		d6,d7
	not.w		d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
;	and.w		d6,d4
;	or.w		d7,d4
	jsr		(a6)
	ifeq	both
	move.w		d4,(a1)+
	endc
	ifne	both
	move.w		d4,(a4)+
	move.w		d4,(a1)+
	endc

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	jsr		10(a6)
	ifeq	both
	move.w		d7,(a1)+
	endc
	ifne	both
	move.w		d7,(a4)+
	move.w		d7,(a1)+
	endc
	dbra		d6,.loop2
.loop2_end:

	addq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	moveq		#-1,d6
	lsr.w		d2,d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
;	and.w		d6,d4
;	not.w		d6
;	and.w		d6,d7
;	or.w		d7,d4
	jsr		(a6)
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc

	add.w		d5,a1
	ifne	both
	add.w		d5,a4
	endc
	swap		d5
	add.w		d5,a0
	swap		d5

	swap		d0
	dbra		d0,.loop1

	move.l		(a7)+,a6
	moveq		#1,d0
	rts


;right_op:
	label	right_op
	add.l		#$20000,d5
	neg.w		d1
	swap		d0
.loop1:
	swap		d0
	move.w		(a0),d7
	lsr.w		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
;	swap		d7
;	and.w		d6,d7
	not.w		d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
;	and.w		d6,d4
;	or.w		d7,d4
	jsr		(a6)
	ifeq	both
	move.w		d4,(a1)+
	endc
	ifne	both
	move.w		d4,(a4)+
	move.w		d4,(a1)+
	endc

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
;	addq.l		#2,a0
	move.l		(a0),d7
	lsr.l		d1,d7
;	swap		d7
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	jsr		10(a6)
	ifeq	both
	move.w		d7,(a1)+
	endc
	ifne	both
	move.w		d7,(a4)+
	move.w		d7,(a1)+
	endc
	addq.l		#2,a0
	dbra		d6,.loop2
.loop2_end:

;	addq.l		#2,a0
	move.l		(a0),d7
	lsr.l		d1,d7
;	swap		d7
	moveq		#-1,d6
	lsr.w		d2,d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
;	and.w		d6,d4
;	not.w		d6
;	and.w		d6,d7
;	or.w		d7,d4
	jsr		(a6)
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc

	add.w		d5,a1
	ifne	both
	add.w		d5,a4
	endc
	swap		d5
	add.w		d5,a0
	swap		d5

	swap		d0
	dbra		d0,.loop1

	move.l		(a7)+,a6
	moveq		#1,d0
	rts

	dc.b		"pan_back_op"

;pan_backwards_op:
	label	pan_backwards_op
	addq.l		#2,a7			; skip op check for now ********

	move.w		d0,d7
	add.w		d7,d7
	addq.w		#2,d7
	add.w		d7,a0
	add.w		d7,a1
	ifne	both
	add.w		d7,a4
	endc
	add.w		d7,d7
	add.w		d7,d5
	swap		d5
	add.w		d7,d5
	swap		d5
	sub.w		d3,d1			; d1 - shift length
	jump	blt,pright_op

	swap		d0
.loop1:
	swap		d0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	moveq		#-1,d6
	lsr.w		d2,d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
;	and.w		d6,d4
	not.w		d6
;	and.w		d6,d7
;	or.w		d7,d4
	jsr		(a6)
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc
	subq.l		#2,a1
	ifne	both
	subq.l		#2,a4
	endc

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	subq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	jsr		10(a6)
	ifeq	both
	move.w		d7,(a1)
	endc
	ifne	both
	move.w		d7,(a4)
	move.w		d7,(a1)
	endc
	subq.l		#2,a1
	ifne	both
	subq.l		#2,a4
	endc
	dbra		d6,.loop2
.loop2_end:

	subq.l		#2,a0
	move.l		(a0),d7
	lsl.l		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
	swap		d7
;	and.w		d6,d7
;	not.w		d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
;	and.w		d6,d4
;	or.w		d7,d4
	jsr		(a6)
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc

	add.w		d5,a1
	ifne	both
	add.w		d5,a4
	endc
	swap		d5
	add.w		d5,a0
	swap		d5

	swap		d0
	dbra		d0,.loop1

	move.l		(a7)+,a6
	moveq		#1,d0
	rts

;pright_op:
	label	pright_op
	subq.l		#2,a0
	sub.l		#$20000,d5
	neg.w		d1
	swap		d0
.loop1:
	swap		d0
	move.l		(a0),d7
	lsr.l		d1,d7
	moveq		#-1,d6
	lsr.w		d2,d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
;	and.w		d6,d4
	not.w		d6
;	and.w		d6,d7
;	or.w		d7,d4
	jsr		(a6)
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc
	subq.l		#2,a0
	subq.l		#2,a1
	ifne	both
	subq.l		#2,a4
	endc

	move.w		d0,d6			; Good idea?
	beq		.loop2_end
	subq.w		#1,d6
.loop2:
	move.l		(a0),d7
	lsr.l		d1,d7
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
	jsr		10(a6)
	ifeq	both
	move.w		d7,(a1)
	endc
	ifne	both
	move.w		d7,(a4)
	move.w		d7,(a1)
	endc
	subq.l		#2,a0
	subq.l		#2,a1
	ifne	both
	subq.l		#2,a4
	endc
	dbra		d6,.loop2
.loop2_end:

	addq.l		#2,a0
	move.w		(a0),d7
	lsr.w		d1,d7
	moveq		#-1,d6
	lsr.w		d3,d6
;	swap		d7
;	and.w		d6,d7
;	not.w		d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
;	and.w		d6,d4
;	or.w		d7,d4
	jsr		(a6)
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc

	add.w		d5,a1
	ifne	both
	add.w		d5,a4
	endc
	swap		d5
	add.w		d5,a0
	swap		d5

	swap		d0
	dbra		d0,.loop1

	move.l		(a7)+,a6
	moveq		#1,d0
	rts


;single_op:
	label	single_op
	move.l		a6,-(a7)
	lsl.w		#2,d7
	lea		operations,a6
	move.l		(a6,d7.w),a6

	sub.w		d3,d1			; d1 - shift length
	jump	blt,sright_op

	swap		d0
	moveq		#-1,d6
	lsr.w		d3,d6
	move.w		#-1,d4
	lsr.w		d2,d4
	not.w		d4
	and.w		d4,d6
	not.w		d6
.loop1:
	move.l		(a0),d7
	lsl.l		d1,d7
	swap		d7
;	and.w		d6,d7
;	not.w		d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
;	and.w		d6,d4
;	not.w		d6
;	or.w		d7,d4
	jsr		(a6)
	not.w		d6
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc

	add.w		d5,a1
	ifne	both
	add.w		d5,a4
	endc
	swap		d5
	add.w		d5,a0
	swap		d5

	dbra		d0,.loop1

	move.l		(a7)+,a6
	moveq		#1,d0
	rts

;sright_op:
	label	sright_op
	neg.w		d1
	swap		d0

	moveq		#-1,d6
	lsr.w		d3,d6
	move.w		#-1,d4
	lsr.w		d2,d4
	not.w		d4
	and.w		d4,d6
	not.w		d6
.loop1:
	move.w		(a0),d7
	lsr.w		d1,d7

;	swap		d7
;	and.w		d6,d7
;	not.w		d6
	ifeq	both
	move.w		(a1),d4
	endc
	ifne	both
	move.w		(a4),d4
	endc
;	and.w		d6,d4
;	not.w		d6
;	or.w		d7,d4
	jsr		(a6)
	not.w		d6
	ifeq	both
	move.w		d4,(a1)
	endc
	ifne	both
	move.w		d4,(a4)
	move.w		d4,(a1)
	endc

	add.w		d5,a1
	ifne	both
	add.w		d5,a4
	endc
	swap		d5
	add.w		d5,a0
	swap		d5

	dbra		d0,.loop1

	move.l		(a7)+,a6
	moveq		#1,d0
	rts
