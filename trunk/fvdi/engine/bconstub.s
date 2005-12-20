* fVDI Bconout(con) redirection calling stub
*
* Copyright 2005, Standa Opichal
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

	xdef	_bconout_stub
	xdef	_bconout_address

	xref	_screen_wk

	text

* XBRA chain for Bconout()
	dc.b	"XBRA"
	dc.b	"fVDI"
_bconout_address:
	ds.l	1

_bconout_stub:
	movem.l	d0-d2/a0-a2,-(a7)
	move.w	6*4+4+2(a7),d0

	move.l	d0,-(a7)
	jsr	_bconout_char
	addq.l	#4,a7

	movem.l	(a7)+,d0-d2/a0-a2

	tst.l	_screen_wk
	beq	.bconout_orig
	
	rts

.bconout_orig:
	move.l	_bconout_address,a0
	jmp	(a0)

	end
