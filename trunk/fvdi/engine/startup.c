/*
 * fVDI startup
 *
 * Copyright 1999-2001, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#ifdef __PUREC__
   #include <tos.h>
#else
   #include <osbind.h>
   #ifdef __LATTICE__
      #include <dos.h>
      #include <tos.h>
   #endif
#endif

#include "fvdi.h"

#define DEBUG

#define SYSNAME "fvdi.sys"

#define VERSION	0x0959
#define BETA	1
#define VERmaj	(VERSION >> 12)
#define VERmin	(((VERSION & 0x0f00) >> 8) * 100 + ((VERSION & 0x00f0) >> 4) * 10 + (VERSION & 0x000f))

#define STACK_SIZE	4096		/* Used to be 2048 */

#define fvdi_magic	1969
#define ACTIVE		1		/* fVDI installed */
#define BOOTED		2		/* fVDI can't be removed */

#if 0
 #define BLOCKS		2		/* Number of memory blocks to allocate for internal use */
 #define BLOCK_SIZE	10*1024		/* The size of those blocks */
#endif

#define MAX_NVDI_SEARCH		100	/* Words of forward search from the initial NVDI dispatcher */
#define MAX_NVDI_DISTANCE	10000	/* Allowed distance between the two dispatchers */

#if 0
#define disp(text) 	Cconws(text)
#define disp_nl(text)	{ Cconws(text); Cconws("\x0a\x0d"); }
#else
extern void puts(const char* text);
#define disp(text) 	puts(text)
#define disp_nl(text)	{ puts(text); puts("\x0a\x0d"); }
#define scrdisp(text) 	Cconws(text)
#define scrdisp_nl(text)	{ Cconws(text); Cconws("\x0a\x0d"); }
#endif
#define key_wait(time)	Crawcin()
#define ABS(x)		(((x) >= 0) ? (x) : -(x))

#ifdef DEBUG
#include "relocate.h"
extern short debug;
extern Access *access;
#define MIN(x,y)	(((x) < (y)) ? (x) : (y))
#endif

/*
 * External functions called
 */

extern int init_utility(void);
extern int load_prefs(Virtual *vwk, char *sysname);
extern Virtual *initialize_vdi(void);
extern int initialize_pool(long size, long n);
extern void copy_workstations(Virtual *def, long really_copy);
extern void setup_fallback(void);
extern long vq_gdos(void);
extern int remove_xbra(long vector, long id);
extern long get_cookie(const unsigned char *name, long super);
extern int set_cookie(const unsigned char *name, long value);
extern void shut_down(void);
extern ltoa(char *buf, long n, unsigned long base);
extern void copymem(void *s, void *d, long n);
extern void *malloc(long size, long type);
extern long free(void *addr);
extern long free_all(void);
extern long get_protected_l(void *addr);
extern void set_protected_l(void *addr, long v);
extern void error(const char *text1, const char *text2);
extern long tokenize(const char *buffer);

/*
 * Global variables
 */

extern List *driver_list;
extern short debug;
extern short disabled;
extern short booted;
extern short fakeboot;
extern short nvdifix;
extern short xbiosfix;
extern short singlebend;
extern short blocks;
extern short stand_alone;
extern long block_size;
extern long log_size;
extern long pid_addr;
extern long *pid;
extern long trap2_address;
extern long vdi_address;
extern long trap14_address;
extern long lineA_address;
extern void *trap2_temp;
extern void *trap14;
extern void *lineA;
extern void *vdi_dispatch;
extern void *eddi_dispatch;
extern void *init;
extern void *data_start;
extern void *bss_start;

extern long mint;
extern long magic;

extern void* dummy_vdi;

long basepage;
char fake_bp[256];

long old_gdos = -2;

short initialized = 0;

long remove_fvdi(void);
long setup_fvdi(unsigned long, long);
int nvdi_patch(void);

struct fVDI_cookie {
	short version;
	short flags;
	long (*remove)(void);
	long (*setup)(unsigned long type, long value);
	struct fVDI_log *log;
};	/* cookie = {VERSION, 0, remove_fvdi, setup_fvdi, &fvdi_log}; */

struct FSMC_cookie {
	long type;
	short versions;
	short dummy;
};	/* fsmc_cookie = {"_FSM", 0x0100, -1}; */

struct Readable_data {
	struct fVDI_cookie cookie;
	struct FSMC_cookie fsmc_cookie;
} *readable = 0;

struct Super_data *super = 0;

long old_eddi = 0;
long old_fsmc = 0;
long base_page;

char vdi_stack[2048];
long stack_address;


/*
 * Turn string (max four characters) into a long
 */
long str2long(const unsigned char *text)
{
	long v;

	v = 0;
	while (*text) {
		v <<= 8;
		v += *text++;
	}

	return v;
}


/*
 * Top level fVDI initialization
 */
long startup(void)
{
	Virtual *base_vwk, *first_vwk;
	char buffer[10];
	List *element;
	Driver *driver;

	disp_nl("");

	if (!init_utility()) {				/* Make the utility routines ready for use */
		error("Error while initializing utility routines.", 0);
		return 0;
	}

	if (!(super = malloc(sizeof(struct Super_data), 0x4033))) {
		error("Could not allocate space for supervisor accessible data.", 0);
		return 0;
	}
	super->fvdi_log.active = 0;
	super->fvdi_log.start = 0;
	super->fvdi_log.current = 0;
	super->fvdi_log.end = 0;

	if (!(readable = malloc(sizeof(struct Readable_data), 0x4043))) {
		error("Could not allocate space for world-readable data.", 0);
		return 0;
	}
	readable->cookie.version = VERSION;
	readable->cookie.flags = 0;
	readable->cookie.remove = remove_fvdi;
	readable->cookie.setup = setup_fvdi;
	readable->cookie.log = &super->fvdi_log;
	readable->fsmc_cookie.type = str2long("_FSM");
	readable->fsmc_cookie.versions = 0x0100;

	if (!(base_vwk = initialize_vdi())) {		/* Setup initial real and virtual workstations */
		error("Error while initializing VDI.", 0);
		return 0;
	}

	if (!load_prefs(base_vwk, SYSNAME)) {		/* Load preferences (and load all fonts and device drivers specified) */
		error("Aborted while loading preferences.", 0);
		return 0;
	}

	if (debug) {				/* Set up log table if asked for */
		if (super->fvdi_log.start = malloc(log_size * sizeof(long), 3)) {
			super->fvdi_log.active = 1;
			super->fvdi_log.current = super->fvdi_log.start;
			super->fvdi_log.end = &super->fvdi_log.start[log_size - 8];
		}
	}

	if (!initialize_pool(block_size, blocks)) {	/* Initialize the internal memory pool */
		error("Error while initializing memory pool.", 0);
		return 0;
	}

	if (remove_xbra(34*4, str2long("fVDI")) && debug)	/* fVDI might already be installed */
		disp_nl("Removing previous XBRA.");

	if (nvdifix && nvdi_patch() && debug)
		disp_nl("Patching NVDI dispatcher");

#ifdef __PUREC__
	if (booted && !fakeboot && !singlebend) {
		trap2_address = (long)Setexc(34, (void (*)())&trap2_temp);	/* Install a temporary trap handler if real boot (really necessary?) */
	} else {
		vdi_address = (long)Setexc(34, (void (*)())&vdi_dispatch);	/*   otherwise the dispatcher directly */
	}

#if 0
	if (xbiosfix)
#endif
		trap14_address = (long)Setexc(46, (void (*)())&trap14);	/* Install an XBIOS handler */
#else
	if (booted && !fakeboot && !singlebend) {
		trap2_address = (long)Setexc(34, (void *)&trap2_temp);	/* Install a temporary trap handler if real boot (really necessary?) */
	} else {
		vdi_address = (long)Setexc(34, (void *)&vdi_dispatch);	/*   otherwise the dispatcher directly */
	}

#if 0
	if (xbiosfix)
#endif
		trap14_address = (long)Setexc(46, (void *)&trap14);	/* Install an XBIOS handler */
#endif

	lineA_address = (long)Setexc(10, (void *)&lineA);		/* Install a LineA handler */
	
	stack_address = (long)vdi_stack;

	scrdisp("fVDI v");
	ltoa(buffer, VERmaj, 10);
	scrdisp(buffer);
	scrdisp(".");
	ltoa(buffer, VERmin, 10);
	scrdisp(buffer);
	if (BETA)
		scrdisp("beta");
	scrdisp_nl(" now installed.");

	if (debug) {
		ltoa(buffer, (long)&init, 16);
		disp("fVDI engine Text: $");
		disp(buffer);
		ltoa(buffer, (long)&data_start, 16);
		disp("   Data: $");
		disp(buffer);
		ltoa(buffer, (long)&bss_start, 16);
		disp("   Bss: $");
		disp_nl(buffer);
		if (super->fvdi_log.active) {
			ltoa(buffer, (long)&super->fvdi_log, 16);
			disp("Logging at $");
			disp_nl(buffer);
		}
	}


	/*
	 * During the load process, all installed drivers were linked onto a list.
	 * Go through the list and call all post install driver initialization routines.
	 */

	if (debug)
		disp_nl("Post install initialization of drivers");

	first_vwk = 0;
	element = driver_list;
	while (element) {
#if 0
		if (element->type != 1)
****			bad list type
#endif
		driver = (Driver *)element->value;
		if (driver->flags & 1) {
			if (!first_vwk)
				first_vwk = driver->default_vwk;
			if (debug) {
				disp(" ");
				disp(driver->name);
				ltoa(buffer, (long)driver->initialize, 16);
				disp(" at $");
				disp_nl(buffer);
			}
			((void (*)(Virtual *))(driver->initialize))(driver->default_vwk);
		}
		element = element->next;
	}

	/*
	 * Open and initialize copies of previous workstations for fVDI,
	 * unless this is a boot (in which case a fall-back is set up instead).
	 */

	if (!booted) {
		if (debug)
			disp_nl("Copying available virtual workstations");
		copy_workstations(first_vwk, !fakeboot);	/* f.vwk - default vwk to set up for, fall-through if fakeboot */
	} else if (!disabled) {
		if (!fakeboot && !stand_alone) {
			if (debug) {
				disp_nl("About to set up VDI fallback. Press any key.");
				key_wait(10);			/* It's too late to wait for a key afterwards */
			}
			setup_fallback();
			readable->cookie.flags |= BOOTED;
		}
	}
	if (!stand_alone)
		old_gdos = vq_gdos();
	if (!disabled) {
		*(short *)((long)&vdi_dispatch + 2) = 0x0073;	/* Finally make fVDI take normal VDI calls */
		readable->cookie.flags |= ACTIVE;
	}


	/*
	 * Set up cookies
	 */

	old_eddi = get_cookie("EdDI", 0);
	set_cookie("EdDI", (long)&eddi_dispatch);

	old_fsmc = get_cookie("FSMC", 0);			/* Experimental */
	set_cookie("FSMC", (long)&readable->fsmc_cookie);

	if (set_cookie("fVDI", (long)&readable->cookie) && debug)
		disp_nl("Replacing previous cookie");


	/*
	 * Some trickery to make it possible for a TSR
	 * to allocate and release memory under TOS.
	 */

	if (pid = (long *)pid_addr) {
		copymem((void *)basepage, fake_bp, 256);
		*pid = (long)fake_bp;
	}

	if (debug && (!booted || disabled))
		key_wait(10);

	initialized = 1;

	return 1;
}


/*
 * Shutdown support
 * Unlinks fVDI and releases all allocated memory.
 */
long remove_fvdi(void)
{
	long ret;

	ret = 0;
	if ((readable->cookie.flags & ACTIVE) && !(readable->cookie.flags & BOOTED)) {
		if (old_eddi)
			set_cookie("EdDI", old_eddi);
		if (old_fsmc)
			set_cookie("FSMC", old_fsmc);
		remove_xbra(34 * 4, str2long("fVDI"));	/* Trap #2 handler */
		remove_xbra(46 * 4, str2long("fVDI"));	/* Trap #14 handler */
		remove_xbra(10 * 4, str2long("fVDI"));	/* LineA handler */
		ret = free_all();
		shut_down();
		readable->cookie.flags = 0;
	}

	return ret;
}


/* 
 * If (n > 0) return driver n
 * else return next driver after -n.
 */ 
Driver *find_driver(long n)
{
	List *element;
	Driver *driver;

	element = driver_list;
	while(element) {
#if 0
		if (element->type != 1)
****			bad list type
#endif
		driver = (Driver *)element->value;
		if (((n > 0) && (driver->id == n)) || ((n <= 0) && (driver->id > -n)))
			return driver;
		element = element->next;
	}

	return 0;
}


/*
 * Post-install setup
 */
long setup_fvdi(unsigned long type, long value)
{
	Driver *driver;
	long ret;

	ret = -1;
	if (type >> 16) {
		driver = find_driver(type >> 16);
		if (driver)
			ret = ((long (*)(unsigned long, long))driver->setup)(type & 0xffff, value);
	} else {
		switch(type) {
		case Q_NEXT_DRIVER:
			if (driver = find_driver(-value))
				ret = driver->id;
			break;
		case Q_FILE:
			if (driver = find_driver(value))
				ret = (long)driver->file_name;
			break;
		case S_DEBUG:
			if (value != -1)
				debug = value;
			ret = debug;
			break;
		case S_OPTION:
			ret = tokenize((char *)value);
			break;
		}
	}

	return ret;
}


/*
 * Modify a loaded NVDI so that it will never try
 * to move itself forward in the Trap #2 chain.
 */
int nvdi_patch(void)
{
	long xbra_v, nvdi_v;
	long *addr, *link, *nvdi, *test;
	int i, found;

	xbra_v = str2long("XBRA");
	nvdi_v = str2long("NVDI");

	/*
	 * Search Trap #2 XBRA chain for NVDI
	 */

	link = (long *)0x88;
	addr = (long *)get_protected_l(link);
	while ((addr[-3] == xbra_v) && (addr[-2] != nvdi_v)) {
		link = &addr[-1];
		addr = (long *)*link;
	}

	if (addr[-2] != nvdi_v)
		return 0;				/* No NVDI found! */

	/*
	 * Somewhere in the initial NVDI dispatcher there is, hopefully,
	 * a pointer to the 'real' one. Find this by checking all potential
	 * pointers for another XBRA id.
	 */

	nvdi = addr;
	found = 0;
	for(i = 0; i < MAX_NVDI_SEARCH; i++) {
		addr = (long *)((long)addr + 2);
		test = (long *)*addr;
		if ((ABS(test - nvdi) < MAX_NVDI_DISTANCE / 4) && (test[-3] == xbra_v)) {
			found = 1;
			break;
		}
	}
			
	if (found) {
		set_protected_l(link, (long)test);
		test[-1] = nvdi[-1];
	}

	return found;
}

void recheck_mtask(void)
{
   long addr;
   
   if ((addr = get_cookie("MiNT", 1)) != -1)
      mint = addr;
   if ((addr = get_cookie("MagX", 1)) != -1)
      magic = addr;
   if (mint | magic)
      pid = 0;
}

#ifdef DEBUG
void vdi_debug(VDIpars *pars, char *vector)
{
   static long count = 1;
   static int entered = 0;
   static int current = 0;
   static short set[] = {9100, 109, 110, 121};
   char buf[10];
   int i;
   short func;
   short display;
   char key;
   MFDB *mfdb;

   if (entered)
      return;
   entered = 1;
   
   func = pars->control->function;
   display = 0;
   if ((func == set[current]) || (func == 1984) || (func == 2001) || (func == 1969))
      display = 1;
      
   if (display || ((debug > 2) && (--count == 0))) {
      if (vector && !*--vector) {        /* If there is a name, */
         while(*--vector);     /*   locate it! */
         vector++;
         access->funcs.puts(vector);
         buf[0] = ' ';
         access->funcs.ltoa(buf + 1, func, 10);
         access->funcs.puts(buf);
         access->funcs.puts("\x0a\x0d");
      } else {
         access->funcs.puts("VDI ");
         access->funcs.ltoa(buf, func, 10);
         access->funcs.puts(buf);
         access->funcs.puts("\x0a\x0d");
      }

      if (pars->control->l_intin) {
         access->funcs.puts("  Int");
         access->funcs.ltoa(buf, pars->control->l_intin, 10);
         access->funcs.puts(buf);
         access->funcs.puts(" = ");
         for(i = 0; i < MIN(pars->control->l_intin, 12); i++) {
            access->funcs.ltoa(buf, pars->intin[i], 10);
            access->funcs.puts(buf);
            access->funcs.puts(" ");
         }
         access->funcs.puts("\x0a\x0d");
      }

      if (pars->control->l_ptsin) {
         access->funcs.puts("  Pts");
         access->funcs.ltoa(buf, pars->control->l_ptsin, 10);
         access->funcs.puts(buf);
         access->funcs.puts(" = ");
         for(i = 0; i < MIN(pars->control->l_ptsin * 2, 12); i += 2) {
            access->funcs.ltoa(buf, pars->ptsin[i], 10);
            access->funcs.puts(buf);
            access->funcs.puts(",");
            access->funcs.ltoa(buf, pars->ptsin[i + 1], 10);
            access->funcs.puts(buf);
            access->funcs.puts(" ");
         }
         access->funcs.puts("\x0a\x0d");
      }

      if ((func == 109) || (func == 110) || (func == 121)) {
         mfdb = (MFDB *)pars->control->addr1;
         for(i = 0; i < 2; i++) {
            if (i == 0)
               access->funcs.puts("  MFDB src = $");
            else
               access->funcs.puts("  MFDB dst = $");
            access->funcs.ltoa(buf, (long)mfdb->address, 16);
            access->funcs.puts(buf);
            access->funcs.puts(" ");
            access->funcs.ltoa(buf, mfdb->width, 10);
            access->funcs.puts(buf);
            access->funcs.puts("(");
            access->funcs.ltoa(buf, mfdb->wdwidth, 10);
            access->funcs.puts(buf);
            access->funcs.puts(") ");
            access->funcs.ltoa(buf, mfdb->height, 10);
            access->funcs.puts(buf);
            if (mfdb->standard)
               access->funcs.puts(" standard ");
            else
               access->funcs.puts(" specific ");
            access->funcs.ltoa(buf, mfdb->bitplanes, 10);
            access->funcs.puts(buf);
            access->funcs.puts("\x0a\x0d");
            mfdb = (MFDB *)pars->control->addr2;
         }
      }

      access->funcs.puts("Trap #2: ");
      access->funcs.ltoa(buf, *(long *)0x88, 16);
      access->funcs.puts(buf);
      access->funcs.puts("\x0a\x0d");

      key = key_wait(10);
      switch (key) {
      case 'a':
         count = -1;
         current = 0;
         break;
      case 'b':
         count = -1;
         current = 1;
         break;
      case 'c':
         count = -1;
         current = 2;
         break;
      case 'd':
         count = -1;
         current = 3;
         break;
      case '1':
         count = 5;
         break;
      case '2':
         count = 10;
         break;
      case '3':
         count = 50;
         break;
      case '4':
         count = 250;
         break;
      case '5':
         count = 1000;
         break;
      case '6':
         count = 5000;
         break;
      case '7':
         count = 25000;
         break;
      case '8':
         count = 100000L;
         break;
      case '9':
         count = 1000000L;
         break;
      case 27:
         count = -1;
         break;
      default:
         count = 1;
         break;
      }
   }
   entered = 0;
}

void trap2_debug(long type, VDIpars *pars, long *stack)
{
   char buf[10];
   char key;
   int i;

   stack = (long *)((long)stack + 10);
   access->funcs.puts("Stack: \x0a\x0d");
   for(i = 0; i < 16; i++) {
      access->funcs.ltoa(buf, *stack++, 16);
      access->funcs.puts(buf);
      access->funcs.puts(" ");
   }
   access->funcs.puts(")\x0a\x0d");

   type &= 0xffff;
   if (type == 0x73)
      vdi_debug(pars, 0);
   else {
      access->funcs.puts("Trap #2: ");
      access->funcs.ltoa(buf, type, 16);
      access->funcs.puts(buf);
      access->funcs.puts(" (");
      access->funcs.ltoa(buf, *(long *)0x88, 16);
      access->funcs.puts(buf);
      access->funcs.puts(")\x0a\x0d");

      key = key_wait(10);
   }
}

void lineA_debug(long opcode, long pc)
{
   char buf[10];
   static int entered = 0;

   if (entered)
      return;
   entered = 1;

   access->funcs.puts("LineA call ($a00");
   access->funcs.ltoa(buf, opcode, 16);
   access->funcs.puts(buf);
   access->funcs.puts(") at $");
   access->funcs.ltoa(buf, pc, 16);
   access->funcs.puts(buf);
   access->funcs.puts(".\x0a\x0d");
   
   entered = 0;
}
#endif
