#ifndef _MEMORY_H
#define _MEMORY_H

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
