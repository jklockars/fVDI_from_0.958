#ifndef GLOBALS_H
#define GLOBALS_H
/*
 * fVDI global variable declarations
 *
 * $Id: globals.h,v 1.13 2006-02-20 17:03:15 standa Exp $
 *
 * Copyright 2003, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "relocate.h"

extern Virtual *handle[];
extern Virtual *default_virtual;
extern Virtual *non_fvdi_vwk;
extern Workstation *non_fvdi_wk;
extern Workstation *screen_wk;
extern List *driver_list;
extern long old_gdos;

extern short old_wk_handle;

extern short vbl_handler_installed;

extern long basepage;
extern short key_pressed;

extern List *driver_list;
extern long *pid;

extern long trap2_address;
extern long vdi_address;
extern long trap14_address;
extern long lineA_address;
extern long bconout_address;

extern void *trap2_temp;
extern void *trap14;
extern void *lineA;
extern void *bconout_stub;
extern void *vdi_dispatch;
extern void *eddi_dispatch;
extern void *init;
extern void *data_start;
extern void *bss_start;

extern long bconout_address;
extern void *bconout_stub;

extern long mint;
extern long magic;

extern void* dummy_vdi;

extern Function default_functions[];
extern void *default_opcode5[];
extern void *default_opcode11[];

extern void *default_line;
extern void *default_text;
extern void *default_fill;
extern void *default_expand;
extern void *default_blit;


extern short width;
extern short height;

extern long sub_call;


extern Access real_access;
extern Access *access;

extern long *pid;

extern Workstation *screen_wk;
extern Virtual     *screen_vwk;

extern char *vdi_stack_top;
extern long vdi_stack_size;

/*
 * Option values
 */
extern short disabled;
extern short booted;
extern short fakeboot;
extern short oldmouse;
extern short debug;
extern short nvdifix;
extern short lineafix;
extern short bconout;
extern short bconout_redir;
extern short xbiosfix;
extern short singlebend;
extern short memlink;
extern short blocks;
extern long  block_size;
extern long  log_size;
extern short arc_split;
extern short arc_min;
extern short arc_max;
extern short debug_out;
extern short interactive;
extern short stand_alone;
extern short nvdi_cookie;
extern short speedo_cookie;
extern short calamus_cookie;
extern char silent[];
extern char silentx[];
extern unsigned short sizes[];
extern short size_count;
extern short old_malloc;
extern short fall_back;
extern short move_mouse;
extern short ext_malloc;
extern short check_mem;
extern short bconout;
extern short file_cache_size;
extern char *debug_file;

extern long pid_addr;


/*
 * VDI call arrays
 */
extern short control[];
extern short int_in[];
extern short pts_in[];
extern short int_out[];
extern short pts_out[];

#endif

