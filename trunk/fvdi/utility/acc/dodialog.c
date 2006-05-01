/*
 * dialog.c - Handle dialog windows
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __GNUC__
 #if defined(NEW_GEMLIB)
   #include <gem.h>
 #else
   #include <aesbind.h>
   #include <vdibind.h>
 #endif
   #include <support.h>       /* No ltoa otherwise! */
   #define ltoa _ltoa
#else
   #include <aes.h>
   #include <vdi.h>
#endif
#ifdef __PUREC__
   #include <tos.h>
#else
   #include <osbind.h>
#endif

#include "fvdiacc.h"

#include "gem.h"
#include "form.h"
#include "wind.h"

#define MAX(x,y)  (((x) > (y)) ? (x) : (y))
#define MIN(x,y)  (((x) < (y)) ? (x) : (y))

extern form_def form[];

extern int vdi_handle1;

extern int ap_id;

extern int button_func(Window *wind, int but);
extern int xobjc_edit(Window *wind, int (*redraw)(int, GRECT *, OBJECT *, int), OBJECT *tree, int ed_obj, int keycode, short *curpos, int kind);
extern void remove_window(Window *);

int xwind_redraw(Window *wind, GRECT *p, OBJECT *dlog, int object, int (*redraw)(int, GRECT *, OBJECT *, int));


void clear_radios(Window *wind, int (*redraw)(int, GRECT *, OBJECT *, int), OBJECT *tree, int start)
{
   int next, current;

   current = start;
   next = tree[current].ob_next;
   while(tree[next].ob_tail != current) {    /* Check all later siblings */
      current = next;
      if (tree[current].ob_state & SELECTED) {
         tree[current].ob_state &= ~SELECTED;
         xwind_redraw(wind, NULL, tree, current, redraw);               
      }
      next = tree[current].ob_next;
   }
   next = tree[next].ob_head;
   while(next != start) {    /* Check all earlier siblings */
      current = next;
      if (tree[current].ob_state & SELECTED) {
         tree[current].ob_state &= ~SELECTED;
         xwind_redraw(wind, NULL, tree, current, redraw);               
      }
      next = tree[current].ob_next;
   }
}

int xform_button(Window *wind, int (*redraw)(int, GRECT *, OBJECT *, int), OBJECT *tree, int obj, int breturn, short *nextob)
{
   short x, y;
   int was_set, new_state, ret;
   int which;
   short msg[8], button, kstate, kreturn, breturn_;
   int outside;
   short bx, by;
   int bw, bh;
   int thickness;

   ret = 1;
   *nextob = 0;

   if (tree[obj].ob_state & DISABLED)
      return -1;                         /* -1 = 'Nothing' clicked, handled as 1 */
   if (!(tree[obj].ob_flags & (RBUTTON | SELECTABLE | TOUCHEXIT | EXIT | EDITABLE)))
      return -1;

   thickness = 0;
   switch(tree[obj].ob_type & 0x00ff) {
   case G_BOX:
   case G_IBOX:
   case G_BOXCHAR:
#ifdef __GNUC__
 #if defined(NEW_GEMLIB)
      thickness = tree[obj].ob_spec.obspec.framesize;
 #else
      thickness = (tree[obj].ob_spec >> 16) & 0x00ff;
 #endif
#else
      thickness = (int)((long)tree[obj].ob_spec >> 16) & 0x00ff;
#endif
      thickness = (thickness >= 128) ? thickness - 256 : thickness;
      break;
   case G_BUTTON:
      if (tree[obj].ob_flags & DEFAULT)
         thickness -= 3;
      else
         thickness = -2;
      break;
   case G_STRING:
   case G_TITLE:
      thickness = 0;
      break;
   case G_TEXT:
   case G_BOXTEXT:
   case G_FTEXT:
   case G_FBOXTEXT:
#if defined(__GNUC__) && defined(NEW_GEMLIB)
      thickness = tree[obj].ob_spec.tedinfo->te_thickness;
#else
      thickness = ((TEDINFO *)tree[obj].ob_spec)->te_thickness;
#endif
      break;
   }
   if (tree[obj].ob_state & OUTLINED)
      thickness = -3;
   if ((tree[obj].ob_flags & 0x0600) && ((tree[obj].ob_flags & 0x0600) != 0x0400))
      thickness -= 2;   /* 3D (standard is probably 2) */
   thickness = (thickness < 0) ? -thickness : 0;

   was_set = 0;
   if (tree[obj].ob_flags & RBUTTON) {
      if (!(tree[obj].ob_state & SELECTED)) {
         tree[obj].ob_state |= SELECTED;
         xwind_redraw(wind, NULL, tree, obj, redraw);
      }
   } else if (tree[obj].ob_flags & SELECTABLE) {
      was_set = tree[obj].ob_state & SELECTED;
      tree[obj].ob_state ^= SELECTED;
      xwind_redraw(wind, NULL, tree, obj, redraw);
   }

   if (tree[obj].ob_flags & RBUTTON) {
      clear_radios(wind, redraw, tree, obj);
   } else if (tree[obj].ob_flags & SELECTABLE) {
      outside = 1;
      objc_offset(tree, obj, &bx, &by);
      bx -= thickness;
      by -= thickness;
      bw = tree[obj].ob_width + 2 * thickness;
      bh = tree[obj].ob_height + 2 * thickness;
      do {
#if defined(__GNUC__)
         which = evnt_multi(MU_BUTTON | MU_M1,
                            1, 1, 0,       /* mouse */
                            outside, bx, by, bw, bh,  /* rectangle 1 */
                            0, 0, 0, 0, 0, /* rectangle 2 */
                            msg,           /* message buffer */
                            0L,            /* return immediately */
                            &x, &y,
                            &button, &kstate,
                            &kreturn, &breturn_);
#else
         which = evnt_multi(MU_BUTTON | MU_M1,
                            1, 1, 0,       /* mouse */
                            outside, bx, by, bw, bh,  /* rectangle 1 */
                            0, 0, 0, 0, 0, /* rectangle 2 */
                            msg,           /* message buffer */
                            0, 0,          /* return immediately */
                            &x, &y,
                            &button, &kstate,
                            &kreturn, &breturn_);
#endif

         if (which & MU_M1) {
            new_state = tree[obj].ob_state & ~SELECTED;
            if (!outside)
               new_state |= was_set ^ SELECTED;
            else
               new_state |= was_set;
            if (tree[obj].ob_state != new_state) {
               tree[obj].ob_state = new_state;
               xwind_redraw(wind, NULL, tree, obj, redraw);
            }
            outside ^= 1;
         }
      } while(!(which & MU_BUTTON));
   }

   if (tree[obj].ob_flags & TOUCHEXIT) {
      *nextob = obj;
      ret = 0;
   } else if (tree[obj].ob_flags & EXIT) {
      if (tree[obj].ob_state & SELECTED) {
         *nextob = obj;
         ret = 0;
      }
   } else if (tree[obj].ob_flags & EDITABLE) {
      *nextob = obj;
   }

   return ret;
}


int find_object(OBJECT *tree, int current, int flags, int *types)
{
   int next, start;
   int i, found;

   start = current;     /* We've already seen 'current' */
   if (current == -1)   /* unless top start flag was supplied */
      current = 0;
   do {
      if ((tree[current].ob_flags & flags) && (current != start)) {
         if (types) {
            found = 0;
            for(i = 0; types[i] != -1; i++) {
               if (tree[current].ob_type == types[i]) {
                  found = 1;
                  break;
               }
            }
         } else
            found = 1;
         
         if (found) { 
            return current;
         }
      }
		
      if (tree[current].ob_head != -1) {     /* Any children? */
         current = tree[current].ob_head;
      } else {
         next = tree[current].ob_next;       /* Try for a sibling */

         while((next != -1) && (tree[next].ob_tail == current)) {    /* Trace back up tree if no more siblings */
            current = next;
            next = tree[current].ob_next;
         }
         current = next;
      }
   } while(current != -1);    /* If 'current' is -1 then we have finished */

   return -1;   /* Bummer - didn't find the object, so return error */
}


int xrc_intersect(const GRECT *rect1, GRECT *rect2)
{
   int x, y, w, h;
   
   x = MAX(rect1->g_x, rect2->g_x);
   y = MAX(rect1->g_y, rect2->g_y);
   w = MIN(rect1->g_x + rect1->g_w, rect2->g_x + rect2->g_w) - x;
   h = MIN(rect1->g_y + rect1->g_h, rect2->g_y + rect2->g_h) - y;
   rect2->g_x = x;
   rect2->g_y = y;
   rect2->g_w = w;
   rect2->g_h = h;
   if ((w > 0) && (h > 0))
      return 1;
   else
      return 0;
}

int xobjc_draw(Window *wind, GRECT *p, OBJECT *dlog, int object, int (*redraw)(int, GRECT *, OBJECT *, int))
{
   GRECT area;
   int ok = 1;
   int handle;
   
   handle = wind->handle;

   wind_get(handle, WF_FIRSTXYWH, &area.g_x, &area.g_y, &area.g_w, &area.g_h);
   while(area.g_w && area.g_h) {
      if (!p || xrc_intersect(p, &area)) {
         if (!(ok = redraw(handle, &area, dlog, object)))
            break;
      }
      wind_get(handle, WF_NEXTXYWH, &area.g_x, &area.g_y, &area.g_w, &area.g_h);
   }

   return ok;
}

int xcursor(int handle, GRECT *p, OBJECT *dlog, int object)
{
   short rect[4];

   rect[0] = p->g_x;
   rect[1] = p->g_y;
   rect[2] = p->g_x + p->g_w - 1;
   rect[3] = p->g_y + p->g_h - 1;
   vr_recfl(vdi_handle1, rect);
   return 1;
}

void calc_curpos(Window *wind, OBJECT *dlog, int edit, GRECT *p)
{
   int i, pos, dpos, len;
   int w, h;
   char *tmp;
   
#if defined(__GNUC__) && defined(NEW_GEMLIB)
   i = dlog[edit].ob_spec.tedinfo->te_tmplen;
#else
   i = ((TEDINFO *)dlog[edit].ob_spec)->te_tmplen;
#endif
   pos = -1;
#if defined(__GNUC__) && defined(NEW_GEMLIB)
   tmp = dlog[edit].ob_spec.tedinfo->te_ptmplt;
#else
   tmp = ((TEDINFO *)dlog[edit].ob_spec)->te_ptmplt;
#endif
   len = (int)strlen(tmp);
   for(; i >= 0; i--) {
      pos++;
      dpos = 0;
      while (*tmp && *tmp++ != '_')
         dpos++;
      if (*tmp)               /* Don't end up after trailing text */
         pos += dpos;
   }
   objc_offset(dlog, edit, &p->g_x, &p->g_y);
   w = dlog[edit].ob_width;
   h = dlog[edit].ob_height;
#if defined(__GNUC__) && defined(NEW_GEMLIB)
   switch (dlog[edit].ob_spec.tedinfo->te_just) {
#else
   switch (((TEDINFO *)dlog[edit].ob_spec)->te_just) {
#endif
   case 0:
      p->g_x = p->g_x + pos * 8;
      break;
   case 1:
      p->g_x = p->g_x + w - (len - pos) * 8;
      break;
   case 2:
      p->g_x = p->g_x + (w - len * 8) / 2 + pos * 8;
      break;
   }
   p->g_w = 1;
   p->g_h = h;
}

int safe_xwind_redraw(Window *wind, GRECT *p, OBJECT *dlog, int object, int (*redraw)(int, GRECT *, OBJECT *, int))
{
   short junk;
   int was_edit;

   was_edit = dlog[wind->tmp[3]].ob_state & IS_EDIT;
   if (was_edit)
      xobjc_edit(wind, redraw, dlog, wind->tmp[3], 0, 0, EDEND);
   xwind_redraw(wind, p, dlog, object, redraw);
   if (was_edit)
      xobjc_edit(wind, redraw, dlog, wind->tmp[3], 0, &junk, EDINIT);
   
   return 1;
}

int xwind_redraw(Window *wind, GRECT *p, OBJECT *dlog, int object, int (*redraw)(int, GRECT *, OBJECT *, int))
{
   GRECT cp;
   int ok = 1;
   short *edit;
   int cursor_on;
   
   edit = &wind->tmp[3];

   if (!redraw || (*edit && (dlog[*edit].ob_state & IS_EDIT))) {
      calc_curpos(wind, dlog, *edit, &cp);
      if (redraw && p)
         xrc_intersect(p, &cp);
      cursor_on = 1;
   } else
      cursor_on = 0;
   
   wind_update(BEG_UPDATE);
   graf_mouse(M_OFF, 0);

   if (redraw) {
      if (cursor_on)
         xobjc_draw(wind, &cp, dlog, *edit, xcursor);
      xobjc_draw(wind, p, dlog, object, redraw);
      if (cursor_on) {
#if 0
         ((TEDINFO *)dlog[edit].ob_spec)->te_tmplen = strlen(((TEDINFO *)dlog[edit].ob_spec)->te_ptext);
         calc_curpos(wind, dlog, *edit, &cp);
         if (p)
            xrc_intersect(p, &cp);
#endif
         xobjc_draw(wind, &cp, dlog, *edit, xcursor);
      }
   } else
      xobjc_draw(wind, &cp, dlog, *edit, xcursor);

   graf_mouse(M_ON, 0);
   wind_update(END_UPDATE);
   return ok;
}

int redraw(int handle, GRECT *p, OBJECT *dlog, int object)
{
   objc_draw(dlog, object, MAX_DEPTH, p->g_x, p->g_y, p->g_w, p->g_h);
   return 1;
}

int win_form_do(Window *wind, int which, short *msg, int x, int y, int button, int kstate, short kreturn, int breturn)
{
   short *next, *edit, *cont, *idx, *dialog;
   OBJECT *dlog;
   short junk;
   int tmp;
   short message[8];
   
   dlog = (OBJECT *)wind->ptr;
   dialog = &wind->tmp[0];
   next = &wind->tmp[1];
   cont = &wind->tmp[2];
   edit = &wind->tmp[3];
   idx  = &wind->tmp[4];

   if (wind->new) {
      wind->new = 0;
      *cont = 1;
      *edit = 0;
      if (!*next)
         if ((*next = find_object(dlog, -1, EDITABLE, NULL)) == -1)
            *next = 0;
   }

   if (which & MU_KEYBD) {
      *cont = form_keybd(dlog, *edit, 0, kreturn, &*next, &kreturn);
      if (kreturn && *edit)
         xobjc_edit(wind, redraw, dlog, *edit, kreturn, &*idx, EDCHAR);
   }
   if (which & MU_BUTTON) {
      *next = objc_find(dlog, ROOT, MAX_DEPTH, x, y);
      if (*next == -1) {
         Bconout(2, '\a');
         *next = 0;
      } else {
         *cont = xform_button(wind, redraw, dlog, *next, breturn, &*next);
         if (*cont == -1) {        /* Special 'nothing clicked' marker */
            *cont = 1;
            message[0] = WM_TOPPED;
            message[1] = ap_id;
            message[2] = sizeof(message) - 16;
            message[3] = wind->handle;
            appl_write(ap_id, sizeof(message), message);
         }
      }
   }
   if (which & MU_MESAG) {
      switch (msg[0]) {
#if 0
      case WM_TOPPED:
         wind_set(msg[3], WF_TOP, msg[3]);
         break;
#endif

      case WM_REDRAW:
         xwind_redraw(wind, (GRECT *)&msg[4], dlog, ROOT, redraw);
         break;

      case WM_MOVED:
         wind_set(wind->handle, WF_CURRXYWH, msg[4], msg[5], msg[6], msg[7]);
         wind_calc(WC_WORK, NAME | MOVER, msg[4], msg[5], msg[6], msg[7],
                   &dlog[ROOT].ob_x, &dlog[ROOT].ob_y, &junk, &junk);
         dlog[ROOT].ob_x += 3;
         dlog[ROOT].ob_y += 3;
         form[*dialog].pos[0] = msg[4];
         form[*dialog].pos[1] = msg[5];
         form[*dialog].pos[2] = msg[6];
         form[*dialog].pos[3] = msg[7];
         break;

      case WM_CLOSED:
         wind_close(wind->handle);
         wind_delete(wind->handle);
         remove_window(wind);
         form[*dialog].window = NULL;
         break;
      }
   }
      
   if (!*cont) {
      *cont = 1;
      tmp = *next;
      *next = 0;
      button_func(wind, tmp);
   } else if ((*next != 0) && (*edit != *next)) {
      if (*edit)
         xobjc_edit(wind, redraw, dlog, *edit, 0, &*idx, EDEND);
      *edit = *next;
      *next = 0;
      xobjc_edit(wind, redraw, dlog, *edit, 0, &*idx, EDINIT);
   }
   
   return 1;
}
