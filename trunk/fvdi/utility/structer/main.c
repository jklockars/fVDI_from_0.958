/*
 * C struct to assembly equ converter
 *
 * $Id: main.c,v 1.3 2002-07-01 21:08:56 johan Exp $
 *
 * Copyright 1997-2002, Johan Klockars
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "hash.h"
#include "expr.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define IFMT       if (opt & 0x02)

extern List definitions;

extern int yyparse(void);

extern convert(char *, List);

int opt;
char *filename = NULL;
extern FILE *yyin;

int main(int argc, char **argv)
{
   int i, total, usage, collisions;
   int count;
   char *name[10];

   yyin = stdin;
   count = 0;
   opt = 0;
   if (argc > 1) {
      for (i = 1; i < argc; i++) {
         if (strcmp(argv[i], "-c") == 0)
            opt |= 0x01;
         else if (strcmp(argv[i], "-d") == 0)
            opt |= 0x02;
         else if (strcmp(argv[i], "-file") == 0) {
            if (i < argc - 1) {
               i++;
               filename = argv[i];
            } else {
               printf("No file name given!\n");
               exit(1);
            }
            if (!(yyin = fopen(filename, "r"))) {
               printf("Could not open file!\n");
               exit(1);
            }
         } else {
            name[count] = argv[i];
            count++;
#if 0
            printf("Usage:  clfl [-c] [-file filename]\n");
            exit(1);
#endif
         }
      }
   }

   if (!yyparse()) {
      IFMT hash_stat(&total, &usage, &collisions);
      IFMT printf("Used %d/%d hash table entries.\n", usage, total);
      IFMT printf("Maximum number of collisions: %d\n\n", collisions);
      IFMT printdefs(definitions);
      for(i = 0; i < count; i++) {
	 convert(name[i], definitions);
      }
   } else {
      printf("Error while parsing!\n");
      return 1;
   }
   return 0;
}
