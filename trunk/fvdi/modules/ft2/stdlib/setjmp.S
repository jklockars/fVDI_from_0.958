|
| setjmp.S
|

	.text
	.even

	.globl __mint_setjmp
__mint_setjmp:
	movel	sp@(4),a0		| address of jmp_buf[]
SETJMP:
	movel	sp@,a0@			| save return address
	moveml	d2-d7/a2-a7,a0@(4)	| save registers d2-d7/a2-a7
	clrl	d0			| return value is 0
	rts


	.globl __mint_longjmp
__mint_longjmp:
	movel	sp@(4),a0		| address of jmp_buf[]
#ifdef __MSHORT__
	movew	sp@(8),d0		| value to return
#else
	movel	sp@(8),d0		| value to return
#endif
	jne	L1			| may not be 0
	movql	#1, d0
L1:
	moveml	a0@(4),d2-d7/a2-a7	| restore saved reggies
	movl	a0@,sp@			| and the saved return address
	rts
