/*
 * Conversion of some assembly syntax to gas.
 * This is an ugly hack, but it does its job for now.
 * Might be improved later, perhaps.
 *
 * $Id: conv2gas.c,v 1.1 2004-10-17 21:44:11 johan Exp $
 *
 * Copyright 2004, Johan Klockars
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "stdio.h"
#include "string.h"
#include "ctype.h"

struct branches_t {
  char *macro;
  char *instr;
};

struct branches_t branches[] = {
  "lbra", "bra",
  "lbeq", "beq",
  "lbne", "bne",
  "lbge", "bge",
  "lbgt", "bgt",
  "lble", "ble",
  "lblt", "blt",
  "lbhi", "bhi",
  "lbls", "bls",
  "lblo", "blo",
  "lbcc", "bcc",
  "lbcs", "bcs",
  "lbpl", "bpl",
  "lbmi", "bmi"
};

struct branches_t dbranches[] = {
  "ldbra", "dbra",
  "ldbeq", "dbeq"
};


char get_token(char **buf, char *token, char end_marker)
{
  char ch;

  for(; **buf; (*buf)++) {
    if (!isspace(**buf))
      break;
  }
  if (!**buf)
    return 0;

  if (**buf == '\"') {
    *token++ = *(*buf)++;
    for(; **buf; (*buf)++) {
      if (**buf == '\"')
        break;
      else
	*token++ = **buf;
    }
  }
  for(; **buf; (*buf)++) {
    if (isspace(**buf) || (**buf == end_marker))
      break;
    else
      *token++ = **buf;
  }
  if (!**buf)
    return 0;

  *token = 0;
  ch = **buf;
  (*buf)++;

  return ch;
}


int main(int argc, char **argv)
{
  FILE *infile;
  char buf[1024], label[32], token1[32], token2[32], token3[32], *ptr;
  int delimiter, i, found, tmp;


  if (argc != 2)
  {
    printf("labeler <file>\n");
    exit(-1);
  }

  if (!(infile = fopen(argv[1], "r")))
  {
    printf("Could not open %s\n", argv[1]);
    exit(-1);
  }

  while (fgets(buf, sizeof(buf), infile))
  {
    ptr = buf;
    delimiter = get_token(&ptr, token1, ':');

    found = 0;
    if (delimiter == ':') {                        // Non-local label
      if (token1[0] != '.')
        strcpy(label, token1);
      else
        strcpy(label, &token1[1]);
      printf("%s", buf);
      found = 1;
    } else if (delimiter) {
      if ((strcmp("label", token1) == 0) &&        // Local label
          (get_token(&ptr, token1, ',') == ',')) {
        printf("%s___%s:\n", token1, label);
        found = 1;
      } else if (strcmp("xref", token1) == 0) {    // External declaration
        while (get_token(&ptr, token2, ','))
          printf("*\t.external\t%s\n", token2);
        found = 1;
      } else if (strcmp("xdef", token1) == 0) {    // Global declaration
        while (get_token(&ptr, token2, ','))
          printf("\t.global\t%s\n", token2);
        found = 1;
      } else if ((strcmp(".byte", token1) == 0) &&  // Declare bytes with string
                 strchr(ptr, '\"')) {
        while (get_token(&ptr, token2, ',')) {
          if (token2[0] != '\"')
            printf("\t.byte\t%s\n", token2);
          else
            printf("\t.ascii\t%s\n", token2);
        }
        found = 1;
      } else {                                     // Look for macro branches
        for(i = 0; i < sizeof(branches) / sizeof(branches[0]); i++)
        {
          if ((strcmp(branches[i].macro, token1) == 0) &&
              (get_token(&ptr, token2, ',') == ',')) {
            printf("\t%s\t%s___%s\n", branches[i].instr, token2, label);
            found = 1;
            break;
          }
        }
        if (!found)
        {                                          // Look for macro dbranches
          for(i = 0; i < sizeof(dbranches) / sizeof(dbranches[0]); i++)
          {
            if ((strcmp(dbranches[i].macro, token1) == 0) &&
                (get_token(&ptr, token2, ',') == ',') &&
                (get_token(&ptr, token3, ',') == ',')) {
              printf("\t%s\t%s,%s___%s\n", dbranches[i].instr,
                     token2, token3, label);
              found = 1;
              break;
            }
          }
        }
        if (!found)
        {                                          // Character constants
          if ((get_token(&ptr, token2, ',') == ',') &&
              (token2[0] == '#') && (token2[1] == '\'') &&
              (token2[strlen(token2) - 1] == '\'')) {
            tmp = 0;
            for(i = 2; token2[i] && (token2[i] != '\''); i++)
              tmp = tmp * 256 + token2[i];
            printf("\t%s\t#0x%x,%s\n", token1, tmp, ptr);
            found = 1;
          }
        }
      }
    }
    if (!found)
      printf("%s", buf);
  }
}
