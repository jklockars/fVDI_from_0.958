#ifndef _EXPR_H
#define _EXPR_H
/*
 * C structure expression declarations
 *
 * $Id: expr.h,v 1.2 2002-05-13 01:28:11 johan Exp $
 *
 * Copyright 1997-2002, Johan Klockars
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "misc.h"
#include "list.h"

enum Exprs {
   _Idexpr, _Typedefexpr, _Structexpr, _Unionexpr, _Typeexpr, _Varexpr
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

      struct _Union_Expr {
         Identifier name;
         List defs;
      } unjon;

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
  mkunion(Identifier, List),
  mktype(int, Identifier, Expression),
  mkvar(Expression, Expression);

extern void printdefs(List);

#define EXPR(pointer)  VAL(Expression, pointer)

#endif
