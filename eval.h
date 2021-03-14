#ifndef EVAL_H
#define EVAL_H

#include "ast.h"

/***** Values and Environments *****/

enum ValueKind { UnitV, IntV, BoolV, StringV, CharV,
		 ProcV, FixV, RecordV, VariantV, HandlerV };
struct ValueS;

struct EnvS {
  char* var;
  struct ValueS* val;
  struct EnvS* rest;
};
typedef struct EnvS Env;

struct ValueListS {
  struct ValueS* value;
  struct ValueListS* next;
};
typedef struct ValueListS ValueList;

struct HandlerS { 
  char* name; 
  struct ValueS* fun; 
  struct HandlerS* next;
};
typedef struct HandlerS Handler;

struct ValueS {
  enum ValueKind tag;
  union {
    int _int;
    int _bool;
    char _char;
    char* str;
    struct { struct ValueS** data; int len; } array;
    struct { VarList* params; Term* body; Env* env; } proc;
    struct { Env* fields; } record;
    struct { char* name; struct ValueS* val; } variant;
    Handler* handler;
    struct { struct ValueS* left; struct ValueS* right; } join;
    struct { struct ValueS* fun; } fix;
  } u;
};
typedef struct ValueS Value;

Value* make_unit_value();
Value* make_int_value(int n);
Value* make_bool_value(int b);
Value* make_string_value(char* str);
Value* make_char_value(char str);
Value* make_procedure(VarList* params, Term* body,Env* env);
Value* make_record_value(Env*);
Value* make_variant_value(char* name, Value* val);
Handler* make_handler(char* name, Value* fun, Handler* rest);
Value* make_handler_value(Handler* h);
Value* make_fix_value(Value* h);
Value* make_array_value(ValueList* inits);

Env* make_env(char*, Value*, Env*);

Value* eval(Term*, Env*,int);

void print_value(Value*);
void print_value_list(ValueList*);
void print_env(Env*);

char *read_file(FILE* fp);

char* input;

int value_list_len(ValueList* l);

#endif
