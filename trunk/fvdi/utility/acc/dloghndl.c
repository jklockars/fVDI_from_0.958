/*
 * dloghndl.c
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __GNUC__
   #include <aesbind.h>
   #include <vdibind.h>
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

#include "form.h"

#include "wind.h"

#define WIND

#define WF_BEVENT  24

extern char *formtext(int, int);

extern int do_popup(OBJECT *popup, int defobj, OBJECT *parent, int selobj);

extern int win_form_do(Window *wind, int which, int *msg, int x, int y, int button, int kstate, int kreturn, int breturn);
extern int xwind_redraw(Window *wind, GRECT *p, OBJECT *dlog, int object, int (*redraw)(int, GRECT *, OBJECT *, int));
extern int redraw(int handle, GRECT *p, OBJECT *dlog, int object);
extern void event_loop(void);
extern void menu_update(void);

Window *add_window(int handle, int (*handler)(Window *, int, int*, int, int, int, int, int, int));
int remove_window(Window *wind);


/* global variables */

extern int finished;

extern int   ap_id;

extern char dlogtext[];

extern form_def form[];

void set_old(int);

int frm_find(int name)
{
   int i;
   
   for(i = 0; form[i].index != -1; i++)
      if(form[i].index == name)
         return i;
   return 0;
}

void init_strings(void)
{
   int   i, j;
   char  n;
   
   for(i = 0; form[i].index != -1; i++) {
      if (form[i].texts)
         for(j = 0; j < form[i].texts->number * form[i].texts->length; j++) {
            n = form[i].texts->text[j];
            switch(n) {
            case '_':
               n = 0;
               break;
            default:
               break;
            }
            form[i].texts->text[j] = n;
         }
   }
}

void set_switch(OBJECT *tree, int button, int set)
{
   if (set)
      tree[button].ob_state |= SELECTED;
   else
      tree[button].ob_state &= ~SELECTED;
}

void en_switch(OBJECT *tree, int button, int enable)
{
   if (enable)
      tree[button].ob_state &= ~DISABLED;
   else
      tree[button].ob_state |= DISABLED;
}

/*
 * copy a string into a TEDINFO structure.
 */
void set_tedinfo(OBJECT *tree, int obj, char *source)
{
   char *dest;

#ifdef __TURBOC__
   dest = tree[obj].ob_spec.tedinfo->te_ptext;
#else   
   dest = ((TEDINFO *)tree[obj].ob_spec)->te_ptext;
#endif
   strcpy(dest, source);
}

/*
 * copy the string from a TEDINFO into another string
 */
void get_tedinfo(OBJECT *tree, int obj, char *dest)
{
   char *source;

#ifdef __TURBOC__
   source = tree[obj].ob_spec.tedinfo->te_ptext;
#else   
   source = ((TEDINFO *)tree[obj].ob_spec)->te_ptext;   /* extract address */
#endif
   strcpy(dest, source);
}

void set_button(OBJECT *tree, int parent, int button)
{
   int b;
   
   for (b = tree[parent].ob_head; b != parent; b = tree[b].ob_next)
      if (b == button)
         tree[b].ob_state |= SELECTED;
      else
         tree[b].ob_state &= ~SELECTED;
}

int get_button(OBJECT *tree, int parent)
{
   int b;

   b = tree[parent].ob_head;
   for (; b != parent && !(tree[b].ob_state & SELECTED); b = tree[b].ob_next)
      ;

   return b;
}

int get_switch(OBJECT *tree, int button)
{
   return ((tree[button].ob_state & SELECTED) != 0);
}

void set_old(int dialog)
{
   int i;
   OBJECT *dlog, *popup;
   char buf[40];
   
   rsrc_gaddr(R_TREE, form[dialog].index, &dlog);

   if (form[dialog].texts)   
      for(i = 0; i < form[dialog].texts->number; i++) {
         set_tedinfo(dlog, form[dialog].texts->object[i],
                     formtext(form[dialog].index, i));
      }
      
   if(form[dialog].radios)
      for(i = 0; i < form[dialog].radios->number; i++) {
         set_button(dlog, form[dialog].radios->object[i],
                    form[dialog].radios->which[i]);
         form[dialog].radios->twhich[i] = form[dialog].radios->which[i];
      }
      
   if (form[dialog].switches)
      for(i = 0; i < form[dialog].switches->number; i++) {
         set_switch(dlog, form[dialog].switches->button[i],
                    *(form[dialog].switches->variable[i]));
      }

   if (form[dialog].popups)
      for(i = 0; i < form[dialog].popups->number; i++) {
         rsrc_gaddr(R_TREE, form[dialog].popups->pop[i], &popup);
         form[dialog].popups->twhich[i] = form[dialog].popups->which[i];
         get_tedinfo(popup, form[dialog].popups->which[i], buf);
         set_tedinfo(dlog, form[dialog].popups->header[i], buf);
      }
}

void get_new(int dialog)
{
   int  i;
   OBJECT   *dlog;

   rsrc_gaddr(R_TREE, form[dialog].index, &dlog);

   if (form[dialog].texts)   
      for(i = 0; i < form[dialog].texts->number; i++) {
         get_tedinfo(dlog, form[dialog].texts->object[i],
                     formtext(form[dialog].index, i));
      }
   
   if (form[dialog].radios)
      for(i = 0; i < form[dialog].radios->number; i++) {
         form[dialog].radios->which[i] = get_button(dlog, form[dialog].radios->object[i]);
      }
      
   if (form[dialog].switches)
      for(i = 0; i < form[dialog].switches->number; i++) {
         *(form[dialog].switches->variable[i]) = get_switch(dlog, form[dialog].switches->button[i]);
      }

   if (form[dialog].popups)
      for(i = 0; i < form[dialog].popups->number; i++) {
         form[dialog].popups->which[i] = form[dialog].popups->twhich[i];
      }
}

static int try_callback(int dialog, int button)
{
   int   i, found;
   OBJECT   *dlog;

   if (!form[dialog].callbacks)
      return 0;

   found = 0;
   for(i = 0; i < form[dialog].callbacks->number; i++) {
      if (form[dialog].callbacks->button[i] == button) {
         found = button;
         break;
      }
   }
   if (!found)
      return 0;

   if ((form[dialog].callbacks->flags[i] & 0x01) == 0) {
      rsrc_gaddr(R_TREE, form[dialog].index, &dlog);
      set_switch(dlog, found, 0);
      xwind_redraw(form[dialog].window, NULL, dlog, found, redraw);
   }
   return form[dialog].callbacks->func[i](dialog, found);
}

static int try_radio(int dialog, int button)
{
   int i, found, parent, obj, old;
   OBJECT *dlog;

   if (!form[dialog].radios)
      return 0;

   found = -1;
   obj = -1;
   rsrc_gaddr(R_TREE, form[dialog].index, &dlog);
   for(i = 0; i < form[dialog].radios->number; i++) {
      parent = form[dialog].radios->object[i];
      obj = dlog[parent].ob_head;
      while ((obj != dlog[parent].ob_tail) && (obj != button)) {
         obj = dlog[obj].ob_next;
      }
      if (obj == button) {
         found = i;
         break;
      }
   }
   if (found < 0)
      return 0;

   old = form[dialog].radios->twhich[found];
   if ((obj != old) && !(dlog[button].ob_state & DISABLED)) {
      form[dialog].radios->twhich[found] = obj;
      set_switch(dlog, old, 0);
      set_switch(dlog, obj, 1);
      if (form[dialog].radios->func[found])
         form[dialog].radios->func[found](obj, old);
   }
   return 1;
}

static int try_popups(int dialog, int button)
{
   int n, i, found;
   OBJECT *dlog, *popup;
   char buf[40];

   if (!form[dialog].popups)
      return 0;

   found = 0;
   for(i = 0; i < form[dialog].popups->number; i++) {
      if (form[dialog].popups->header[i] == button) {
         found = button;
         break;
      }
   }
   if (!found)
      return 0;

   rsrc_gaddr(R_TREE, form[dialog].popups->pop[i], &popup);
   rsrc_gaddr(R_TREE, form[dialog].index, &dlog);
   n = do_popup(popup, form[dialog].popups->twhich[i], dlog, form[dialog].popups->header[i]);
   if (n > 0) {
      form[dialog].popups->twhich[i] = n;
      get_tedinfo(popup, n, buf);
      set_tedinfo(dlog, form[dialog].popups->header[i], buf);
   }   
#if 0
   return form[dialog].popups->func[i](found);
#endif
   set_switch(dlog, found, 0);
#if 0
   objc_draw(dlog, found, MAX_DEPTH, 0, 0, 1000, 1000);
#else
   xwind_redraw(form[dialog].window, NULL, dlog, found, redraw);
#endif
   return 1;
}

#if 0
static int try_switches(int dialog, int button)
{
   int   i;
   
   if (!form[dialog].switches)
      return 0;

   for(i = 0; i < form[dialog].switches->number; i++) {
      if (form[dialog].switches->button[i] == button) {
         *(form[dialog].switches->variable[i]) ^= 1;
         printf("Switch: %d", *(form[dialog].switches->variable[i]));
         return 1;
      }
   }
   
   return 0;
}
#endif

int add_xdialog(int dialog, int (*return_func)(int), int gadgets, char *text)
{
   OBJECT *dlog;
   GRECT p;
   int handle;
   Window *wind;
   int editnum = 0;
   int message[8];

   if (form[dialog].window) {
#if 0
      wind_set(form[dialog].window->handle, WF_TOP, 0, 0, 0, 0);
#else
      message[0] = WM_TOPPED;
      message[1] = ap_id;
      message[2] = sizeof(message) - 16;
      message[3] = form[dialog].window->handle;
      appl_write(ap_id, sizeof(message), message);
#endif
      return 0;
   }

   rsrc_gaddr(R_TREE, form[dialog].index, &dlog);

   if ((form[dialog].pos[0] == -1) && (form[dialog].pos[1] == -1) &&
       (form[dialog].pos[2] == -1) && (form[dialog].pos[3] == -1)) {
      form_center(dlog, &p.g_x, &p.g_y, &p.g_w, &p.g_h);

      wind_calc(WC_BORDER, gadgets, p.g_x, p.g_y, p.g_w, p.g_h, &p.g_x, &p.g_y, &p.g_w, &p.g_h);
   } else {
      p.g_x = form[dialog].pos[0];
      p.g_y = form[dialog].pos[1];
      p.g_w = form[dialog].pos[2];
      p.g_h = form[dialog].pos[3];
   }
   if ((handle = wind_create(gadgets, p.g_x, p.g_y, p.g_w, p.g_h)) < 0) {
      form_alert(1, "[1][Too many windows open!][ OK ]");
      return 0;
   }

#if 0   
   wind_title(handle, dlogtext);
#endif
   wind_set(handle, WF_NAME, text);
   wind_set(handle, WF_BEVENT, 0x0001, 0, 0 ,0);
   wind_open(handle, p.g_x, p.g_y, p.g_w, p.g_h);  

   wind = add_window(handle, win_form_do);
   wind->func = return_func;
   (OBJECT *)wind->ptr = dlog;
   wind->tmp[0] = dialog;
   wind->tmp[1] = editnum;

   form[dialog].window = wind;

   set_old(dialog);

   if (form[dialog].init)
      form[dialog].init(dialog);

#if 0
   xwind_redraw(wind, &p, dlog, ROOT, redraw);
#endif

   return 1;
}

int add_dialog(int dialog, int (*return_func)(int))
{
   return add_xdialog(dialog, return_func, NAME | MOVER | CLOSER, dlogtext);
}

int button_func(Window *wind, int but)
{
   int again, dialog, result;
   
   but &= 0x7fff;     /* Mask off double click */
   dialog = wind->tmp[0];

   again = try_callback(dialog, but);
   again = try_radio(dialog, but) || again;
   again = try_popups(dialog, but) || again;
#if 0
   again = try_switches(dialog, but) || again;  /* or else */
#endif

   if (!again) {
      wind_close(wind->handle);
      wind_delete(wind->handle);
      remove_window(wind);
      form[dialog].window = NULL;

      ((OBJECT *)wind->ptr)[but].ob_state &= ~SELECTED;      /* de-select exit button */
      result = but;
      if (form[dialog].fixup)
         if (form[dialog].fixup(dialog))
            result = -but;    /* Notify that menu/dialogs need updating */
      
      if (abs(result) != form[dialog].Cancel) {
         get_new(dialog);
         if (result < 0)
            menu_update();
      } else
         result = 0;

      if (wind->func)
         wind->func(result);     /* Tell 'calling' function what happened */
   } else {
#if 0
      wind->new = 1;
#endif
      wind->tmp[1] = 0;       /* next (editnum) */
      wind->tmp[2] = 1;       /* cont */
   }
   
   return 1;
}

int do_return(int dummy)
{
   finished = 1;    /* Just to make event_loop quit */
   return 1;
}

int do_dialog(int dialog)
{
   int fin;

   fin = finished;
   if (!add_xdialog(dialog, do_return, NAME | MOVER, dlogtext))
      return 0;
   event_loop();
   finished = fin;

   return 1;
}
