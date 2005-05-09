
#ifndef _SETJMP_H
#define _SETJMP_H 1


typedef long jmp_buf[14]; /* retaddr, 12 regs, sigmask */


int   _cdecl _mint_setjmp  (jmp_buf);
void  _cdecl _mint_longjmp (jmp_buf, int);


# define setjmp			_mint_setjmp
# define longjmp		_mint_longjmp


#endif /* _SETJMP_H */
