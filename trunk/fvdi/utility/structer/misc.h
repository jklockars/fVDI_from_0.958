#ifndef _MISC_H
#define _MISC_H

#include "hash.h"

typedef NTentry *Identifier;
typedef Identifier Name;
typedef long Numeral;
typedef char *String;

typedef enum _Type {_Char, _Short, _Int, _Long, _Struct, _Structdef, _Typedef, _Pointer} Type;

typedef enum _Listtype {_Deflist} Listtype;

#endif
