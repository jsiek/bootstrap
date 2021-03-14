#include <stdio.h>
#include <string.h>
#include "ast.h"


/* Term Constructors */

Term* make_unit(int lineno) {
  Term* t = malloc(sizeof(Term));
  t->tag = UnitTerm; 
  t->lineno = lineno;
  return t;
}

Term* make_int(int lineno, int n) {
  Term* t = malloc(sizeof(Term));
  t->tag = Int; 
  t->lineno = lineno;
  t->u._int = n;
  return t;
}

Term* make_bool(int lineno, int n) {
  Term* t = malloc(sizeof(Term));
  t->tag = Bool; 
  t->lineno = lineno;
  t->u._int = n;
  return t;
}

Term* make_char(int lineno, char c) {
  Term* t = malloc(sizeof(Term));
  t->tag = Char; 
  t->lineno = lineno;
  t->u._char = c;
  return t;
}

Term* make_string(int lineno, char* c) {
  Term* t = malloc(sizeof(Term));
  t->tag = String; 
  t->lineno = lineno;
  t->u.str = malloc(strlen(c) + 1);
  strcpy(t->u.str, c);
  return t;
}

Term* make_var(int lineno, char* x) {
  Term* t = malloc(sizeof(Term));
  t->tag = Var; 
  t->lineno = lineno;
  t->u.var = x;
  return t;
}

Term* make_op(int lineno, enum OpKind tag, TermList* args) {
  Term* t = malloc(sizeof(Term));
  t->tag = Op; 
  t->lineno = lineno;
  t->u.op.tag = tag;
  t->u.op.args = args;
  return t;
}

Term* make_ifthen(int lineno, Term* cond, Term* thn, Term* els) {
  Term* t = malloc(sizeof(Term));
  t->tag = IfThen; 
  t->lineno = lineno;
  t->u.ifthen.cond = cond;
  t->u.ifthen.thn = thn;
  t->u.ifthen.els = els;
  return t;
}

Term* make_lambda(int lineno, VarList* params, Term* body) {
  Term* t = malloc(sizeof(Term));
  t->tag = Lam; 
  t->lineno = lineno;
  t->u.lam.params = params;
  t->u.lam.body = body;
  return t;
}

Term* make_app(int lineno, Term* rator, TermList* rands) {
  Term* t = malloc(sizeof(Term));
  t->tag = App; 
  t->lineno = lineno;
  t->u.app.rator = rator;
  t->u.app.rands = rands;
  return t;
}

Term* make_recursive(int lineno, char* var, Term* body) {
  Term* t = malloc(sizeof(Term));
  t->tag = Recursive; 
  t->lineno = lineno;
  t->u.rec.var = var;
  t->u.rec.body = body;
  return t;
}

Term* make_let(int lineno, char* var, Term* rhs, Term* body) {
  Term* t = malloc(sizeof(Term));
  t->tag = Let; 
  t->lineno = lineno;
  t->u.let.var = var;
  t->u.let.rhs = rhs;
  t->u.let.body = body;
  return t;
}

TermBindingList* make_binding(char* field, Term* init, TermBindingList* rest)
{
  TermBindingList* l = malloc(sizeof(TermBindingList));
  l->field = field;
  l->initializer = init;
  l->rest = rest;
  return l;
}

Term* make_record(int lineno, TermBindingList* fields) {
  Term* t = malloc(sizeof(Term));
  t->tag = Record; 
  t->lineno = lineno;
  t->u.record.fields = fields;
  return t;
}

Term* make_field_access(int lineno, Term* rec, char* field) {
  Term* t = malloc(sizeof(Term));
  t->tag = Field; 
  t->lineno = lineno;
  t->u.field_access.rec = rec;
  t->u.field_access.field = field;
  return t;
}

Term* make_field_update(int lineno, Term* rec, char* field, Term* replacement) {
  Term* t = malloc(sizeof(Term));
  t->tag = FieldUpdate; 
  t->lineno = lineno;
  t->u.field_update.rec = rec;
  t->u.field_update.field = field;
  t->u.field_update.replacement = replacement;
  return t;
}

Term* make_case(int lineno, Term* descr, Term* handler) {
  Term* t = malloc(sizeof(Term));
  t->tag = Case; 
  t->lineno = lineno;
  t->u._case.descr = descr;
  t->u._case.handler = handler;
  return t;
}

Term* make_variant(int lineno, char* name, Term* init) {
  Term* t = malloc(sizeof(Term));
  t->tag = Variant; 
  t->lineno = lineno;
  t->u.variant.name = name;
  t->u.variant.init = init;
  return t;
}

Term* make_handler_term(int lineno, char* var, Term* body) {
  Term* t = malloc(sizeof(Term));
  t->tag = HandlerTerm; 
  t->lineno = lineno;
  t->u.handler.var = var;
  t->u.handler.body = body;
  return t;
}

void print_term_list(TermList* t) {
  while (t != 0) {
    print_term(t->term);
    if (t->next) {
      printf(", ");
    }
    t = t->next;
  }
}

void print_fields(TermBindingList* t) {
  while (t != 0) {
    printf("%s: ", t->field);
    print_term(t->initializer);
    if (t->rest) {
      printf(", ");
    }
    t = t->rest;
  }
}

void print_var_list(VarList* t) {
  while (t != 0) {
    printf("%s", t->var);
    if (t->next) {
      printf(", ");
    }
    t = t->next;
  }
}

void print_term(Term* e) {
  switch (e->tag) {
  case Var:
    printf("%s", e->u.var);
    break;
  case Int:
    printf("%d", e->u._int);
    break;
  case Bool:
    if (e->u._bool) {
      printf("true");
    } else {
      printf("false");
    }
    break;
  case Field:
    print_term(e->u.field_access.rec);
    printf("'s %s", e->u.field_access.field);
    break;
  case App:
    print_term(e->u.app.rator);
    printf("(");
    print_term_list(e->u.app.rands);
    printf(")");
    break;
  case Lam:
    printf("fun (");
    print_var_list(e->u.lam.params);
    printf(") {...}");
    break;
  case UnitTerm:
    printf("()");
    break;
  case String:
    printf("\"%s\"", e->u.str);
    break;
  case Char:
    printf("#%c", e->u._char);
    break;
  case Recursive:
    printf("rec (");
    printf("%s", e->u.rec.var);
    printf(") ");
    print_term(e->u.rec.body);
    break;
  case Let:
    printf("%s", e->u.let.var);
    printf(": ");
    print_term(e->u.let.body);
    printf("; ");
    break;
  case Record:
    printf("record {");
    print_fields(e->u.record.fields);
    printf("}");
    break;
  case Op:
    printf("%s(", op_to_string(e->u.op.tag));
    print_term_list(e->u.op.args);
    printf(")");
    break;
  case FieldUpdate:
    printf("<field update>");
    break;
  case Variant:
    printf("(tag ");
    print_term(e->u.variant.init);
    printf(" as %s)", e->u.variant.name);
    break;
  case HandlerTerm:
    printf("handler");
    break;
  case Case:
    printf("case");
    break;
  case IfThen:
    printf("if-then");
    break;
  }
}

static char* op_names[] = { "=", "+", "-", "*", "/", "and", "or", "<", "mod",
                            "len", "not", "-" };

char* op_to_string(enum OpKind binop) {
  return op_names[binop];
}

/* lists */

VarList* insert_var(char* t, VarList* next) {
  VarList* l = malloc(sizeof(VarList));
  l->var = t;
  l->next = next;
  return l;
}

TermList* insert_term(Term* t, TermList* next) {
  TermList* l = malloc(sizeof(TermList));
  l->term = t;
  l->next = next;
  return l;
}




