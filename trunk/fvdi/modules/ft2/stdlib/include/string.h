#include "libkern.h"

char * strdup(const char *s);

#define memmove(d,s,l) bcopy(s,d,l)
