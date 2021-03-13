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
   TermList* term_list;
   VarList* var_list;
   TermBindingList* fields;
};

%token <num> INT
%token <str> ID
%token <str> STR
%token <str> CHAR
%type <term> expr
%type <term_list> expr_list
%type <var_list> var_list
%type <term> simple_expr
%type <fields> fields
%token ARRAY
%token NOT
%token AND
%token OR
%token FUN
%token REC
%token TRUE
%token FALSE
%token COLON
%token SEMICOLON
%token IS
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
%token BUT
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
%token IF
%token THEN
%token ELSE
%token OF
%token ONE
%token WITH
%token READ
%token WRITE
%right PERIOD
%nonassoc IS
%left BUT
%left BAR
%nonassoc IF THEN ELSE
%nonassoc LP RP
%nonassoc LS RS LT GT 
%left AND OR
%nonassoc EQUAL
%left PLUS MINUS
%left TIMES DIV
%left POSSES
%right FUNTY
%nonassoc FUN TAG
%start input
%locations
%%
input:
  /* empty */ { }
| expr PERIOD {
  Value* v = eval($1, 0, 0); 
  if (v == 0) {
    printf("error during evaluation\n");
  } else {
    print_value(v);
    printf("\n");
  }
  }
;
/*
line: 
  PERIOD
| expr PERIOD {
 }
;
*/
fields:
  /* empty */ { $$ = 0; }
| ID COLON expr { $$ = make_binding($1, $3, 0); }
| ID COLON expr COMMA fields { $$ = make_binding($1, $3, $5); }
;
var_list:
  /* empty */      { $$ = 0; }
| ID               { $$ = insert_var($1, 0); }
| ID COMMA var_list { $$ = insert_var($1, $3); }
;
expr_list:
  /* empty */          { $$ = 0; }
| expr                 { $$ = insert_term($1, 0); }
| expr COMMA expr_list { $$ = insert_term($1, $3); }
;
simple_expr:
  LP RP             { $$ = make_unit(yylineno); }
| INT               { $$ = make_int(yylineno, $1); }
| TRUE              { $$ = make_bool(yylineno, 1); }
| FALSE             { $$ = make_bool(yylineno, 0); }
| ID                { $$ = make_var(yylineno, $1); }
| STR               { $$ = make_string(yylineno, $1); }
| CHAR              { $$ = make_char(yylineno, $1[0]); }
| FUN var_list LC expr RC { $$ = make_lambda(yylineno, $2, $4); }
| LP expr RP       { $$ = $2; }
| BAR expr BAR     { $$ = make_op(yylineno, Len, insert_term($2, 0)); }
| simple_expr LP expr_list RP  { $$ = make_app(yylineno, $1, $3); }
| expr PERIOD ID   { $$ = make_field_access(yylineno, $1, $3); }
;
expr:
  simple_expr      { $$ = $1; }
| REC LP ID RP expr { $$ = make_recursive(yylineno, $3, $5); }
| IF expr THEN expr ELSE expr { $$ = make_ifthen(yylineno, $2, $4, $6); }
| expr EQUAL expr   { $$ = make_op(yylineno, Equal,
				   insert_term($1, insert_term($3, 0))); }
| expr PLUS expr   { $$ = make_op(yylineno, Add,
				  insert_term($1, insert_term($3, 0))); }
| expr MINUS expr  { $$ = make_op(yylineno, Sub, insert_term($1, insert_term($3, 0))); }
| expr TIMES expr  { $$ = make_op(yylineno, Mul, insert_term($1, insert_term($3, 0))); }
| expr DIV expr    { $$ = make_op(yylineno, Div, insert_term($1, insert_term($3, 0))); }
| expr AND expr    { $$ = make_op(yylineno, And, insert_term($1, insert_term($3, 0))); }
| expr OR expr    { $$ = make_op(yylineno, Or, insert_term($1, insert_term($3, 0))); }
| expr LT expr    { $$ = make_op(yylineno, Less, insert_term($1, insert_term($3, 0))); }
| expr MOD expr    { $$ = make_op(yylineno, Mod, insert_term($1, insert_term($3, 0))); }
| NOT expr         { $$ = make_op(yylineno, Not, insert_term($2, 0)); }
| MINUS expr       { $$ = make_op(yylineno, Neg, insert_term($2, 0)); }
| RECORD fields    { $$ = make_record(yylineno, $2); }
| TAG expr AS ID { $$ = make_variant(yylineno, $4, $2); }
| TAG ID HANDLE expr { 
    $$ = make_handler_term(yylineno, $2, $4);
  }
| CASE expr OF expr { $$ = make_case(yylineno, $2, $4); }
| ID COLON expr SEMICOLON expr { $$ = make_let(yylineno, $1, $3, $5); }
| expr PERIOD ID COLON expr { $$ = make_field_update(yylineno, $1, $3, $5); }
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
