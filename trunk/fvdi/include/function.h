#ifndef FUNCTION_H
#define FUNCTION_H
/*
 * fVDI function declarations
 *
 * $Id: function.h,v 1.17 2006-11-26 07:30:56 standa Exp $
 *
 * Copyright 2003, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

extern Fontheader **linea_fonts(void);
extern void linea_setup(Workstation *);

extern long vector_call(void *vector, long data);

extern void do_nothing(void);
extern void mouse_move(void);
extern void mouse_timer(void);
extern void vbl_handler(void);

extern void bad_or_non_fvdi_handle(void);

extern int load_prefs(Virtual *vwk, char *sysname);
extern Virtual *initialize_vdi(void);
extern void copy_workstations(Virtual *def, long really_copy);
extern void setup_fallback(void);
extern void shut_down(void);
extern long tokenize(const char *buffer);

extern void v_bez_accel(long vwk, short *points, long num_points, long totmoves,
                        short *xmov, long pattern, long colour, long mode);
extern void lib_v_pline(Virtual *, void *);
extern void c_pline(Virtual *vwk, long num_pts, long colour, short *points);
extern void filled_poly(Virtual *vwk, short p[][2], long n, long colour, short *pattern, short *points, long mode, long interior_style);
extern void fill_poly(Virtual *vwk, short *p, long n, long colour, short *pattern, short *points, long mode, long interior_style);
extern void fill_area(Virtual *vwk, long x1, long y1, long x2, long y2, long colour);
extern void get_extent(Virtual *vwk, long length, short *text, short points[]);
extern void draw_text(Virtual *vwk, long x, long y, short *text, long length, long colour);
#if 0
extern void hline(void *, int x1, int y1, int y2, int colour, short *pattern);
#endif
extern void fill_spans(void *, short *, long n, long colour, short *pattern, long mode, long interior_style);

void do_arrow(Virtual *vwk, short *pts, int numpts, int colour, short *points, long mode);

extern void fill_poly(Virtual *vwk, short *p, long n, long colour, short *pattern,
                      short *points, long mode, long interior_style);

#if 0
extern	void	GEXT_DCALL(short *parmblock[5]);
#endif


#ifdef __PUREC__
extern void lib_vdi_s(void *, void *, short);
extern void lib_vdi_sp(void *, void *, short, void *);
extern void lib_vdi_spppp(void *, void *, short, void *, void *, void *, void *);
extern void lib_vdi_pp(void *, void *, void *, void *);
#else
 #ifdef __GNUC__
void *set_stack_call_pvlpl(void *stk, long size, void *func, Virtual *,   long, void *,   long);
long  set_stack_call_lppll(void *stk, long size, void *func,    void *, void *,   long,   long);
long  set_stack_call_lpppll(void *stk, long size, void *func,   void *, void *, void *,   long,   long);
long  set_stack_call_lplll(void *stk, long size, void *func,    void *,   long,   long,   long);
long  set_stack_call_lvplp(void *stk, long size, void *func, Virtual *, void *,   long, void *);
long  set_stack_call_lvppl(void *stk, long size, void *func, Virtual *, void *, void *,   long);
void lib_vdi_s(void *, void *, long);
void lib_vdi_sp(void *, void *, long, void *);
void lib_vdi_spppp(void *, void *, long, void *, void *, void *, void *);
void lib_vdi_pp(void *, void *, void *, void *);
 #else
void lib_vdi_s(void *, void *, short);
void lib_vdi_sp(void *, void *, short, void *);
void lib_vdi_spppp(void *, void *, short, void *, void *, void *, void *);
void lib_vdi_pp(void *, void *, void *, void *);
#define LIB_CALL {"224f4e92";}			/* move.l a7,a1   jsr (a2) */
#pragma inline lib_vdi_s(a2, a0, (short)) LIB_CALL
#pragma inline lib_vdi_sp(a2, a0, (short),) LIB_CALL
#pragma inline lib_vdi_spppp(a2, a0, (short),,,,) LIB_CALL
#pragma inline lib_vdi_pp(a2, a0,,) LIB_CALL
 #endif
#endif

extern void link_mouse_routines(void);
extern void unlink_mouse_routines(void);
extern void setup_vbl_handler(void);
extern void shutdown_vbl_handler(void);

extern int lib_vst_font(Virtual *vwk, long fontID);
extern int lib_vst_point(Virtual *vwk, long height, short *charw, short *charh,
			 short *cellw, short *cellh);
extern long lib_vst_color(Virtual *vwk, unsigned long colour);
extern void *lib_vsl_color;
extern void *lib_vsl_type;
extern void *lib_vsm_color;
extern void *lib_vsm_type;
extern void *lib_vsf_color;
extern void *lib_vsf_interior;
extern void *lib_vsf_style;
extern void *lib_vs_clip;
extern void *lib_vr_trn_fm;
extern void *lib_vrt_cpyfm;
extern void *lib_vrt_cpyfm_nocheck;
extern void *lib_vro_cpyfm;
#if 0
extern void lib_vst_color(Virtual *, short);
extern void lib_vst_font(Virtual *, short);
extern void lib_vst_point(Virtual *, short, short *, short *, short *, short *);
extern void lib_vsl_color(Virtual *, short);
extern void lib_vsl_type(Virtual *, short);
extern void lib_vsm_color(Virtual *, short);
extern void lib_vsm_type(Virtual *, short);
extern void lib_vsf_color(Virtual *, short);
extern void lib_vsf_interior(Virtual *, short);
extern void lib_vsf_style(Virtual *, short);
extern void lib_vs_clip(Virtual *, short, short *);
extern void lib_vr_trn_fm(Virtual *, MFDB *, MFDB *);
#endif
extern void opnvwk_values(Virtual *, VDIpars *);

extern short isqrt(unsigned long x);

extern long         (*external_init)(void);
extern Fontheader* (*external_load_font)(Virtual *vwk, const char *font);
extern long        (*external_vqt_extent)(Virtual *vwk, Fontheader *font, short *text, long length);
extern long        (*external_vqt_width)(Virtual *vwk, Fontheader *font, long ch);
extern Fontheader* (*external_vst_point)(Virtual *vwk, long size, short *sizes);
extern long        (*external_renderer)(Virtual *vwk, unsigned long coords,
					short *text, long length);
extern void*       (*external_char_bitmap)(Virtual *vwk, Fontheader *font, long ch, short *bitmap_info);
extern void*       (*external_char_advance)(Virtual *vwk, Fontheader *font, long ch, short *advance_info);

extern void        (*external_xfntinfo)(Virtual *vwk, Fontheader *font, long flags, XFNT_INFO *info);
extern void        (*external_fontheader)(Virtual *vwk, Fontheader *font, VQT_FHDR *fhdr);
#endif
