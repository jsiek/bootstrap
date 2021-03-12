#ifndef AST_H
#define AST_H

#include <stdlib.h>

/***** Types *****/
struct TypeEnvS;
struct TypeS;

enum TypeKind { UnitT, IntT, BoolT, StringT, CharT,
		FunT, RecordT, VariantT, HandlerT, ArrayT,
		RecursiveT, VarT, AllT, 
		LamT, AppT };

struct TypeListS {
  struct TypeS* type;
  struct TypeListS* next;
};
typedef struct TypeListS TypeList;

struct TypeS {
  int lineno;
  enum TypeKind tag;
  struct TypeS* source; // Remember the original source type for printing errors
  union {
    char* var;
    struct { char* var; struct TypeS* body; struct TypeEnvS* env; } tylam;
    struct { struct TypeS* rator; struct TypeS* rand; } tyapp;
    struct { TypeList* params; struct TypeS* ret; } fun;
    struct { char* var; struct TypeS* body; } all;
    struct { char* var; struct TypeS* body; } rec;
    struct { struct TypeEnvS* fields; } record;
    struct { struct TypeS* elt; } array;
    struct { struct TypeEnvS* fields; } variant;
    struct { struct TypeEnvS* fields; struct TypeS* ret; } handler;
  } u;
};
typedef struct TypeS Type;

struct TypeEnvS {
  char* var;
  Type* type;
  struct TypeEnvS* rest;
};
typedef struct TypeEnvS TypeEnv;

TypeList* insert_type(Type* t, TypeList* next);

Type* make_var_type(int lineno, char* var);
Type* make_lam_type(int lineno, char* var, TypeEnv* env, Type* body);
Type* make_app_type(int lineno, Type* rator, Type* rand);
Type* make_unit_type(int lineno);
Type* make_int_type(int lineno);
Type* make_string_type(int lineno);
Type* make_char_type(int lineno);
Type* make_bool_type(int lineno);
Type* make_array_type(int lineno, Type* elt);
Type* make_all_type(int lineno, char* var, Type* body);
Type* make_fun_type(int lineno, TypeList* params, Type* ret);
Type* make_recursive_type(int lineno, char* var, Type* body);
TypeEnv* make_type_binding(char* field, Type* type, TypeEnv* rest);
Type* make_record_type(int lineno, TypeEnv* fields);
Type* make_variant_type(int lineno, TypeEnv* fields);
Type* make_handler_type(int lineno, TypeEnv* fields, Type* ret);

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

struct TermS {
  int lineno;
  enum TermKind tag;
  union {
    int _int;
    int _bool;
    char* var;
    char* str;
    char _char;
    struct { Type* type; struct TermS* body; } fold;
    struct { Type* type; struct TermS* body; } unfold;
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
    struct { char* name; Type* type; struct TermS* body; } handler;
    struct { struct TermS* cond; struct TermS* thn;
      struct TermS* els; } ifthen;
    struct { char* var; struct TermS* rhs; struct TermS* body; } let;
    struct { char* var; Type* type; struct TermS* body; } alias;
    struct { char* var; Type* type; struct TermS* body; } rec;
    struct { char* var; struct TermS* body; } generic;
    struct { struct TermS* poly; Type* type;  } inst;
    struct { struct TermS* term; Type* type;  } ascribe;
    struct { char* label; TermList* inputs; struct TermS* term; } trace;
  } u;
};
typedef struct TermS Term;

struct TermBindingListS {
  char* field;
  Term* initializer;
  struct TermBindingListS* rest;
};
typedef struct TermBindingListS TermBindingList;

TermList* insert_term(Term*, TermList*);

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
Term* make_recursive(int lineno, char* var, Type* ty, Term* body);

/* Generics */
Term* make_generic(int lineno, char* var, Term* body);
Term* make_inst(int lineno, Term*, Type*);

/* Miscellaneous */
Term* make_ascribe(int lineno, Term*, Type*);
Term* make_let(int lineno, char*, Term*, Term*);
Term* make_alias(int lineno, char*, Type*, Term*);
Term* make_trace(int lineno, char*, TermList*, Term*);

/* Records */
TermBindingList* make_binding(char* field, Term* init, TermBindingList* rest);
Term* make_record(int lineno, TermBindingList* fields);
Term* make_field_access(int lineno, Term* rec, char* field);
Term* make_field_update(int lineno, Term* rec, char* field, Term* replacement);

/* Variants */
Term* make_variant(int lineno, char*, Term*);
Term* make_handler_term(int lineno, char*, Type*, Term*);
Term* make_case(int lineno, Term*, Term*);

/* Recursive Types */
Term* make_fold(int lineno, Type* ty, Term* body);
Term* make_unfold(int lineno, Type* ty, Term* body);

void print_term(Term*);

void print_type(Type*);
void print_type_list(TypeList*);
void print_type_env(TypeEnv* env);

char* binop_to_string(enum BinopKind binop);
char* uniop_to_string(enum UniopKind binop);

int type_list_len(TypeList* l);
Type* type_list_nth(TypeList* l, int i);
int term_list_len(TermList* l);
Term* term_list_nth(TermList* l, int i);

#endif
