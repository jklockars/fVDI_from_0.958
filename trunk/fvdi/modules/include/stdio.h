#include "libkern.h"

#if 0
#define sprintf	 ksprintf_old
#define snprintf ksprintf
#endif
extern int sprintf(char *str, const char *format, ...);

/* The possibilities for the third argument to `fseek'.
   These values should not be changed.  */
#define	SEEK_SET	0	/* Seek from beginning of file. */
#define	SEEK_CUR	1	/* Seek from current position. */
#define	SEEK_END	2	/* Seek from end of file. */
