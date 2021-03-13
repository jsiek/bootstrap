#ifndef AST_H
#define AST_H

#include <stdlib.h>

/***** Terms *****/

enum TermKind {
  UnitTerm, Int, Bool, String, Char,
  Var, Lam, App, Recursive, Let, 
  Op, 
  Record, Field, FieldUpdate, Variant, HandlerTerm, Case, IfThen,
};
enum OpKind {
  Equal=0, Add=1, Sub=2, Mul=3, Div=4, And=5, Or=6, Less=7, Mod=8,
  Len=9, Not=10, Neg=11, Read=12, Write=13,
};
struct TermBindingListS;

struct TermListS {
  struct TermS* term;
  struct TermListS* next;
};
typedef struct TermListS TermList;

struct VarListS {
  char* var;
  struct VarListS* next;
};
typedef struct VarListS VarList;

struct TermS {
  int lineno;
  enum TermKind tag;
  union {
    int _int;
    int _bool;
    char* var;
    char* str;
    char _char;
    struct TermS* write;
    struct { VarList* params; struct TermS* body; } lam;
    struct { struct TermS* rator; TermList* rands; } app;
    struct { TermList* inits; } array;
    struct { struct TermS* size; struct TermS* init_fun; } array_comp;
    struct { enum OpKind tag; 
             TermList* args; } op;
    struct { struct TermBindingListS* fields; } record;
    struct { struct TermS* rec; char* field; } field_access;
    struct { struct TermS* rec; char* field; struct TermS* replacement; } field_update;
    struct { struct TermS* array; struct TermS* i; } index;
    struct { char* name; struct TermS* init; } variant;
    struct { struct TermS* descr; struct TermS* handler; } _case;
    struct { char* var; struct TermS* body; } handler;
    struct { struct TermS* cond; struct TermS* thn;
      struct TermS* els; } ifthen;
    struct { char* var; struct TermS* rhs; struct TermS* body; } let;
    struct { char* var; struct TermS* body; } rec;
  } u;
};
typedef struct TermS Term;

struct TermBindingListS {
  char* field;
  Term* initializer;
  struct TermBindingListS* rest;
};
typedef struct TermBindingListS TermBindingList;

Term* make_var(int lineno, char* x);

/* Primitives */

Term* make_unit(int lineno);
Term* make_int(int lineno, int n);
Term* make_bool(int lineno, int n);
Term* make_string(int lineno, char* str);
Term* make_char(int lineno, char c);
Term* make_op(int lineno, enum OpKind tag, TermList* args);
Term* make_ifthen(int lineno, Term*, Term*, Term*);

/* Procedures */
Term* make_lambda(int lineno, VarList* params, Term* body);
Term* make_app(int lineno, Term* rator, TermList* rands);
Term* make_recursive(int lineno, char* var, Term* body);

/* Miscellaneous */
Term* make_let(int lineno, char*, Term*, Term*);

/* Records */
TermBindingList* make_binding(char* field, Term* init, TermBindingList* rest);
Term* make_record(int lineno, TermBindingList* fields);
Term* make_field_access(int lineno, Term* rec, char* field);
Term* make_field_update(int lineno, Term* rec, char* field, Term* replacement);

/* Variants */
Term* make_variant(int lineno, char*, Term*);
Term* make_handler_term(int lineno, char*, Term*);
Term* make_case(int lineno, Term*, Term*);

void print_term(Term*);

char* op_to_string(enum OpKind binop);

/* lists */

TermList* insert_term(Term*, TermList*);
VarList* insert_var(char*, VarList*);

#endif
