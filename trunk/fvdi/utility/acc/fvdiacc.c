/*
 * clock.c  - simple clock in a window Desk Accessory
 *
 * Note: When NOT_A_DA is defined the program is built as a true program,
 * but if optimising some unused assignments will be reported.
 */

#ifdef __PUREC__
   #include <tos.h>
#else
   #include <osbind.h>
#endif

#include <aes.h>
#include <vdi.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "fvdiacc.h"

#include "gem.h"
#include "form.h"
#include "wind.h"
#include "misc.h"

#ifndef NOT_A_DA
#include <acc.h>

STACK(4096);	// Hopefully plenty
#endif

#ifndef NOT_A_DA
#define W_TYPE (NAME | MOVER | CLOSER)
#else
#define W_TYPE (NAME | MOVER)
#endif

#define rsc_file "fvdiacc.rsc"

extern void get_tedinfo(OBJECT *, int, char *);

static int initialise(void);
void deinitialise(void);

int ap_id;
int aes_version;
int finished;
int MultiAES;
int bkgndinput = 1;
int vdi_handle1;
int popfix = 0;
Window window[MAX_WINDOWS];
char dlogtext[30] = "fVDI Setup";

struct Info *info;
unsigned long driver_no;

int buffer[4096];

long get_cookie(char *cname)
{
   long oldstack, *ptr, value, name;

   name = 0;
   while(*cname)
      name = (name << 8) | (int)*cname++;

   oldstack = (long)Super(0L);
   ptr = (long *)*(long *)0x5a0;

   if (ptr) {
      while ((*ptr != 0) && (*ptr != name))
         ptr += 2;
      if (*ptr == name)
         value = ptr[1];
      else
         value = -1;
   } else
      value = -1;

   Super((void *)oldstack);
   return value;
}

long call_fvdi(long func, long data)
{
   long oldstack, value;

   oldstack = (long)Super(0L);
   value = info->setup(func, data);
   Super((void *)oldstack);

   return value;
}

int locate_driver(void)
{
   long tmp;
	
   if ((tmp = get_cookie("fVDI")) == -1)
      return 0;
	
   info = (struct Info *)tmp;

   driver_no = call_fvdi(Q_NEXT_DRIVER, 0);
   if (driver_no == -1)
      return 0;

   return 1;
}

int init_main(int a)
{
	return 1;
}

int aesbuf(int dialog, int but)
{
	short high, low, dummy;
	long aes_buffer;

	wind_get(DESK, WF_SCREEN, &high, &low, &dummy, &dummy);
	aes_buffer = ((long)high << 16) | ((long)low & 0xffff);
	call_fvdi((driver_no << 16) | S_AESBUF, aes_buffer);
	return 1;
}

int set_doblit(int dialog, int but)
{
	call_fvdi((driver_no << 16) | S_DOBLIT, 1);
	return 1;
}

int set_screen(int dialog, int but)
{
	call_fvdi((driver_no << 16) | S_SCREEN, (long)Logbase());
	return 1;
}

int xfsel_exinput(char *dir, char *fname, int *button, char *title)
{
   if ((aes_version >= 0x130)) // || options.xtra.fselexinput)
      return fsel_exinput(dir, fname, button, title);
   else
      return fsel_input(dir, fname, button);
}

int saver(int dialog, int but)
{
	int button;
	static char file[50] = "saved.raw";
	static char directory[50] = "c:\\";
    char fname[100];
	FILE *outfile;
	int work_out[57];
	int w, h, depth, row;
	int i;
	MFDB src, dst;
	int points[8];
   
	if (xfsel_exinput(directory, file, &button, "File to load")) {
		if (button) {
			strcpy(fname, directory);
			strcat(fname, file);
			if (!(outfile = fopen(fname, "wb")))
				return 1;

			vq_extnd(vdi_handle1, 0, work_out);
			w = work_out[0] + 1;
			h = work_out[1] + 1;
			vq_extnd(vdi_handle1, 1, work_out);
			depth = work_out[4];
			row = (short)((long)w * depth / 8);
			src.fd_addr = 0;
			dst.fd_addr = buffer;
			dst.fd_w = w;
			dst.fd_h = 1;
			dst.fd_wdwidth = w / 16;
			dst.fd_stand = 0;
			dst.fd_nplanes = depth;
			points[0] = 0;
			points[2] = w - 1;
			points[4] = 0;
			points[5] = 0;
			points[6] = 0;
			points[7] = 0;
			for(i = 0; i < h; i++) {
				points[1] = i;
				points[3] = i;
				vro_cpyfm(vdi_handle1, 3, points, &src, &dst);
				if (fwrite(buffer, 1, row, outfile) != row)
					break;
			}
			fclose(outfile);
		}
	}

	return 1;
}

int option(int dialog, int but)
{
	char buf[64];
	OBJECT *dlog;
	
	rsrc_gaddr(R_TREE, FRM_MAIN, &dlog);
	get_tedinfo(dlog, Option, buf);
	if (buf[0] == '!')
		call_fvdi((driver_no << 16) | S_DRVOPTION, (long)buf + 1);
	else
		call_fvdi(S_OPTION, (long)buf);
	return 1;
}

#ifdef NOT_A_DA
static void no_more(void)
{
	finished = 1;
}
#endif

int main(void)
{
	if (locate_driver()) {
#ifndef NOT_A_DA
		aesbuf(0, 0);
		set_screen(0, 0);
#endif
	}

	if (!initialise())
		return 0;

#ifndef NOT_A_DA
	menu_register(ap_id, "  fVDI");	// Register as a DA
#else
	add_xdialog(frm_find(FRM_MAIN), no_more, W_TYPE, dlogtext);
#endif

	event_loop();

#ifdef NOT_A_DA
	appl_exit();
	return 0;
#endif
}

void menu_update(void)
{
}

static int initialise(void)
{
   int junk;
   int work_in[11] = {1,  SOLID, 1,  1, 1,  1, 1,  FIS_SOLID, 0, 1,  2};
   int work_out[57];

   if ((ap_id = appl_init()) == -1)
      return 0;

   if (!rsrc_load(rsc_file)) {
      form_alert(1, "[1][Resource file error][ Quit ]");
      appl_exit();
      return 0;
   }
   
   vdi_handle1 = graf_handle(&junk, &junk, &junk, &junk);
   v_opnvwk(work_in, &vdi_handle1, work_out);
   vswr_mode(vdi_handle1, MD_XOR);

   init_windows();

   finished = 0;

//   *edit = 0;   
}

void deinitialise(void)
{
   rsrc_free();
   v_clsvwk(vdi_handle1);
   appl_exit();
}
