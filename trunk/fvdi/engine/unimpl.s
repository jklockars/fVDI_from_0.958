*****
* fVDI unimplemented functions
*
* $Id: unimpl.s,v 1.7 2004-10-17 21:44:11 johan Exp $
*
* Copyright 1997-2002, Johan Klockars 
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

transparent	equ	1		; Fall through?


*****
*
* What about:
* v_font   Lattice manual p260
* v_offset Lattice manual p267
*
*****
	include	"vdi.inc"
	include	"macros.inc"

	xref	redirect
	xref	_no_vex

	xdef	nothing
	xdef	v_clrwk,v_updwk
	xdef	vrq_locator,vrq_valuator,vrq_choice,vrq_string,vsin_mode
	xdef	v_contourfill
	xdef	vqin_mode
	xdef	vex_timv,vex_butv,vex_motv,vex_curv
	xdef	vq_mouse,vq_key_s
	xdef	v_cellarray,vq_cellarray
	xdef	vst_name,vst_width,vst_charmap,vst_kern,v_getbitmap_info
	xdef	v_getoutline,vst_scratch
	xdef	vst_error,vst_arbt,vqt_advance,vq_devinfo,v_savecache
	xdef	v_loadcache,v_flushcache,vst_setsize,vst_skew,vqt_cachesize
	xdef	vqt_get_table,vqt_fontheader,vqt_trackkern,vqt_pairkern
	xdef	v_set_app_buff
	xdef	vq_tabstatus,v_hardcopy,v_dspcur,v_rmcur,v_form_adv
	xdef	v_output_window,v_clear_disp_list,v_bit_image
;	xdef	v_rbox,v_rfbox
	xdef	start_unimpl,end_unimpl
	xdef	special_5,special_11


	text

start_unimpl:
	dc.b	"Unimplemented functions",0

* xxxx - Standard Trap function
* Unimplemented traps
* Todo: Implement them!

	dc.b	0,0,"No known function",0
* No known function
nothing:
special_5:
special_11:
	bra	redirect

	dc.b	0,"Physical Workstation",0
* Physical workstation manipulation
v_clrwk:
v_updwk:
	done_return

	dc.b	0,"Strange IO",0
* Strange mouse/keyboard functions
vrq_locator:
vrq_valuator:
vrq_choice:
vrq_string:
vqin_mode:
	bra	redirect
	dc.b	0,0,"vsin_mode",0
vsin_mode:			; An experiment
	move.l	intin(a1),a0
	move.w	2(a0),d1	; Mode
	move.w	0(a0),a0	; Device
	add.l	#setting,a0
	cmp.b	0(a0),d1
	beq	.done
	move.b	d1,0(a0)
	bra	redirect
.done:
	done_return


	data
setting:
	dc.b	-1,-1,-1,-1,-1,0


	text

	dc.b	0,0,"Normal IO",0
* Normal mouse/keyboard functions
vq_mouse:
vq_key_s:
	bra	redirect

	dc.b	0,0,"Vector exchange",0
* Vector exchange functions
vex_timv:
vex_butv:
vex_motv:
vex_curv:
	tst.w	_no_vex
	beq	redirect
	done_return

	dc.b	0,0,"Miscellaneous",0
vq_cellarray:
	bra	redirect

	dc.b	0,"NVDI",0
vst_name:
vst_width:
vst_charmap:
vst_kern:
v_getbitmap_info:
v_getoutline:
vst_scratch:
vst_error:
vst_arbt:
vqt_advance:
vq_devinfo:
v_savecache:
v_loadcache:
v_flushcache:
vst_setsize:
vst_skew:
vqt_cachesize:
vqt_get_table:
vqt_fontheader:
vqt_trackkern:
vqt_pairkern:
v_set_app_buff:
	done_return

	dc.b	0,0,"ESC",0
vq_tabstatus:
v_hardcopy:
v_dspcur:
v_rmcur:
v_form_adv:
v_output_window:
v_clear_disp_list:
v_bit_image:
	bra	redirect

	dc.b	0,0,"Drawing",0
v_contourfill:
v_cellarray:

  ifne 0
v_rbox:
v_rfbox:
	done_return
  endc

end_unimpl:

	end
