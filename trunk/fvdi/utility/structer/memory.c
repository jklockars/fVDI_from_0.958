#include <string.h>

#include "memory.h"

#define TEST
#define IFMT         if (opt & 0x100)

extern int opt;

long amount = AMOUNT;

static Block *freemem = NULL;
static long block_count = 0;

static Area *areas = NULL;
static long area_count = 0;

void *memalloc(long size)
{
   Block *newblock;
   Area *newarea;
   int i;
   long area_size;

   if (size > sizeof(Block)) {
      printf("Can't allocate block of size %ld!\n", size);
      exit(-1);
   }   
   if (!freemem) {
      area_size = sizeof(Area) + amount * sizeof(Block);
      newarea = (Area *)malloc(area_size);
      if (!newarea) {
	 printf("memalloc: Out of memory!\n");
	 exit(-1);
      }
      newarea->size = area_size;
      newarea->no_blocks = amount;
      newarea->other = areas;
      areas = newarea;
      newblock = newarea->blocks;
#ifdef TEST
      memset(newblock, -1, size);
#endif
      for(i = amount - 1; i >= 0; i--) {
         newblock[i].next = freemem;
         freemem = &newblock[i];
      }
   }
   newblock = freemem;
   freemem = newblock->next;
   block_count++;
#if 0
   IFMT printf("Now %ld blocks(%ld)\n", block_count, size);
#endif
   return newblock;
}

void memfree(void *block)
{
#ifdef TEST
   memset(block, -1, sizeof(Block));
#endif
   ((Block *)block)->next = freemem;
   freemem = block;
   block_count--;
#if 0
   IFMT printf("Free, now %ld blocks\n", block_count);
#else
   IFMT printf("Free %ld, now %ld blocks\n", (long)block, block_count);
#endif
}

int allocated(void *block)
{
   Block *bptr;
   Area *aptr;
   
   bptr = freemem;
   while (bptr) {
      if (bptr == block)
         return 0;
      bptr = bptr->next;
   }
   
   aptr = areas;
   while (aptr) {
      if (((void *)aptr->blocks <= block) &&
          ((void *)&aptr->blocks[aptr->no_blocks] > block)) {
         if ((block - (void *)aptr->blocks) % sizeof(Block))
            return -1;        /* Not on even block boundary */
         else
            return 1;
      }
      aptr = aptr->other;
   }
   return 0;
}
