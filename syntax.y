%{
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

extern FILE* yyin;
extern int yylineno;
char* input_filename;

void yyerror(char*s)  {
  fprintf(stderr, "%s:%d: %s\n", input_filename, yylineno, s);
  exit(-1);
}
// void yyerror(char *s, ...);

extern int yylex();
extern int yywrap();

#include "eval.h"

static Term* program;
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
%nonassoc OF
%nonassoc LC RC BAR 
%nonassoc IF THEN ELSE
%left AND OR
%nonassoc NOT WRITE
%nonassoc EQUAL
%nonassoc LT GT 
%left PLUS MINUS
%left TIMES DIV MOD
%right COLON_COLON
%nonassoc HEAD TAIL
%left PERIOD
%nonassoc LP RP
%nonassoc TAG HANDLE LEFT_ARROW
%left SEMICOLON
%start input
%locations
%%
input: expr {
  printf("program:\n");
  print_term($1);
  printf("\n");
  printf("------------------------\n");
  Value* v = eval($1, 0, 0); 
  if (v == 0) {
    printf("error during evaluation\n");
  } else {
    /*print_value(v);*/
    printf("\n");
  }
  }
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
  LP RP             { $$ = make_unit(); }
| INT               { $$ = make_int($1); }
| TRUE              { $$ = make_bool(1); }
| FALSE             { $$ = make_bool(0); }
| STR               { $$ = make_string($1); }
| CHAR              { $$ = make_char($1[0]); }
| ID                { $$ = make_var(yylineno, $1); }
| HEAD expr         { $$ = make_op(yylineno, "head", make_list($2, make_unit())); }
| TAIL expr         { $$ = make_op(yylineno, "tail", make_list($2, make_unit())); }
| expr COLON_COLON expr
    { $$ = make_op(yylineno, "make_list", make_list($1, make_list($3, make_unit()))); }
| FUN LP var_list RP LC expr RC { $$ = make_lambda(yylineno, $3, $6); }
| LP expr RP       { $$ = $2; }
| BAR expr BAR     { $$ = make_op(yylineno, "length", make_list($2, make_unit())); }
| expr LP expr_list RP  { $$ = make_app(yylineno, $1, $3); }
| expr PERIOD ID   { $$ = make_field_access(yylineno, $1, $3); }
| REC LP ID RP expr { $$ = make_recursive(yylineno, $3, $5); }
| IF expr THEN expr ELSE expr { $$ = make_ifthen(yylineno, $2, $4, $6); }
| expr EQUAL expr   { $$ = make_op(yylineno, "equal",
				   make_list($1, make_list($3, make_unit()))); }
| expr PLUS expr   { $$ = make_op(yylineno, "add",
				  make_list($1, make_list($3, make_unit()))); }
| expr MINUS expr  { $$ = make_op(yylineno, "sub", make_list($1, make_list($3, make_unit()))); }
| expr TIMES expr  { $$ = make_op(yylineno, "mul", make_list($1, make_list($3, make_unit()))); }
| expr DIV expr    { $$ = make_op(yylineno, "div", make_list($1, make_list($3, make_unit()))); }
| expr AND expr    { $$ = make_op(yylineno, "and", make_list($1, make_list($3, make_unit()))); }
| expr OR expr    { $$ = make_op(yylineno, "or", make_list($1, make_list($3, make_unit()))); }
| expr LT expr    { $$ = make_op(yylineno, "less", make_list($1, make_list($3, make_unit()))); }
| expr GT expr    { $$ = make_op(yylineno, "less", make_list($3, make_list($1, make_unit()))); }
| expr MOD expr    { $$ = make_op(yylineno, "mod", make_list($1, make_list($3, make_unit()))); }
| NOT expr         { $$ = make_op(yylineno, "not", make_list($2, make_unit())); }
| MINUS expr       { $$ = make_op(yylineno, "neg", make_list($2, make_unit())); }
| WRITE expr       { $$ = make_op(yylineno, "write", make_list($2, make_unit())); }
| READ             { $$ = make_op(yylineno, "read", make_unit()); }
| RECORD LC fields RC    { $$ = make_record(yylineno, $3); }
| TAG expr AS ID { $$ = make_variant_term(yylineno, $4, $2); }
| TAG ID HANDLE expr { 
    $$ = make_handler_term(yylineno, $2, $4);
  }
| TAG ID COLON ID HANDLE expr { 
    $$ = make_handler_term(yylineno, $2, make_lambda(yylineno,
						     make_list(make_string($4), make_unit()), $6));
  }
| CASE expr OF expr { $$ = make_case(yylineno, $2, $4); }
| ID COLON expr SEMICOLON expr { $$ = make_let(yylineno, $1, $3, $5); }
| expr PERIOD ID LEFT_ARROW expr { $$ = make_field_update(yylineno, $1, $3, $5); }
;
%%
int main(int argc, char* argv[])  { 
  /*yydebug = 1;*/

  if (argc > 1) {
    input_filename = argv[1];
    yyin = fopen(argv[1], "r");
  }
  if (argc > 2) {
    FILE* program = fopen(argv[2], "r");
    input = read_file(program);
  }
  yyparse(); 
  return 0;
}

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
