/*
 * Replacement objc_edit()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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


#include "wind.h"
#include "gem.h"


#define CGd   0x01  /* numeric digit */
#define CGa   0x02  /* alpha */
#define CGs   0x04  /* whitespace */
#define CGu   0x08  /* upper case */
#define CGp   0x10  /* punctuation character */
#define CGdt  0x20  /* dot */
#define CGw   0x40  /* wild card */
#define CGxp  0x80  /* extended punctuation */


extern int xwind_redraw(Window *wind, GRECT *p, OBJECT *dlog, int object, int (*redraw)(int, GRECT *, OBJECT *, int));
int redraw(int, GRECT *, OBJECT *, int);

const unsigned char character_type[] = {
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            CGs,0,0,0,0,0,0,0,0,0,CGw,0,0,0,CGdt,0,
            CGd,CGd,CGd,CGd,CGd,CGd,CGd,CGd,CGd,CGd,CGp,0,0,0,0,CGw,
            0,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,
            CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,CGu|CGa,0,CGp,0,0,CGxp,
            0,CGa,CGa,CGa,CGa,CGa,CGa,CGa,CGa,CGa,CGa,CGa,CGa,CGa,CGa,CGa,
            CGa,CGa,CGa,CGa,CGa,CGa,CGa,CGa,CGa,CGa,CGa,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

int xobjc_edit(Window *wind, int (*redraw)(int, GRECT *, OBJECT *, int), OBJECT *tree, int ed_obj, int keycode, short *curpos, int kind)
{
   TEDINFO *ed_txt;
   char *txt;
   short cursor_pos, o, x;
   int key, tmask, n, chg, update;

#if defined(__GNUC__) && defined(NEW_GEMLIB)
   ed_txt = tree[ed_obj].ob_spec.tedinfo;
#else
   ed_txt = (TEDINFO*)tree[ed_obj].ob_spec;
#endif
   txt = ed_txt->te_ptext;
   cursor_pos = ed_txt->te_tmplen;

   update = 0;
   switch(kind) {
   case 0:
      break;
         
   case 1:         /* ED_INIT - set current edit field */
      o = 0;
      do {
         tree[o].ob_state &= ~IS_EDIT;
      } while(!(tree[++o].ob_flags & LASTOB));
         
      tree[ed_obj].ob_state |= IS_EDIT;
#if 1
      ed_txt->te_tmplen = (int)strlen(txt);
#else
      ed_txt->te_tmplen = 0;
#endif
      *curpos = ed_txt->te_tmplen;
#if 1
#if 0
      xwind_redraw(wind, NULL, tree, ed_obj, NULL);
#endif
#else
      update = 1;
#endif

      break;

   case 2:         /* ED_CHAR - process a character */
      xwind_redraw(wind, NULL, tree, ed_obj, NULL);   /* Turn cursor off */
      switch(keycode) {
      case 0x011b:      /* ESCAPE clears the field */
         txt[0] = '\0';
         ed_txt->te_tmplen = 0;
         update = 1;
         break;
   
      case 0x537f:      /* DEL deletes character under cursor */
         if (txt[cursor_pos]) {
            for(x = cursor_pos; x < ed_txt->te_txtlen - 1; x++)
               txt[x] = txt[x + 1];
            
            update = 1;
         }
         break;
            
      case 0x0e08:      /* BACKSPACE deletes character behind cursor (if any) */
         if (cursor_pos) {
            for(x = cursor_pos; x < ed_txt->te_txtlen; x++)
               txt[x - 1] = txt[x];
                  
            ed_txt->te_tmplen--;
      
            update = 1;
         }
         break;
               
      case 0x4d00:   /* RIGHT ARROW moves cursor right */
         if ((txt[cursor_pos]) && (cursor_pos < ed_txt->te_txtlen - 1)) {
            ed_txt->te_tmplen++;
            update = 1;
         }
         break;
   
      case 0x4d36:   /* SHIFT+RIGHT ARROW move cursor to far right of current text */
         for(x = 0; txt[x]; x++)
            ;

         if (x != cursor_pos) {
            ed_txt->te_tmplen = x;
            update = 1;
         }
         break;
            
      case 0x4b00:   /* LEFT ARROW moves cursor left */
         if (cursor_pos) {
            ed_txt->te_tmplen--;
            update = 1;
         }
         break;
            
      case 0x4b34:   /* SHIFT+LEFT ARROW move cursor to start of field */
      case 0x4700:   /* CLR/HOME also moves to far left */
         if (cursor_pos) {
            ed_txt->te_tmplen = 0;
            update = 1;
         }
         break;

      default:      /* Just a plain key - insert character */
         chg = 0;      /* Ugly hack! */
         if (cursor_pos == ed_txt->te_txtlen - 1) {
            cursor_pos--;
            ed_txt->te_tmplen--;
            chg = 1;
         }
               
         key = keycode & 0xff;
         tmask=character_type[key];

         n = (int)strlen(ed_txt->te_pvalid) - 1;
         if (cursor_pos < n)
            n = cursor_pos;

         switch(ed_txt->te_pvalid[n]) {
         case '9':
            tmask &= CGd;
            break;
         case 'a':
            tmask &= CGa|CGs;
            break;
         case 'n':
            tmask &= CGa|CGd|CGs;
            break;
         case 'p':
            tmask &= CGa|CGd|CGp|CGxp;
            /*key = toupper((char)key);*/
            break;
         case 'A':
            tmask &= CGa|CGs;
            key = toupper((char)key);
            break;
         case 'N':
            tmask &= CGa|CGd|CGs;
            key = toupper((char)key);
            break;
         case 'F':
            tmask &= CGa|CGd|CGp|CGxp|CGw;
            /*key = toupper((char)key);*/
            break;
         case 'f':
            tmask &= CGa|CGd|CGp|CGxp|CGw;
            /*key = toupper((char)key);*/
            break;
         case 'P':
            tmask &= CGa|CGd|CGp|CGxp|CGw;
            /*key = toupper((char)key);*/
            break;
         case 'X':
            tmask = 1;
            break;
         case 'x':
            tmask = 1;
            key = toupper((char)key);
            break;
         default:
            tmask = 0;
            break;         
         }
         
         if (!tmask) {
            for(n = x = 0; ed_txt->te_ptmplt[n]; n++) {
               if (ed_txt->te_ptmplt[n] == '_')
                  x++;
               else if ((ed_txt->te_ptmplt[n] == key)
                        && (x >= cursor_pos))
                  break;
            }
            if (key && (ed_txt->te_ptmplt[n] == key)) {
               for(n = cursor_pos; n < x; n++)
                  txt[n] = ' ';
               txt[x] = '\0';
               ed_txt->te_tmplen = x;
            } else {
               ed_txt->te_tmplen += chg;      /* Ugly hack! */
               xwind_redraw(wind, NULL, tree, ed_obj, NULL);       /* XOR cursor */
               return 1;                             /* Should end up at the bottom like the rest */
            }
         } else {
            txt[ed_txt->te_txtlen - 2] = '\0';   /* Needed! */
            for(x = ed_txt->te_txtlen - 1; x > cursor_pos; x--)
               txt[x] = txt[x - 1];

            txt[cursor_pos] = (char)key;
   
            ed_txt->te_tmplen++;
         }
   
         update = 1;
         break;
      }
      *curpos = ed_txt->te_tmplen;
      break;
         
   case 3:      /* ED_END - turn off the cursor */
      tree[ed_obj].ob_state &= ~IS_EDIT;
#if 1
#if 0
      xwind_redraw(wind, NULL, tree, ed_obj, NULL);
#endif
#else
      update = 1;
#endif
      break;
   }
   if (update) {      /* Moved from a number of places above. */
      tree[ed_obj].ob_state &= ~IS_EDIT;               /* No need to remove cursor, */
      xwind_redraw(wind, NULL, tree, ed_obj, redraw);
      tree[ed_obj].ob_state |= IS_EDIT;                /* but we do have a cursor. */
      *curpos = ed_txt->te_tmplen;
   }
   xwind_redraw(wind, NULL, tree, ed_obj, NULL);       /* XOR cursor */
   
   return 1;
}
