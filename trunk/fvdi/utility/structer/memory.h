#ifndef _MEMORY_H
#define _MEMORY_H
/*
 * Memory declarations
 *
 * $Id: memory.h,v 1.2 2002-05-13 01:25:24 johan Exp $
 *
 * Copyright 1997-2002, Johan Klockars
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#define AMOUNT 20L

struct _Block {
   struct _Block *next;
   long dummy[4];
} ;

typedef struct _Block Block;

struct _Area {
   long size;
   long no_blocks;
   struct _Area *other;
   Block blocks[0];
} ;

typedef struct _Area Area;

extern void *memalloc(long);
extern void memfree(void *);
extern int allocated(void *);

#endif
