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

#include "typecheck.h"
#include "eval.h"

static Term* program;
%}
%union
 {
   char* str;
   int  num;
   Type* type;
   TypeList* type_list;
   Term* term;
   TermList* term_list;
   TermBindingList* fields;
   TypeEnv* field_types;
};

%token <num> INT
%token <str> ID
%token <str> STR
%token <str> CHAR
%type <term> expr
%type <term_list> expr_list
%type <term> simple_expr
%type <type> type
%type <type> simple_type
%type <type_list> type_list
%type <fields> fields
%type <field_types> field_types
%token ALL
%token ARRAY
%token NOT
%token AND
%token OR
%token FUN
%token FOLD
%token UNFOLD
%token TYPE
%token AT
%token REC
%token TRUE
%token FALSE
%token UNITTY
%token INTTY
%token STRINGTY
%token BOOLTY
%token CHARTY
%token INPUT
%token COLON
%token SEMICOLON
%token IS
%token PERIOD
%token COMMA
%token POSSES
%token QUOTE
%token TAG
%token AS
%token GROUP
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
%token TRACE
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
| PERIOD { }
| expr PERIOD {
  Type* t = typecheck($1, 0, 0);
  Value* v = eval($1, 0, 0); 
  if (v == 0) {
    printf("error during evaluation\n");
  } else {
    print_value(v);
    printf(" : ");
    print_type(t);
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
field_types:
  /* empty */ { $$ = 0; }
| ID COLON type { $$ = make_type_binding($1, $3, 0); }
| ID COLON type COMMA field_types { $$ = make_type_binding($1, $3, $5); }
;
type_list:
  /* empty */ { $$ = 0; }
| type { $$ = insert_type($1, 0); }
| type COMMA type_list { $$ = insert_type($1, $3); }
;
simple_type:
  ID                { $$ = make_var_type(yylineno, $1); }
| INTTY             { $$ = make_int_type(yylineno); }
| BOOLTY            { $$ = make_bool_type(yylineno); }
| CHARTY            { $$ = make_char_type(yylineno); }
| STRINGTY          { $$ = make_string_type(yylineno); }
| UNITTY            { $$ = make_unit_type(yylineno); }
| LP type RP { $$ = $2; }
| LS type RS { $$ = make_array_type(yylineno, $2); }
| simple_type LP type RP   { $$ = make_app_type(yylineno, $1, $3); }
;
type:
  simple_type { $$ = $1; }
| FUN LP ID RP type { $$ = make_lam_type(yylineno, $3, 0, $5); }
| LP type_list RP FUNTY type   { $$ = make_fun_type(yylineno, $2, $5); }
| type FUNTY type   { $$ = make_fun_type(yylineno, insert_type($1,0), $3); }
| GROUP field_types { $$ = make_record_type(yylineno, $2); }
| ONE OF field_types { $$ = make_variant_type(yylineno, $3); }
| WITH TAG field_types HANDLE type { $$ = make_handler_type(yylineno, $3, $5); }
| REC ID PERIOD type { $$ = make_recursive_type(yylineno, $2, $4); }
| ALL ID PERIOD type { $$ = make_all_type(yylineno, $2, $4); }
;
fields:
  /* empty */ { $$ = 0; }
| ID IS expr { $$ = make_binding($1, $3, 0); }
| ID IS expr COMMA fields { $$ = make_binding($1, $3, $5); }
;
expr_list:
  /* empty */          { $$ = 0; }
| expr                 { $$ = insert_term($1, 0); }
| expr COMMA expr_list { $$ = insert_term($1, $3); }
;
simple_expr:
  LP RP            { $$ = make_unit(yylineno); }
| INT              { $$ = make_int(yylineno, $1); }
| TRUE             { $$ = make_bool(yylineno, 1); }
| FALSE            { $$ = make_bool(yylineno, 0); }
| ID               { $$ = make_var(yylineno, $1); }
| STR              { $$ = make_string(yylineno, $1); }
| CHAR             { $$ = make_char(yylineno, $1[0]); }
| FUN LP field_types RP LC expr RC { $$ = make_lambda(yylineno, $3, $6); }
| LP expr RP       { $$ = $2; }
| LS expr_list RS { $$ = make_array(yylineno, $2); }
| BAR expr BAR     { $$ = make_uniop(yylineno, Len, $2); }
| simple_expr LP expr_list RP  { $$ = make_app(yylineno, $1, $3); }
| simple_expr AT LS type RS { $$ = make_inst(yylineno, $1, $4); }
| simple_expr LS expr RS  { $$ = make_index(yylineno, $1, $3); }
| INPUT { $$ = make_string(yylineno, input); }
| expr POSSES ID   { $$ = make_field_access(yylineno, $1, $3); }
;
expr:
  simple_expr      { $$ = $1; }
| ARRAY OF simple_expr LP ID RP LC expr RC {
  TypeEnv* params = make_type_binding($5, make_int_type(yylineno), 0);
  $$ = make_array_comp(yylineno, $3, make_lambda(yylineno, params, $8));
}
| ALL ID expr      { $$ = make_generic(yylineno, $2, $3); }
| REC LP ID COLON type RP expr { $$ = make_recursive(yylineno, $3, $5, $7); }
| FOLD LP type RP expr { $$ = make_fold(yylineno, $3, $5); }
| UNFOLD LP type RP expr { $$ = make_unfold(yylineno, $3, $5); }
| IF expr THEN expr ELSE expr { $$ = make_ifthen(yylineno, $2, $4, $6); }
| expr EQUAL expr   { $$ = make_binop(yylineno, Equal, $1, $3); }
| expr PLUS expr   { $$ = make_binop(yylineno, Add, $1, $3); }
| expr MINUS expr  { $$ = make_binop(yylineno, Sub, $1, $3); }
| expr TIMES expr  { $$ = make_binop(yylineno, Mul, $1, $3); }
| expr DIV expr    { $$ = make_binop(yylineno, Div, $1, $3); }
| expr AND expr    { $$ = make_binop(yylineno, And, $1, $3); }
| expr OR expr    { $$ = make_binop(yylineno, Or, $1, $3); }
| expr LT expr    { $$ = make_binop(yylineno, Less, $1, $3); }
| expr MOD expr    { $$ = make_binop(yylineno, Mod, $1, $3); }
| NOT expr         { $$ = make_uniop(yylineno, Not, $2); }
| MINUS expr       { $$ = make_uniop(yylineno, Neg, $2); }
| GROUP fields     { $$ = make_record(yylineno, $2); }
| TAG expr AS ID { $$ = make_variant(yylineno, $4, $2); }
| TAG ID COLON type HANDLE expr { 
    $$ = make_handler_term(yylineno, $2, $4, $6);
  }
| TAG ID COLON type LC expr RC { 
  TypeEnv* params = make_type_binding($2, $4, 0);
  $$ = make_handler_term(yylineno, $2, $4, make_lambda(yylineno, params, $6));
}
| CASE expr OF expr { $$ = make_case(yylineno, $2, $4); }
| ID IS expr PERIOD expr { $$ = make_let(yylineno, $1, $3, $5); }
| ID COLON type IS expr PERIOD expr {
  Term* e = make_ascribe(yylineno, $5, $3);
  $$ = make_let(yylineno, $1, e, $7);
}
| TYPE ID IS type PERIOD expr { $$ = make_alias(yylineno, $2, $4, $6); }
| expr BUT ID IS expr { $$ = make_field_update(yylineno, $1, $3, $5); }
| TRACE ID LP expr_list RP expr { $$ = make_trace(yylineno, $2, $4, $6); }
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
