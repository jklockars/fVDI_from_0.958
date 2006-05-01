#ifndef _GEM_H
#define _GEM_H
/*
 * Defines and typedefs
 */

#ifdef __GNUC__
 #if defined(NEW_GEMLIB)
  #include <gem.h>
 #else
  #include <aesbind.h>
 #endif
#else
#include <aes.h>
#endif

#define IS_EDIT   0x8000  /* XaAES special - this object has the text focus */

#define VA_START      0x4711
#if defined(__GNUC__) && !defined(NEW_GEMLIB)
 #define WM_BOTTOMED   33
#endif
#define WM_M_BDROPPED 100
#define WF_BOTTOM     25

#define FIS_HOLLOW	0
#define HOLLOW	FIS_HOLLOW
#define FIS_SOLID	1
#if defined(__GNUC__) && !defined(NEW_GEMLIB)
 #define SOLID	FIS_SOLID
#endif
#define FIS_PATTERN	2
#define PATTERN	FIS_PATTERN
#define FIS_HATCH	3
#define HATCH	FIS_HATCH
#define FIS_USER	4
#define UDFILLSTYLE	FIS_USER

typedef struct {
   char title;
   char item;
   char shift;
   char key;
} short_t;

typedef struct {
   OBJECT *ptr;
   int    handle;    /* Not used anywhere */
} Menu;

#endif
