#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ast.h"
#include "eval.h"
#include "syntax.tab.h"

extern int yylex_init (yyscan_t* scanner);
void yyset_in  ( FILE * _in_str , yyscan_t yyscanner );
int yylex_destroy ( yyscan_t yyscanner );

extern Term* program;
extern char* input_filename;
extern int trace;

int main(int argc, char* argv[])  {
  yyscan_t scanner;
  yylex_init(&scanner);
 
  if (argc > 1) {
    input_filename = argv[1];
    yyset_in(fopen(input_filename, "r"), scanner);
  }
  yyparse(scanner);
  yylex_destroy(scanner);
  
  if (argc > 2 && 0 == strcmp("-trace", argv[2])) {
    trace = 1;
  }
  if (trace) {
    printf("program:\n");
    print_term(program);
    printf("\n");
    printf("------------------------\n");
  }
  Value* result = eval(program, 0, 0); 
  if (result == 0) {
    printf("error during evaluation\n");
  } else {
    /*print_value(result);*/
    /*printf("\n");*/
  }
  return 0;
}
