	.macro	dc_name	string=""
	.byte	0,0
l\@a:
	.ascii	"\string"
l\@b:
	.byte	0
  .if (l\@b-l\@a) % 1
	.byte	0
  .endc
	.endm

  .ifndef	stack
*	.external	stack_address
*	.external	vdi_stack
  .endc

	.set	special_stack,0
SPECIAL_STACK_SIZE	=	512

  .ifdef	xbra_chain
*	.external	vdi_address
  .endc

  .ifdef	_stand_alone
*	xref	_stand_alone
  .endc


	.macro	save_regs
	movem.l	a1-a2,-(a7)
	.endm

	.macro	uses_d1
  .if	transparent
	move.l	d1,-(a7)
  .endif
	.endm

	.macro	used_d1
  .if	transparent
	move.l	(a7)+,d1
  .endif
	.endm

	.macro	restore_regs
	movem.l	(a7)+,a1-a2
	.endm

	.macro	return
  .ifne	special_stack
	move.l	(a7)+,a7
	add.l	#SPECIAL_STACK_SIZE,stack_address
	.set	special_stack,0
  .endif
	restore_regs
  .if transparent
	tst.w	_stand_alone
	bne	return\@
	moveq	#0x73,d0
	move.l	vdi_address(pc),-(a7)
	rts
return\@:
	rte
  .else
	rte
  .endif
	.endm

	.macro	real_return
  .ifne	special_stack
	move.l	(a7)+,a7
	add.l	#SPECIAL_STACK_SIZE,stack_address
	.set	special_stack,0
  .endif
	restore_regs
	rte
	.endm

	.macro	done_return
  .ifne	only_fvdi
	real_return
  .else
	return
  .endif
	.endm

	.macro	use_special_stack
	.set	special_stack,1
	move	sr,d0
	or	#$700,sr
	move.l	stack_address,a2
	cmp.l	#vdi_stack,a2
	beq	\@
	illegal
\@:
	move.l	a7,-(a2)
	move.l	a2,a7
	sub.w	#SPECIAL_STACK_SIZE-4,a2
	move.l	a2,stack_address
	move	d0,sr
	.endm

	.macro	ijsr indirect
  .if mc68000 == 1
	pea	\@
	move.l	indirect,-(a7)
	rts
\@:
  .else
	jsr	([indirect])
  .endif
	.endm

	.macro	ijmp indirect
  .ifne mc68000 == 1
	move.l	indirect,-(a7)
	rts
  .else
	jmp	([indirect])
  .endif
	.endm


