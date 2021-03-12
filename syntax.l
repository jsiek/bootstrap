%{
#include <stdlib.h>
#include "ast.h"
#include "syntax.tab.h"
%}
%option yylineno

TRACE  "trace"
ARRAY  "array"
BUT  "but"
MOD  "mod"
NOT  "not"
AND  "and"
OR  "or"
INPUT  "input"
GROUP  "group"
TAG  "tag"
WITH  "with"
AS  "as"
ALL  "all"
FOLD  "fold"
UNFOLD  "unfold"
TYPE  "type"
FUN  "fun"
REC  "rec"
UNITTY  "unit"
INTTY  "int"
BOOLTY  "bool"
CHARTY  "char"
STRINGTY  "string"
TRUE  "true"
FALSE  "false"
FUNTY  "->"
IS  "is"
HANDLE  "=>"
IF  "if"
THEN  "then"
ELSE  "else"
CASE  "case"
OF "of"
ONE "one"
BAR "|"
PLUS  "+"
TIMES "*"
MINUS "-"
DIV   "/"
COLON   ":"
SEMICOLON   ";"
PERIOD   "."
COMMA   ","
POSSES   "'s"
AT   "@"
EQUAL   "="
LT    "<"
GT    ">"
LP    "("
RP    ")"
LC    "{"
RC    "}"
LS    "["
RS    "]"
NL    [\n]
INT   [0-9]+
ID    [A-Za-z_][A-Za-z0-9_]*
STR   \"[^\"]*\"
CHAR   \#.
COMMENT \/\/[^\n]*\n
%%
{TYPE} { return TYPE; }
{FUN} { return FUN; }
{ALL} { return ALL; }
{TRACE} { return TRACE; }
{ARRAY} { return ARRAY; }
{BUT} { return BUT; }
{MOD} { return MOD; }
{NOT} { return NOT; }
{AND} { return AND; }
{OR} { return OR; }
{INPUT} { return INPUT; }
{GROUP} { return GROUP; }
{TAG} { return TAG; }
{AS} { return AS; }
{WITH} { return WITH; }
{FOLD} { return FOLD; }
{UNFOLD} { return UNFOLD; }
{REC} { return REC; }
{UNITTY} { return UNITTY; }
{IF} { return IF; }
{THEN} { return THEN; }
{ELSE} { return ELSE; }
{CASE} { return CASE; }
{OF} { return OF; }
{ONE} { return ONE; }
{HANDLE} { return HANDLE; }
{BAR} { return BAR; }
{TRUE} { return TRUE; }
{FALSE} { return FALSE; }
{INTTY} { return INTTY; }
{BOOLTY} { return BOOLTY; }
{CHARTY} { return CHARTY; }
{STRINGTY} { return STRINGTY; }
{FUNTY} { return FUNTY; }
{IS} { return IS; }
{COLON} { return COLON; }
{SEMICOLON} { return SEMICOLON; }
{AT} { return AT; }
{PERIOD} { return PERIOD; }
{COMMA} { return COMMA; }
{POSSES} { return POSSES; }
{PLUS}   { return PLUS; }
{TIMES}  { return TIMES; }
{MINUS}  { return MINUS; }
{EQUAL}  { return EQUAL; }
{DIV}    { return DIV; }
{LT}     { return LT; }
{GT}     { return GT; }
{LP}     { return LP; }
{RP}     { return RP; }
{LC}     { return LC; }
{RC}     { return RC; }
{LS}     { return LS; }
{RS}     { return RS; }
{ID}     {
           int n = strlen(yytext);
           yylval.str = (char*)malloc((n + 1) * sizeof(char));
           strncpy(yylval.str, yytext, n + 1);
           return ID;
         }
{INT}    {yylval.num = atof(yytext);  return INT;}
{STR}     {
           int n = strlen(yytext) - 2;
           yylval.str = (char*)malloc((n + 1) * sizeof(char));
           strncpy(yylval.str, yytext+1, n + 1);
           yylval.str[n] = 0;
           return STR;
         }
{CHAR}     {
           int n = strlen(yytext) - 1;
           yylval.str = (char*)malloc((n + 1) * sizeof(char));
           strncpy(yylval.str, yytext+1, n + 1);
           yylval.str[n] = 0;
           return CHAR;
         }
[ \t\n]+   ;
{COMMENT} ;
.     {return yytext[0];}
%%
int yywrap()  {return 1;}

