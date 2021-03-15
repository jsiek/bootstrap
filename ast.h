#ifndef AST_H
#define AST_H

#include <stdlib.h>

#include "value.h"

/***** Terms *****/

Term* make_var(int lineno, char* x);

/* Primitives */

Term* make_op(int lineno, char* op, Value* args);
Term* make_ifthen(int lineno, Term*, Term*, Term*);

/* Procedures */
Term* make_lambda(int lineno, Value* params, Term* body);
Term* make_app(int lineno, Term* rator, Value* rands);
Term* make_recursive(int lineno, char* var, Term* body);

/* Miscellaneous */
Term* make_let(int lineno, char*, Term*, Term*);

/* Records */
Term* make_record_term(int lineno, Value* fields);
Term* make_field_access(int lineno, Term* rec, char* field);
Term* make_field_update(int lineno, Term* rec, char* field, Term* replacement);

/* Variants */
Term* make_variant_term(int lineno, char*, Term*);
Term* make_handler_term(int lineno, char*, Term*);
Term* make_case(int lineno, Term*, Term*);

void print_term(Term*);

#endif
