/*
 * wind.c - Top level window handling
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __GNUC__
   #include <aesbind.h>
   #include <vdibind.h>
   #include <support.h>       /* No ltoa otherwise! */
   #define ltoa _ltoa
#else
   #include <aes.h>
   #include <vdi.h>
#endif
#ifndef __GNUC__
   #include <tos.h>
#else
   #include <osbind.h>
#endif

#include "fvdiacc.h"

#include "gem.h"
#include "wind.h"

#define BKGND

extern void handle_menu(int, int);

extern int check_events(int);

extern void fixup(int, int);

extern Window window[];

extern no_shorts;
extern short_t shortcut[];

extern char last_loaded[];

extern int v_x_max;
extern int v_y_max;

extern bkgndinput;
extern finished;
extern Menu menu;

#ifndef __GNUC__
KEYTAB *keys;
#else
_KEYTAB *keys;
#endif

static int win_count = 0;
static Window *top = NULL;
static Window *unused = NULL;

void init_windows(void)
{
   int i;
   
#if 0
   for(i = 0; i < MAX_WINDOWS; i++)
      window[i].handle = -1;
#else
   for(i = 0; i < MAX_WINDOWS; i++) {
      window[i].next = unused;
      unused = &window[i];
   }
#endif
}

Window *find_window(int handle, int x, int y)
{
   Window *current;
   
   if (!handle)
      if (!(handle = wind_find(x, y)))
         return NULL;
   current = top;
   while (current) {
      if (current->handle == handle) {
         return current;
      }
      current = current->next;
   }

   return NULL;
}

Window *add_window(int handle, int (*handler)(Window *, int, int*, int, int, int, int, int, int))
{
   Window *new;

   if (new = unused) {
      unused = new->next;
      new->next = top;
      top = new;
      new->handle = handle;
      new->handler = handler;
      new->new = 1;
      win_count++;
      return new;
   }
   return NULL;
}

int unlink_window(Window *wind)
{
   Window *current;

   current = top;
   if (current == wind) {
      top = current->next;
      return 1;
   } else {
      while (current) {
         if (current->next == wind) {
            current->next = wind->next;
            return 1;
         }
         current = current->next;
      }
   }
   return 0;
}

int remove_window(Window *wind)
{
   win_count--;
   if (unlink_window(wind)) {
      wind->next = unused;
      unused = wind;
      return 1;
   }
   return 0;
}

int top_window(Window *wind)
{
   if (unlink_window(wind)) {
      wind->next = top;
      top = wind;
      return 1;
   }
   return 0;
}

int bottom_window(Window *wind)
{
   Window *current;
   
   if (unlink_window(wind)) {
      wind->next = NULL;
      if (!top)
         top = wind;
      else {
         current = top;
         while (current->next)
            current = current->next;
         current->next = wind;
      }
      return 1;
   }
   return 0;
}

int event_loop(void)
{
   check_events(0);

   return 1;
}

void no_more(void)
{
}

int check_events(int quick)
{
   int x, y, kstate, button, kreturn, breturn;
   int which, msg[8];
   Window *wind;
   int events, finish;
   
   events = MU_KEYBD | MU_BUTTON | MU_MESAG;
   if (quick)
      events |= MU_TIMER;

   finish = 0;
   while (!finished && !finish) {
      which = evnt_multi(events,
                         1, 1, 1,       /* mouse */
                         0, 0, 0, 0, 0, /* rectangle 1 */
                         0, 0, 0, 0, 0, /* rectangle 2 */
                         msg,           /* message buffer */
#ifndef __GNUC__
                         0, 0,          /* return immediately */
#else
                         0L,            /* return immediately */
#endif
                         &x, &y,
                         &button, &kstate,
                         &kreturn, &breturn);
      if (which & MU_KEYBD) {
         if (bkgndinput) {
            if (!(wind = find_window(0, x, y)))
               wind = top;
         } else
            wind = top;
         if (wind && (kstate < 4)) {
            wind->handler(wind, which & MU_KEYBD, msg, x, y, button, kstate, kreturn, breturn);
         }
      }
      if (which & MU_BUTTON) {
         wind = find_window(0, x, y);
#ifndef BKGND
         if (wind && (wind == top))
#else
         if (wind)
#endif
            wind->handler(wind, which & MU_BUTTON, msg, x, y, button, kstate, kreturn, breturn);
      }
      if (which & MU_MESAG) {
         switch (msg[0]) {
         case VA_START:
            break;

#ifndef NOT_A_DA
         case AC_OPEN:
            add_dialog(frm_find(FRM_MAIN), no_more);
            break;

         case AC_CLOSE:
            break;
#endif

         case MN_SELECTED:
            break;

         case WM_TOPPED:
            if (wind = find_window(msg[3], 0, 0)) {
               top_window(wind);
               wind_set(msg[3], WF_TOP, msg[3]);
            }
            break;

         case WM_M_BDROPPED:
         case WM_BOTTOMED:
            if (wind = find_window(msg[3], 0, 0)) {
               bottom_window(wind);
               wind_set(msg[3], WF_BOTTOM, 0, 0, 0, 0);
            }
            break;

         case WM_REDRAW:
#if 0
            if (!top)
               top = wind;
#endif
         case WM_FULLED:
         case WM_CLOSED:
         case WM_ARROWED:
         case WM_SIZED:
         case WM_MOVED:
         case WM_HSLID:
         case WM_VSLID:
            if (wind = find_window(msg[3], 0, 0)) {
               wind->handler(wind, which & MU_MESAG, msg, x, y, button, kstate, kreturn, breturn);
            }
            break;
         default:
            break;
         }
      }
      if (which & MU_TIMER) {
         if (quick)
            finish = 1;
      } 
   }
   return 1;
}
