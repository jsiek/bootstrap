%define api.pure full
%param { yyscan_t scanner }
%code top {
  #include <stdio.h>
  #include "eval.h"
}
%code requires {
  typedef void* yyscan_t;
}
%code {
  int yylex(YYSTYPE* yylvalp, YYLTYPE* yyllocp, yyscan_t scanner);
  void yyerror(YYLTYPE* yyllocp, yyscan_t unused, const char* msg);
}
%{
#include <stdlib.h>
#include <stdarg.h>

  //extern FILE* yyin;
  //extern int yylineno;
char* input_filename;

  //extern int yywrap();

Term* program;
%}
%union
 {
   char* str;
   int  num;
   Term* term;
   Value* term_list;
   Value* var_list;
   Value* fields;
};

%{
  void yyerror(YYLTYPE* yyllocp, yyscan_t unused, const char* msg)  {
    fprintf(stderr, "%s:%d:%d: %s\n", input_filename, yyllocp->first_line, yyllocp->first_column, msg);
    exit(-1);
  }
%}

%token <num> INT
%token <str> ID
%token <str> STR
%token <str> CHAR
%type <term> expr
%type <term_list> expr_list
%type <var_list> var_list
%type <fields> fields
%token NOT
%token AND
%token OR
%token FUN
%token REC
%token TRUE
%token FALSE
%token COLON
%token COLON_COLON
%token SEMICOLON
%token HEAD
%token TAIL
%token PERIOD
%token COMMA
%token QUOTE
%token TAG
%token AS
%token RECORD
%token LP
%token RP
%token LC
%token RC
%token LS
%token RS
%token NL
%token PLUS
%token TIMES
%token MINUS
%token DIV
%token MOD
%token EQUAL
%token LT
%token GT
%token BAR
%token CASE
%token HANDLE
%token LEFT_ARROW
%token IF
%token THEN
%token ELSE
%token OF
%token WITH
%token READ
%token WRITE
%token STRING_OF
%token PARSE
%nonassoc OF
%nonassoc LC RC BAR
%nonassoc HANDLE
%nonassoc IF THEN ELSE
%left AND OR
%nonassoc NOT WRITE STRING_OF PARSE
%nonassoc EQUAL
%nonassoc LT GT 
%left PLUS MINUS
%left TIMES DIV MOD
%right COLON_COLON
%nonassoc HEAD TAIL
%left PERIOD
%nonassoc LP RP
%nonassoc TAG LEFT_ARROW
%left SEMICOLON
%start input
%locations
%%
input: expr { program = $1; }
;
fields:
  /* empty */ { $$ = make_unit(); }
| ID COLON expr { $$ = make_list(make_string($1), $3); }
| ID COLON expr COMMA fields { $$ = make_list(make_list(make_string($1), $3), $5); }
;
var_list:
  /* empty */      { $$ = make_unit(); }
| ID               { $$ = make_list(make_string($1), make_unit()); }
| ID COMMA var_list { $$ = make_list(make_string($1), $3); }
;
expr_list:
  /* empty */          { $$ = make_unit(); }
| expr                 { $$ = make_list($1, make_unit()); }
| expr COMMA expr_list { $$ = make_list($1, $3); }
;
expr:
  LP RP             { $$ = make_unit_term(yylloc.first_line); }
| INT               { $$ = make_int_term(yylloc.first_line, $1); }
| TRUE              { $$ = make_bool_term(yylloc.first_line, 1); }
| FALSE             { $$ = make_bool_term(yylloc.first_line, 0); }
| STR               { $$ = make_string_term(yylloc.first_line, $1); }
| CHAR              { $$ = make_char_term(yylloc.first_line, $1[0]); }
| ID                { $$ = make_var(yylloc.first_line, $1); }
| HEAD expr         { $$ = make_op(yylloc.first_line, "head", make_list($2, make_unit())); }
| TAIL expr         { $$ = make_op(yylloc.first_line, "tail", make_list($2, make_unit())); }
| expr COLON_COLON expr
    { $$ = make_op(yylloc.first_line, "make_list", make_list($1, make_list($3, make_unit()))); }
| FUN LP var_list RP LC expr RC { $$ = make_lambda(yylloc.first_line, $3, $6); }
| LP expr RP       { $$ = $2; }
| BAR expr BAR     { $$ = make_op(yylloc.first_line, "length", make_list($2, make_unit())); }
| expr LP expr_list RP  { $$ = make_app(yylloc.first_line, $1, $3); }
| expr PERIOD ID   { $$ = make_field_access(yylloc.first_line, $1, $3); }
| REC LP ID RP expr { $$ = make_recursive(yylloc.first_line, $3, $5); }
| IF expr THEN expr ELSE expr { $$ = make_ifthen(yylloc.first_line, $2, $4, $6); }
| expr EQUAL expr   { $$ = make_op(yylloc.first_line, "equal",
				   make_list($1, make_list($3, make_unit()))); }
| expr PLUS expr   { $$ = make_op(yylloc.first_line, "add",
				  make_list($1, make_list($3, make_unit()))); }
| expr MINUS expr  { $$ = make_op(yylloc.first_line, "sub", make_list($1, make_list($3, make_unit()))); }
| expr TIMES expr  { $$ = make_op(yylloc.first_line, "mul", make_list($1, make_list($3, make_unit()))); }
| expr DIV expr    { $$ = make_op(yylloc.first_line, "div", make_list($1, make_list($3, make_unit()))); }
| expr AND expr    { $$ = make_op(yylloc.first_line, "and", make_list($1, make_list($3, make_unit()))); }
| expr OR expr    { $$ = make_op(yylloc.first_line, "or", make_list($1, make_list($3, make_unit()))); }
| expr LT expr    { $$ = make_op(yylloc.first_line, "less", make_list($1, make_list($3, make_unit()))); }
| expr GT expr    { $$ = make_op(yylloc.first_line, "less", make_list($3, make_list($1, make_unit()))); }
| expr MOD expr    { $$ = make_op(yylloc.first_line, "mod", make_list($1, make_list($3, make_unit()))); }
| NOT expr         { $$ = make_op(yylloc.first_line, "not", make_list($2, make_unit())); }
| MINUS expr       { $$ = make_op(yylloc.first_line, "neg", make_list($2, make_unit())); }
| WRITE expr       { $$ = make_op(yylloc.first_line, "write", make_list($2, make_unit())); }
| STRING_OF expr   { $$ = make_op(yylloc.first_line, "string_of", make_list($2, make_unit())); }
| PARSE STR        { $$ = make_op(yylloc.first_line, "parse", make_list(make_string_term(yyloc.first_line, $2), make_unit())); }
| READ             { $$ = make_op(yylloc.first_line, "read", make_unit()); }
| RECORD LC fields RC    { $$ = make_record(yylloc.first_line, $3); }
| TAG expr AS ID { $$ = make_variant_term(yylloc.first_line, $4, $2); }
| TAG ID HANDLE expr { 
    $$ = make_handler_term(yylloc.first_line, $2, $4);
  }
| TAG ID COLON ID HANDLE expr { 
    $$ = make_handler_term(yylloc.first_line, $2, make_lambda(yylloc.first_line,
						     make_list(make_string($4), make_unit()), $6));
  }
| CASE expr OF expr { $$ = make_case(yylloc.first_line, $2, $4); }
| ID COLON expr SEMICOLON expr { $$ = make_let(yylloc.first_line, $1, $3, $5); }
| expr PERIOD ID LEFT_ARROW expr { $$ = make_field_update(yylloc.first_line, $1, $3, $5); }
;
%%

/*
void yyerror(char *s, ...)
{
  va_list ap;
  va_start(ap, s);

  if(yylloc.first_line) {
    fprintf(stderr, "%d.%d-%d.%d: error: ", 
	    yylloc.first_line, yylloc.first_column,
	    yylloc.last_line, yylloc.last_column);
  }
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}

void lyyerror(YYLTYPE t, char *s, ...)
{
  va_list ap;
  va_start(ap, s);

  if(t.first_line) {
    fprintf(stderr, "%d.%d-%d.%d: error: ", 
	    t.first_line, t.first_column,
	    t.last_line, t.last_column);
  }
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}
*/
