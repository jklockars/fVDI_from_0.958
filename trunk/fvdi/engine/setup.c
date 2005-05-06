/*
 * fVDI workstation setup functions
 *
 * $Id: setup.c,v 1.4 2005-05-06 12:29:37 johan Exp $
 *
 * Copyright 1999-2000/2003, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "os.h"
#include "fvdi.h"
#include "relocate.h"
#include "utility.h"
#include "function.h"
#include "globals.h"

#define HANDLES 32            /* Important things regarding this also in fvdi.s */
#define WHITE 0
#define BLACK 1
#define MAX_OLD_HANDLE 16


/*
 * Global variables
 */

Virtual *default_virtual = 0;
Virtual *screen_virtual = 0;
Virtual *handle[HANDLES];
Virtual **handle_link = 0;

Virtual *non_fvdi_vwk = 0;     /* Only func ptrs, handle == -1 for bad else fall through */
Workstation *non_fvdi_wk = 0;

short old_wk_handle = 0;   /* Was 1, [010109] */

Workstation *screen_wk = 0;
void *old_curv = 0;
void *old_timv = 0;

/*
 * Set up initial real and virtual workstations.
 * Returns a suitable template virtual.
 */
Virtual *initialize_vdi(void)
{
   Workstation *wk, *dummy_wk;
   Virtual *vwk, *dummy_vwk;
   char *tmp;
   int i;
   long func_tab_start;
   
   /*
    * non_fvdi_wk  - A dummy workstation with all function pointers set to 'bad_or_non_vdi_handle'.
    *                The function table is the only part that actually exists in memory.
    * non_fvdi_vwk - A dummy virtual workstation with the above as 'base'.
    *                Only the pointer to the above and the 'standard handle' (-1) actually exists.
    *                Used for all unallocated entries in the handled table.
    */

   if (!(tmp = (char *)malloc(sizeof(Workstation *) + sizeof(short) + 257 * sizeof(Function))))
      return 0;

   func_tab_start = (long)&((Workstation *)tmp)->function[-1] - (long)tmp;
   dummy_wk = (Workstation *)(tmp + sizeof(Workstation *) + sizeof(short) - func_tab_start);
   dummy_vwk = (Virtual *)tmp;
   dummy_vwk->real_address = (void *)dummy_wk;
   dummy_vwk->standard_handle = -1;
   for(i = -1; i < 256; i ++) {
      dummy_wk->function[i].retvals[0] = 0;
      dummy_wk->function[i].retvals[1] = 0;
      dummy_wk->function[i].code = bad_or_non_fvdi_handle;
   }
   non_fvdi_wk = dummy_wk;
   non_fvdi_vwk = dummy_vwk;
   for(i = 0; i < HANDLES; i++)
      handle[i] = dummy_vwk;


   if (!(wk = (Workstation *)malloc(sizeof(Workstation)))) {
      free(tmp);
      return 0;
   }

   if (!(vwk = (Virtual *)malloc(sizeof(Virtual)))) {
      free(wk);
      free(tmp);
      return 0;
   }

   /*
    * Set up the default real and virtual workstations.
    * These are never actually used, but can be considered templates with some
    * reasonable initial values.
    * The virtual workstation is both returned and stored as 'default_virtual'.
    */
/* Screen */
   wk->driver = 0;
   wk->screen.type = 0;
   wk->screen.palette.colours = 0;
   wk->screen.palette.transformation = 0;
   wk->screen.mfdb.address = 0;
   wk->screen.mfdb.wdwidth = 0;
   wk->screen.mfdb.standard = 0;
   wk->screen.linea = 0;
   wk->screen.shadow.buffer = 0;
   wk->screen.shadow.address = 0;
   wk->screen.shadow.wrap = 0;
/* */
   wk->screen.pixel.width = 238;       /* Used to be 353 */
   wk->screen.pixel.height = 149;
/* */
   wk->screen.coordinates.course = 0;
/* */
   wk->screen.coordinates.min_x = 0;
   wk->screen.coordinates.min_y = 0;
/* */
   wk->writing.fonts = 0;
   wk->writing.first_font = 0;
   wk->writing.effects = 0x1f;              /* Outline/underline/italic/light/bold supported */
   wk->writing.rotation.possible = 0;
   wk->writing.rotation.type = 0;
   wk->writing.justification = 0;
/* */
   wk->writing.size.possibilities = 3;	    /* Get these some real way! */
   wk->writing.size.height.min = 4;
   wk->writing.size.height.max = 13;
   wk->writing.size.width.min = 5;
   wk->writing.size.width.max = 7;
/* */
   wk->drawing.primitives.supported = 8;    /* Everything but rounded rectangles */
   wk->drawing.primitives.attributes = ((3 + 1) <<  0) + ((0 + 1) <<  3) + ((3 + 1) <<  6) +
	                                    ((3 + 1) <<  9) + ((3 + 1) << 12) + ((0 + 1) << 15) +
	                                    ((3L + 1) << 18) + ((0L + 0) << 21) + ((0L + 0) << 24) + ((2L + 1) << 27);
   wk->drawing.rubber_banding = 0;
   wk->drawing.flood_fill = 0;
   wk->drawing.writing_modes = 4;
   wk->drawing.fill.possible = 1;
   wk->drawing.fill.patterns = 24;
   wk->drawing.fill.hatches = 12;
   wk->drawing.marker.types = 6;
   wk->drawing.marker.size.possibilities = 8;
   wk->drawing.marker.size.height.min = 11;
   wk->drawing.marker.size.height.max = 88;
   wk->drawing.marker.size.width.min = 15;
   wk->drawing.marker.size.width.max = 120;
   wk->drawing.line.types = 7;
   wk->drawing.line.wide.width.possibilities = 0;
   wk->drawing.line.wide.width.max = 31;	/* Was 255 */
   wk->drawing.line.wide.width.min = 1;
   wk->drawing.line.wide.types_possible = 1;
   wk->drawing.line.wide.writing_modes = 0;
   wk->drawing.bezier.available = 1;
   wk->drawing.bezier.depth_scale.min = 9;
   wk->drawing.bezier.depth_scale.max = 0;
   wk->drawing.bezier.depth.min = 2;
   wk->drawing.bezier.depth.max = 7;
   wk->drawing.cellarray.available = 0;
   wk->raster.scaling = 0;
/* 16x16 op/s */
   wk->various.input_type = 0;
/* Nedsvartning */
   wk->various.max_ptsin = 1024;		/* Was -1 */
   wk->various.max_intin = -1;
   wk->various.buttons = 0;
   wk->various.cursor_movement = 0;
   wk->various.number_entry = 0;
   wk->various.selection = 0;
   wk->various.typing = 0;
   wk->various.workstation_type = 0;
/* Console */
/* Mouse */
   wk->mouse.type = 0;           /* Default to old VDI mouse */
   wk->mouse.hide = 0;
   wk->mouse.position.x = 0;
   wk->mouse.position.y = 0;
   wk->r.set_palette = 0;
   wk->r.get_colour = 0;
   wk->r.set_pixel = 0;
   wk->r.get_pixel = 0;
   wk->r.line = &default_line;
   wk->r.expand = &default_expand;
   wk->r.fill = &default_fill;
   wk->r.fillpoly = 0;
   wk->r.blit = &default_blit;
   wk->r.text = &default_text;
   wk->r.mouse = 0;

   copymem(&default_functions[-1], &wk->function[-1], 257 * sizeof(Function));
   wk->opcode5_count = *(short *)((long)default_opcode5 - 2);
   copymem(default_opcode5, wk->opcode5, (wk->opcode5_count + 1) * sizeof(void *));
   wk->opcode11_count = *(short *)((long)default_opcode11 - 2);
   copymem(default_opcode11, wk->opcode11, (wk->opcode11_count + 1) * sizeof(void *));

   vwk->clip.on = 0;
   vwk->clip.rectangle.x1 = 0;
   vwk->clip.rectangle.y1 = 0;
   vwk->clip.rectangle.x2 = 0;
   vwk->clip.rectangle.y2 = 0;
   vwk->text.colour.background = WHITE;
   vwk->text.colour.foreground = BLACK;
   vwk->text.effects = 0;
   vwk->text.alignment.horizontal = 0;
   vwk->text.alignment.vertical = 0;
   vwk->text.rotation = 0;
   vwk->text.font = 0;
   vwk->text.current_font = 0;        /* Address will be set on first call to vst_font */
   vwk->line.colour.background = WHITE;
   vwk->line.colour.foreground = BLACK;
   vwk->line.width = 1;
   vwk->line.type = 1;
   vwk->line.ends.beginning = 0;
   vwk->line.ends.end = 0;
   vwk->line.user_mask = 0xffff;
   vwk->bezier.on = 0;                /* Should these really be per vwk? */
   vwk->bezier.depth_scale = 0;
   vwk->marker.colour.background = WHITE;
   vwk->marker.colour.foreground = BLACK;
   vwk->marker.size.height = 11;
   vwk->marker.size.width = 15;
   vwk->marker.type = 3;
   vwk->fill.colour.background = WHITE;
   vwk->fill.colour.foreground = BLACK;
   vwk->fill.interior = 0;
   vwk->fill.style = 1;
   vwk->fill.perimeter = 1;
   vwk->fill.user.pattern.in_use = 0;
   vwk->fill.user.pattern.extra = 0;
   vwk->fill.user.multiplane = 0;
   vwk->mode = 1;

   vwk->real_address = (void *)wk;
   vwk->standard_handle = 1;
   vwk->palette = 0;

   default_virtual = vwk;     /* handle[0]? */

   return vwk;
}


/*
 * Copy colours from an existing virtual workstation into an fVDI one.
 */
void setup_colours(Virtual *vwk)
{
   Workstation *wk;
   short colours[256][3];
   short intout[45];
   int i, size;
   int handle;
   
   handle = vwk->standard_handle;
	
   wk = vwk->real_address;
   size = wk->screen.palette.size;
   for(i = 0; i < size; i++) {
      vq_color(handle, i, 0, intout);
      if (intout[0] == -1) {
         size = i;           /* Should not really happen */
         break;
      }
      colours[i][0] = intout[1];
      colours[i][1] = intout[2];
      colours[i][2] = intout[3];
   }
   initialize_palette(vwk, 0, size, colours, wk->screen.palette.colours);
}


/*
 * Copy all available information from existing virtual workstation with handle 'vwk_no'
 * into an fVDI one created using 'def' as a template.
 */
void copy_setup(Virtual *def, int vwk_no, short intout[], short ptsout[])
{
   Virtual *vwk;
   short tmp;
   
   vwk = (Virtual *)malloc(sizeof(Virtual) + 32);  /* Extra for fill pattern */
   if (!vwk)
      return;

   copymem(def, vwk, sizeof(Virtual));
   vwk->fill.user.pattern.in_use = (short *)&vwk[1];  /* Right behind vwk */
   vwk->fill.user.pattern.extra = 0;
   vwk->fill.user.multiplane = 0;
   vwk->standard_handle = vwk_no;
   handle[vwk_no] = vwk;

#if 0
   if ((vwk_no == old_wk_handle) || !vwk->real_address->screen.look_up_table)
#else
   if ((vwk_no == old_wk_handle) || (vwk->real_address->driver->device->clut == 2))
#endif
      setup_colours(vwk);

   set_inout(0, 0, intout, ptsout);
   vdi(vwk_no, 35, 0, 0);     /* vql_attributes */
   set_inout(0, 0, 0, 0);
   int_in[0] = intout[2];        /* Mode */
   fvdi(vwk_no, 32, 0, 1);    /* vswr_mode */
   int_in[0] = intout[0];        /* Type */
   fvdi(vwk_no, 15, 0, 1);    /* vsl_type */
   int_in[0] = intout[1];        /* Colour */
   fvdi(vwk_no, 16, 0, 1);    /* vsl_color */
   int_in[0] = intout[3];        /* Beginning */
   int_in[1] = intout[4];        /* End */
   fvdi(vwk_no, 108, 0, 2);   /* vsl_ends */
   pts_in[0] = ptsout[0];        /* Width */
   pts_in[1] = 0;
   fvdi(vwk_no, 16, 1, 0);         /* vsl_width */

   set_inout(0, 0, intout, ptsout);
   vdi(vwk_no, 36, 0, 0);     /* vqm_attributes */
   set_inout(0, 0, 0, 0);
   int_in[0] = intout[0];        /* Type */
   fvdi(vwk_no, 18, 0, 1);    /* vsm_type */
   int_in[0] = intout[1];        /* Colour */
   fvdi(vwk_no, 20, 0, 1);    /* vsm_color */
   pts_in[0] = ptsout[0];        /* Width (0 according to docs) */
   pts_in[1] = ptsout[1];        /* Height */
   fvdi(vwk_no, 19, 1, 0);    /* vsm_height */

   set_inout(0, 0, intout, 0);
   vdi(vwk_no, 37, 0, 0);     /* vqf_attributes */
   set_inout(0, 0, 0, 0);
   int_in[0] = intout[0];        /* Interior */
   fvdi(vwk_no, 23, 0, 1);    /* vsf_interior */
   int_in[0] = intout[1];        /* Colour */
   fvdi(vwk_no, 25, 0, 1);    /* vsf_color */
   int_in[0] = intout[2];        /* Style */
   fvdi(vwk_no, 24, 0, 1);    /* vsf_style */
   int_in[0] = intout[4];        /* Perimeter */
   fvdi(vwk_no, 104, 0, 1);   /* vsf_perimeter */

   set_inout(0, 0, intout, 0);
   vdi(vwk_no, 38, 0, 0);     /* vqt_attributes */
   set_inout(0, 0, 0, 0);
   int_in[0] = intout[0];        /* Font */
   fvdi(vwk_no, 21, 0, 1);    /* vst_font */
   int_in[0] = intout[1];        /* Colour */
   fvdi(vwk_no, 22, 0, 1);    /* vst_color */
   int_in[0] = intout[2];        /* Rotation */
   fvdi(vwk_no, 13, 0, 1);    /* vst_rotation */
   int_in[0] = intout[3];        /* Horizontal alignment */
   int_in[1] = intout[4];        /* Vertical alignment */
   fvdi(vwk_no, 39, 0, 2);    /* vst_alignment */

   set_inout(0, 0, intout, ptsout);
   vdi(vwk_no, 131, 0, 0);    /* vqt_font_info */
   set_inout(0, 0, 0, 0);
   pts_in[0] = 0;
   pts_in[1] = ptsout[9];        /* Baseline to top line */
   fvdi(vwk_no, 12, 1, 0);    /* vst_height */
   
   set_inout(0, 0, intout, ptsout);
   vdi(vwk_no, 102, 0, 1);    /* vq_extnd */
   set_inout(0, 0, 0, 0);
   tmp = int_out[19];
   if (tmp & 0xfffe)
      tmp = 0;
   if ((ptsout[0] >= 0) && (ptsout[0] < vwk->real_address->screen.mfdb.width))
      pts_in[0] = ptsout[0];
   else
      tmp = 0;
   if ((ptsout[1] >= 0) && (ptsout[1] < vwk->real_address->screen.mfdb.height))
      pts_in[1] = ptsout[1];
   else
      tmp = 0;
   if ((ptsout[2] >= ptsout[0]) && (ptsout[2] < vwk->real_address->screen.mfdb.width))
      pts_in[2] = ptsout[2];
   else
      tmp = 0;
   if ((ptsout[3] >= ptsout[1]) && (ptsout[3] < vwk->real_address->screen.mfdb.height))
      pts_in[3] = ptsout[3];
   else
      tmp = 0;
   int_in[0] = tmp;
   fvdi(vwk_no, 129, 2, 1);    /* vs_clip */

   /* This should perhaps only be done per Workstation */
   vdi(vwk_no, 124, 0, 0);    /* vq_mouse */
   vwk->real_address->mouse.position.x = pts_out[0];
   vwk->real_address->mouse.position.y = pts_out[1];
   vwk->real_address->mouse.hotspot.x = 0;
   vwk->real_address->mouse.hotspot.y = 0;
}


void link_mouse_routines(void)
{
      *(long *)&control[7] = (long)mouse_move;
      if (booted && !fakeboot)
         sub_vdi(old_wk_handle, 127, 0, 0);
      else
         vdi(old_wk_handle, 127, 0, 0);
      old_curv = (void *)*(long *)&control[9];
      *(long *)&control[7] = (long)mouse_timer;
      if (booted && !fakeboot)
         sub_vdi(old_wk_handle, 118, 0, 0);
      else
         vdi(old_wk_handle, 118, 0, 0);
      old_timv = (void *)*(long *)&control[9];
}


void unlink_mouse_routines(void)
{
   if (old_curv) {
      *(long *)&control[7] = (long)old_curv;
      vdi(old_wk_handle, 127, 0, 0);
   }
   if (old_timv) {
      *(long *)&control[7] = (long)old_timv;
      vdi(old_wk_handle, 118, 0, 0);
   }
}


/*
 * Find all previously opened workstations and copy their setup.
 * Also links in the mouse routines under some circumstances.
 */
void copy_workstations(Virtual *def, long really_copy)
{
   short new_handle, last_handle, i, j, n;
   short handles[MAX_OLD_HANDLE];
   short intout[45], ptsout[12];
   char *tmp;

   screen_virtual = def;

   appl_init();
   old_wk_handle = graf_handle();
   appl_exit();

   /*
    * Try opening a large amount of new virtual workstations.
    * The handles we don't get represent the already opened ones.
    */

   n = 0;
   do {
      new_handle = call_v_opnvwk(old_wk_handle, intout, ptsout);
      handles[n++] = new_handle;
   } while (new_handle <= MAX_OLD_HANDLE);
   for(i = n - 1; i >= 0; i--)
      call_v_clsvwk(handles[i]);

   /*
    * Copy all the virtual screen workstations that were found,
    * and make dummy virtuals for everything else.
    */

#if 0     /* This needs some more thinking [010324] */
   if (!really_copy && (n <= MAX_OLD_HANDLE))                      /* This is for the dummy virtuals */
      tmp = (char *)malloc((sizeof(Workstation *) + sizeof(short)) * (MAX_OLD_HANDLE - n + 1), 3);
#else
   if (n <= MAX_OLD_HANDLE)                      /* This is for the dummy virtuals */
   tmp = (char *)malloc((sizeof(Workstation *) + sizeof(short)) * (MAX_OLD_HANDLE - n + 1));
#endif

   last_handle = 0;
   for(j = 0; j < n; j++) {
      new_handle = handles[j];
      for(i = last_handle + 1; i < new_handle; i++) {
         vq_extnd(i, 1, intout, ptsout);
         if (!intout[0]) {                                         /* Not a screen device? */
            handle[i] = (Virtual *)tmp;
            tmp += sizeof(Workstation *) + sizeof(short);
            handle[i]->real_address = (void *)non_fvdi_wk;
            handle[i]->standard_handle = i | 0x8000;               /* Don't try to open virtuals for this one */
            continue;
         }
         if (really_copy)
            copy_setup(def, i, intout, ptsout);
         else {
            handle[i] = (Virtual *)tmp;                            /* If we're not copying, */
            tmp += sizeof(Workstation *) + sizeof(short);          /*  set up dummies for pass-through. */
            handle[i]->real_address = (void *)non_fvdi_wk;
            handle[i]->standard_handle = i;
         }
      }
      last_handle = new_handle;
   }
   
   if (!disabled && !oldmouse && really_copy) {
      screen_wk = handle[1]->real_address;
      link_mouse_routines();
   }
}


/*
 * Really supposed to handle most shut-down operations.
 */
void shut_down(void)
{
   unlink_mouse_routines();
}


/*
 * When fVDI is being booted, this will open a
 * screen workstation using the old VDI.
 * This is then used mainly for mouse/keyboard input.
 */
void setup_fallback(void)
{
   short intout[45], ptsout[12];

   sub_call = get_sub_call();
#if 1
 #if 1
   old_wk_handle = call_v_opnwk(1, intout, ptsout);
 #else
   old_wk_handle = scall_v_opnwk(1, intout, ptsout);
 #endif
#else
  intout[0] = ptsout[0];
#endif
#if 0
   pts_in[0] = 50;
   pts_in[1] = 50;
   pts_in[2] = 100;
   pts_in[3] = 100;
   vdi(old_wk_handle, 114, 4, 0);   /* vr_recfl */
   int_in[0] = 0;
   vdi(old_wk_handle, 122, 0, 1);   /* v_show_c */
#endif
}
