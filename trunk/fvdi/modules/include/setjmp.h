#ifndef SETJMP_H
#define SETJMP_H

typedef long jmp_buf[14]; /* retaddr, 12 regs, sigmask */

extern int   _cdecl _setjmp  (jmp_buf);
extern void  _cdecl _longjmp (jmp_buf, int);

#endif /* SETJMP_H */
