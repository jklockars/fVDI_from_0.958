#ifdef __GNUC__
   #include <aesbind.h>
   #include <vdibind.h>
   #include <support.h>       /* No ltoa otherwise! */
   #define ltoa _ltoa
#else
   #ifdef LATTICE
      #define ltoa(a,b,c)   stcl_d(b, (long)a)
   #endif
   #include <aes.h>
   #include <vdi.h>
#endif
#ifdef __PUREC__
   #include <tos.h>
#else
   #include <osbind.h>
#endif


extern int popfix;


/* popup = address of the popup dialog & number of 'default object'. */
/* parent & selobj = from which object popup menu is to be 'hanged'. */
/* NOTICE: popup.defobj is aligned with parent.selobj! */
/* Popup may contain any objects (text, images etc.) */
/* Returns selected object number or NO_OBJECT. */

/* The 'popup box' should not have a border as that will leave some */
/* traces to screen when redrawn. */

int do_popup(OBJECT *popup, int defobj, OBJECT *parent, int selobj)
{
   OBJECT *item;
   int  x, y, w, h, status;
   int  evnt_m, pmx, pmy, check = 1, i, m_state = 0;
   int  mbuf[8];
   int  deskx, desky, deskw, deskh;
   int  border, pborder, xbox, ybox, wbox, hbox;
   int  events, newobj, start = 0, button;
   long timer;

   events = MU_BUTTON | MU_M1;
   timer = 0;
   if (popfix) {
      events |= MU_TIMER;
      timer = 4;
   }
   
   /* get desktop work area */
   wind_get(0, WF_WORKXYWH, &deskx, &desky, &deskw, &deskh);

   /* popup size & place */
   form_center(popup, &x, &y, &w, &h);
   objc_offset(parent, selobj, &x, &y);

   /* NO OBJECT -> popup below & fit into desktop window */
   if (defobj == -1)
   {
      defobj = 0;
      y += parent[selobj].ob_height;
      x = min(max(deskx, x), deskx + deskw - w);
      y = min(max(desky, y), desky + deskh - h);
   } else {
      x = min(max(deskx, x) - popup[defobj].ob_x, deskx + deskw - w);
      y = min(max(desky, y) - popup[defobj].ob_y, desky + deskh - h);
   }
   popup->ob_x = x;
   popup->ob_y = y;

#ifdef __GNUC__
   border = (int)(signed char)((popup[ROOT].ob_spec >> 16) & 0xff);
   pborder = 2 * (int)(signed char)((parent[ROOT].ob_spec >> 16) & 0xff);
#else
 #ifdef __PUREC__
   border = (int)(popup[ROOT].ob_spec.obspec.framesize);
   pborder = 2 * (int)(parent[ROOT].ob_spec.obspec.framesize);
 #else
   border = (int)(signed char)(((long)popup[ROOT].ob_spec >> 16) & 0xff);
   pborder = 2 * (int)(signed char)(((long)parent[ROOT].ob_spec >> 16) & 0xff);
 #endif
#endif

#if 1
   evnt_button(1, 1, 0, &i, &i, &i, &i);		/* until button up */
   m_state = 1;
   start = 1;
#endif

   /* select current popup item */
   item = &popup[defobj];
   if (defobj && (item->ob_flags & SELECTABLE) && !(item->ob_state & DISABLED))
#if 0
      item->ob_state ^= SELECTED;
#else
      item->ob_state |= SELECTED;
#endif

   /* draw popup */
#if 0
   wind_update(BEG_UPDATE);
#endif
   
   xbox = x + border;
   ybox = y + border;
   if (border > 0) {
      wbox = w;
      hbox = h;
   } else {
      wbox = w - 2 * border;
      hbox = h - 2 * border;
   }
#if 0
   form_dial(FMD_START, xbox, ybox, wbox, hbox, xbox, ybox, wbox, hbox);
#endif

   objc_draw(popup, R_TREE, MAX_DEPTH, deskx, desky, deskw, deskh);

   status = item->ob_state;
   do {
      /* wait until mouse button released or mouse goes outside popup item */
#if 0
      evnt_m = evnt_multi(MU_BUTTON | MU_M1,
#else
      evnt_m = evnt_multi(events,
#endif
                          1, 1, m_state,
                          check, x + item->ob_x, y + item->ob_y, item->ob_width, item->ob_height,
                          0, 0, 0, 0, 0,
                          mbuf,
#if 0
#ifdef __GNUC__
                          0L,
#else
                          0, 0,
#endif
#else
#ifdef __GNUC__
                          timer,
#else
                          0, (int)timer,
#endif
#endif
                          &pmx, &pmy, &button ,&i,
                          &i, &i);

#if 1
      newobj = objc_find(popup, R_TREE, MAX_DEPTH, pmx, pmy);
#endif

      /* redraw old item */
#if 1
      if (start && (newobj != defobj)) {
#endif
         if (check) {
            /* deselect old item */
            if (defobj && (item->ob_flags & SELECTABLE) && !(item->ob_state & DISABLED)) {
#if 0
               status ^= SELECTED;
#else
               status &= ~SELECTED;
#endif
               objc_change(popup, defobj, 0, x + item->ob_x, y + item->ob_y, item->ob_width, item->ob_height, status, 1);
            }
         } else {
            check = 1;
            x = popup->ob_x;
            y = popup->ob_y;
         }
#if 1
      }
#endif

      /* checking for MU_M1 might give some trouble with doubleclicks */
      if (!(evnt_m & MU_BUTTON)) {
         /* find new object */
#if 1
         if (start && (newobj != defobj)) {
#endif
#if 0
            defobj = objc_find(popup, R_TREE, MAX_DEPTH, pmx, pmy);
#else
            defobj = newobj;
#endif
            if (defobj != -1) {           /* -1 was NO_OBJECT */
               /* select new item */
               item = &popup[defobj];
               status = item->ob_state;
               /* only show if intended */
               if (defobj && (item->ob_flags & SELECTABLE) && !(item->ob_state & DISABLED)) {
#if 0
                  status ^= SELECTED;
#else
                  status |= SELECTED;
#endif
                  objc_change(popup, defobj, 0, x + item->ob_x, y + item->ob_y, item->ob_width, item->ob_height, status, 1);
               }
            } else {
             /* outside of the whole popup menu */
                check = x = y = 0;
                item = popup;
            }
#if 1
         }
#endif
      } else {
         /* if mouse button released on 'parent' button, wait for new click */
         if (m_state == 0 && objc_find (parent, R_TREE, MAX_DEPTH, pmx, pmy) == selobj) {
            /* change click and drag into ST-style click and click */
            if (defobj && (item->ob_flags & SELECTABLE) && !(item->ob_state & DISABLED)) {
               status ^= SELECTED;
               objc_change(popup, defobj, 0, x + item->ob_x, y + item->ob_y, item->ob_width, item->ob_height, status, 1);
            }
            m_state = 1;
            evnt_m = 0;
#if 1
            start = 1;
#endif
         }
      }
   } while (!(evnt_m & MU_BUTTON) && !(popfix && (button == 1)));

   if (!popfix)
      evnt_button(1, 1, 0, &i, &i, &i, &i);		/* until button up */
   else {
      do {
         evnt_m = evnt_multi(MU_TIMER,
                             0, 0, 0,
                             0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0,
                             mbuf,
#ifdef __GNUC__
                             0L,
#else
                             0, 0,
#endif
                             &i, &i, &button ,&i,
                             &i, &i);
      } while (button);
   }

   /* restore area below popup */
#if 0
   form_dial(FMD_FINISH, x, y, w, h, x, y, w, h);
#endif
#if 0
form_dial(FMD_FINISH, parent[ROOT].ob_x + 1, parent[ROOT].ob_y + 1, 10, 10,
                      parent[ROOT].ob_x + 1, parent[ROOT].ob_y + 1, 10, 10);
form_dial(FMD_FINISH, xbox + 1, ybox + 1, wbox, 10,
                      xbox + 1, ybox + 1, wbox, 10);
form_dial(FMD_FINISH, xbox + 1, ybox + 1, 10, hbox,
                      xbox + 1, ybox + 1, 10, hbox);
#endif
#if 0
   if (ybox < parent[ROOT].ob_y + pborder)
      form_dial(FMD_FINISH, xbox, ybox, wbox, ybox - (parent[ROOT].ob_y + pborder),
                            xbox, ybox, wbox, ybox - (parent[ROOT].ob_y + pborder));
   if (ybox + hbox > parent[ROOT].ob_y + parent[ROOT].ob_height + pborder)
      form_dial(FMD_FINISH, xbox, parent[ROOT].ob_y + parent[ROOT].ob_height + pborder, wbox, ybox + hbox - (parent[ROOT].ob_y + parent[ROOT].ob_height + pborder),
                            xbox, parent[ROOT].ob_y + parent[ROOT].ob_height + pborder, wbox, ybox + hbox - (parent[ROOT].ob_y + parent[ROOT].ob_height + pborder));
#endif

   objc_draw(parent, ROOT, MAX_DEPTH, xbox, ybox, wbox, hbox);
#if 0
   wind_update(END_UPDATE);
#endif

   /* check if object was meant for selection */    /* -1 was NO_OBJECT */
   if (defobj != -1 && (!(item->ob_flags & SELECTABLE) || (item->ob_state & DISABLED)))
      return(-1);

   return(defobj);
}
