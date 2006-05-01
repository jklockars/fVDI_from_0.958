/*
 * dlogtble.c - the definitions for the dialogs
 */

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

#include "fvdiacc.h"


//#include "mgifgem.h"
#include "form.h"

extern int frm_find(int);


extern int init_main(int);

extern int option(int, int);
extern int saver(int, int);


//int cbrets[]              = {0, 0, 0};
//int (*cbfunc[])(int, int) = {apply, revert};


/*
 * Setup
 */

int Setupcbbuttons[]            = {EnterOption, ScreenSaver};
int Setupcbrets[]               = {0, 0};
int (*Setupcbfuncs[])(int, int)  = {option, saver};
form_callbacks Setupcallbacks    = {2, Setupcbbuttons, Setupcbrets, Setupcbfuncs};

//int Initswbuttons[]           = {Singlebuffer, Flickerspace, RGBbuffer, Singlescreen};
//int *Initswvars[]             = {&options.mem.single, &options.mem.flicker, &options.mem.rgb, &options.xtra.single};
//form_switches  Initswitches   = {4, Initswbuttons, Initswvars};

char Optiontxtstrings[]        = "______________";
int  Optiontxtfields[]         = {Option};
form_texts     Optiontexts     = {1, 15, Optiontxtstrings, Optiontxtfields};

//form_radios    Convradios     = {2, {LOWPASS1, N}, {0, 0}, {Filterbox, Dirbox}, {convtypefunc, dirfunc}};

form_popups    Cachepopups     = {1, {POP_CACHE}, {AESbufCache}, {AESbufCache}, {CacheType}};

#define POS  {-1, -1, -1, -1}

/*                  Form          Init         ??            ??              Editables       Radios           Callbacks           Switches         Popups        ??    ??   ?? */
form_def form[] = {{FRM_MAIN,     init_main,   NULL,         DONE,           &Optiontexts,   NULL,            &Setupcallbacks,    NULL,            &Cachepopups, NULL, POS, ""},
                   {-1,           NULL,        NULL,         0,              NULL,           NULL,            NULL,               NULL,            NULL,         NULL, POS, ""}
                  };

char *formtext(int dialog, int text_no)
{
   int no;
   char *ptr;
   
   no = frm_find(dialog);
   ptr = &form[no].texts->text[text_no * form[no].texts->length];
   
   return(ptr);
}
