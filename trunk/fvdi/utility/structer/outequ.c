#undef GIVE_UP_ON_ERROR

#include "misc.h"
#include "expr.h"
#include "list.h"
#include "memory.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#undef TEST
#define OT(text)     if (opt & 0x04) printf("outequ.c: %s\n", text)

extern int opt;

char *outer[10];
int count;


void push(char *name)
{
   outer[count] = name;
   count++;
}

void pop(void)
{
   count--;
}

void out(char *name, int pos)
{
   int i;
   
   for(i = 0; i < count; i++) {
      printf("%s_", outer[i]);
   }
   printf("%s\tequ\t%d\n", name, pos);
}

int compare(char *name, Identifier struct_id)
{
   if (!struct_id)
      return 0;
   return !strcmp(name, struct_id->string);
}

Expression locate_typedef(char *name, List all_defs)
{
   Listelement ptr;
   Expression cur_expr, type_expr, expr;
   int type;

   ptr = FIRST(all_defs);
   while(ptr) {
      cur_expr = EXPR(ptr);
      if (cur_expr->type == _Typedefexpr) {
         if (!cur_expr->info.var.id) {
            printf("No id!\n");
#ifdef GIVE_UP_ON_ERROR
            exit(-1);
#endif
         } else
            if (compare(name, cur_expr->info.var.id->info.id.name))
               return cur_expr;
      }
      NEXT(ptr);
   }
   return NULL;
}

int expr_to_equ(Expression cur_expr, int pos, List all_defs, int *size)
{
   Expression type_expr, expr;
   int type;
   char *name, *typedef_name;
   int oldpos;
   int count;
   int dummy;

   count = cur_expr->info.var.id->info.id.count;
   type_expr = cur_expr->info.var.type;
   if (type_expr->type != _Typeexpr) {
      printf("Unsupported construction (B)!\n");
#ifdef GIVE_UP_ON_ERROR
      exit(-1);
#endif
   }
   name = cur_expr->info.var.id->info.id.name->string;

   switch (type_expr->info.def.sort) {
   case _Char:
#if 0
      printf("char %d\n", pos);
#endif
#if 0
      if (count > 0)
         pos = (pos + 1) & 0xfffe;
#endif
      out(name, pos);
      *size = 1;
      break;
   case _Short:
#if 0
      printf("short %d\n", pos);
#endif
      pos = (pos + 1) & 0xfffe;
      out(name, pos);
      *size = 2;
      break;
   case _Int:
#if 0
      printf("int %d\n", pos);
#endif
      pos = (pos + 1) & 0xfffe;
      out(name, pos);
      *size = 4;
      break;
   case _Long:
#if 0
      printf("long %d\n", pos);
#endif
      pos = (pos + 1) & 0xfffe;
      out(name, pos);
      *size = 4;
      break;
   case _Structdef:
#if 0
      printf("structdef %d\n", pos);
#endif
      expr = type_expr->info.def.type;
      if (expr->type != _Structexpr) {
         printf("Bad type (E)!\n");
#ifdef GIVE_UP_ON_ERROR
         exit(-1);
#endif
      }
      pos = (pos + 1) & 0xfffe;
      out(name, pos);
      dummy = list_to_equ(name, expr->info.strct.defs, pos, all_defs, size);
      *size = (*size + 1) & 0xfffe;
      break;
   case _Struct:
#if 0
      printf("struct %d\n", pos);
#endif
      printf("Not yet supported (C)!\n");
#ifdef GIVE_UP_ON_ERROR
      exit(-1);
#endif
      break;
   case _Typedef:
#if 0
      printf("typedef %d\n", pos);
#endif
      if (expr = locate_typedef(type_expr->info.def.name->string, all_defs)) {
                      /* Below is an ugly hack! */
         typedef_name = expr->info.var.id->info.id.name->string;
         expr->info.var.id->info.id.name->string = name;
         dummy = expr_to_equ(expr, pos, all_defs, size);
         if (pos + *size != dummy)
            pos++;
         expr->info.var.id->info.id.name->string = typedef_name;
      } else {
         printf("Unknown typedef!\n");
#ifdef GIVE_UP_ON_ERROR
         exit(-1);
#endif
      }
      break;
   case _Pointer:
#if 0
      printf("pointer %d\n", pos);
#endif
      pos = (pos + 1) & 0xfffe;
      out(name, pos);
      *size = 4;
      break;
   default:
      printf("Bad type (%d)!\n", type_expr->info.def.sort);
#ifdef GIVE_UP_ON_ERROR
      exit(-1);
#endif
      break;
   }
   if (count >= 0) {             /* Used to be >, 981114 */
#if 0
      printf("Array of %d elements a %d\n", count, *size);
#endif
      pos += *size * count;
      *size *= count;
   } else
      pos += *size;
   
   return pos;
}

int list_to_equ(char *base, List defs, int pos, List all_defs, int *size)
{
   Listelement ptr;
   Expression cur_expr, type_expr, expr;
   int type;
   int oldpos;
   int dummy;

   oldpos = pos;
   push(base);
   ptr = FIRST(defs);
   while(ptr) {
      cur_expr = EXPR(ptr);
      if (cur_expr->type != _Varexpr) {
         printf("Unsupported construction (A)!\n");
#ifdef GIVE_UP_ON_ERROR
         exit(-1);
#endif
      }
      pos = expr_to_equ(cur_expr, pos, all_defs, &dummy);
      NEXT(ptr);
   }
   pop();
   *size = pos - oldpos;
   return pos;
}

void convert(char *name, List defs)
{
   Listelement ptr;
   Expression expr;
   int type;
   int size;
   char *equ_name;
   int dummy;

   count = 0;
   if (equ_name = strchr(name, '=')) {
      *equ_name = '\0';
      equ_name++;
   } else
      equ_name = name;
   size = 0;
   
   ptr = FIRST(defs);
   while(ptr) {
      expr = EXPR(ptr);
      type = expr->type;
      if (type == _Structexpr) {
         printf("Checking structexpr\n");
         if (compare(name, expr->info.strct.name)) {
            size = list_to_equ(equ_name, expr->info.strct.defs, 0, defs, &dummy);
            break;
         } else {
#if 0
            printf("Mismatch: %s, %s\n", name, expr->info.strct.name->string);
#endif
         }
      } else if (type == _Typedefexpr) {
         if (compare(name, expr->info.var.id->info.id.name)) {
            expr = expr->info.var.type;
            if ((expr->type == _Typeexpr) && (expr->info.def.sort == _Structdef)) {
               size = list_to_equ(equ_name, expr->info.def.type->info.strct.defs, 0, defs, &dummy);
               break;
            } else {
               printf("Not a struct typedef (%d)!\n", expr->type);
               exit(-1);
            }
         } else {
#if 0
            printf("Mismatch: %s, %s\n", name, expr->info.var.id->info.id.name->string);
#endif
         }
      } else if (type == _Varexpr) {
         if (compare(name, expr->info.var.id->info.id.name)) {
            expr = expr->info.var.type;
            if ((expr->type == _Typeexpr) && (expr->info.def.sort == _Structdef)) {
               size = list_to_equ(equ_name, expr->info.def.type->info.strct.defs, 0, defs, &dummy);
               break;
            } else {
               printf("Not a struct variable (%d)!\n", expr->type);
               exit(-1);
            }
         } else {
#if 0
            printf("Mismatch: %s, %s\n", name, expr->info.var.id->info.id.name->string);
#endif
         }
      } else {
         printf("Expression type: %d\n", type);
      }
      NEXT(ptr);
   }
   if (size) {
      printf("%s_struct_size\tequ\t%d\n\n", equ_name, size);
   }
}
