#ifndef _HASH_H
#define _HASH_H

#define NHASH 255

typedef struct _Scopeitem *Scopeitem;

struct _NTentry {
  char *string;
  struct _NTentry*link;
} ;

typedef struct _NTentry NTentry;

extern void init_hash(int, unsigned int (*)(char *));
extern NTentry *new_string(char *);
extern void hash_stat(int *, int *, int *);

#endif


