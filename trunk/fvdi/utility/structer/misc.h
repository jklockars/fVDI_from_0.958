#ifndef _MISC_H
#define _MISC_H
/*
 * Miscellaneous declarations
 *
 * $Id: misc.h,v 1.2 2002-05-13 01:28:11 johan Exp $
 *
 * Copyright 1997-2002, Johan Klockars
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

#include "hash.h"

typedef NTentry *Identifier;
typedef Identifier Name;
typedef long Numeral;
typedef char *String;

typedef enum _Type {_Char, _Short, _Int, _Long, _Struct, _Union,
                    _Structdef, _Uniondef, _Typedef, _Pointer} Type;

typedef enum _Listtype {_Deflist} Listtype;

#endif
