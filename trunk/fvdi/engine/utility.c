/*
 * fVDI utility functions
 *
 * $Id: utility.c,v 1.7 2002-07-10 22:03:07 johan Exp $
 *
 * Copyright 1997-2002, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */
 
#ifdef __PUREC__
   #include <tos.h>
#else
   #include <osbind.h>
   #ifdef __LATTICE__
      #include <dos.h>
   #endif
#endif

#include "fvdi.h"
#include "relocate.h"

#if 0
#define disp(text) 	Cconws(text)
#define disp_nl(text)	{ Cconws(text); Cconws("\x0a\x0d"); }
#else
#define disp(text) 	puts(text)
#define disp_nl(text)	{ puts(text); puts("\x0a\x0d"); }
#endif


/*
 * External functions called
 */

extern long set_cookie(const unsigned char *cname, long value);
extern long fixup_font(Fontheader *font, char *buffer, long flip);
extern long unpack_font(Fontheader *header, long format);
extern long insert_font(Fontheader **first_font, Fontheader *new_font);
extern long get_size(const char *name);
extern long allocate_block(long size);
extern void free_block(long address);
extern void cache_flush(void);

/*
 * Global variables
 */

extern long basepage;
extern short key_pressed;
extern short debug;
extern short memlink;
extern short debug_out;


Access real_access;
Access *access = &real_access;

typedef struct _Circle {
   struct _Circle *prev;
   struct _Circle *next;
   long size;
} Circle;

#if 0
Circle list_head = {0, 0, 0};
Circle *mblocks = &list_head;
#else
static Circle *mblocks = 0;
#endif

short Falcon = 0;
short TT = 0;
long cpu = 0;
long fpu = 0;
long frb = 0;
long video = 0;

long nvdi = 0;
long eddi = 0;
long mint = 0;
long magic = 0;

long pid_addr = 0;        /* Copied into 'pid' when fVDI is installed */
long *pid = 0;
short mxalloc = 0;

long ARAnyM_out = 0x71354e75L;   /* ARAnyM native printing subroutine */

void puts(const char *text);


/*
 * Returns the value of a cookie, or -1
 */
long get_cookie(const unsigned char *cname, long super)
{
   long oldstack, *ptr, value, name;

   name = 0;
   while(*cname)
      name = (name << 8) | (long)*cname++;

   if (!super)
      oldstack = (long)Super(0L);
   ptr = (long *)*(long *)0x5a0;

   value = -1;
   if (ptr) {
      while ((*ptr != 0) && (*ptr != name))
         ptr += 2;
      if (*ptr == name)
         value = ptr[1];
   }

   if (!super)
      Super((void *)oldstack);
   return value;
}


/*
 * Fetch some interesting cookies
 */
void check_cookies(void)
{
   long addr;

   cpu = get_cookie("_CPU", 0);
   fpu = get_cookie("_FPU", 0);
   frb = get_cookie("_FRB", 0);
   video = get_cookie("_VDO", 0);
   switch((int)(video >> 16)) {   /* Probably a good idea. */
   case 0x0003:
      Falcon = 1;
      break;
   case 0x0002:
      TT = 1;
      break;
   }
   if ((addr = get_cookie("NVDI", 0)) != -1)
      nvdi = *(long *)addr;
   if ((addr = get_cookie("EdDI", 0)) != -1)
      eddi = (long)addr;
   if ((addr = get_cookie("MiNT", 0)) != -1)
      mint = (long)addr;
   if ((addr = get_cookie("MagX", 0)) != -1)
      magic = (long)addr;
}


void copymem(void *s, void *d, long n)
{
   char *src, *dest;

   src = (char *)s;
   dest = (char *)d;
   for(n = n - 1; n >= 0; n--)
      *dest++ = *src++;
}

void setmem(void *d, long v, long n)
{
   char *dest;

   dest = (char *)d;
   for(n = n - 1; n >= 0; n--)
      *dest++ = (char)v;
}


void copy(const char *src, char *dest)
{
   while (*dest++ = *src++)
      ;
}


void cat(const char *src, char *dest)
{
   while(*dest++)
      ;
   copy(src, dest - 1);
}


long length(const char *text)
{
   int n;

   n = 0;
   while(*text++)
      n++;

   return n;
}


long numeric(long ch)
{
   if ((ch >= '0') && (ch <= '9'))
      return 1;
   
   return 0;
}


int check_base(char ch, int base)
{
   if (numeric(ch) && (ch < '0' + base))
      return ch - '0';
   if ((ch >= 'a') && (ch <= 'z'))
      ch -= 'a' - 'A';
   if ((ch >= 'A') && (ch < 'A' + base - 10))
      return ch - 'A' + 10;
   
   return -1;
}


long atol(const char *text)
{
   long n;
   int minus, base, ch;

   minus = 0;   
   if (*text == '-') {
      minus = 1;
      text++;
   }
   base = 10;
   if (*text == '$') {
      base = 16;
      text++;
   } else if (*text == '%') {
      base = 2;
      text++;
   }

   n = 0;
   while ((ch = check_base(*text++, base)) >= 0)
      n = n * base + ch;

   if (minus)
      n = -n;
   
   return n;
}


void ltoa(char *buf, long n, unsigned long base)
{
   unsigned long un;
   char *tmp, ch;
   
   un = n;
   if ((base == 10) && (n < 0)) {
      *buf++ = '-';
      un = -n;
   }
   
   tmp = buf;
   do {
      ch = un % base;
      un = un / base;
      if (ch <= 9)
         ch += '0';
      else
         ch += 'a' - 10;
      *tmp++ = ch;
   } while (un);
   *tmp = '\0';
   while (tmp > buf) {
      ch = *buf;
      *buf++ = *--tmp;
      *tmp = ch;
   }
}


void *malloc(long size, long type)
{
   Circle *new;
   long bp;

   if (pid) {           /* Pretend to be fVDI if possible */
      bp = *pid;
      *pid = basepage;
   }
   if (mint | magic) {
      if (!(type & 0xfff8))	/* Simple type? */
         type |= 0x4030;	/* Keep around, supervisor accessible */
      new = (Circle *)Mxalloc(size + sizeof(Circle), type);
   } else {
      type &= 3;
      if (mxalloc)      /* Alternative if possible */
         new = (Circle *)Mxalloc(size + sizeof(Circle), type);
      else
         new = (Circle *)Malloc(size + sizeof(Circle));
   }
   if (pid) {
      *pid = bp;
   }
   
   if ((long)new > 0) {
      if (debug > 2) {
         char buffer[10];
         ltoa(buffer, (long)new, 16);
         disp("Allocation at $");
         disp(buffer);
         disp(", ");
         ltoa(buffer, size, 10);
         disp(buffer);
         disp_nl(" bytes");
      }
      if (memlink) {
         if (mblocks) {
            new->prev = mblocks->prev;
            new->next = mblocks;
            mblocks->prev->next = new;
            mblocks->prev = new;
         } else {
            mblocks = new;
            new->prev = new;
            new->next = new;
         }
      }
      new->size = size + sizeof(Circle);
      return (void *)&new[1];
   } else
      return new;
}


long free(void *addr)
{
   Circle *current;
   long bp, size, ret;
   
   current = &((Circle *)addr)[-1];
   if (memlink) {
      current->prev->next = current->next;
      current->next->prev = current->prev;
   }
   size = current->size;

   if (pid) {           /* Pretend to be fVDI if possible */
      bp = *pid;
      *pid = basepage;
   }
   if (debug > 2) {
      char buffer[10];
      ltoa(buffer, (long)addr, 16);
      disp("Freeing at $");
      disp_nl(buffer);
   }
   ret = Mfree(current);
   if (pid) {
      *pid = bp;
   }
   
   if (ret)
      return ret;
   else
      return size;
}


long free_all(void)
{
   long oldstack, total, ret, err;

   if (!mblocks)
      return 0;
   
   oldstack = (long)Super(0L);

   err = 0;
   total = 0;
   while (mblocks->prev != mblocks) {    /* Remove the last until there is no more */
      ret = free((void *)&mblocks->prev[1]);
      if (ret > 0)
         total += ret;
      else
         err = 1;
   }

   Super((void *)oldstack);
   
   if (err)
      total = -total;
   return total;
}


void puts(const char *text)
{
   if (debug_out == -2)
      Cconws(text);
   else if (debug_out == -1)
      ((void CDECL (*)(const char *))&ARAnyM_out)(text);
   else
      while (*text)
         Bconout(debug_out, *text++);
}


long equal(const char *str1, const char *str2)
{
   char ch1, ch2;
   
   do {
      ch1 = *str1++;
      ch2 = *str2++;
      if (ch1 != ch2) {
         if ((ch1 >= 'A') && (ch1 <= 'Z')) {
            if ((ch1 | 32) != ch2)
               return 0;
         } else
            return 0;
      }
   } while (ch1 && ch2);
   
   return 1;
}


long misc(long func, long par, const char *token)
{
   switch(func) {
   case 0:
      switch(par) {
      case 0:
         return key_pressed;
         break;
      case 1:
         return debug;
         break;
      }
   }

   return 0;
}


void error(const char *text1, const char *text2)
{
   puts(text1);
   if (text2)
      puts(text2);
   puts("\x0a\x0d");
}


const char *next_line(const char *ptr)
{
   while (1) {
      if (!*ptr)
         return 0;
      else if ((*ptr == 10) || (*ptr == 13))
         break;
      ptr++;
   }
   ptr++;
   while (1) {
      if (!*ptr)
         return 0;
      else if ((*ptr != 10) && (*ptr != 13))
         break;
      ptr++;
   }
   return ptr;
}


const char *skip_space(const char *ptr)
{
   if (!ptr)
      return 0;
   while (1) {
      if (!*ptr)
         return 0;
      else if (*ptr <= ' ')
         ;
      else if (*ptr == '#') {
         if (!(ptr = next_line(ptr)))
            return 0;
         else
            continue;     /* Updating of ptr already done! */
      } else
         return ptr;
      ptr++;
   }
}


const char *get_token(const char *ptr, char *buf, long n)
{
   if (*ptr != '\"') {    /* Get ordinary token */
      while (--n) {
         if (*ptr <= ' ')
            break;
         *buf++ = *ptr++;
      }
   } else {               /* Get quoted token */
      ptr++;
      while (--n) {
         if (((*ptr < ' ') && (*ptr != '\t')) || (*ptr == '\"'))
            break;
         *buf++ = *ptr++;
      }
      if (*ptr == '\"')
         ptr++;
   }
   *buf = '\0';
   
   return ptr;
}


int init_utility(void)
{
   long tmp, oldstack;
   
   check_cookies();
   
   tmp = (long)Mxalloc(10, 3);      /* Try allocating a little memory */
   if (tmp == -32)
      mxalloc = 0;
   else if (tmp < 0)
      return 0;                     /* Should not happen */
   else {
      mxalloc = 1;
      Mfree((void *)tmp);
   }

   oldstack = (long)Super(0L);
   tmp = *(long *)0x4f2;            /* _sysbase */
   if (*(short *)(tmp + 2) < 0x0102)
      tmp = 0;
   else {
      tmp = *(long *)(tmp + 40);       /* p_run (ptr to current base page) */
      if (*(long *)tmp != basepage)    /* Not what it should be? */
         tmp = 0;
   }
   Super((void *)oldstack);

   if (!mint && !magic)     /* Probably a bad idea under multitasking */
      pid_addr = tmp;

#if 0
   mblocks->prev = mblocks;
   mblocks->next = mblocks;
#endif

   real_access.funcs.copymem = copymem;
   real_access.funcs.next_line = next_line;
   real_access.funcs.skip_space = skip_space;
   real_access.funcs.get_token = get_token;
   real_access.funcs.equal = equal;
   real_access.funcs.length = length;
   real_access.funcs.copy = copy;
   real_access.funcs.cat = cat;
   real_access.funcs.numeric = numeric;
   real_access.funcs.error = error;
   real_access.funcs.malloc = malloc;
   real_access.funcs.free = free;
   real_access.funcs.puts = puts;
   real_access.funcs.ltoa = ltoa;
   real_access.funcs.atol = atol;
   real_access.funcs.get_cookie = get_cookie;
   real_access.funcs.set_cookie = set_cookie;
   real_access.funcs.fixup_font = fixup_font;
   real_access.funcs.unpack_font = unpack_font;
   real_access.funcs.insert_font = insert_font;
   real_access.funcs.get_size = get_size;
   real_access.funcs.allocate_block = allocate_block;
   real_access.funcs.free_block = free_block;
   real_access.funcs.cache_flush = cache_flush;
   real_access.funcs.misc = misc;

   return 1;
}
