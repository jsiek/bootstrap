#ifndef AST_H
#define AST_H

#include <stdlib.h>

/***** Terms *****/

enum TermKind { UnitTerm, Int, Bool, String, Char,
		Var, Lam, App, Recursive, Let, 
		Alias, BinOp, UniOp, Index,
		Record, Field, FieldUpdate, Variant, HandlerTerm, Case, IfThen,
		Array, ArrayComp, Ascribe,
		Fold, Unfold,
		Generic, Inst,
		Trace };
enum BinopKind { Equal=0, Add=1, Sub=2, Mul=3, Div=4, And=5, Or=6, Less=7, Mod=8 };
enum UniopKind { Len=0, Not=1, Neg=2 };
struct TermBindingListS;

struct TermListS {
  struct TermS* term;
  struct TermListS* next;
};
typedef struct TermListS TermList;

struct NameListS {
  struct char* name;
  struct NameListS* next;
};
typedef struct NameListS NameList;

struct TermS {
  int lineno;
  enum TermKind tag;
  union {
    int _int;
    int _bool;
    char* var;
    char* str;
    char _char;
    struct { TypeEnv* params; struct TermS* body; } lam;
    struct { struct TermS* rator; TermList* rands; } app;
    struct { TermList* inits; } array;
    struct { struct TermS* size; struct TermS* init_fun; } array_comp;
    struct { enum BinopKind tag; 
             struct TermS* left; 
             struct TermS* right; } binop;
    struct { enum UniopKind tag; 
             struct TermS* expr; } uniop;
    struct { struct TermBindingListS* fields; } record;
    struct { struct TermS* rec; char* field; } field_access;
    struct { struct TermS* rec; char* field; struct TermS* replacement; } field_update;
    struct { struct TermS* array; struct TermS* i; } index;
    struct { char* name; struct TermS* init; } variant;
    struct { struct TermS* descr; struct TermS* handler; } _case;
    struct { char* name; struct TermS* body; } handler;
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
Term* make_binop(int lineno, enum BinopKind tag, Term* left, Term* right);
Term* make_uniop(int lineno, enum UniopKind tag, Term* expr);
Term* make_ifthen(int lineno, Term*, Term*, Term*);

Term* make_index(int lineno, Term* array, Term* i);
Term* make_array(int lineno, TermList* inits);
Term* make_array_comp(int lineno, Term* size, Term* initer);

/* Procedures */
Term* make_lambda(int lineno, TypeEnv* params, Term* body);
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

char* binop_to_string(enum BinopKind binop);
char* uniop_to_string(enum UniopKind binop);

/* lists */

int term_list_len(TermList* l);
Term* term_list_nth(TermList* l, int i);
TermList* insert_term(Term*, TermList*);

int name_list_len(NameList* l);
char* name_list_nth(NameList* l, int i);
NameList* insert_name(char*, NameList*);

#endif
