*****
* VDI performance logger v0.6
*
* $Id: vdi_prof.s,v 1.2 2002-05-14 01:25:10 johan Exp $
*
* Copyright 1997 & 2002, Johan Klockars 
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

VERSION		equ	0*16+7
TABLE_VERSION	equ	1*16+0
TABLE_SIZE	equ	256
ENTRIES		equ	4
FIVES		equ	20
ELEVENTHS	equ	15

ACTIVE		equ	$0001

	text

_main:	
	move.l	4(a7),a5	; Address of BP
	move.l	$c(a5),d0	; Length of TEXT
	add.l	$14(a5),d0	; Length of DATA
	add.l	$1c(a5),d0	; Length of BSS
	add.l	#256,d0		; Stack size
	add.l	#$100,d0	; BP
;	lea	program_size(pc),a0
;	move.l	d0,(a0)
	move.l	d0,a6		; Program size
	move.l	a5,d1
	add.l	d0,d1
	and.w	#$fffe,d1	; Even stack address
	move.l	d1,a7
	move.l	d0,-(a7)	; Keep d0 bytes
	move.l	a5,-(a7)	; From address a5
	move.w	d0,-(a7)	; dummy
	move.w	#$4a,-(a7)	; Mshrink
	trap	#1
	lea	12(a7),a7

* Set up Trap #2 intercept
	move.l	xbra_id,-(a7)
	move.l	#(34*4),-(a7)
	bsr	remove_xbra
	addq.l	#8,a7
	tst.l	d0
	beq	.no_remove

	pea	removing(pc)
	move.w	#$9,-(a7)	; Cconws
	trap	#1
	addq.l	#6,a7

.no_remove:
	pea	vdi_chk(pc)
	move.w	#34,-(a7)
	move.w	#$5,-(a7)	; Setexc
	trap	#13
	addq.l	#8,a7
	lea	vdi_address(pc),a0
	move.l	d0,(a0)

* Set up cookie stuff
	lea	cookie_values(pc),a0
	move.l	a0,a5
	move.b	#VERSION,(a0)+		; version
	move.b	#TABLE_VERSION,(a0)+	; table version
	move.w	#ACTIVE,(a0)+		; flags
	move.l	#0,(a0)+		; start time
	lea	count_table(pc),a1
	move.l	a1,(a0)+		; table address
	move.w	#TABLE_SIZE,(a0)+	; table size
	move.b	#FIVES,(a0)+		; number of routine 5 sub totals
	move.b	#ELEVENTHS,(a0)+	; number of routine 11 sub totals
;	move.l	#0,(a0)+		; total count
	lea	clear_table(pc),a1	; address of routine that clears the table
	move.l	a1,(a0)+
	lea	unlink_profiler(pc),a1	; address of routine that unlinks the profiler
	move.l	a1,(a0)+
	lea	relink_profiler(pc),a1	; address of routine that relinks the profiler
	move.l	a1,(a0)+
	
	move.l	xbra_id,-(a7)
	move.l	a5,-(a7)
	bsr	set_cookie
	addq.l	#8,a7
	tst.l	d0
	beq	.no_replace

	pea	replacing(pc)
	move.w	#$9,-(a7)	; Cconws
	trap	#1
	addq.l	#6,a7

.no_replace:
	pea	installed(pc)
	move.w	#$9,-(a7)	; Cconws
	trap	#1
	addq.l	#6,a7

	bsr	clear_table

* Terminate and stay resident
	move.w	#0,-(a7)
;	move.l	program_size(pc),-(a7)
	move.l	a6,-(a7)
	move.w	#$31,-(a7)	; Ptermres
	trap	#1
	illegal


* Clear function call count table
* and reset log start time
clear_table:
	movem.l	d0-d1/a0,-(a7)
	lea	count_table(pc),a0
	move.w	#((TABLE_SIZE+1)*ENTRIES)-1,d0
	moveq	#0,d1
.loop:	
	move.l	d1,(a0)+
	dbra	d0,.loop

	bsr	get_time
	lea	start_time(pc),a0
	move.l	d0,(a0)

	movem.l	(a7)+,d0-d1/a0
	rts

* Removes the logger from the Trap #2 chain
* Relies on all later 'linkees' supporting XBRA
unlink_profiler:
	movem.l	d0/a0,-(a7)
	lea	flags(pc),a0
	move.w	(a0),d0
	and.w	#ACTIVE,d0
	beq	.end_up

	move.l	xbra_id,-(a7)
	move.l	#(34*4),-(a7)
	bsr	remove_xbra
	addq.l	#8,a7
	tst.l	d0
	beq	.end_up
	
	eor.w	#ACTIVE,(a0)
.end_up:
	movem.l	(a7)+,d0/a0
	rts

* Reinserts the logger in the Trap #2 chain
relink_profiler:
	movem.l	d0/a0,-(a7)
	lea	flags(pc),a0
	move.w	(a0),d0
	and.w	#ACTIVE,d0
	bne	.end_rp

	eor.w	#ACTIVE,(a0)
.end_rp:
	movem.l	(a7)+,d0/a0
	rts


* XBRA chain for Trap #2
xbra:
	dc.b	"XBRA"
xbra_id:	
	dc.b	"VDIp"
vdi_address:
	ds.l	1

* Trap #2 interceptor
vdi_chk:
	cmp.w	#$73,d0
	bne	.go_vdi

	movem.l	d0/a0,-(a7)

	lea	original_ssp(pc),a0
	move.l	a7,d0
	addq.l	#8,d0
	move.l	d0,(a0)

	move.l	d1,a0
	move.l	(a0),a0		; control
	move.w	(a0),d0		; function number
	cmp.w	#11,d0
	bhi	.above_11
	bne	.below_11
	add.w	#(FIVES-1),d0
	add.w	10(a0),d0
	bra	.checked
.below_11:
	cmp.w	#5,d0
	bhi	.above_5
	bne	.checked
	add.w	10(a0),d0
	bra	.checked
.above_11:
	add.w	#(ELEVENTHS-1),d0
.above_5:
	add.w	#(FIVES-1),d0
.checked:
	cmp.w	#TABLE_SIZE,d0
	blo	.ok
	move.w	#TABLE_SIZE,d0
.ok:
	lsl.w	#4,d0
	lea	count_table(pc),a0
	add.w	d0,a0
	addq.l	#1,(a0)

	move.l	a0,d0
	lea	current_entry(pc),a0
	move.l	d0,(a0)

	lea	entry_time(pc),a0
.read_time_1:
	move.l	$4ba,d0
	move.b	$fffffa23.w,4(a0)
	cmp.l	$4ba,d0
	bne	.read_time_1
	move.l	d0,(a0)

	movem.l	(a7)+,d0/a0

* Pretend the call was from this code
* We need to check the time when the VDI returns
	move.w	#$88,-(a7)		; In case we're on >='020
	pea	vdi_ret(pc)
	move.w	sr,-(a7)

.go_vdi:	
	move.l	vdi_address(pc),-(a7)
	rts


* Record time after the VDI returns
vdi_ret:	
	movem.l	d0/a0,-(a7)

	lea	exit_time(pc),a0
.read_time_2:
	move.l	$4ba,d0
	move.b	$fffffa23.w,(a0)
	cmp.l	$4ba,d0
	bne	.read_time_2

	sub.l	entry_time(pc),d0
	move.l	current_entry(pc),a0
	add.l	d0,4(a0)
	moveq	#0,d0
	move.b	(entry_time+4)(pc),d0
	sub.b	exit_time(pc),d0
	bcc	.sign_ok
	or.l	#$ffffff00,d0
.sign_ok:
	add.l	d0,8(a0)

	movem.l	(a7)+,d0/a0
	move.l	original_ssp(pc),a7
	rte


***********
* Support routines 
***********

* Get the value of the 200Hz clock
get_time:
	move.l	#$4ba,-(a7)	; hz200
	bsr	get_protected_l
	addq.l	#4,a7
	rts

* Follow an XBRA chain and remove a link
* Returns d0 != 0 if found
remove_xbra:
	movem.l	a0-a1,-(a7)
	move.l	(4+2*4)(a7),a1
	move.l	a1,-(a7)
	bsr	get_protected_l		; Probably an exception vector
	addq.l	#4,a7
	move.l	d0,a0
	move.l	(8+2*4)(a7),d0
	move.l	xbra,d1
.search_xbra:	
	cmp.l	-12(a0),d1
	bne	.xbra_not
	cmp.l	-8(a0),d0
	beq	.xbra_found
	lea	-4(a0),a1
	move.l	(a1),a0
	bra	.search_xbra
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

* Set a cookie value
* Replace if there already is one (d0 != 0)
* Create/expand jar if needed
set_cookie:
	movem.l	d1-d2/d6-d7/a0-a1,-(a7)
	move.l	#$5a0,-(a7)	; _p_cookies
	bsr	get_protected_l
	addq.l	#4,a7
	move.l	d0,a1
	tst.l	d0
	beq	.full
	move.l	d0,a0
	moveq	#0,d0

	move.l	(8+6*4)(a7),d2	; Cookie name
.search_cookie:
	move.l	(a0),d1
	beq	.no_more
	addq.w	#1,d0
;	cmp.l	#'VDIp',d1
	cmp.l	d2,d1
	beq	.end_found
	addq.l	#8,a0
	bra	.search_cookie

.no_more:
	move.l	4(a0),d1
	cmp.l	d0,d1
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
;	move.l	#'VDIp',(a0)
;	lea	pointer_list(pc),a1
;	move.l	a1,4(a0)
	move.l	(8+6*4)(a7),(a0)	; Cookie name
	move.l	(4+6*4)(a7),4(a0)	; Cookie value
	movem.l	(a7)+,d1-d2/d6-d7/a0-a1
	rts
	
* Get a long value from low (protected) memory
get_protected_l:
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

* Set a long value in low (protected) memory
set_protected_l:
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


	data

removing:	dc.b	"Removing previous XBRA."
		dc.b	13,10,0
replacing:	dc.b	"Replacing previous Cookie."
		dc.b	13,10,0
installed:	dc.b	"VDI performance logger v0.5 now installed."
		dc.b	13,10,0

	even


	bss

;program_size:	ds.l	1
cookie_values:	
version:	ds.b	1
table_version:	ds.b	1
flags:		ds.w	1
start_time:	ds.l	1
table_address:	ds.l	1
table_size:	ds.w	1
no_subs:	ds.b	2
;total_count:	ds.l	1
clear_routine:	ds.l	1
unlink_routine:	ds.l	1
relink_routine:	ds.l	1

original_ssp:	ds.l	1

entry_time:	ds.l	1
		ds.b	1
exit_time:	ds.b	1

		even

current_entry:	ds.l	1
count_table:	ds.l	(TABLE_SIZE+1)*ENTRIES

	end
