#ifndef OS_H
#define OS_H
/*
 * Includes compiler specific OS headers
 *
 * $Id: os.h,v 1.2 2006-02-19 01:18:08 johan Exp $
 *
 * Copyright 2003, Johan Klockars 
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#ifdef __PUREC__
   #include <tos.h>
#else
   #include <osbind.h>
   #ifndef __GNUC__
      #include <dos.h>
      #include <tos.h>
   #endif
#endif

/* A couple of file related definitions */
#define O_RDONLY 0
#define O_WRONLY 1
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#endif

