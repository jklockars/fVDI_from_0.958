/*
 * fVDI Calamus declarations
 *
 * $Id: calamus.h,v 1.3 2005-12-15 15:06:48 standa Exp $
 *
 * Copyright 2004, Standa Opichal
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "fvdi.h"


struct DCSD_BLITARGS;

struct DCSD_cookie {
	short version;
	void  CDECL (*init)(void);
	void  CDECL (*exit)(void);
	long  CDECL (*active)(void);
	void* CDECL (*getbase)(void);
	void  CDECL (*gettlt)(unsigned char tlt[256]);
	void  CDECL (*blit_from_screen)(struct DCSD_BLITARGS *args);
	void  CDECL (*blit_to_screen)(struct DCSD_BLITARGS *args);
	long  CDECL (*custom)(long par);      /* Customer function */
};


void calamus_initialize_cookie(struct DCSD_cookie *cookie, short version);
