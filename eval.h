#ifndef EVAL_H
#define EVAL_H

#include "value.h"
#include "ast.h"

Value* eval(Term*, Env*,int);
char *read_file(FILE* fp);

char* input;

#endif
