typedef union
{
   int        line;
   Numeral    num;
   Name       name;
   String     string;
   List       list;
   Expression expr;
} YYSTYPE;
#define	TYPEDEF	258
#define	STRUCT	259
#define	CHAR	260
#define	SHORT	261
#define	INT	262
#define	LONG	263
#define	IDENTIFIER	264
#define	NUMERAL	265
#define	STRING	266
#define	LEXERROR	267


extern YYSTYPE yylval;
