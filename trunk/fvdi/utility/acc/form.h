#ifndef _FORM_H
#define _FORM_H

#include "wind.h"

/* MAX_OBJS only limits radio boxes and popups now */
#define MAX_OBJS     4 

struct Form_texts {
   int   number;
   int   length;
   char  *text;
   int   *object;
} ;

typedef struct Form_texts form_texts;

struct Form_radios {           /* Radios with optional callback */
   int   number;
   int   which[MAX_OBJS];      /* ID of selected one (last)*/
   int   twhich[MAX_OBJS];     /* Temporary selection */
   int   object[MAX_OBJS];     /* Surrounding box */
   int   (*func[MAX_OBJS])(int, int);
} ;

typedef struct Form_radios form_radios;

struct Form_callbacks {
   int   number;
   int   *button;
   int   *flags;            /* 1 - don't return to unselected */
   int   (*(*func))(int, int);
} ;

typedef struct Form_callbacks form_callbacks;

struct Form_switches {
   int   number;
   int   *button;
   int   **variable;
} ;

typedef struct Form_switches form_switches;

struct Form_popups {
   int   number;
   int   pop[MAX_OBJS];        /* Popup dialog index */
   int   which[MAX_OBJS];     /* ID of selected one */
   int   twhich[MAX_OBJS];     /* Temporary selection */
   int   header[MAX_OBJS];    /* ID of opener */
} ;

typedef struct Form_popups form_popups;

struct Form_def {
   int   index;
   int   (*init)(int);
   int   (*fixup)(int);
   int   Cancel;
   form_texts     *texts;
   form_radios    *radios;
   form_callbacks *callbacks;
   form_switches  *switches;
   form_popups    *popups;
   Window         *window;
   int  pos[4];
   char *help_link;
} ;

typedef struct Form_def form_def;

#endif
