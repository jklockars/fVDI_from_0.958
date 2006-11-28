/*
 * fVDI preferences and driver loader
 *
 * $Id: loader.c,v 1.35 2006-11-28 12:18:14 johan Exp $
 *
 * Copyright 1997-2003, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "os.h" 
#include "fvdi.h"
#include "relocate.h"
#include "utility.h"
#include "function.h"
#include "globals.h"

#define BLOCKS           2              /* Default number of memory blocks to allocate for internal use */
#define BLOCK_SIZE      10              /* Default size of those blocks, in kbyte */

#define MAGIC     "InitMagic"

#define TOKEN_SIZE  160                 /* Also used for driver option lines */
#define PATH_SIZE   80
#define NAME_SIZE   100


/*
 * Global variables
 */

#ifdef FT2
/* Headers to ft2_* functions ... FIXME: to be moved */
long        ft2_init(void);
Fontheader* ft2_load_font(Virtual *vwk, const char *filename);
long        ft2_char_width(Virtual *vwk, Fontheader *font, long ch);
long        ft2_text_width(Virtual *vwk, Fontheader *font, short *s, long slen);
Fontheader* ft2_vst_point(Virtual *vwk, long ptsize, short *sizes);
long        ft2_text_render_default(Virtual *vwk, unsigned long coords,
                                    short *s, long slen);
long        ft2_set_effects(Virtual *vwk, Fontheader *font, long effects);
void*       ft2_char_bitmap(Virtual *vwk, Fontheader *font, long ch, short *bitmap_info);
void*       ft2_char_advance(Virtual *vwk, Fontheader *font, long ch, short *advance_info);
void        ft2_xfntinfo(Virtual *vwk, Fontheader *font, long flags, XFNT_INFO *info);
void        ft2_fontheader(Virtual *vwk, Fontheader *font, VQT_FHDR *fhdr);

long        (*external_init)(void) = ft2_init;
Fontheader* (*external_load_font)(Virtual *vwk, const char *font) = ft2_load_font;
long        (*external_vqt_extent)(Virtual *vwk, Fontheader *font, short *text, long length) = ft2_text_width;
long        (*external_vqt_width)(Virtual *vwk, Fontheader *font, long ch) = ft2_char_width;
long        (*external_vst_effects)(Virtual *vwk, Fontheader *font, long effects) = ft2_set_effects;
Fontheader* (*external_vst_point)(Virtual *vwk, long size, short *sizes) = ft2_vst_point;
long        (*external_renderer)(Virtual *vwk, unsigned long coords,
                                 short *text, long length) = ft2_text_render_default;
void*       (*external_char_bitmap)(Virtual *vwk, Fontheader *font, long ch, short *bitmap_info) = ft2_char_bitmap;
void*       (*external_char_advance)(Virtual *vwk, Fontheader *font, long ch, short *advance_info) = ft2_char_advance;
void        (*external_xfntinfo)(Virtual *vwk, Fontheader *font, long flags, XFNT_INFO *info) = ft2_xfntinfo;
void        (*external_fontheader)(Virtual *vwk, Fontheader *font, VQT_FHDR *fhdr) = ft2_fontheader;
#else
long        (*external_init)(void) = 0;
Fontheader* (*external_load_font)(Virtual *vwk, const char *font) = 0;
long        (*external_vqt_extent)(Virtual *vwk, Fontheader *font, short *text, long length) = 0;
long        (*external_vqt_width)(Virtual *vwk, Fontheader *font, long ch) = 0;
Fontheader* (*external_vst_point)(Virtual *vwk, long size, short *sizes) = 0;
long        (*external_renderer)(Virtual *vwk, unsigned long coords,
                                 short *text, long length) = 0;
void*       (*external_char_bitmap)(Virtual *vwk, Fontheader *font, long ch, short *bitmap_info) = 0;
void*       (*external_char_advance)(Virtual *vwk, Fontheader *font, long ch, short *advance_info) = 0;
void        (*external_xfntinfo)(Virtual *vwk, Fontheader *font, long flags, XFNT_INFO *info) = 0;
void        (*external_fontheader)(Virtual *vwk, Fontheader *font, VQT_FHDR *fhdr) = 0;
#endif

List *driver_list = 0;
List *module_list = 0;

short disabled = 0;
short booted = 0;
short fakeboot = 0;
short oldmouse = 0;
short keep = 0;
short debug = 0;
short nvdifix = 0;
short key_pressed = 0;
short key_stored = 0;
short lineafix = 0;
short xbiosfix = 0;
short singlebend = 0;
short memlink = 1;
short blocks = BLOCKS;
long block_size = BLOCK_SIZE * 1024;
long log_size = 1000;
short arc_split = 16384;  /* 1/4 as many lines as largest ellipse axel radius in pixels */
short arc_min = 16;       /* Minimum number of lines in an ellipse */
short arc_max = 256;      /* Maximum */
short no_vex = 0;
short debug_out = -2;
short interactive = 0;
short stand_alone = 0;
short nvdi_cookie = 0;
short speedo_cookie = 0;
short calamus_cookie = 0;
char silent[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char silentx[1] = {0};
char vq_gdos_value[] = "fVDI";
unsigned short sizes[16] = {8, 9, 10, 11, 12, 14, 18, 24, 36, 48, 0xffff};
short size_count = 11;
short size_user = 0;
short old_malloc = 0;
short fall_back = 0;
short move_mouse = 0;
short ext_malloc = 0;
#if 1
short check_mem = 0;
#else
short check_mem = 1;
#endif
short bconout = 0;
short file_cache_size = 0;
short antialiasing = 0;
char *debug_file = 0;
static char path[PATH_SIZE];

static long set_path(Virtual *vwk, const char **ptr);
static long load_fonts(Virtual *vwk, const char **ptr);
static long wait_key(Virtual *vwk, const char **ptr);
static long exit_key(Virtual *vwk, const char **ptr);
static long swap_key(Virtual *vwk, const char **ptr);
static long go_to(Virtual *vwk, const char **ptr);
static long case_key(Virtual *vwk, const char **ptr);
static long echo_text(Virtual *vwk, const char **ptr);
static long set_pid(Virtual *vwk, const char **ptr);
static long set_width(Virtual *vwk, const char **ptr);
static long set_height(Virtual *vwk, const char **ptr);
static long set_blocks(Virtual *vwk, const char **ptr);
static long set_block_size(Virtual *vwk, const char **ptr);
static long set_log_size(Virtual *vwk, const char **ptr);
static long set_arc_split(Virtual *vwk, const char **ptr);
static long set_arc_min(Virtual *vwk, const char **ptr);
static long set_arc_max(Virtual *vwk, const char **ptr);
static long load_palette(Virtual *vwk, const char **ptr);
static long specify_cookie(Virtual *vwk, const char **ptr);
static long specify_vqgdos(Virtual *vwk, const char **ptr);
static long use_module(Virtual *vwk, const char **ptr);
static long set_silent(Virtual *vwk, const char **ptr);
static long set_size(Virtual *vwk, const char **ptr);
static long pre_allocate(Virtual *vwk, const char **ptr);
static long file_cache(Virtual *vwk, const char **ptr);
static long set_debug_file(Virtual *vwk, const char **ptr);

static Option options[] = {
   {"path",       set_path,       -1},  /* path = str, where to look for fonts and drivers */
   {"fonts",      load_fonts,     -1},  /* fonts = str, where to look for FreeType2 fonts */
   {"debug",      &debug,          2},  /* debug, turn on debugging aids */
   {"waitkey",    wait_key,       -1},  /* waitkey n, wait for key press for n seconds */
   {"exitkey",    exit_key,       -1},  /* exitkey c, quit fVDI if 'c' was pressed */
   {"setkey",     &key_pressed,    3},  /* setkey c, default to 'c' if no key was pressed */
   {"swapkey",    swap_key,       -1},  /* swapkey, swap current key with the extra one */
   {"casekey",    case_key,       -1},  /* casekey c label, go to :label if 'c' was pressed */
   {"goto",       go_to,          -1},  /* goto label, go to :label */
   {"echo",       echo_text,      -1},  /* echo str, write some text to the display */
   {"booted",     &booted,         1},  /* booted, fVDI really runs from the AUTO folder */
   {"fakeboot",   &fakeboot,       1},  /* fakeboot, pretend that fVDI runs from the AUTO folder */
   {"oldmouse",   &oldmouse,       1},  /* oldmouse, use only the previous VDI for mouse handling */
   {"nvdifix",    &nvdifix,        1},  /* nvdifix, patch NVDI for 'background' operation */
   {"lineafix",   &lineafix,       1},  /* lineafix, modify more lineA variables than normally */
   {"xbiosfix",   &xbiosfix,       1},  /* xbiosfix, return correct screen addresses from the XBIOS */
   {"nopidalloc", set_pid,        -1},  /* nopidalloc, don`t modify PID when allocating memory */
   {"singlebend", &singlebend,     1},  /* singlebend, don't bend Trap #2 except at installation */
   {"nomemlink",  &memlink,        0},  /* nomemlink, don't maintain a list of allocated blocks of memory */
   {"keep",       &keep,           1},  /* keep, doesn't do anything at all currently */
#if 0
   {"alias",      &alias,          0},  /* alias, doesn't do anything at all currently */
#endif
   {"disable",    &disabled,       1},  /* disable, the fVDI magic number is never changed from 1969 to $73 */
   {"novex",      &no_vex,         1},  /* novex, disable all the vex_ functions */
   {"width",      set_width,      -1},  /* width, set screen width in mm */
   {"height",     set_height,     -1},  /* height, set screen height in mm */
   {"blocks",     set_blocks,     -1},  /* blocks n, number of memory blocks to allocate */
   {"blocksize",  set_block_size, -1},  /* blocksize n, size of memory blocks in kbyte */
   {"logsize",    set_log_size,   -1},  /* logsize n, size of log in kbyte */
   {"arcsplit",   set_arc_split,  -1},  /* arcsplit n, % of largest ellipse radius to give # of lines to use */
   {"arcmin",     set_arc_min,    -1},  /* arcmin n, minimum number of line to use in an ellipse */
   {"arcmax",     set_arc_max,    -1},  /* arcmax n, maximum number of lines to use in an ellipse */
   {"palette",    load_palette,   -1},  /* palette filename, loads the palette (3*short per colour) specified */
   {"debugout",   &debug_out,      4},  /* debugout n, send all debug (and similar) output to device number n */
   {"interactive",&interactive,    1},  /* interactive, turns on key controlled debugging */
   {"standalone", &stand_alone,    1},  /* standalone, forces fVDI to refrain from relying on an underlying VDI */
   {"cookie",     specify_cookie, -1},  /* cookie speedo/nvdi/calamus = value, allows for setting cookie values */
   {"vqgdos",     specify_vqgdos, -1},  /* vqgdos str, specify a vq_gdos reply */
   {"module",     use_module,     -1},  /* module str, specify a module to load */
   {"silent",     set_silent,     -1},  /* silent n, no debug for VDI call n */
   {"size",       set_size,       -1},  /* size n, specify a default available font size */
   {"oldmalloc",  &old_malloc,     1},  /* oldmalloc, use only the standar Malloc/Free */
   {"fallback",   &fall_back,      1},  /* fallback, forces fVDI to open workstation on an underlying VDI */
   {"movemouse",  &move_mouse,     1},  /* movemouse, forces fVDI to call its movement vector explicitly */
   {"extmalloc",  &ext_malloc,     4},  /* extalloc n, extend all malloc's by n bytes */
   {"checkmem",   &check_mem,      4},  /* checkmem n, check memory allocation consistency at every nth VDI call */
   {"preallocate",pre_allocate,   -1},  /* preallocate n, allocate n kbyte at startup */
   {"filecache",  file_cache,     -1},  /* filecache n, allocate n kbyte for FreeType2 font files */
   {"antialias",  &antialiasing,   1},  /* use FT2 antialiasing */
   {"debugfile",  set_debug_file, -1},  /* debugfile str, file to use for debug output */
   {"bconout",    &bconout,         1}   /* bconout, enables handling of BConout the the screen in fVDI */
};


/* Allocate for size of Driver since the module might be one. */
Module *init_module(Virtual *vwk, const char **ptr, List **list)
{
   char token[TOKEN_SIZE], name[NAME_SIZE], *tmp;
   const char *opts;
   List *list_elem;
   Module *module;

   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   copy(path, name);
   cat(token, name);
   opts = *ptr;
   *ptr = next_line(*ptr);               /* Rest of line is parameter data */
   if (*ptr) {                           /* Assumed no larger than a maximum length token */
      copymem((char *)opts, (char *)token, *ptr - opts);
      token[*ptr - opts] = '\0';
   } else
      copy(opts, token);

   if (!(tmp = (char *)malloc(sizeof(List) + sizeof(Driver) + length(name) + 1)))
      return 0;

   list_elem = (List *)tmp;
   module = (Module *)(tmp + sizeof(List));
   list_elem->next = 0;
   list_elem->type = 1;
   list_elem->value = module;
   module->id = -1;
   module->flags = 1;                     /* Resident */
   module->file_name = tmp + sizeof(List) + sizeof(Driver);
   copy(name, module->file_name);

   if (!load_driver(name, module, vwk, token)) {
      error("Failed to load module: ", name);
      free(tmp);
      return 0;
   } else {
      if (list) {
         if (!*list)
            *list = list_elem;
         else {
            list_elem->next = *list;
            *list = list_elem;
         }
      }
   }

   return module;
}


long use_module(Virtual *vwk, const char **ptr)
{
   Module *module;

   if (!(*ptr = skip_space(*ptr))) {
      error("Bad module specification", 0);
      return -1;
   }

   module = init_module(vwk, ptr, &module_list);
   if (!module)
      return -1;

   return 1;
}


long specify_cookie(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];
   short nvdi_val, speedo_val, calamus_val;

   nvdi_val = 0;
   speedo_val = 0;
   calamus_val = 0;

   if (!(*ptr = skip_space(*ptr))) {
      error("Bad cookie setting!", 0);
      return -1;
   }
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   if (equal(token, "nvdi")) {
#ifdef FT2
      nvdi_val   = 0x0400;
#else
      nvdi_val   = 0x0250;
#endif
   } else if (equal(token, "speedo")) {
      speedo_val = 0x0500;
   } else if (equal(token, "calamus")) {
      calamus_val = 0x0100;
   }

   *ptr = skip_space(*ptr);
   if (**ptr == '=') {
      *ptr = get_token(*ptr, token, TOKEN_SIZE);
      if (equal(token, "=")) {
         if (!(*ptr = skip_space(*ptr))) {
            error("Bad cookie setting!", 0);
            return -1;
         }
         *ptr = get_token(*ptr, token, TOKEN_SIZE);

         if (nvdi_val)
            nvdi_val   = (short)atol(token);
         else if (speedo_val)
            speedo_val = (short)atol(token);
	 else if (calamus_val)
            calamus_val = (short)atol(token);
      }
   }

   if (nvdi_val)
      nvdi_cookie = (short)nvdi_val;
   else if (speedo_val)
      speedo_cookie = (short)speedo_val;
   else if (calamus_val)
      calamus_cookie = (short)calamus_val;

   return 1;
}


long specify_vqgdos(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];
   long value;

   if (!(*ptr = skip_space(*ptr))) {
      error("Bad vq_gdos setting!", 0);
      return -1;
   }
   *ptr = get_token(*ptr, token, TOKEN_SIZE);

   if ((token[0] == '$') || ((token[0] >= '0') && (token[0] <= '9'))) {
      value = atol(token);
      vq_gdos_value[3] = (char)value;
      value >>= 8;
      vq_gdos_value[2] = (char)value;
      value >>= 8;
      vq_gdos_value[1] = (char)value;
      value >>= 8;
      vq_gdos_value[0] = (char)value;
   } else {
      vq_gdos_value[0] = token[0];
      vq_gdos_value[1] = token[1];
      vq_gdos_value[2] = token[2];
      vq_gdos_value[3] = token[3];
   }

   return 1;
}


long get_pathname(const char **ptr, char *dest)
{
   char token[TOKEN_SIZE];

   if (!(*ptr = skip_space(*ptr))) {
      error("Bad path setting!", 0);
      return -1;
   }
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   if (!equal(token, "=")) {
      error("Bad path setting!", 0);
      return -1;
   }
   if (!(*ptr = skip_space(*ptr))) {
      error("Bad path setting!", 0);
      return -1;
   }
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   copy(token, dest);
   switch (dest[length(dest) - 1]) {
   case '\\':
   case '/':
      break;
   default:
      cat("\\", dest);
   }
   
   return 1;
}

long set_path(Virtual *vwk, const char **ptr)
{
   return get_pathname(ptr, path);
}

long wait_key(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];
   char key;
   long endtime;
   
   if (!(*ptr = skip_space(*ptr)))
      ;  /* *********** Error, somehow */
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   endtime = atol(token);
   if (endtime > 999)
      endtime = 999;
   endtime = endtime * 200 + get_protected_l(0x4ba);
   key = 0;
   while (!key && (endtime > get_protected_l(0x4ba))) {
      if (Cconis())
         key = Crawcin() & 0xff;
   }
   if (key)
      key_pressed = key;

   return 1;
}

long exit_key(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];

   if (!(*ptr = skip_space(*ptr)))
      ;  /* *********** Error, somehow */
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   if (key_pressed == token[0])
      return -1;

   return 1;
}

long swap_key(Virtual *vwk, const char **ptr)
{
   char tmp;

   tmp = key_stored;
   key_stored = key_pressed;
   key_pressed = tmp;

   return 1;
}

long go_to(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];
   char label[TOKEN_SIZE + 1];

   if (!(*ptr = skip_space(*ptr)))
      ;  /* *********** Error, somehow */
   label[0] = ':';
   *ptr = get_token(*ptr, &label[1], TOKEN_SIZE);

   while (*ptr) {
      *ptr = skip_space(*ptr);
      *ptr = get_token(*ptr, token, TOKEN_SIZE);
      if (equal(token, label))
         break;
   }

   return 1;
}

long case_key(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];

   if (!(*ptr = skip_space(*ptr)))
      ;  /* *********** Error, somehow */
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   if (key_pressed == token[0])
      return go_to(vwk, ptr);
   else
      *ptr = next_line(*ptr);

   return 1;
}
   
long echo_text(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];

   if (!(*ptr = skip_space(*ptr)))
      ;  /* *********** Error, somehow */
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   Cconws(token);
   Cconws("\x0d\x0a");

   return 1;
}

long set_pid(Virtual *vwk, const char **ptr)
{
   pid = 0;

   return 1;
}

long set_width(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];
   int width;

   if (!vwk)
      return -1;

   if (!(*ptr = skip_space(*ptr)))
      ;  /* *********** Error, somehow */
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   width = atol(token);
   if (width > 0) {
      if (width < 100)
         width = 100;
   } else {                                 /* Negative width - fixed DPI */
      if (-width < 10)
         width = -10;
   }
   vwk->real_address->screen.pixel.width = width;   /* Currently no other way */

   return 1;
}

long set_height(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];
   int height;

   if (!vwk)
      return -1;
   
   if (!(*ptr = skip_space(*ptr)))
      ;  /* *********** Error, somehow */
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   height = atol(token);
   if (height > 0) {
      if (height < 100)
         height = 100;
   } else {                                 /* Negative height - fixed DPI */
      if (-height < 10)
         height = -10;
   }
   vwk->real_address->screen.pixel.height = height;   /* Currently no other way */

   return 1;
}

long set_blocks(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];

   if (!(*ptr = skip_space(*ptr)))
      ;  /* *********** Error, somehow */
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   blocks = atol(token);
   if (blocks < 2)
      blocks = 2;

   return 1;
}

long set_block_size(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];

   if (!(*ptr = skip_space(*ptr)))
      ;  /* *********** Error, somehow */
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   block_size = atol(token) * 1024;
   if (block_size < 10 * 1024)
      block_size = 10 * 1024;

   return 1;
}

long set_log_size(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];

   if (!(*ptr = skip_space(*ptr)))
      ;  /* *********** Error, somehow */
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   log_size = atol(token) * 256;             /* Really number of long words */
   if (log_size < 1000)
      log_size = 1000;

   return 1;
}

long set_arc_split(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];

   if (!(*ptr = skip_space(*ptr)))
      ;  /* *********** Error, somehow */
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   arc_split = atol(token);
   if (arc_split < 3)
      arc_split = 3;
   if (arc_split > 100)
      arc_split = 100;
   arc_split = (arc_split * 65536L + 50) / 100;  /* Percentage turned into part of 64k for easier calulation */

   return 1;
}

long set_arc_min(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];

   if (!(*ptr = skip_space(*ptr)))
      ;  /* *********** Error, somehow */
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   arc_min = atol(token);
   if (arc_min < 3)
      arc_min = 3;

   return 1;
}

long set_arc_max(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];

   if (!(*ptr = skip_space(*ptr)))
      ;  /* *********** Error, somehow */
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   arc_max = atol(token);
   if (arc_max > 1000)
      arc_max = 1000;

   return 1;
}

long pre_allocate(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];
   int amount;

   if (!(*ptr = skip_space(*ptr)))
      ;  /* *********** Error, somehow */
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   amount = atol(token);
   if (amount > 0)
     allocate(amount);

   return 1;
}

long file_cache(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];
   int amount;

   if (!(*ptr = skip_space(*ptr)))
      ;  /* *********** Error, somehow */
   *ptr = get_token(*ptr, token, TOKEN_SIZE);
   amount = atol(token);
   if ((amount > 0) && (amount <= 32767))
     file_cache_size = amount;

   return 1;
}

long load_palette(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE], name[NAME_SIZE];
   long size;
   long colours;
   int file;
   void *palette;

   if (!(*ptr = skip_space(*ptr))) {
      error("No palette file name!", 0);
      return -1;
   }
   *ptr = get_token(*ptr, token, TOKEN_SIZE);

   copy(token, name);
   if ((size = get_size(name)) < 0) {
      copy(path, name);
      cat(token, name);
      if ((size = get_size(name)) < 0) {
         error("Can't find palette file!", 0);
         return -1;
      }
   }

   if (size % (3 * sizeof(short))) {
      error("Wrong palette file size!", 0);
      return -1;
   }

   colours = size / (3 * sizeof(short));
   switch (colours) {
   case 2:
   case 4:
   case 16:
   case 256:
      break;
   default:
      error("Wrong palette file size!", 0);
      return -1;
   }

   if (!(palette = malloc(size))) {
      error("Can't allocate memory for palette!", 0);
      return -1;
   }

   if ((file = Fopen(name, O_RDONLY)) < 0) {
      error("Can't open palette file!", 0);
      free(palette);
      return -1;
   }

   Fread(file, size, palette);

   Fclose(file);

   vwk->real_address->screen.palette.colours = palette; /* Currently no other way */

   return 1;
}

long set_debug_file(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];
   int file, bytes;

   if (!(*ptr = skip_space(*ptr))) {
      error("No debug file name!", 0);
      return -1;
   }
   *ptr = get_token(*ptr, token, TOKEN_SIZE);

   if ((file = Fcreate(token, 0)) < 0) {
      error("Can't create debug file!", 0);
      return -1;
   }

   bytes = Fwrite(file, 19, "fVDI debug output\x0d\x0a");
   Fclose(file);

   if (bytes != 19) {
      error("Can't write to debug file!", 0);
      return -1;
   }

   debug_file = malloc(strlen(token));
   if (!debug_file) {
      error("Can't store debug file name!", 0);
      return -1;
   }
   copy(token, debug_file);
   debug_out = -3;

   return 1;
}

long set_silent(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];
   long call;
   int i;

   do {
      if (!(*ptr = skip_space(*ptr)))
         ;  /* *********** Error, somehow */
      *ptr = get_token(*ptr, token, TOKEN_SIZE);
      if (equal(token, "oldallocation")) {
         silentx[0] ^= 0x01;
      } else if (equal(token, "allocation")) {
         silentx[0] ^= 0x03;
      } else {
         call = atol(token);
         if ((call >= 0) && (call <= 255))
            silent[call >> 3] ^= 1 << (call & 7);
         else {
            for(i = 0; i < 32; i++)
            silent[i] = 0xff;
         for(i = 0; i < 1; i++)
            silentx[i] = 0xff;
         }
      }
      *ptr = skip_only_space(*ptr);
   } while (*ptr && (numeric(**ptr) || ((**ptr == '-') && numeric(*(*ptr + 1)))));

   return 1;
}

long set_size(Virtual *vwk, const char **ptr)
{
   char token[TOKEN_SIZE];
   long size;
   int i;

   do {
      if (!(*ptr = skip_space(*ptr)))
         ;  /* *********** Error, somehow */
      *ptr = get_token(*ptr, token, TOKEN_SIZE);
      size = atol(token);
      if ((size > 0) && (size <= 100) &&
          (size_count < sizeof(sizes) / sizeof(sizes[0]))) {
         if (!size_user) {
            size_user = 1;
            sizes[0] = 0xffff;
            size_count = 1;
         }
         for(i = size_count - 1; i >= 0; i--)
            if (size == sizes[i])
               break;
         if (i < 0) {
            for(i = size_count; i > 0; i--) {
               if (size > sizes[i - 1]) {
                  break;
               }
               sizes[i] = sizes[i - 1];
            }
            sizes[i] = size;
            size_count++;
         }
      }
      *ptr = skip_only_space(*ptr);
   } while (*ptr && numeric(**ptr));

   return 1;
}

long check_token(Virtual *vwk, char *token, const char **ptr)
{
   int i;
   int normal;
   char *xtoken;

   xtoken = token;
   switch (token[0]) {
   case '+':
      xtoken++;
      normal = 1;
      break;
   case '-':
      xtoken++;
      normal = 0;
      break;
   case ':':    /* Label */
     return 1;
   default:
      normal = 1;
      break;
   }

   for(i = 0; i < sizeof(options) / sizeof(Option); i++) {
      if (equal(xtoken, options[i].name)) {
         switch (options[i].type) {
         case -1:     /* Function call */
            return ((long (*)(Virtual *, const char **))options[i].varfunc)(vwk, ptr);
         case 0:      /* Default 1, set to 0 */
            *(short *)options[i].varfunc = 1 - normal;
            return 1;
         case 1:     /* Default 0, set to 1 */
            *(short *)options[i].varfunc = normal;
            return 1;
         case 2:     /* Increase */
            *(short *)options[i].varfunc += -1 + 2 * normal;
            return 1;
         case 3:     /* Single character */
           if (!(*ptr = skip_space(*ptr)))
              ;  /* *********** Error, somehow */
            *ptr = get_token(*ptr, token, TOKEN_SIZE);
           *(short *)options[i].varfunc = token[0];
           return 1;
         case 4:     /* Number */
           if (!(*ptr = skip_space(*ptr)))
              ;  /* *********** Error, somehow */
            *ptr = get_token(*ptr, token, TOKEN_SIZE);
           *(short *)options[i].varfunc = atol(token);
           return 1;
         }
      }
   }

   return 0;
}


long tokenize(const char *buffer)
{
   const char *ptr;
   char token[TOKEN_SIZE];
   long ret;
   int count;

   if (!(ptr = skip_space(buffer)))
      return 0;

   count = 0;
   while (ptr) {
      ptr = get_token(ptr, token, TOKEN_SIZE);
      ret = check_token(0, token, &ptr);
      if (!ret || (ret == -1))
         break;
      else
         count++;
      ptr = skip_space(ptr);
   }

   /* Take whatever final steps might be necessary. */
{
   extern struct Super_data *super;

   if (debug && !super->fvdi_log.start) {   /* Set up log table if there isn't one */
      if (super->fvdi_log.start = malloc(log_size * sizeof(long))) {
         super->fvdi_log.active = 1;
         super->fvdi_log.current = super->fvdi_log.start;
         super->fvdi_log.end = &super->fvdi_log.start[log_size - 8];
      }
   }
}
   linea_setup(screen_wk);

   return count;
}


/*
 * Find the magic startup struct and
 * call the initialization function given there.
 */
int initialize(const unsigned char *addr, long size, Driver *driver, Virtual *vwk, char *opts)
{
   long i;
   int j;
   Locator *locator;

   for(i = 0; i < size - sizeof(MAGIC); i++) {
      for(j = 0; j < sizeof(MAGIC); j++) {
         if (addr[j] != MAGIC[j])
            break;
      }
      if (j == sizeof(MAGIC)) {
         locator = (Locator *)addr;
         if ((locator->version & 0xfff0) < (MODULE_IF_VER & 0xfff0)) {
            puts_nl("Module compiled with unsupported interface version.");
            return 0;
         }
         return locator->init(&real_access, driver, vwk, opts);
      }
      addr++;
   }

   return 0;
}


/*
 * Do a complete relocation
 */
void relocate(unsigned char *prog_addr, Prgheader *header)
{
   unsigned char *code, *rtab, rval;

   rtab = prog_addr + header->tsize + header->dsize + header->ssize;
   code = prog_addr + *(long *)rtab;
   rtab += 4;

   *(long *)code += (long)prog_addr;
   while (rval = *rtab++) {
      if (rval == 1)
         code += 254;
      else {
         code += rval;
         *(long *)code += (long)prog_addr;
      }
   }
}      


/*
 * Load, relocate and initialize driver
 */
int load_driver(const char *name, Driver *driver, Virtual *vwk, char *opts)
{
   long file_size, program_size;
   int file;
   unsigned char *addr;
   Prgheader header;
   int init_result;

   if ((file_size = get_size(name) - sizeof(header)) < 0)
      return 0;

   if ((file = Fopen(name, O_RDONLY)) < 0)
      return 0;

   Fread(file, sizeof(header), &header);
   program_size = header.tsize + header.dsize + header.bsize;

   if (!(addr = (char *)malloc(MAX(file_size, program_size)))) {
      Fclose(file);
      return 0;
   }

   Fread(file, file_size, addr);
   Fclose(file);

   relocate(addr, &header);

   /* This will cause trouble if ever called from supervisor mode! */
   Supexec((long)cache_flush);

   init_result = 0;
   if (!(init_result = initialize(addr, header.tsize + header.dsize, driver, vwk, opts))) {
      free(addr);
      error("Initialization failed!", 0);
      return 0;
   }

   return init_result;
}


/* This should really be handled somewhat differently. */
/* Probably ought to be in another file. */
long load_fonts(Virtual *vwk, const char **ptr)
{
#ifdef __GNUC__
   _DTA info;
#else
   DTA info;       /* Thanks to tos.h for Lattice C */
#endif
   static char fonts[PATH_SIZE], *pathtail;
   int error;

   if (!external_init) {
      puts_nl("Font directory specified without FreeType support!");
      return -1;
   }

   if (get_pathname(ptr, fonts) != 1)
      return -1;

   /* Point past the last char */
   pathtail = &fonts[length(fonts)];

   copy("*.*", pathtail);
   
#if 0
   puts("Fonts: ");
   puts_nl(fonts);
#else
   puts_nl("");
#endif

   /* Initialize FreeType2 module */
   external_init();

   Fsetdta((void *)&info);
   error = Fsfirst(fonts, 7);
   while (error == 0) {
      Fontheader *new_font;

#ifdef __GNUC__
      info.dta_name[12] = 0;
      copy(info.dta_name, pathtail);
#else
      info.d_fname[12] = 0;
      copy(info.d_fname, pathtail);
#endif

#if 0
      puts("   Load font: ");
      puts_nl(fonts);
#endif

      if ((new_font = external_load_font(vwk, fonts))) {
         /* It's assumed that a device has been initialized (driver exists) */
         if (insert_font(&vwk->real_address->writing.first_font, new_font))
            vwk->real_address->writing.fonts++;
      }

      error = Fsnext();
   }

#if 0
   {
      char buf[10];
      puts("   Load fonts done: ");
      ltoa(buf, vwk->real_address->writing.fonts, 10);
      puts_nl(buf);
   }
#endif
}


/*
 * Load and parse FVDI.SYS
 */
int load_prefs(Virtual *vwk, char *sysname)
{
   long file_size;
   char *buffer, token[TOKEN_SIZE], name[NAME_SIZE];
   const char *ptr;
   int file;
   int device;
   int driver_loaded, font_loaded, system_font;
   Fontheader *new_font;
   char *after_path;
   Driver *driver;
   int ret;

   copy(sysname, path);
   after_path = path;
   if ((file_size = get_size(path)) < 0) {
      copy("c:\\", path);
      cat(sysname, path);
      after_path = &path[3];
      if ((file_size = get_size(path)) < 0) {
         path[0] = 'a';
         if ((file_size = get_size(path)) < 0) {
            error("Can't find FVDI.SYS!", 0);
            return 0;
         }
      }
   }
   
   if (!(buffer = (char *)malloc(file_size + 1)))
      return 0;

   if ((file = Fopen(path, O_RDONLY)) < 0) {
      free(buffer);
      return 0;
   }

   Fread(file, file_size, buffer);

   Fclose(file);

   device = -1;
   driver_loaded = font_loaded = 0;

   buffer[file_size] = '\0';

   copy("gemsys\\", after_path);

   
   if (!(ptr = skip_space(buffer))) {
      error("Empty config file!", 0);
      free(buffer);
      return 0;
   }
   while (ptr) {
      ptr = get_token(ptr, token, TOKEN_SIZE);

      ret = check_token(vwk, token, &ptr);
      if (ret == -1)
         return 0;
      else if (ret)
         ;
      else if (numeric(token[0]) && numeric(token[1]) &&
               (!token[2] || equal(&token[2], "r") || equal(&token[2], "p"))) {
         /* ##[r]    ('p' is also allowed for now (same effect as 'r')) */
         /* There is definitely more to do here! */
         /* r - load and keep in ram */
         /* p - check at mode change */

         device = (token[0] - '0') * 10 + token[1] - '0';
         if (equal(&token[2], "r")) {             /* Resident */
            if (!(ptr = skip_space(ptr))) {
               error("Bad device driver specification: ", token);
               break;
            }

            driver = (Driver *)init_module(vwk, &ptr, &driver_list);
            if (!driver)
               break;
            driver->module.id = device;
            driver->module.flags = 1;                     /* Resident */
            driver_loaded = 1;
         } else {       /* Load when needed */
            /* ........... */
         }
      } else {                                    /* Anything that isn't recognized above must be a font */
         if (device == -1) {
            error("Font specified before device driver: ", token);
            free(buffer);
            return 0;
         }

         /* More to do here? */

         if (equal(token, "s")) {                 /* An 's' before a font name means it's a system font */
            if (!(ptr = skip_space(ptr))) {
               error("Bad system font specification!", 0);
               break;
            }
            ptr = get_token(ptr, token, TOKEN_SIZE);
            system_font = 1;
         } else
            system_font = 0;
         copy(path, name);
         cat(token, name);
         if (!(new_font = load_font(name))) {
            error("Failed to load font: ", name);
         } else {
            font_loaded = 1;
            if (system_font) {
               new_font->id = 1;
               new_font->flags |= 0x01;
            }
            if (insert_font(&driver->default_vwk->real_address->writing.first_font, new_font))
               driver->default_vwk->real_address->writing.fonts++;        /* It's assumed that a device has been initialized (driver exists) */
         }
         ptr = skip_space(ptr);
      }
      ptr = skip_space(ptr);
   }

   if (driver_list) {              /* Some driver loaded? */
      Fontheader **system_font, *header;
      long header_size = sizeof(Fontheader) - sizeof(Fontextra);
      Workstation *wk;
      List *tmp = driver_list;
      while (tmp) {                /* For all drivers */
         wk = ((Driver *)tmp->value)->default_vwk->real_address;
         if (!wk->writing.first_font || (wk->writing.first_font->id != 1)) {  /* No system font? */
            system_font = linea_fonts();                                      /*   Find one in the ROM */
            if (!(header = (Fontheader *)malloc(sizeof(Fontheader) * 3)))
               break;
            copymem(system_font[0], &header[0], header_size);
            copymem(system_font[1], &header[1], header_size);
            copymem(system_font[2], &header[2], header_size);
            fixup_font(&header[0], (char *)header_size, 0);
            fixup_font(&header[1], (char *)header_size, 0);
            fixup_font(&header[2], (char *)header_size, 0);
            unpack_font(&header[0], 1);
            unpack_font(&header[1], 1);
            unpack_font(&header[2], 1);
            insert_font(&wk->writing.first_font, &header[0]);
            insert_font(&wk->writing.first_font, &header[1]);
            insert_font(&wk->writing.first_font, &header[2]);
            wk->writing.fonts++;
            font_loaded = 1;
         }
         tmp = tmp->next;
      }
   }

#if 0
   if (font_loaded) {
      first_font = vwk->real_address->writing.first_font;
      vwk->text.font = first_font->id;
      vwk->text.current_font = first_font;
      vwk->text.character.width = first_font->widest.character;
      vwk->text.character.height = first_font->distance.top;
      vwk->text.cell.width = first_font->widest.cell;
      vwk->text.cell.height = first_font->distance.top + 1 + first_font->distance.bottom;
   }
#endif

   free(buffer);

   return driver_loaded && font_loaded;
}
