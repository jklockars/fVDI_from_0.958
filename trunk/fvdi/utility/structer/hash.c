#include <stdio.h>
#include <string.h>
#include <math.h>

#include "hash.h"
#include "memory.h" 

#define IFHT       if (opt & 0x08)

extern int opt;

static unsigned int fast_hash(char *);

static unsigned int hashsize = NHASH;
static unsigned int (*hash)(char *str) = fast_hash;

#if 0
static NTentry *nhash[NHASH];
#else
static NTentry **nhash = NULL;
#endif


static NTentry *find(char *str)
{
   NTentry *np;
  
   np = nhash[hash(str)];
   while (np && (strcmp(np->string, str)))
      np = np->link;
   return np;
}

#if 0
static double mod(double a, double b)
{
   return a - floor(a / b) * b;
}

static int slow_hash(char *str)
{
   double k, k2, c;
   int key, i;

   key = i = 0;
   k = pow(2.0, 15.0);
   k2 = pow((k - 1), 2.0);
   c = sqrt(k2 / k);

   while (*str != '\0') 
      key = mod((key + ((unsigned)*str++) * pow(128.0, abs(4 - i++) % 4)), k);
   return (int)mod((key * key / c), (double)(hashsize + 1));
}
#else
static unsigned int fast_hash(char *str)
{
   unsigned int key;

   key = 0;   
   while (*str) {
      key = (key << 5) ^ *str++ ^ key;
   }
   return key % hashsize;
}
#endif

static char *copy(char *str)
{
   char *new;
   int len;
   
   len = strlen(str); 

   if (!(new = (char *)malloc(len + 1))) {
      printf("Can't allocate storage for new identifier!\n");
      exit(-1);
   }
   strncpy(new, str, len);
   new[len] = '\0';
   return new;
}

NTentry *new_string(char *str)
{
   NTentry *new, *fnd;
   char *cpy;
   int t;

   if (!nhash) {
      IFHT printf("Allocating hash table\n");
      nhash = (NTentry **)malloc(hashsize * sizeof(NTentry *));
      if (!nhash) {
         printf("Out of memory!\n");
         exit(-1);
      }
      memset(nhash, 0, hashsize * sizeof(NTentry *));
   }
   if (fnd = find(str)) {
      IFHT printf("Successful hash search\n");
      return fnd;
   } else {
      new = (NTentry *)memalloc(sizeof(NTentry));
      cpy = copy(str);
      new->string = cpy;
      t = hash(cpy);
      new->link = nhash[t];
      nhash[t] = new;
      return new;
   }
}

void hash_stat(int *total, int *used, int *maxno)
{
   int i, count;
   NTentry *ptr;

   *total = hashsize;   
   *used = *maxno = 0;
   for(i = 0; i < hashsize; i++) {
      if (nhash[i]) {
         (*used)++;
         count = 0;
         ptr = nhash[i];
         while(ptr) {
            count++;
            ptr = ptr->link;
         }
         if (count > *maxno)
            *maxno = count;
      }
   }
}

void init_hash(int size, unsigned int (*func)(char *))
{
   if (size)
      hashsize = size;
   if (func)
      hash = func;
}
