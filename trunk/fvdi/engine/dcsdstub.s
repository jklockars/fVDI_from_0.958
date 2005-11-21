*****
* fVDI Calamus stub functions
*
* Copyright 2004, Standa Opichals
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

	xref	_dcsd_offscreen_mfdb
	xref	_is_active

	xref	_dcsd_gettlt
	xref	_dcsd_blit_from_screen
	xref	_dcsd_blit_to_screen

	xdef	_dcsd_stub_active
	xdef	_dcsd_stub_getbase
	xdef	_dcsd_stub_gettlt
	xdef	_dcsd_stub_blit_from_screen
	xdef	_dcsd_stub_blit_to_screen


	text

	dc.b	0,0,"dcsd_stub",0
_dcsd_stub_active:
	move.l	_is_active,d0
	rts

_dcsd_stub_getbase:
	move.l	_dcsd_offscreen_mfdb,a0
	rts

_dcsd_stub_gettlt:
	movem.l	d3-d7/a2-a6,-(a7)
	move.l	a0,-(a7)
	jsr	_dcsd_gettlt
	addq.l	#4,a7
	movem.l	(a7)+,d3-d7/a2-a6
	rts

_dcsd_stub_blit_from_screen:
	movem.l	d3-d7/a2-a6,-(a7)
	move.l	a0,-(a7)
	jsr	_dcsd_blit_from_screen
	addq.l	#4,a7
	movem.l	(a7)+,d3-d7/a2-a6
	rts

_dcsd_stub_blit_to_screen:
	movem.l	d3-d7/a2-a6,-(a7)
	move.l	a0,-(a7)
	jsr	_dcsd_blit_to_screen
	addq.l	#4,a7
	movem.l	(a7)+,d3-d7/a2-a6
	rts

	end
