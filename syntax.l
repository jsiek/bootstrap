%{
#include <stdlib.h>
#include "ast.h"
#include "syntax.tab.h"
%}
%option yylineno reentrant bison-bridge bison-locations

HEAD    "head"
TAIL    "tail"
AS      "as"
MOD     "mod"
NOT     "not"
AND     "and"
OR      "or"
READ    "read"
WRITE   "write"
STRING_OF "string_of"
PARSE   "parse"
RECORD  "record"
TAG     "tag"
WITH    "with"
FUN     "fun"
REC     "rec"
TRUE    "true"
FALSE   "false"
IF      "if"
THEN    "then"
ELSE    "else"
SWITCH   "switch"
CASE    "case"
OF      "of"
BAR     "|"
PLUS    "+"
TIMES   "*"
MINUS   "-"
DIV     "/"
COLON_COLON   "::"
COLON   ":"
SEMICOLON   ";"
PERIOD  "."
COMMA   ","
EQUAL   "="
LT      "<"
GT      ">"
LP      "("
RP      ")"
LC      "{"
RC      "}"
LS      "["
RS      "]"
HANDLE  "=>"
LEFT_ARROW "<-"
NL      [\n]
INT     [0-9]+
ID      [A-Za-z_][A-Za-z0-9_]*
STR     \"[^\"]*\"
CHAR    \#.
COMMENT \/\/[^\n]*\n
%%
{FUN} { return FUN; }
{REC} { return REC; }
{MOD} { return MOD; }
{NOT} { return NOT; }
{AND} { return AND; }
{OR} { return OR; }
{READ} { return READ; }
{WRITE} { return WRITE; }
{STRING_OF} { return STRING_OF; }
{PARSE} { return PARSE; }
{RECORD} { return RECORD; }
{TAG} { return TAG; }
{AS} { return AS; }
{WITH} { return WITH; }
{IF} { return IF; }
{THEN} { return THEN; }
{ELSE} { return ELSE; }
{SWITCH} { return SWITCH; }
{CASE} { return CASE; }
{OF} { return OF; }
{HANDLE} { return HANDLE; }
{LEFT_ARROW} { return LEFT_ARROW; }
{BAR} { return BAR; }
{TRUE} { return TRUE; }
{FALSE} { return FALSE; }
{HEAD} { return HEAD; }
{TAIL} { return TAIL; }
{COLON_COLON} { return COLON_COLON; }
{COLON} { return COLON; }
{SEMICOLON} { return SEMICOLON; }
{PERIOD} { return PERIOD; }
{COMMA} { return COMMA; }
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
           yylval->str = (char*)malloc((n + 1) * sizeof(char));
           strncpy(yylval->str, yytext, n + 1);
           return ID;
         }
{INT}    {yylval->num = atof(yytext);  return INT;}
{STR}     {
           int n = strlen(yytext) - 2;
           yylval->str = (char*)malloc((n + 1) * sizeof(char));
           strncpy(yylval->str, yytext+1, n + 1);
           yylval->str[n] = 0;
           return STR;
         }
{CHAR}     {
           int n = strlen(yytext) - 1;
           yylval->str = (char*)malloc((n + 1) * sizeof(char));
           strncpy(yylval->str, yytext+1, n + 1);
           yylval->str[n] = 0;
           return CHAR;
         }
[ \t\n]+   ;
{COMMENT} ;
.     {return yytext[0];}
%%
int yywrap()  {return 1;}

