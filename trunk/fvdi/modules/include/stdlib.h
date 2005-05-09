#include "libkern.h"

/* From fvdi/engine/utility.c */
extern void *malloc(long size);
extern void *realloc(void *addr, long new_size);
extern long free(void *addr);
