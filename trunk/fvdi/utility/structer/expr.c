#include "misc.h"
#include "expr.h"
#include "list.h"
#include "memory.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#undef TEST
#define ET(text)     if (opt & 0x01) printf("expr.c: %s\n", text)

extern int opt;

void printdefs(List);
void printtype(Expression);


static Expression new(void)
{
   Expression exprcell;

   exprcell = (Expression)memalloc(sizeof(struct _Expr));
   if (!exprcell)
      exit(1);
   return exprcell;
}


Expression mkid(Identifier name, int count)
{
   Expression expr = new();

   ET("mkid");
   expr->type = _Idexpr;
   expr->info.id.name = name;
   expr->info.id.count = count;
   return expr;
}


void printid(Expression expr)
{
   if (expr->info.id.count >= 0)        /* Use to be >, 981114 */
      printf("%s[%d];\n", expr->info.id.name->string, expr->info.id.count);
   else
      printf("%s;\n", expr->info.id.name->string);
}


Expression mktypedef(Expression expr)
{
   ET("mktypedef");
   expr->type = _Typedefexpr;
   return expr;
}


void printtypedef(Expression expr)
{
   printf("typedef\n");
   printtype(expr->info.var.type);
   printid(expr->info.var.id);
}


Expression mkstruct(Identifier name, List defs)
{
   Expression expr = new();

   ET("mkstruct");
   expr->type = _Structexpr;
   expr->info.strct.name = name;
   expr->info.strct.defs = defs;
   return expr;
}


void printstruct(Expression expr)
{
   printf("struct %s {\n", expr->info.strct.name->string);
   printdefs(expr->info.strct.defs);
   printf("} ");
}


Expression mktype(int n, Identifier name, Expression type)
{
   Expression expr = new();

   ET("mktype");
   expr->type = _Typeexpr;
   expr->info.def.sort = n;
   expr->info.def.name = name;
   expr->info.def.type = type;
   return expr;
}


void printtype(Expression expr)
{
   switch(expr->info.def.sort) {
   case _Char:
      printf("char ");
      break;
   case _Short:
      printf("short ");
      break;
   case _Int:
      printf("int ");
      break;
   case _Long:
      printf("long ");
      break;
   case _Struct:
      printf("struct %s ", expr->info.def.name->string);
      break;
   case _Structdef:
      printstruct(expr->info.def.type);
      break;
   case _Typedef:
      printf("%s ", expr->info.def.name->string);
      break;
   case _Pointer:
      printtype(expr->info.def.type);
      printf("* ");
      break;
   }
}


Expression mkvar(Expression type, Expression id)
{
   Expression expr = new();

   ET("var");
   expr->type = _Varexpr;
   expr->info.var.id = id;
   expr->info.var.type = type;
   return expr;
}


void printvar(Expression expr)
{
   printtype(expr->info.var.type);
   printid(expr->info.var.id);
}

void printdefs(List defs)
{
   Listelement ptr;
   
   ptr = FIRST(defs);
   while(ptr) {
      switch(EXPR(ptr)->type) {
      case _Typedefexpr:
         printtypedef(EXPR(ptr));
         break;
      case _Varexpr:
         printvar(EXPR(ptr));
         break;
      case _Structexpr:
         printstruct(EXPR(ptr));
         break;
      }
      NEXT(ptr);
   }
}

