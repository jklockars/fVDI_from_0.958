#ifndef _EXPR_H
#define _EXPR_H

#include "misc.h"
#include "list.h"

enum Exprs {
   _Idexpr, _Typedefexpr, _Structexpr, _Typeexpr, _Varexpr
   } ;

struct _Expr {

   enum Exprs type;

   union _info {
      struct _Id_Expr {
         Identifier name;
         int count;
      } id;

      struct _Var_Expr {
         struct _Expr *id;
         struct _Expr *type;
      } var;

      struct _Struct_Expr {
         Identifier name;
         List defs;
      } strct;

      struct _Type_Expr {
         Type sort;
         Identifier name;
         struct _Expr *type;
      } def;
   } info;
} ;

typedef struct _Expr *Expression;

extern Expression
  mkid(Identifier, int),
  mktypedef(Expression),
  mkstruct(Identifier, List),
  mkntype(int, Identifier, Expression),
  mkvar(Expression, Expression);

extern void printdefs(List);

#define EXPR(pointer)  VAL(Expression, pointer)

#endif
