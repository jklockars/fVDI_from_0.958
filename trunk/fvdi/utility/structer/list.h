#ifndef _LIST_H
#define _LIST_H
/*
 * List declarations
 *
 * $Id: list.h,v 1.2 2002-05-13 01:25:24 johan Exp $
 *
 * Copyright 1997-2002, Johan Klockars
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "misc.h"

struct _Voidelem {
  void *element;
  struct _Voidelem *next;
} ;

struct _Voidlist {
  Listtype listtype;
  struct _Voidelem *first;
  struct _Voidelem *last;
} ;

typedef struct _Voidlist *Voidlist;
typedef struct _Voidelem *Voidelem;

typedef struct _Voidlist List_str;
typedef Voidlist List;
typedef Voidelem Listelement;

extern Voidlist
  empty(Listtype),
  single(Listtype, void *),
  append(void *, Voidlist),
  prepend(void *, Voidlist),
  remove_item(Voidelem, Voidlist),
  copy_list(Voidelem, Voidlist);

extern void
  destroy(Voidlist),
  tail_nd(Voidlist),
  tail(Voidlist, void (*)(void *));

extern int count_list(Listelement);

#define TYPE(list)         ((list)->listtype)
#define FIRST(list)        ((list)->first)
#define LAST(list)         ((list)->last)
#define NEXT(pointer)      (pointer) = (pointer)->next
#define ELEM(pointer)      ((pointer)->element)
#define VAL(type,pointer)  ((type)ELEM(pointer))

extern List definitions;

#endif
