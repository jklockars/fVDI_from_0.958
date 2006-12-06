/*
 * fVDI utility functions
 *
 * $Id: utility.c,v 1.35 2006-12-06 14:34:46 standa Exp $
 *
 * Copyright 1997-2003, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "stdarg.h"
 
#include "os.h"
#include "fvdi.h"
#include "relocate.h"
#include "utility.h"
#include "globals.h"


typedef unsigned long size_t;

/*
 * Global variables
 */

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

/* NatFeat functions defined */
#if 1
static long _NF_getid = 0x73004e75L;
static long _NF_call  = 0x73014e75L;
#define nfGetID(n)  (((long __CDECL (*)(const char *))&_NF_getid)n)
#define nfCall(n)   (((long __CDECL (*)(long, ...))&_NF_call)n)

long nf_print_id = 0;
#else
typedef struct {  /* NatFeat code */
        long magic;
        long CDECL(*nfGetID) (const char *);
        long CDECL(*nfCall) (long ID, ...);
} NatFeatCookie;

NatFeatCookie *nf_ptr = 0;
long nf_printf_id = 0;
#endif

long pid_addr = 0;        /* Copied into 'pid' when fVDI is installed */
long *pid = 0;
short mxalloc = 0;

char *block_chain = 0;


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


long get_l(long addr)
{
   return *(long *)addr;
}


/*
 * Get a long value after switching to supervisor mode
 */
long get_protected_l(long addr)
{
   long oldstack, v;

   oldstack = (long)Super(0L);
   v = *(long *)addr;
   Super((void *)oldstack);

   return v;
}


void set_l(long addr, long value)
{
   *(long *)addr = value;
}


/*
 * Set a long value after switching to supervisor mode
 */
void set_protected_l(long addr, long value)
{
   long oldstack;

   oldstack = (long)Super(0L);
   *(long *)addr = value;
   Super((void *)oldstack);
}


/*
 * Returns the value of a cookie, or -1
 */
long get_cookie(const unsigned char *cname, long super)
{
   long ptr, value, cname_l;
   long (*get)(long addr);

   cname_l = str2long(cname);

#if 0
   if (gemdos(340, -1, 0L, 0L) == 0) {
      /* Ssystem is available */
      return gemdos(340, 8, cname_l, 0L);
   }
#endif

   get = super ? get_l : get_protected_l;

   ptr = get(0x5a0);

   value = -1;
   if (ptr) {
      long v = get(ptr);
      while (v && (v != cname_l))
      {
         ptr += 8;
         v = get(ptr);
      }
      if (v == cname_l)
         value = get(ptr + 4);
   }

   return value;
}


/*
 * Follows an XBRA chain and removes a link if found.
 * Returns 0 if XBRA id not found.
 */
long remove_xbra(long vector, const unsigned char *name)
{
   long link, *addr, name_l, xbra_l;
   link = vector;
   addr = (long *)get_protected_l(link);    /* Probably an exception vector */
   xbra_l = str2long("XBRA");
   name_l = str2long(name);

   while ((addr[-3] == xbra_l) && (addr[-2] != name_l)) {
      link = (long)&addr[-1];
      addr = (long *)addr[-1];
   }
   if (addr[-3] != xbra_l)
      return 0;

   set_protected_l(link, addr[-1]);    /* Might well be an exception vector */

   return 1;
}


/*
 * Set a cookie value. Replace if there already is one.
 * Create/expand jar if needed.
 * Returns != 0 if an old cookie was replaced.
 */
long set_cookie(const unsigned char *name, long value)
{
   long *addr, *old_addr;
   long name_l;
   int count;

   count = 0;
   name_l = str2long(name);
   addr = old_addr = (long *)get_protected_l(0x5a0);    /* _p_cookies */

   if (addr) {
      while (*addr && (*addr != name_l)) {
         count++;
         addr += 2;
      }
      if (*addr == name_l) {
         addr[1] = value;
         return 1;
      }

      /* Must make sure there is room for the final count!  [010109] */

      if (count != addr[1] - 1) {
         addr[2] = 0;
         addr[3] = addr[1];
         addr[0] = name_l;
         addr[1] = value;
         return 0;
      }
   }

   addr = malloc((count + 8) * 8);

   copymem(old_addr, addr, count * 8);

   addr[count * 2 + 0] = name_l;
   addr[count * 2 + 1] = value;
   addr[count * 2 + 2] = 0;
   addr[count * 2 + 3] = count + 8;

   set_protected_l(0x5a0, (long)addr);    /* _p_cookies */

   return 0;
}


/*
 * Initialize an internal memory pool.
 * Returns zero on error.
 */
long initialize_pool(long size, long n)
{
   char *addr, *ptr;

   if ((size <= 0) || (n <= 0))
      return 0;

   if (!(addr = malloc(size * n)))
      return 0;

   block_size = size;
   ptr = 0;
   for(n = n - 1; n >= 0; n--) {
      block_chain = addr;
      *(char **)addr = ptr;
      ptr = addr;
      addr += size;
   }

   return 1;
}


/*
 * Allocate a block from the internal memory pool.
 */
char *allocate_block(long size)
{
   char *addr;

   if ((size > block_size) || !block_chain)
      return 0;

   addr = block_chain;
   block_chain = *(char **)addr;
   *(long *)addr = block_size;    /* Make size info available */

   return addr;
}


/*
 * Free a block and return it to the internal memory pool.
 */
void free_block(void *addr)
{
   *(char **)addr = block_chain;
   block_chain = addr;
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
   if ((addr = get_cookie("__NF", 0)) != -1) {
#if 1
      nf_print_id = nfGetID(("NF_STDERR"));
#else
      nf_ptr = (NatFeatCookie *)addr;
      if (nf_ptr->magic != 0x20021021L)   /* NatFeat magic constant */
        nf_ptr = 0;
      else {
        nf_printf_id = nf_ptr->nfGetID("DEBUGPRINTF");
#endif
   }
}


void copymem(const void *s, void *d, long n)
{
   char *src, *dest;

   src  = (char *)s;
   dest = (char *)d;
   for(n = n - 1; n >= 0; n--)
      *dest++ = *src++;
}


void copymem_aligned(const void *s, void *d, long n)
{
   long *src, *dest, n4;

   src  = (long *)s;
   dest = (long *)d;
   for(n4 = (n >> 2) - 1; n4 >= 0; n4--)
      *dest++ = *src++;

   if (n & 3) {
      char *s1 = (char *)src;
      char *d1 = (char *)dest;
      switch(n & 3) {
      case 3:
         *d1++ = *s1++;
      case 2:
         *d1++ = *s1++;
      case 1:
         *d1++ = *s1++;
         break;
      }
   }
}


#ifndef USE_LIBKERN
void *memcpy(void *dest, const void *src, size_t n)
{
   if (n > 3) {
     if (!((long)dest & 1)) {
       if (!((long)src & 1)) {
          copymem_aligned(src, dest, n);
          return dest;
       }
     } else if ((long)src & 1) {
       *(char *)dest++ = *(char *)src++;
       copymem_aligned(src, dest, n - 1);
       return dest - 1;
     }
   }

   copymem(src, dest, n);

   return dest;
}


void *memmove(void *dest, const void *src, long n)
{
   char *s1, *d1;

   if (((long)dest >= (long)src + n) || ((long)dest + n <= (long)src))
      return memcpy(dest, src, n);

   if ((long)dest < (long)src) {
      s1 = (char *)src;
      d1 = (char *)dest;
      for(n--; n >= 0; n--)
         *d1++ = *s1++;
   } else {
      s1 = (char *)src + n;
      d1 = (char *)dest + n;
      for(n--; n >= 0; n--)
         *(--d1) = *(--s1);
   }

   return dest;
}
#endif


void setmem(void *d, long v, long n)
{
   char *dest;

   dest = (char *)d;
   for(n = n - 1; n >= 0; n--)
      *dest++ = (char)v;
}


void setmem_aligned(void *d, long v, long n)
{
   long *dest;
   long n4;

   dest = (long *)d;
   for(n4 = (n >> 2) - 1; n4 >= 0; n4--)
      *dest++ = v;

   if (n & 3) {
      char *d1 = (char *)dest;
      switch(n & 3) {
      case 3:
         *d1++ = (char)(v >> 24);
      case 2:
         *d1++ = (char)(v >> 16);
      case 1:
         *d1++ = (char)(v >> 8);
         break;
      }
   }
}


#ifndef USE_LIBKERN
/* This function needs an 'int' parameter
 * to be compatible with gcc's built-in
 * version.
 * For module use, a separate version will
 * be needed since they can't be guaranteed
 * to have the same size for 'int'.
 */
void *memset(void *s, int c, size_t n)
{
   if ((n > 3) && !((long)s & 1)) {
      unsigned long v;
      v = ((unsigned short)c << 8) | (unsigned short)c;
      v = (v << 16) | v;
      setmem_aligned(s, v, n);
   } else
      setmem(s, c, n);

   return s;
}


long strlen(const char *s)
{
   const char *p = s;
   while (*p++)
      ;

   return (long)(p - s) - 1;
}


long strcmp(const char *s1, const char *s2)
{
   char c1;

   do {
       if (!(c1 = *s1++)) {
           s2++;
           break;
       }
   } while (c1 == *s2++);

   return (long)(c1 - s2[-1]);
}


long strncmp(const char *s1, const char *s2, size_t n)
{
   char c1;
   long ns;     /* size_t can't be negative */

   ns = n;
   for(ns--; ns >= 0; ns--) {
      if (!(c1 = *s1++)) {
         s2++;
         break;
      }
      if (c1 != *s2++)
         break;
   }

   if (ns < 0)
      return 0L;

   return (long)(c1 - s2[-1]);
}


long memcmp(const void *s1, const void *s2, size_t n)
{
   char *s1c, *s2c;
   long ns;     /* size_t can't be negative */

   ns = n;
   s1c = (char *)s1;
   s2c = (char *)s2;
   for(ns--; ns >= 0; ns--) {
      if (*s1c++ != *s2c++)
         return (long)(s1c[-1] - s2c[-1]);
   }

   return 0L;
}
#endif


void copy(const char *src, char *dest)
{
   while (*dest++ = *src++)
      ;
}


#ifndef USE_LIBKERN
char *strcpy(char *dest, const char *src)
{
   copy(src, dest);

   return dest;
}


char *strncpy(char *dest, const char *src, size_t n)
{
   char c1, *d;
   long ns;

   d = dest;
   ns = n;
   for(ns--; ns >= 0; ns--) {
      c1 = *src++;
      *dest++ = c1;
      if (!c1)
         break;
   }
   for(ns--; ns >= 0; ns--)
      *dest++ = 0;

   return d;
}


char *strdup(const char *s)
{
   char *d;

   if (d = (char *)malloc(strlen(s) + 1))
      strcpy(d, s);

   return d;
}
#endif


void cat(const char *src, char *dest)
{
   while(*dest++)
      ;
   copy(src, dest - 1);
}


#ifndef USE_LIBKERN
char *strcat(char *dest, const char *src)
{
   cat(src, dest);

   return dest;
}


char *strchr(const char *s, long c)
{
   char ch, c1;

   if (!c)
      return (char *)s + strlen(s);

   c1 = c;
   while(ch = *s++) {
      if (ch == c1)
         return (char *)s - 1;
   }

   return 0;
}


char *strrchr(const char *s, long c)
{
   char *found, ch, c1;

   if (!c)
      return (char *)s + strlen(s);

   c1 = c;
   found = 0;
   while(ch = *s++) {
      if (ch == c1)
         found = (char *)s;
   }

   if (found)
      return found - 1;

   return 0;
}


void *memchr(const void *s, long c, size_t n)
{
   char ch, c1;
   char *m;
   long ns;

   m = (char *)s;
   c1 = c;
   ns = n;
   for(ns--; ns >= 0; ns--) {
      ch = *m++;
      if (ch == c1)
         return (void *)m - 1;
   }

   return 0;
}


char *memrchr(const void *s, long c, size_t n)
{
   char ch, c1;
   char *m;
   long ns;

   m = (char *)s + n;
   c1 = c;
   ns = n;
   for(ns--; ns >= 0; ns--) {
      ch = *--m;
      if (ch == c1)
         return (void *)m;
   }

   return 0;
}
#endif


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


long check_base(char ch, long base)
{
   if (numeric(ch) && (ch < '0' + base))
      return ch - '0';
   if ((ch >= 'a') && (ch <= 'z'))
      ch -= 'a' - 'A';
   if ((ch >= 'A') && (ch < 'A' + base - 10))
      return ch - 'A' + 10;
   
   return -1;
}


#ifndef USE_LIBKERN
long isdigit(long c)
{
   return numeric(c);
}


long isxdigit(long c)
{
   return check_base(c, 16) >= 0;
}


long isalnum(long c)
{
    return check_base(c, 36) >= 0;   /* Base 36 has 0-9, A-Z */
}
#endif


long isspace(long c)
{
   switch(c) {
   case ' ':
   case '\f':
   case '\n':
   case '\r':
   case '\t':
   case '\v':
      return 1;
   }

   return 0;
}


long atol(const char *text)
{
   long n;
   int minus, base, ch;

   while(isspace(*text))
       text++;

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


void ultoa(char *buf, unsigned long un, unsigned long base)
{
   char *tmp, ch;
   
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


#ifndef USE_LIBKERN
/* Mostly complete, but only simple things tested */
long vsprintf(char *str, const char *format, va_list args)
{
   int mode = 0;
   char *s, *text, ch;
   long val_l;
   int base = 10;
   int field = 0;
   int precision = 0;
   int opts = 0;
   char *num_start;

   s = str;

   while (ch = *format++) {
      if (mode) {
         switch(ch) {
         case 's':
            text = va_arg(args, char *);
            while (*str++ = *text++)
               ;
            str--;
            mode = 0;
            break;

         case 'c':
            *str++ = va_arg(args, int);    /* char promoted to int */
            mode = 0;
            break;

         case 'p':
            opts |= 16;
            mode = 4;
         case 'x':
         case 'X':
            base = 16;
            if (opts & 16) {
               *str++ = '0';
               *str++ = 'x';
            }
         case 'o':
            if (base == 10) {
               base = 8;
               if (opts & 16) {
                  *str++ = '0';
               }
            }
         case 'u':
            opts |= 32;   /* Unsigned conversion */
         case 'd':
         case 'i':
           if (!(opts & 0x0100) && (opts & 1) && !(opts & 8)) {
               precision = field;
               field = 0;
            }
            switch (mode) {
            case 4:
               val_l = va_arg(args, long);
               break;
            case 3:
               if (opts & 32)
                  val_l = va_arg(args, unsigned char);
               else
                  val_l = va_arg(args, signed char);
               break;
            case 2:
               if (opts & 32)
                  val_l = va_arg(args, unsigned short);
               else
                  val_l = va_arg(args, short);
               break;
            case 1:
               if (opts & 32)
                  val_l = va_arg(args, unsigned int);
               else
                  val_l = va_arg(args, int);
               break;
            default:
               break;
            }
            if (!(opts & 32)) {
               if (val_l > 0) {
                  if (opts & 4)
                     *str++ = '+';
                  else if (opts & 2)
                     *str++ = ' ';
               } else if (val_l < 0) {
                  *str++ = '-';
                  val_l = -val_l;
               }
            }
            if (val_l || !(opts & 0x0100) || (precision != 0))
               ultoa(str, val_l, base);
            val_l = strlen(str);
            if (val_l < precision) {
              memmove(str + precision - val_l, str, val_l + 1);
              memset(str, '0', precision - val_l);
            }
            str += strlen(str);
            mode = 0;
            break;

         case '%':
            *str++ = '%';
            mode = 0;
            break;


         case 'h':
            opts |= 0x8000;
            if (mode == 1)
               mode = 2;
            else if (mode == 2)
               mode = 3;
            else
               opts |= 0x4000;
            break;

         case 'z':
         case 't':
         case 'l':
            opts |= 0x8000;
            if (mode == 1)
               mode = 4;
            else
               opts |= 0x4000;
            break;


         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
            if (mode == 1) {
               ch -= '0';
               if (!ch && !(opts & 0x8000)) {
                  if (!(opts & 1))
                     opts |= 1;   /* Zero padding */
                  else
                     opts |= 0x4000;
               } else {
                  opts |= 0x8000;
                  if (opts & 0x0100) {
                     if (!(opts & 0x0200)) {
                        opts |= 0x0400;
                        precision = precision * 10 + ch;
                     } else
                        opts |= 0x4000;
                  } else if (!(opts & 0x0800)) {
                     opts |= 0x0400;
                     field = field * 10 + ch;
                  } else
                     opts |= 0x4000;
               }
            } else
               opts |= 0x4000;
            break;

         case '*':
            if (mode == 1) {
               if (opts & 0x0100) {
                  if (!(opts & 0x0600)) {
                     opts |= 0x0200;
                     precision = va_arg(args, int);
                     if (precision < 0)
                        precision = 0;
                  } else
                     opts |= 0x4000;
               } else if (!(opts & 0x8c00)) {
                  opts |= 0x8800;
                  field = va_arg(args, int);
                  if (field < 0) {
                     opts |= 8;
                     field = -field;
                  }
               } else
                  opts |= 0x4000;
            } else
               opts |= 0x4000;
            break;

         case ' ':
            if (!(opts & 0x8002)) {
               opts |= 2;   /* Space in front of positive numbers */
            } else
               opts |= 0x4000;
            break;

         case '+':
            if (!(opts & 0x8004)) {
               opts |= 4;   /* Sign in front of all numbers */
            } else
               opts |= 0x4000;
            break;

         case '-':
            if (!(opts & 0x8008)) {
               opts |= 8;   /* Left justified field */
            } else
               opts |= 0x4000;
            break;

         case '#':
            if (!(opts & 0x8010)) {
               opts |= 16;  /* 0x/0 in front of hexadecimal/octal numbers */
            } else
               opts |= 0x4000;
            break;

         case '.':
            if (!(opts & 0x0100) && (mode == 1)) {
               opts &= ~0x0400;
               opts |= 0x8100;
               precision = 0;
            } else
               opts |= 0x4000;
            break;

         default:
            opts |= 0x4000;
            break;
         }

         if (opts & 0x4000) {
           *str++ = '%';
           *str++ = '?';
           mode = 0;
         }

         if ((mode == 0) && (field > str - s)) {
           val_l = field - (str - s);
           if (opts & 8) {
             memset(str, ' ', val_l);
           } else {
             memmove(s + val_l, s, str - s);
             memset(s, ' ', val_l);
           }
           str += val_l;
         }
      } else if (ch == '%') {
         mode = 1;
         base = 10;
         opts = 0;
         field = 0;
         precision = 0;
         s = str;
      } else {
         *str++ = ch;
      }
   }
   *str = 0;

   return strlen(s);
}

long sprintf(char *str, const char *format, ...)
{
   va_list args;
   va_start(args, format);
   vsprintf(str, format, args);
   va_end(args);
}



/* This is really a Shell sort. */
/* Not the best, but short and decent. */
 #if 0
void fvdi_qsort(void *base, long nmemb, long size,
                long (*compar)(const void *, const void *))
{
   long gap, i, j, k, gap_size;
   char *p, *pt, *q, c, *cbase;

   for(gap = nmemb / 2; gap > 0; gap /= 2) {
      gap_size = gap * size;
      cbase = (char *)base + gap_size;
      for(i = gap; i < nmemb; i++) {
         q = cbase;
         p = q - gap_size;
         for(j = i - gap; j >= 0; j -= gap) {
            if ((*compar)(p, q) <= 0)
               break;
            else {
               pt = p;
               for(k = size - 1; k >= 0; k--) {
                  c    = *q;
                  *q++ = *pt;
                  *pt++ = c;
               }
            }
            q = p;
            p -= gap_size;
         }
         cbase += size;
      }
   }
}
 #else
void qsort(void *base, long nmemb, long size,
           long (*compar)(const void *, const void *))
{
    static long incs[16] = { 1391376, 463792, 198768, 86961, 33936, 13776, 
                             4592, 1968, 861, 336, 112, 48, 21, 7, 3, 1 };
    long i, j, k, h, j_size, h_size;
    short n;
    char buf[16], *v, *p1, *p2, *cbase;

    v = buf;
    if (size > sizeof(buf)) {
       v = malloc(size);
       if (!v)       /* Can't sort? */
          return;
    }

   cbase = (char *)base;
   for(k = 0; k < 16; k++) {
      h = incs[k];
      h_size = h * size;
      for(i = h; i < nmemb; i++) {
         j = i;
         j_size = j * size;
         p1 = v;
         p2 = cbase + j_size;
         for(n = size - 1; n >= 0; n--)
            *p1++ = *p2++;
         while ((j >= h) && (compar(v, cbase + j_size - h_size) < 0)) {
            p1 = cbase + j_size;
            p2 = p1 - h_size;
            for(n = size - 1; n >= 0; n--)
               *p1++ = *p2++;
            j -= h;
            j_size -= h_size;
         }
         p1 = cbase + j_size;
         p2 = v;
         for(n = size - 1; n >= 0; n--)
            *p1++ = *p2++;
      }
   } 

   if (size > sizeof(buf))
      free(v);
}
 #endif
#endif


void *fmalloc(long size, long type)
{
   Circle *new;
   long bp;

   if (pid) {           /* Pretend to be fVDI if possible */
      bp = *pid;
      *pid = basepage;
   }
   if (mint | magic) {
      if (!(type & 0xfff8))  /* Simple type? */
         type |= 0x4030;     /* Keep around, supervisor accessible */
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
      if ((debug > 2) && !(silentx[0] & 0x01)) {
         char buffer[10];
         ltoa(buffer, (long)new, 16);
         puts("Allocation at $");
         puts(buffer);
         puts(", ");
         ltoa(buffer, size, 10);
         puts(buffer);
         puts_nl(" bytes");
      }
      if (memlink) {
         if (mblocks) {
            new->prev = mblocks->prev;
            new->next = mblocks;
#if 0
            mblocks->prev->next = new;
            mblocks->prev = new;
#else
            ((Circle *)((long)mblocks->prev & ~1))->next = new;
            mblocks->prev = (Circle *)((long)new | 1);
#endif
         } else {
            mblocks = new;
#if 0
            new->prev = new;
#else
            new->prev = (Circle *)((long)new | 1);
#endif
            new->next = new;
         }
      }
      new->size = size + sizeof(Circle);
      *(long *)&new[1] = size;
      return (void *)&new[1];
   } else
      return new;
}


#if 0
  static short block_space[] = {32, 76, 156, 316, 1008, 2028, 4068, 8148, 16308};
#else
  static short block_space[] = {16, 48, 112, 240, 496, 1008, 2028, 4068, 8148, 16308};
#endif
  static char *block_free[]  = { 0,  0,   0,   0,   0,    0,    0,    0,    0,     0};
#if 0
  static char *block_used[]  = { 0,  0,   0,   0,   0,    0,    0,    0,    0,     0};
#else
#if 0
static Circle block_used[]  = { {0,0,0},  {0,0,0},   {0,0,0},   {0,0,0},   {0,0,0},    {0,0,0},    {0,0,0},    {0,0,0},    {0,0,0},     {0,0,0}};
#else
  static Circle *block_used[]  = { 0,  0,   0,   0,   0,    0,    0,    0,    0,     0};
#endif
#endif
  static short free_blocks[] = { 0,  0,   0,   0,   0,    0,    0,    0,    0,     0};
  static short used_blocks[] = { 0,  0,   0,   0,   0,    0,    0,    0,    0,     0};
  static short allocated = 0;

#define ADDR_NOT_OK 0xfc000003


void memory_statistics(void)
{
  const int sizes = sizeof(block_space) / sizeof(block_space[0]);
  char buf[10];
  int n;

  puts("       ");
  ltoa(buf, allocated, 10);
  puts(buf);
  puts(": ");
  for(n = 0; n < sizes; n++) {
    ltoa(buf, used_blocks[n], 10);
    puts(buf);
    puts("/");
    ltoa(buf, free_blocks[n], 10);
    puts(buf);
    puts(" ");
  }
  puts("\x0d\x0a");
}


void allocate(long amount)
{
  const int sizes = sizeof(block_space) / sizeof(block_space[0]);
  char *buf;
  Circle *link, *last;
  int i;

  amount &= ~0x0fL;
  if (!amount)
    return;

  buf = fmalloc(amount * 1024, 3);
  if (!buf)
    return;

  if ((debug > 2) && !(silentx[0] & 0x02)) {
    puts("       Malloc at ");
    ltoa(buf, (long)buf, 16);
    puts(buf);
    puts("\x0d\x0a");
  }

  last = (Circle *)block_free[sizes - 1];
  for(i = 0; i < amount; i += 16) {
    link = (Circle *)&buf[i * 1024L];
    link->next = last;
    link->prev = 0;
    link->size = sizes - 1;
    last = link;
  }

  block_free[sizes - 1] = (char *)last;
  free_blocks[sizes - 1] += amount >> 4;
  allocated += amount >> 4;
}


void search_links(Circle *srch)
{

  int m, n;
  const int sizes = sizeof(block_space) / sizeof(block_space[0]);
  char *block, buf[10];
  Circle *link, *next, *first, *found;
  long dist, dist_min;
  int dist_n;

  dist_min = 999999999;
  for(n = 0; n < sizes; n++) {
    link = (Circle *)block_free[n];
    while (link) {
      if (((long)link & ADDR_NOT_OK) ||
          ((unsigned int)(link->size & 0xffff) >=
           sizeof(block_space) / sizeof(block_space[0])) ||
          !(link->size >> 16))
        break;

      dist = (long)srch - (long)link;
      if ((dist > 0) && (dist < dist_min)) {
        dist_min = dist;
        dist_n = n | 0x8000;
        found = link;
      }

      link = link->next;
    }

    link = first = block_used[n];
    while (link) {
      if (((long)link & ADDR_NOT_OK) ||
          ((unsigned int)(link->size & 0xffff) >=
           sizeof(block_space) / sizeof(block_space[0])) ||
          !(link->size >> 16))
        break;

      dist = (long)srch - (long)link;
      if ((dist > 0) && (dist < dist_min)) {
        dist_min = dist;
        dist_n = n;
        found = link;
      }

      next = link->next;
      if ((next == first) || (next->prev != link))
        break;
      link = next;
    }
  }

  puts("At distance ");
  ltoa(buf, dist_min, 10);
  puts(buf);
  puts(": ");
  ltoa(buf, (long)found, 16);
  puts(buf);
  puts("(");
  ltoa(buf, dist_n & 0x7fff, 10);
  puts(buf);
  if (dist_n & 0x8000)
    puts(" free)");
  else
    puts(" used)");
  
  puts("\x0d\x0a");
}


void display_links(Circle *first)
{
  Circle *link, *last;
  char buf[10];
  int m = 0;
  int n;

  puts("Links: ");
  ltoa(buf, (long)first, 16);
  puts(buf);

  last = first;
  if (first) {
    link = first->next;
    while (link != first) {
      if (m++ > 1000)
        break;
      puts("->");
      ltoa(buf, (long)link, 16);
      puts(buf);
      if (link->prev != last) {
        puts("(");
        ltoa(buf, (long)link->prev, 16);
        puts(buf);
        puts(")");
        break;
      }
      last = link;
      link = link->next;
      if ((long)link & ADDR_NOT_OK)
        break;
    }
  }
  puts("\x0d\x0a");
}


void check_memory(void)
{
  int m, n;
  const int sizes = sizeof(block_space) / sizeof(block_space[0]);
  char *block, buf[10];
  Circle *link, *next, *first;
  int error, statistics;

  statistics = 0;
  for(n = 0; n < sizes; n++) {
    link = (Circle *)block_free[n];
    m = 0;
    error = 0;
    while (link) {
      m++;
      if ((long)link & ADDR_NOT_OK) {
        puts("Bad free list linkage at ");
        error = 1;
      }
#if 0
      else if ((unsigned int)(link->size & 0xffff) >=
                 sizeof(block_space) / sizeof(block_space[0]) ||
                 !(link->size >> 16)) {
        puts("Bad free list size at ");
        error = 1;
      }
#endif
      if (error) {
        ltoa(buf, (long)link, 16);
        puts(buf);
        puts("(");
        ltoa(buf, m, 10);
        puts(buf);
        puts(",");
        ltoa(buf, n, 10);
        puts(buf);
        puts(")\x0\x0d");
#if 0
        display_links(block_free[n]);
#endif
        break;
      }

      link = link->next;
    }

    if (!error && (m != free_blocks[n])) {
      puts("Wrong number of free blocks (");
      ltoa(buf, m, 10);
      puts(buf);
      puts("/");
      ltoa(buf, free_blocks[n], 10);
      puts(buf);
      puts(")\x0d\x0a");
#if 0
      display_links(block_free[n]);
#endif
      error = 1;
    }

    if (error && !statistics) {
        statistics = 1;
        memory_statistics();
    }

    link = first = block_used[n];
    m = 0;
    error = 0;
    while (link) {
      m++;
      if ((long)link & ADDR_NOT_OK) {
        puts("Bad used list link at ");
        error = 1;
      } else if ((unsigned int)(link->size & 0xffff) >=
                  sizeof(block_space) / sizeof(block_space[0]) ||
                 !(link->size >> 16)) {
        puts("\x0d\x0a");
        search_links(link);
        puts("Bad used list size at ");
        error = 1;
      } else if ((long)link->next & ADDR_NOT_OK) {
        puts("\x0d\x0a");
        search_links(link);
        puts("Bad used list linkage at ");
        error = 1;
      }
      next = link->next;
      if (next->prev != link) {
        puts("\x0d\x0a");
        search_links(next);
        puts("Bad used list prev linkage ");
        ltoa(buf, (long)next, 16);
        puts(buf);
        puts(" ");
        ltoa(buf, (long)next->prev, 16);
        puts(buf);
        puts(" ");
        error = 1;
      }

      if (error) {
        ltoa(buf, (long)link, 16);
        puts(buf);
        puts(" (");
        ltoa(buf, m, 10);
        puts(buf);
        puts(",");
        ltoa(buf, n, 10);
        puts(buf);
        puts(")\x0d\x0a");
        display_links(block_used[n]);
        break;
      }
      if (next == first)
        break;
      link = next;
    }

    if (!error && (m != used_blocks[n])) {
      puts("Wrong number of used blocks (");
      ltoa(buf, n, 10);
      puts(buf);
      puts(":");
      ltoa(buf, m, 10);
      puts(buf);
      puts("/");
      ltoa(buf, used_blocks[n], 10);
      puts(buf);
      puts(")\x0d\x0a");
      display_links(block_used[n]);
      error = 1;
    }

    if (error && !statistics) {
        statistics = 1;
        memory_statistics();
    }
  }
}

#define OS_MARGIN 64
#define MIN_BLOCK 32
#define LINK_SIZE (sizeof(Circle))
#define MIN_KB_BLOCK
void *malloc(long size)
{
  int m, n;
  const int sizes = sizeof(block_space) / sizeof(block_space[0]);
  char *block, buf[10];
  Circle *link, *next;

  size += ext_malloc;
  
#if 0
  debug_out = -1;
#endif

  if (check_mem)
    check_memory();

#if 0
  if (!block_space[0]) {
    block_space[0] = MIN_BLOCK;
    i = 0;
    while (block_space[i++] < MIN_KB_BLOCK)
      block_space[i] = (block_space[i - 1] + LINK_SIZE) * 2;
  }
#endif

  if (old_malloc || (size > 16 * 1024 - OS_MARGIN - LINK_SIZE))
    return fmalloc(size, 3);

  /* This will always break eventually thanks to the if-statement above */
  for(n = 0; n < sizes; n++) {
    if (size <= block_space[n])
      break;
  }

  if ((debug > 2) && !(silentx[0] & 0x02)) {
    ltoa(buf, n, 10);
    puts("Alloc: Need block of size ");
    puts(buf);
    puts("/");
    ltoa(buf, size, 10);
    puts(buf);
    puts("\x0d\x0a");
  }

  if (!block_free[n]) {
    for(m = n + 1; m < sizes; m++) {
      if (block_free[m])
        break;
    }
    if (m >= sizes) {
      m = sizes - 1;
      block_free[m] = fmalloc(16 * 1024 - OS_MARGIN, 3);
      if (!block_free[m])
        return 0;
      if ((debug > 2) && !(silentx[0] & 0x02)) {
        puts("       Malloc at ");
        ltoa(buf, (long)block_free[m], 16);
        puts(buf);
        puts("\x0d\x0a");
      }
      link = (Circle *)block_free[m];
      link->next = 0;
      link->prev = 0;
      link->size = m;
      free_blocks[m]++;
      allocated++;
    }
    for(; m > n; m--) {
      if ((debug > 2) && !(silentx[0] & 0x02)) {
        puts("       Splitting\x0d\x0a");
      }
      block_free[m - 1] = block_free[m];
      link = (Circle *)block_free[m];
      block_free[m] = (char *)link->next;
      free_blocks[m]--;
      next = (Circle *)(block_free[m - 1] + block_space[m - 1] + LINK_SIZE);
      link->next = next;
      link->prev = 0;
      link->size = m - 1;
      free_blocks[m - 1]++;
      link = next;
      link->next = 0;
      link->prev = 0;
      link->size = m - 1;
      free_blocks[m - 1]++;
    }
  } else {
    if ((debug > 2) && !(silentx[0] & 0x02)) {
      puts("       Available\x0d\x0a");
    }
  }

  block = block_free[n];
  block_free[n] = (char *)((Circle *)block)->next;

  if ((debug > 2) && !(silentx[0] & 0x02)) {
    puts("       Allocating at ");
    ltoa(buf, (long)block, 16);
    puts(buf);
    puts(" (next at ");
    ltoa(buf, (long)block_free[n], 16);
    puts(buf);
    puts(")\x0d\x0a");
  }

  ((Circle *)block)->size = (((Circle *)block)->size & 0xffff) + (size << 16);

#if 1
      if (1 || memlink) {
         Circle *new = (Circle *)block;
         if (block_used[n]) {
            new->prev = block_used[n]->prev;
            new->next = block_used[n];
            block_used[n]->prev->next = new;
            block_used[n]->prev = new;
         } else {
            block_used[n] = new;
            new->prev = new;
            new->next = new;
         }
      }
 #if 0
      if (debug > 2) {
        char buf[10];
        ltoa(buf, (long)block_used[n], 16);
        puts(buf);
        puts(" ");
        ltoa(buf, (long)block_used[n]->prev, 16);
        puts(buf);
        puts(" ");
        ltoa(buf, (long)block_used[n]->next, 16);
        puts(buf);
        puts("\x0d\x0a");
      }
 #endif
#endif

  free_blocks[n]--;
  used_blocks[n]++;

  if ((debug > 2) && !(silentx[0] & 0x02)) {
      memory_statistics();
  }

#if 1
  *(long *)(block + sizeof(Circle)) = block_space[n];
  return block + sizeof(Circle);
#else
  return fmalloc(size, 3);
#endif
}


void *realloc(void *addr, long new_size)
{
   Circle *current;
   long old_size;
   void *new;

   if (!addr)
      return malloc(new_size);
   if (!new_size) {
      free(addr);
      return 0;
   }

   new = malloc(new_size);
   if ((long)new <= 0)
      return 0;
   current = &((Circle *)addr)[-1];
#if 0
   if (current->prev)
#else
   if ((long)current->prev & 1)
#endif
      old_size = current->size - sizeof(Circle);
   else
      old_size = current->size >> 16;

   copymem_aligned(addr, new, old_size < new_size ? old_size : new_size);
   free(addr);

   if ((debug > 2) && !(silentx[0] & 0x01)) {
      char buffer[10];
      puts("Reallocation from size ");
      ltoa(buffer, old_size, 10);
      puts(buffer);
      puts(" at $");
      ltoa(buffer, (long)addr, 16);
      puts(buffer);
      puts(" to ");
      ltoa(buffer, new_size, 10);
      puts_nl(buffer);
   }

   return new;
}


long free(void *addr)
{
   Circle *current;
   long bp, size, ret;

   if (!addr)
       return 0;

   current = &((Circle *)addr)[-1];

#if 0
   if (!current->prev) {
#else
   if (!((long)current->prev & 1)) {
#endif
     size = current->size & 0xffff;
     if (((debug > 2) && !(silentx[0] & 0x02)) ||
         (unsigned int)size >= sizeof(block_space) / sizeof(block_space[0]) ||
         !(current->size >> 16)) {
       char buf[10];
       puts("Freeing at ");
       ltoa(buf, (long)current, 16);
       puts(buf);
#if 1
       puts(" (");
       ltoa(buf, (long)current->size, 16);
       puts(buf);
       puts(" ,");
       ltoa(buf, (long)current->prev, 16);
       puts(buf);
       puts(" ,");
       ltoa(buf, (long)current->next, 16);
       puts(buf);
       puts(")");
#endif
       puts("\x0d\x0a");
     }
#if 1
   if (1 || memlink) {
     if (block_used[size] == current) {
       block_used[size] = current->next;
       if (current->next == current->prev)
         block_used[size] = 0;
     }
     current->prev->next = current->next;
     current->next->prev = current->prev;
   }
#endif
     current->next = (Circle *)block_free[size];
     block_free[size] = (char *)current;
     free_blocks[size]++;
     used_blocks[size]--;
     return 0;
   } else {
     if ((debug > 2) && !(silentx[0] & 0x01)) {
       char buf[10];
       puts("Standard free at ");
       ltoa(buf, (long)current, 16);
       puts(buf);
       puts("\x0d\x0a");
     }
   }

   if (memlink) {
#if 0
      current->prev->next = current->next;
#else
      ((Circle *)((long)current->prev & ~1))->next = current->next;
#endif
      current->next->prev = current->prev;
   }
   size = current->size;

   if (pid) {           /* Pretend to be fVDI if possible */
      bp = *pid;
      *pid = basepage;
   }
   if ((debug > 2) && !(silentx[0] & 0x01)) {
      char buffer[10];
      ltoa(buffer, (long)current, 16);
      puts("Freeing at $");
      puts_nl(buffer);
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


long puts(const char *text)
{
   int file, bytes;

   if ((debug_out == -3) && debug_file) {
      file = -1;

#if 0
/* WHY: THIS SEQUENCE MAKES FreeMiNT GO OUT OF PROCESS IO HANDLES */ 
      file = Fopen(debug_file, O_WRONLY);
      Fseek(0, file, SEEK_END);
      Cconws("Write to debug file failed!\x0d\x0a");
      Fclose(file);
#endif

      if (((file = Fopen(debug_file, O_WRONLY)) < 0) ||
          (Fseek(0, file, SEEK_END) < 0) ||
          (Fwrite(file, strlen(text), text) < 0)) {
         free(debug_file);
         debug_file = 0;
         debug_out = -2;
         Cconws("Write to debug file failed!\x0d\x0a");
         Cconws(text);
      }
      if (file >= 0)
         Fclose(file);
   }
   else if (debug_out == -2)
      Cconws(text);
#if 1
   else if ((debug_out == -1) && nf_print_id)
      nfCall((nf_print_id, text));
#else
   else if ((debug_out == -1) && nf_printf_id)
     nf_ptr->nfCall(nf_printf_id, text);
#endif
   else
      while (*text)
         Bconout(debug_out, *text++);

   return 1;
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
   puts("\x0d\x0a");
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


const char *skip_only_space(const char *ptr)
{
   if (!ptr)
      return 0;
   while ((*ptr == ' ') || (*ptr == '\t')) {
      ptr++;
   }

   return ptr;
}


const char *get_token(const char *ptr, char *buf, long n)
{
   if (*ptr == '=')            /* Assignment token? */
      *buf++ = *ptr++;
   else if (*ptr != '\"') {    /* Get ordinary token */
      while (--n) {
        if ((*ptr <= ' ') || (*ptr == '='))
            break;
         *buf++ = *ptr++;
      }
   } else {                    /* Get quoted token */
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


/*
 * Returns the size of a file 
 */
long get_size(const char *name)
{
#ifdef __GNUC__
   _DTA info;
#else
   DTA info;       /* Thanks to tos.h for Lattice C */
#endif
   long file_size;
   int error;

   Fsetdta((void *)&info);
   error = Fsfirst(name, 1);

   if (!error) {
#ifdef __GNUC__
      file_size = info.dta_size;
#else
      file_size = info.d_length;
#endif
   } else
      file_size = -1;

   return file_size;
}


long event(long id_type, long data)
{
   long xy;

   /* Really needs to do something about the id part */
   switch(id_type & 0xffff) {
   case 0:    /* Initialize */
      stand_alone = 1;
      break;
   case 1:    /* Relative mouse movement */
      if (!screen_wk->mouse.forced) {
         if (data) {
            xy = *(long *)&screen_wk->mouse.position.x;
            data = (((xy >> 16) + (data >> 16)) << 16) |
                   ((xy + data) & 0xffff);
            data = vector_call(screen_wk->vector.motion, data);
            data = vector_call(screen_wk->vector.draw, data);
         }
      }
      break;
   case 2:    /* Absolute mouse movement */
      if (!screen_wk->mouse.forced) {
         xy = *(long *)&screen_wk->mouse.position.x;
         if (data != xy) {
            extern void *mouse_move;
            if (move_mouse)
               vector_call(&mouse_move, data);
            data = vector_call(screen_wk->vector.motion, data);
            data = vector_call(screen_wk->vector.draw, data);
         }
      }
      break;
   case 3:    /* Button change */
      if (data != screen_wk->mouse.buttons) {
         screen_wk->mouse.buttons = data;
         data = (data << 16) | (data & 0xffff);
         data = vector_call(screen_wk->vector.button, data);
      }
      break;
   case 4:    /* Wheel movement */
      if (data) {
         data = vector_call(screen_wk->vector.wheel, data);
      }
      break;
   case 5:    /* Vertical blank */
      if (vbl_handler_installed)
         shutdown_vbl_handler();
      data = vector_call(screen_wk->vector.vblank, data);
      break;
   case 6:    /* Forced absolute move */
      xy = *(long *)&screen_wk->mouse.position.x;
      if (data != xy) {
         screen_wk->mouse.forced = 1;
         data = vector_call(screen_wk->vector.motion, data);
         data = vector_call(screen_wk->vector.draw, data);
      }
      break;
   }
}


long init_utility(void)
{
   long tmp;
   
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

   tmp = get_protected_l(0x4f2);       /* _sysbase */
   if ((get_protected_l(tmp) & 0x0000ffff) < 0x0102)
      tmp = 0;
   else {
      tmp = get_protected_l(tmp + 40); /* p_run (ptr to current base page) */
      if (get_protected_l(tmp) != basepage)    /* Not what it should be? */
         tmp = 0;
   }

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
   real_access.funcs.malloc = fmalloc;
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

   real_access.funcs.event = event;

   return 1;
}
