/*
 * List functions
 *
 * $Id: list.c,v 1.2 2002-05-13 01:25:24 johan Exp $
 *
 * Copyright 1997-2002, Johan Klockars
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "list.h"
#include "memory.h"

#include <stdlib.h>
#include <stdio.h>


static Voidlist new(void)
{
   Voidlist voidhead;

   voidhead = (Voidlist)memalloc(sizeof(struct _Voidlist));
   if (!voidhead) {
      printf("Could not allocate list head!\n");
      exit(1);
   }
   return voidhead;
}


static Voidelem new_elem(void)
{
   Voidelem voidcell;

   voidcell = (Voidelem)memalloc(sizeof(struct _Voidelem));
   if (!voidcell) {
      printf("Could not allocate list cell!\n");
      exit(1);
   }
   return voidcell;
}


Voidlist empty(Listtype listtype)
{
   Voidlist voidhead = new();

#ifdef TEST
   printf("Create empty list (%d)\n", (int)listtype);
#endif
   voidhead->listtype = listtype;
   voidhead->first = NULL;
   voidhead->last = NULL;
   return voidhead;
}


Voidlist single(Listtype listtype, void *element)
{
   Voidlist voidhead = empty(listtype);
   Voidelem voidcell = new_elem();

#ifdef TEST
   printf("Create single element list (%d)\n", (int)listtype);
#endif
   voidhead->first = voidcell;
   voidhead->last = voidcell;
   voidcell->element = element;
   voidcell->next = NULL;
   return voidhead;
}


Voidlist append(void *element, Voidlist voidlist)
{
   Voidelem voidcell = new_elem();

#ifdef TEST
   printf("Add element to end of list\n");
#endif
   voidcell->element = element;
   voidcell->next = NULL;
   if (voidlist->last) {
      voidlist->last->next = voidcell;
      voidlist->last = voidcell;
   } else {
      voidlist->first = voidcell;
      voidlist->last = voidcell;
   }
   return voidlist;
}


Voidlist prepend(void *element, Voidlist voidlist)
{
   Voidelem voidcell = new_elem();

#ifdef TEST
   printf("Add element to start of list\n");
#endif
   voidcell->element = element;
   voidcell->next = voidlist->first;
   voidlist->first = voidcell;
   if (!voidlist->last) {
      voidlist->last = voidcell;
   }
   return voidlist;
}


Voidlist remove_item(Voidelem element, Voidlist voidlist)
{
   Voidelem ptr, last;
   
   ptr = FIRST(voidlist);
   if (ptr == element) {
      voidlist->first = ptr->next;
      last = NULL;
   } else {
      do {
	 last = ptr;
	 ptr = ptr->next;
      } while (ptr && (ptr != element));
      if (!ptr) {
	 printf("Couldn't find element in list!\n");
	 exit(-1);
      }
      last->next = ptr->next;
   }
   if (ptr == voidlist->last)
      voidlist->last = last;
   return voidlist;
}


Voidlist copy_list(Voidelem from, Voidlist oldlist)
{
   Voidlist copylist = empty(oldlist->listtype);
   Voidelem voidcell;

#ifdef TEST
   printf("Create a list copy\n");
#endif

   voidcell = oldlist->first;
   while (voidcell) {
      append(voidcell->element, copylist);
      voidcell = voidcell->next;
   }
   
   return copylist;
}


#if 0
Voidlist tail(Voidlist oldlist)
{
   Voidlist taillist = empty(oldlist->listtype);
   Voidelem voidcell;

#ifdef TEST
   printf("Create a tail-list\n");
#endif

   if (oldlist->first == NULL) {
      printf("Tried to make tail of an empty list.\n");
      exit(-1);
   }

   return copy_list(oldlist->first->next, oldlist);
}
#else
/* Non-destructive 'removal' of first element */
void tail_nd(Voidlist oldlist)
{
   if (oldlist->first == NULL) {
      printf("Tried to make tail of an empty list.\n");
      exit(-1);
   } else
      oldlist->first = oldlist->first->next;
}
#endif

/* Destructive removal of first element via 'destructor' */
void tail(Voidlist oldlist, void (*destruct)(void *))
{
   Voidelem first;
   
   if (oldlist->first == NULL) {
      printf("Tried to make tail of an empty list.\n");
      exit(-1);
   } else {
      first = oldlist->first;
      destruct(first->element);
      oldlist->first = first->next;
      memfree(first);
   }
}


void destroy(Voidlist voidlist)
{
   Voidelem voidcell, tmp;

#ifdef TEST
   printf("Remove a list structure (not the contents).\n");
#endif

   voidcell = voidlist->first;
   while (voidcell) {
      tmp = voidcell->next;
      memfree(voidcell);
      voidcell = tmp;
   }
   memfree(voidlist);
}


int count_list(Listelement ptr)
{
   int n;
   
   n = 0;
   while(ptr) {
      n++;
      ptr = ptr->next;
   }
   return n;
}
