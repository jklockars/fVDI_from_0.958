/*
 * global variables for GUI routines
 */

#include "mgifgem.h"
#include "wind.h"

gui_t guiopts;

int   ap_id;
int   MultiAES = 0;

int conv_col[256] = {0, 15, 1, 2, 4, 6, 3, 5,  7, 8,  9, 10, 12, 14, 11, 13, 0};
int iconv_col[256] ={0,  2, 3, 6, 4, 7, 5, 8, 9, 10, 11, 14, 12, 15, 13,  1, 0};

#if 0
int   tkernelsgn[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0,  0, 0};
int   tkernel[13];
#else
int   tkernelsgn[29] = {0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        0, 0,  0, 0};
int   tkernel[29];
#endif

int   rotsign = 0;

char edit[20], directory[80], file[20], tdir[80], tfile[20];
char last_loaded[80] = "";

int no_shorts;
#define MAX_SHORTS 50

short_t shortcut[MAX_SHORTS];

char pending_alert[128] = "";

Window window[MAX_WINDOWS];

char dlogtext[30];
