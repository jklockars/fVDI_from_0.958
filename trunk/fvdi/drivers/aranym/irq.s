*****
* ARAnyM driver event trampoline
*
* $Id: irq.s,v 1.1 2005-05-30 13:06:12 johan Exp $
*
* Copyright 2005, Johan Klockars
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

	xref	_event_handler

	xdef	_event_trampoline
	xdef	_next_handler

	
	text

* XBRA chain for IRQ level 2
	dc.b	"XBRA"
	dc.b	"ARAf"
next_handler:
_next_handler:
	dc.l	0

_event_trampoline:
	movem.l	d0-d2/a0-a2,-(a7)
	jsr	_event_handler
	movem.l	(a7)+,d0-d2/a0-a2
	move.l	next_handler,-(a7)	; Continue to next in chain
	rts

	end
	