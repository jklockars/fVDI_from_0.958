*****
* fVDI trap table
*
* $Id: traptabl.s,v 1.8 2005-07-26 21:37:55 johan Exp $
*
* Copyright 1997-2002, Johan Klockars 
* This software is licensed under the GNU General Public License.
* Please, see LICENSE.TXT for further information.
*****

transparent	equ	1		; Fall through?


	include	"vdi.inc"
	include	"macros.inc"

	xdef	default_functions,default_opcode5,default_opcode11
	xdef	_default_functions,_default_opcode5,_default_opcode11

	xref	opcode5,opcode11
	xref	v_opnvwk,vs_clip,vswr_mode
	xref	vst_color,vst_effects,vst_alignment,vst_rotation
	xref	vst_font,vqt_attributes
	xref	vsl_color,vsl_width,vsl_type,vsl_udsty,vsl_ends,vql_attributes
	xref	vsm_color,vsm_height,vsm_type,vqm_attributes
	xref	vsf_color,vsf_interior,vsf_style,vsf_perimeter,vqf_attributes
	xref	v_bar,vr_recfl,vrt_cpyfm,v_gtext

	xref	nothing,v_opnwk,v_clswk,v_clrwk,v_updwk,v_pline
	xref	v_pmarker,v_fillarea,vs_color,vq_color
	xref	vrq_locator,vrq_valuator,vrq_choice,vrq_string,vsin_mode
	xref	v_clsvwk,vq_extnd,v_contourfill,v_get_pixel,vro_cpyfm
	xref	vr_trn_fm,vsc_form,vsf_udpat,vqin_mode,vqt_extent,vqt_width
	xref	vex_timv,vst_load_fonts,vst_unload_fonts,v_show_c,v_hide_c
	xref	vq_mouse,vex_butv,vex_motv,vex_curv,vq_key_s,vqt_name
	xref	vqt_font_info,vex_wheelv
	xref	vst_height,vst_point,v_cellarray,vq_cellarray
	xref	vqt_xfntinfo,vst_name,vst_width,vst_charmap,vst_kern
	xref	v_getbitmap_info,vqt_f_extent,v_ftext,v_getoutline,vst_scratch
	xref	vst_error,vst_arbpt,vqt_advance,vq_devinfo,v_savecache
	xref	v_loadcache,v_flushcache,vst_setsize,vst_skew,vqt_cachesize
	xref	vqt_get_table,vqt_fontheader,vqt_trackkern,vqt_pairkern
	xref	v_set_app_buff,vq_chcells,v_exit_cur,v_enter_cur,v_curup
	xref	v_curdown,v_curright,v_curleft,v_curhome,v_eeos,v_eeol
	xref	vs_curaddress,v_curtext,v_rvon,v_rvoff,vq_curaddress
	xref	vq_tabstatus,v_hardcopy,v_dspcur,v_rmcur,v_form_adv
	xref	v_output_window,v_clear_disp_list,v_bit_image,v_arc,v_pie
	xref	v_circle,v_ellipse,v_ellarc,v_ellpie,v_rbox,v_rfbox
	xref	v_justified
	xref	vs_fg_color,vs_bg_color,vq_fg_color,vq_bg_color
	xref	vs_x_color.vq_x_color
	xref	special_5,special_11,v_bez_con
	xref	vr_transfer_bits,colour_entry
	xref	set_colour_table,colour_table,inverse_table


	data

	dc.w	0,0
	dc.l	v_set_app_buff		; Yes, this is at -1
_default_functions:
default_functions:
	dc.w	0,0
	dc.l	nothing			; 0
	dc.w	6,45
	dc.l	v_opnwk
	dc.w	0,0
	dc.l	v_clswk
	dc.w	0,0
	dc.l	v_clrwk
	dc.w	0,0
	dc.l	v_updwk
	dc.w	0,0
	dc.l	opcode5
	dc.w	0,0
	dc.l	v_pline			; Also v_bez (sub 13)
	dc.w	0,0
	dc.l	v_pmarker
	dc.w	0,0
	dc.l	v_gtext
	dc.w	0,0
	dc.l	v_fillarea		; Also v_bez_fill (sub 13)
	dc.w	0,0
	dc.l	v_cellarray
	dc.w	0,0
	dc.l	opcode11
	dc.w	2,0
	dc.l	vst_height
	dc.w	0,1
	dc.l	vst_rotation
	dc.w	0,0
	dc.l	vs_color
	dc.w	0,1
	dc.l	vsl_type
	dc.w	1,0
	dc.l	vsl_width
	dc.w	0,1
	dc.l	vsl_color
	dc.w	0,1
	dc.l	vsm_type
	dc.w	1,0
	dc.l	vsm_height
	dc.w	0,1
	dc.l	vsm_color
	dc.w	0,1
	dc.l	vst_font
	dc.w	0,1
	dc.l	vst_color
	dc.w	0,1
	dc.l	vsf_interior
	dc.w	0,1
	dc.l	vsf_style
	dc.w	0,1
	dc.l	vsf_color
	dc.w	0,4
	dc.l	vq_color
	dc.w	0,0			; Don't know this
	dc.l	vq_cellarray
	dc.w	1,1
	dc.l	vrq_locator		; Also vsm_ (vsin_mode decides)
	dc.w	0,2
	dc.l	vrq_valuator		;   the numbers in intout
	dc.w	0,1
	dc.l	vrq_choice		;   and ptsout can vary
	dc.w	0,0
	dc.l	vrq_string		;   for these operations
	dc.w	0,1
	dc.l	vswr_mode
	dc.w	0,1
	dc.l	vsin_mode
	dc.w	0,0
	dc.l	nothing
	dc.w	1,5
	dc.l	vql_attributes
	dc.w	1,3
	dc.l	vqm_attributes
	dc.w	0,5
	dc.l	vqf_attributes
	dc.w	2,6
	dc.l	vqt_attributes
	dc.w	0,2
	dc.l	vst_alignment
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing	; 40-99
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.w	6,45
	dc.l	v_opnvwk		; also v_opnbm
	dc.w	0,0
	dc.l	v_clsvwk		; also v_clsbm
	dc.w	6,45
	dc.l	vq_extnd		; also vq_scrninfo
	dc.w	0,0
	dc.l	v_contourfill
	dc.w	0,1
	dc.l	vsf_perimeter
	dc.w	0,2
	dc.l	v_get_pixel
	dc.w	0,1
	dc.l	vst_effects
	dc.w	2,1
	dc.l	vst_point
	dc.w	0,0
	dc.l	vsl_ends
	dc.w	0,0
	dc.l	vro_cpyfm
	dc.w	0,0
	dc.l	vr_trn_fm
	dc.w	0,0
	dc.l	vsc_form
	dc.w	0,0
	dc.l	vsf_udpat
	dc.w	0,0
	dc.l	vsl_udsty
	dc.w	0,1
	dc.l	vr_recfl
	dc.w	0,1
	dc.l	vqin_mode
	dc.w	4,0
	dc.l	vqt_extent
	dc.w	3,1
	dc.l	vqt_width
	dc.w	0,1
	dc.l	vex_timv
	dc.w	0,1
	dc.l	vst_load_fonts
	dc.w	0,0
	dc.l	vst_unload_fonts
	dc.w	0,0
	dc.l	vrt_cpyfm
	dc.w	0,0
	dc.l	v_show_c
	dc.w	0,0
	dc.l	v_hide_c
	dc.w	1,1
	dc.l	vq_mouse
	dc.w	0,0
	dc.l	vex_butv
	dc.w	0,0
	dc.l	vex_motv
	dc.w	0,0
	dc.l	vex_curv
	dc.w	0,1
	dc.l	vq_key_s
	dc.w	0,0
	dc.l	vs_clip
	dc.w	0,34
	dc.l	vqt_name
	dc.w	5,2
	dc.l	vqt_font_info
	dc.l	0,nothing	; vqt_justified?
	dc.l	0,nothing
	dc.l	0,vex_wheelv
	dc.l	0,nothing,0,nothing	; 135-199
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing
	dc.w	0,0
	dc.l	vr_transfer_bits
	dc.l	0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing
	dc.w	0,1
	dc.l	vs_fg_color		; 200
	dc.w	0,1
	dc.l	vs_bg_color
	dc.w	0,6
	dc.l	vq_fg_color
	dc.w	0,6
	dc.l	vq_bg_color
	dc.w	0,0
	dc.l	colour_entry
	dc.w	0,1
	dc.l	set_colour_table
	dc.w	0,0
	dc.l	colour_table
	dc.w	0,0
	dc.l	vs_x_color
	dc.w	0,0
	dc.l	inverse_table
	dc.w	0,0
	dc.l	vq_x_color
	dc.l	0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing,0,nothing,0,nothing,0,nothing
	dc.l	0,nothing,0,nothing
	dc.w	0,3
	dc.l	vqt_xfntinfo
	dc.w	0,0
	dc.l	vst_name
	dc.w	0,0
	dc.l	vst_width
	dc.w	0,0
	dc.l	vqt_fontheader
	dc.w	0,0
	dc.l	nothing
	dc.w	0,0
	dc.l	vqt_trackkern
	dc.w	0,0
	dc.l	vqt_pairkern
	dc.w	0,0
	dc.l	vst_charmap
	dc.w	0,0
	dc.l	vst_kern		; also vst_track_offset
	dc.w	0,0
	dc.l	nothing
	dc.w	0,0
	dc.l	v_getbitmap_info
	dc.w	0,0
	dc.l	vqt_f_extent
	dc.w	0,0
	dc.l	v_ftext
	dc.w	0,0
	dc.l	nothing
	dc.w	0,0
	dc.l	v_getoutline
	dc.w	0,0
	dc.l	vst_scratch
	dc.w	0,0
	dc.l	vst_error
	dc.w	0,0
	dc.l	vst_arbpt
	dc.w	0,0
	dc.l	vqt_advance
	dc.w	0,0
	dc.l	vq_devinfo		; also vq_ext_devinfo (sub 4242)
	dc.w	0,0
	dc.l	v_savecache
	dc.w	0,0
	dc.l	v_loadcache
	dc.w	0,0
	dc.l	v_flushcache
	dc.w	0,0
	dc.l	vst_setsize
	dc.w	0,0
	dc.l	vst_skew
	dc.w	0,0
	dc.l	vqt_get_table
	dc.w	0,0
	dc.l	vqt_cachesize	; this is number 255

	dc.w	23
_default_opcode5:
default_opcode5:
	dc.l	special_5		; This is function code 0
	dc.l	vq_chcells		; Should have 0,2
	dc.l	v_exit_cur
	dc.l	v_enter_cur
	dc.l	v_curup
	dc.l	v_curdown
	dc.l	v_curright
	dc.l	v_curleft
	dc.l	v_curhome
	dc.l	v_eeos
	dc.l	v_eeol
	dc.l	vs_curaddress
	dc.l	v_curtext
	dc.l	v_rvon
	dc.l	v_rvoff
	dc.l	vq_curaddress		; Should have 0,2
	dc.l	vq_tabstatus		; Should have 0,1
	dc.l	v_hardcopy
	dc.l	v_dspcur
	dc.l	v_rmcur
	dc.l	v_form_adv
	dc.l	v_output_window
	dc.l	v_clear_disp_list
	dc.l	v_bit_image

	dc.w	13
_default_opcode11:
default_opcode11:
	dc.l	special_11
	dc.l	v_bar
	dc.l	v_arc
	dc.l	v_pie
	dc.l	v_circle
	dc.l	v_ellipse
	dc.l	v_ellarc
	dc.l	v_ellpie
	dc.l	v_rbox
	dc.l	v_rfbox
	dc.l	v_justified
	dc.l	nothing
	dc.l	nothing
	dc.l	v_bez_con


  ifne 0
*** This has no known number so far
v_kill_outline

*** Other NVDI things and such

5,24
	dc.l	vq_scan
5,25
	dc.l	v_alpha_text

5,60
	dc.l	vs_palette

5,61
	dc.l	v_sound

5,62
	dc.l	vs_mute

5,76
	dc.l	vs_calibrate
5,77
	dc.l	vq_calibrate

5,81
	dc.l	vt_resolution
5,82
	dc.l	vt_axis
5,83
	dc.l	vt_origin
5,84
	dc.l	vq_tdimensions
5,85
	dc.l	vt_alignment

5,91
	dc.l	vsp_film
5,91
	dc.l	vqp_films
5,92
	dc.l	vqp_filmname
5,92
	dc.l	vqp_state
5,93
	dc.l	vqp_expose
5,93
	dc.l	vsp_state
5,94
	dc.l	vsp_save
5,95
	dc.l	vsp_message
5,96
	dc.l	vqp_error

5,98
	dc.l	v_meta_extents
5,99
	dc.l	v_write_meta
5,99
	dc.l	v_bez_qual
5,99,0
	dc.l	vm_pagesize
5,99,1
	dc.l	vm_coords
5,100
	dc.l	vm_filename
5,101
	dc.l	v_offset
5,102
	dc.l	v_fontinit

5,2000
	dc.l	v_escape2000=v_pgcount

  endc

	end
