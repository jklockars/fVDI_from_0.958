%{
#include "list.h"
#include "misc.h"
#include "expr.h"

extern int lineno;

int yyparse(void);
extern void yyerror();
extern void error();
List definitions;
%}

%union
{
   int        line;
   Numeral    num;
   Name       name;
   String     string;
   List       list;
   Expression expr;
}

%token <line> '{'  '}'  '['  ']'
%token <line> ','  ';'  '*'
%token <line> TYPEDEF  STRUCT
%token <line> CHAR  SHORT  INT  LONG
%token <name> IDENTIFIER
%token <num>  NUMERAL
%token <string> STRING
%token <line> LEXERROR

%type <list>          program  deflist
%type <expr>          def  struct  type  id  var

%start program

%%

program :  deflist
              { definitions = $1; }
;

deflist :  
                 { $$ = empty(_Deflist); }
        |  def deflist
                 { $$ = prepend($1, $2); }
;

def :       var ';'
                 { $$ = $1; }
        |  TYPEDEF var ';'
                 { $$ = mktypedef($2); }
;

struct :   STRUCT IDENTIFIER '{' deflist '}'
                 { $$ = mkstruct($2, $4); }
        |  STRUCT '{' deflist '}'
                 { $$ = mkstruct(0, $3); }
;

type :     CHAR
                 { $$ = mktype(_Char, 0, 0); }
        |  SHORT
                 { $$ = mktype(_Short, 0, 0); }
        |  INT
                 { $$ = mktype(_Int, 0, 0); }
        |  LONG
                 { $$ = mktype(_Long, 0, 0); }
        |  struct
                 { $$ = mktype(_Structdef, 0, $1); }
        |  STRUCT IDENTIFIER
                 { $$ = mktype(_Struct, $2, 0); }
        | IDENTIFIER
                 { $$ = mktype(_Typedef, $1, 0); }
        | type '*'
                 { $$ = mktype(_Pointer, 0, $1); }
;

id :       IDENTIFIER '[' NUMERAL ']'
                 { $$ = mkid($1, $3); }
        |  IDENTIFIER
                 { $$ = mkid($1, -1); }
;

var :      type id
                 { $$ = mkvar($1, $2); }
;

%%

void yyerror(char *str,int arg1, int arg2, int arg3, int arg4)
{
   fprintf(stderr,"Error at line %d: ", lineno);
   fprintf(stderr, str, arg1, arg2, arg3, arg4);
   fprintf(stderr,"\n");
}

void error(int lineno, char *format,int arg1, int arg2, int arg3, int arg4)
{
   fprintf(stderr, "Error at line %d: ", lineno);
   fprintf(stderr, format, arg1, arg2, arg3, arg4);
   fprintf(stderr, "\n");
}
