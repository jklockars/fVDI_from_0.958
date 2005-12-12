/*
 * fVDI startup
 *
 * $Id: startup.c,v 1.39 2005-12-12 01:20:53 johan Exp $
 *
 * Copyright 1999-2003, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "os.h"
#include "fvdi.h"
#include "utility.h"
#include "globals.h"
#include "function.h"
#include "calamus.h"

#define DEBUG

#ifdef DEBUG
 #include "relocate.h"
#endif

#define SYSNAME "fvdi.sys"

#define VERSION	0x0966
#define BETA	6
#define VERmaj	(VERSION >> 12)
#define VERmin	(((VERSION & 0x0f00) >> 8) * 100 + ((VERSION & 0x00f0) >> 4) * 10 + (VERSION & 0x000f))

#define STACK_SIZE	4096		/* Used to be 2048 */

#define fvdi_magic	1969
#define ACTIVE		1		/* fVDI installed */
#define BOOTED		2		/* fVDI can't be removed */

#define MAX_NVDI_SEARCH		100	/* Words of forward search from the initial NVDI dispatcher */
#define MAX_NVDI_DISTANCE	10000	/* Allowed distance between the two dispatchers */

#define key_wait(time)	Crawcin()


/*
 * Global variables
 */

long basepage;
char fake_bp[256];

long old_gdos = -2;

short initialized = 0;

short int_is_short = sizeof(int) == sizeof(short);

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
	short quality;
};	/* fsmc_cookie = {"_FSM", 0x0100, -1}; */

struct NVDI_cookie {
	short version;  /* 0x0502 for version 5.02   */
	long  date;     /* 0x18061990 for 1990-06-18 */
	short flags;    /* 9*reserved, alert, reserved, linea, mouse, gemdos, error, gdos */
};

struct Readable_data {
	struct fVDI_cookie cookie;
	struct FSMC_cookie fsmc_cookie;
	struct NVDI_cookie nvdi_cookie;
	struct DCSD_cookie dcsd_cookie;
} *readable = 0;

struct Super_data *super = 0;

long old_eddi = 0;
long old_fsmc = 0;
long old_nvdi = 0;

long base_page;

/* Stack should probably be allocated dynamically */
char vdi_stack[8192];   /* Used to be 2048, but Standa wants 8192 for FreeType */
char *vdi_stack_top = &vdi_stack[sizeof(vdi_stack)];
long vdi_stack_size = sizeof(vdi_stack);

long stack_address;


/*
 * Top level fVDI initialization
 */
long startup(void)
{
	Virtual *base_vwk, *first_vwk;
	char buffer[10];
	List *element;
	Driver *driver;

	puts_nl("");

	if (!init_utility()) {				/* Make the utility routines ready for use */
		error("Error while initializing utility routines.", 0);
		return 0;
	}

	if (!(super = fmalloc(sizeof(struct Super_data), 0x4033))) {
		error("Could not allocate space for supervisor accessible data.", 0);
		return 0;
	}
	super->fvdi_log.active = 0;
	super->fvdi_log.start = 0;
	super->fvdi_log.current = 0;
	super->fvdi_log.end = 0;

	if (!(readable = fmalloc(sizeof(struct Readable_data), 0x4043))) {
		error("Could not allocate space for world-readable data.", 0);
		return 0;
	}

	if (!(base_vwk = initialize_vdi())) {		/* Setup initial real and virtual workstations */
		error("Error while initializing VDI.", 0);
		return 0;
	}

	if (!load_prefs(base_vwk, SYSNAME)) {		/* Load preferences (and load all fonts and device drivers specified) */
		error("Aborted while loading preferences.", 0);
		return 0;
	}

	readable->cookie.version = VERSION;
	readable->cookie.flags = 0;
	readable->cookie.remove = remove_fvdi;
	readable->cookie.setup = setup_fvdi;
	readable->cookie.log = &super->fvdi_log;
	if (!speedo_cookie) {
		readable->fsmc_cookie.type = str2long("_FNT");  /* Was _FSM */
		readable->fsmc_cookie.versions = 0x0100;
	} else {
		readable->fsmc_cookie.type = str2long("_SPD");
		readable->fsmc_cookie.versions = speedo_cookie;
		readable->fsmc_cookie.quality = 0xffff;
	}
	if (nvdi_cookie) {
		readable->nvdi_cookie.version = nvdi_cookie;
		readable->nvdi_cookie.date    = 0x13052005;
		readable->nvdi_cookie.flags   = 0x0001;  /* GDOS support */
	}
	if (calamus_cookie) {
	    calamus_initialize_cookie(&readable->dcsd_cookie, calamus_cookie);
	}

	if (debug) {				/* Set up log table if asked for */
		if (super->fvdi_log.start = malloc(log_size * sizeof(long))) {
			super->fvdi_log.active = 1;
			super->fvdi_log.current = super->fvdi_log.start;
			super->fvdi_log.end = &super->fvdi_log.start[log_size - 8];
		}
	}

	if (!initialize_pool(block_size, blocks)) {	/* Initialize the internal memory pool */
		error("Error while initializing memory pool.", 0);
		return 0;
	}

	if (remove_xbra(34*4, "fVDI") && debug)		/* fVDI might already be installed */
		puts_nl("Removing previous XBRA.");

	if (nvdifix && nvdi_patch() && debug)
		puts_nl("Patching NVDI dispatcher");

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

#ifdef __PUREC__
	lineA_address = (long)Setexc(10, (void (*)())&lineA);		/* Install a LineA handler */
#else
	lineA_address = (long)Setexc(10, (void *)&lineA);		/* Install a LineA handler */
#endif
	
	stack_address = (long)vdi_stack;

	puts("fVDI v");
	ltoa(buffer, VERmaj, 10);
	puts(buffer);
	puts(".");
	ltoa(buffer, VERmin, 10);
	puts(buffer);
	if (BETA) {
		puts("beta");
		ltoa(buffer, BETA, 10);
		puts(buffer);
	}
	if (!int_is_short) {
		puts("L");
	}
#ifdef __GNUC__
	puts("gcc");
#else
 #ifdef __PUREC__
	puts("purec");
 #endif
#endif
#ifdef FT2
	puts("-FT2");
#endif
	puts_nl(" now installed.");

	if (debug) {
		ltoa(buffer, (long)&init, 16);
		puts("fVDI engine Text: $");
		puts(buffer);
		ltoa(buffer, (long)&data_start, 16);
		puts("   Data: $");
		puts(buffer);
		ltoa(buffer, (long)&bss_start, 16);
		puts("   Bss: $");
		puts_nl(buffer);
		if (super->fvdi_log.active) {
			ltoa(buffer, (long)&super->fvdi_log, 16);
			puts("Logging at $");
			puts_nl(buffer);
		}
	}


	/*
	 * During the load process, all installed drivers were linked onto a list.
	 * Go through the list and call all post install driver initialization routines.
	 */

	if (debug)
		puts_nl("Post install initialization of drivers");

	first_vwk = 0;
	element = driver_list;
	while (element) {
#if 0
		if (element->type != 1)
****			bad list type
#endif
		driver = (Driver *)element->value;
		if (driver->module.flags & 1) {
			if (!first_vwk)
				first_vwk = driver->default_vwk;
			if (debug) {
				puts(" ");
				puts(driver->module.name);
				ltoa(buffer, (long)driver->module.initialize, 16);
				puts(" at $");
				puts_nl(buffer);
			}
			if (!((long (*)(Virtual *))(driver->module.initialize))(driver->default_vwk)) {
				/* Driver that fails initialization should be removed! */
				if (debug) {
				    puts_nl("  Failed initialization!");
				}
			}
		}
		element = element->next;
	}

	/*
	 * Open and initialize copies of previous workstations for fVDI,
	 * unless this is a boot (in which case a fall-back is set up instead).
	 */

	if (!booted) {
		if (debug)
			puts_nl("Copying available virtual workstations");
		copy_workstations(first_vwk, !fakeboot);	/* f.vwk - default vwk to set up for, fall-through if fakeboot */
	} else if (!disabled) {
		if (!fakeboot && (!stand_alone || fall_back)) {
			if (debug) {
				puts_nl("About to set up VDI fallback. Press any key.");
				key_wait(10);			/* It's too late to wait for a key afterwards */
			}
			setup_fallback();
			readable->cookie.flags |= BOOTED;
		}
	}

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

	if (nvdi_cookie) {
		old_nvdi = get_cookie("NVDI", 0);
		set_cookie("NVDI", (long)&readable->nvdi_cookie);
	}

	if (calamus_cookie && (get_cookie("DCSD", 0) == -1)) {
		set_cookie("DCSD", (long)&readable->dcsd_cookie);
		puts_nl("Calamus cookie installed");
	}

	if (set_cookie("fVDI", (long)&readable->cookie) && debug)
		puts_nl("Replacing previous cookie");

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
		if (old_nvdi)
			set_cookie("NVDI", old_nvdi);
		remove_xbra(34 * 4, "fVDI");		/* Trap #2 handler */
		remove_xbra(46 * 4, "fVDI");		/* Trap #14 handler */
		remove_xbra(10 * 4, "fVDI");		/* LineA handler */
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
		if (((n > 0) && (driver->module.id == n)) || ((n <= 0) && (driver->module.id > -n)))
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
			ret = ((long (*)(unsigned long, long))driver->module.setup)(type & 0xffff, value);
	} else {
		switch(type) {
		case Q_NEXT_DRIVER:
			if (driver = find_driver(-value))
				ret = driver->module.id;
			break;
		case Q_FILE:
			if (driver = find_driver(value))
				ret = (long)driver->module.file_name;
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
	long *addr, link, *nvdi, *test;
	int i, found;

	xbra_v = str2long("XBRA");
	nvdi_v = str2long("NVDI");

	/*
	 * Search Trap #2 XBRA chain for NVDI
	 */

	link = 0x88;
	addr = (long *)get_protected_l(link);
	while ((addr[-3] == xbra_v) && (addr[-2] != nvdi_v)) {
		link = (long)&addr[-1];
		addr = *(long **)link;
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

	/*
	 * If the 'real' NVDI dispatch was found, bypass the initial one.
	 */
		
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
   long old_count;

   if (entered)
      return;
   entered = 1;
   
   func = pars->control->function;
   if (silent[func >> 3] & (1 << (func & 7))) {
      entered = 0;
      return;
   }
   display = 0;
   if ((func == set[current]) || (func == 1984) || (func == 2001) || (func == 1969))
      display = 1;

   old_count = count;      
   if (display || ((debug > 2) && (--count == 0))) {
      if (vector && !*--vector) {        /* If there is a name, */
         while(*--vector);               /*   locate it! */
         vector++;
         access->funcs.puts(vector);
         buf[0] = ' ';
         access->funcs.ltoa(buf + 1, func, 10);
      } else {
         access->funcs.puts("VDI ");
         access->funcs.ltoa(buf, func, 10);
      }
      access->funcs.puts(buf);
      if (pars->control->subfunction) {
         buf[0] = ' ';
         access->funcs.ltoa(buf + 1, pars->control->subfunction, 10);
         access->funcs.puts(buf);
      }
      access->funcs.puts("\x0a\x0d");

      if (pars->control->l_intin) {
         access->funcs.puts("  Int");
         access->funcs.ltoa(buf, pars->control->l_intin, 10);
         access->funcs.puts(buf);
         access->funcs.puts(" = ");
         if ((func == 8) || (func == 241) || (func ==  116) || (func == 117)) {
            access->funcs.puts("\"");
            for(i = 0; i < MIN(pars->control->l_intin, 72); i++) {
              buf[0] = '.';
              buf[1] = 0;
              if ((pars->intin[i] >= 32) && (pars->intin[i] < 256))
                 buf[0] = (char)pars->intin[i];
               access->funcs.puts(buf);
            }
            access->funcs.puts("\"");
         } else {
            for(i = 0; i < MIN(pars->control->l_intin, 12); i++) {
               access->funcs.ltoa(buf, pars->intin[i], 10);
               access->funcs.puts(buf);
               access->funcs.puts(" ");
            }
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

      if (debug > 3) {
         access->funcs.puts("Trap #2: ");
         access->funcs.ltoa(buf, *(long *)0x88, 16);
         access->funcs.puts(buf);
         access->funcs.puts("\x0a\x0d");
      }

      count = old_count;
      if (interactive) {
         key = key_wait(10);
         switch (key) {
         case 'q':
            count = -1;
            current = 0;
            break;
         case 'w':
            count = -1;
            current = 1;
            break;
         case 'e':
            count = -1;
            current = 2;
            break;
         case 'r':
            count = -1;
            current = 3;
            break;
         case 'd':
            debug++;
            break;
         case 'D':
            debug--;
            break;
         case 'i':
            interactive = 0;
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
   }
   entered = 0;
}


void display_output(VDIpars *pars)
{
  char buf[10];
  int i;

  if (pars->control->l_intin) {
    access->funcs.puts("  Intout");
    access->funcs.ltoa(buf, pars->control->l_intout, 10);
    access->funcs.puts(buf);
    access->funcs.puts(" = ");
    for(i = 0; i < MIN(pars->control->l_intout, 12); i++) {
      access->funcs.ltoa(buf, pars->intout[i], 10);
      access->funcs.puts(buf);
      access->funcs.puts(" ");
    }
  }
  access->funcs.puts("\x0a\x0d");

  if (pars->control->l_ptsout) {
    access->funcs.puts("  Ptsout");
    access->funcs.ltoa(buf, pars->control->l_ptsout, 10);
    access->funcs.puts(buf);
    access->funcs.puts(" = ");
    for(i = 0; i < MIN(pars->control->l_ptsout * 2, 12); i += 2) {
      access->funcs.ltoa(buf, pars->ptsout[i], 10);
      access->funcs.puts(buf);
      access->funcs.puts(",");
      access->funcs.ltoa(buf, pars->ptsout[i + 1], 10);
      access->funcs.puts(buf);
      access->funcs.puts(" ");
    }
    access->funcs.puts("\x0a\x0d");
  }
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
