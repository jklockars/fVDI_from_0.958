%{
#include <strings.h>
#include <stdlib.h>

#include "misc.h"
#include "expr.h"
#include "list.h"
#include "hash.h"
#include "struct_tab.h"

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
