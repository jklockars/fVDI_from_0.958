#ifndef OS_H
#define OS_H
/*
 * Includes compiler specific OS headers
 *
 * $Id: os.h,v 1.1 2004-10-17 17:53:54 johan Exp $
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

#endif

