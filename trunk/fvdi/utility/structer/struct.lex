/*
 * C structure lexical analyzer
 *
 * $Id: struct.lex,v 1.2 2002-05-13 01:28:11 johan Exp $
 *
 * Copyright 1997-2002, Johan Klockars
 * This software is licensed under the GNU General Public License.
 * Please, see LICENSE.TXT for further information.
 */

%{
#include <strings.h>
#include <stdlib.h>

#include "misc.h"
#include "expr.h"
#include "list.h"
#include "hash.h"
#include "struct.tab.h"

#define LDEBUG	0
#define	DBE	if (LDEBUG) ECHO

int lineno = 1;
char tmpbuf[32];   /* Maximum identifier or number must fit! */

static void lexerror(int,char*);
extern void yyerror(char *,...);

%}

D		[0-9]
identifier	[a-zA-Z_]+[a-zA-Z0-9_]*
%%

"*"	{ DBE; yylval.line = lineno; return '*'; }
"{"	{ DBE; yylval.line = lineno; return '{'; }
"}"	{ DBE; yylval.line = lineno; return '}'; }
"["	{ DBE; yylval.line = lineno; return '['; }
"]"	{ DBE; yylval.line = lineno; return ']'; }
"struct"	{ DBE; yylval.line = lineno; return STRUCT; }
"union"	{ DBE; yylval.line = lineno; return UNION; }
"char"	{ DBE; yylval.line = lineno; return CHAR; }
"short"	{ DBE; yylval.line = lineno; return SHORT; }
"int"	{ DBE; yylval.line = lineno; return INT; }
"long"	{ DBE; yylval.line = lineno; return LONG; }
"typedef"	{ DBE; yylval.line = lineno; return TYPEDEF; }
";"	{ DBE; yylval.line = lineno; return ';'; }

'.'	{ DBE;
	  yylval.num = (long)yytext[0];
	  return NUMERAL;
	}
{identifier}	{ DBE;
                  strncpy(tmpbuf, yytext, yyleng);
                  tmpbuf[yyleng] = '\0';
		  yylval.name = new_string(tmpbuf);
		  return IDENTIFIER;
		}
{D}+ 		{ DBE;
                  strncpy(tmpbuf, yytext, yyleng);
                  tmpbuf[yyleng] = '\0';
		  yylval.num = atol(tmpbuf);
		  return NUMERAL;
               	}
"/*".*"*/"  DBE;
"//".*	DBE;
"#".*	DBE;
[ \t] 	DBE;
"\n"	{DBE; lineno++; }

.	{DBE; error(lineno, "Unallowed Character", yytext); return LEXERROR; }
%%
