#include <stdio.h>
#include <string.h>
#include "ast.h"


/* Type Constructors */

TypeList* insert_type(Type* t, TypeList* next) {
  TypeList* l = malloc(sizeof(TypeList));
  l->type = t;
  l->next = next;
  return l;
}

Type* make_lam_type(int lineno, char* var, TypeEnv* env, Type* body) {
  Type* t = malloc(sizeof(Type));
  t->tag = LamT;
  t->lineno = lineno;
  t->u.tylam.var = var;
  t->u.tylam.env = env;
  t->u.tylam.body = body;
  return t;
}

Type* make_app_type(int lineno, Type* rator, Type* rand) {
  Type* t = malloc(sizeof(Type));
  t->tag = AppT;
  t->lineno = lineno;
  t->u.tyapp.rator = rator;
  t->u.tyapp.rand = rand;
  return t;
}

Type* make_unit_type(int lineno) {
  Type* t = malloc(sizeof(Type));
  t->tag = UnitT;
  t->lineno = lineno;
  return t;
}
Type* make_var_type(int lineno, char* var) {
  Type* t = malloc(sizeof(Type));
  t->tag = VarT;
  t->lineno = lineno;
  t->u.var = var;
  return t;
}
Type* make_int_type(int lineno) {
  Type* t = malloc(sizeof(Type));
  t->source = 0;
  t->tag = IntT;
  t->lineno = lineno;
  return t;
}
Type* make_string_type(int lineno) {
  Type* t = malloc(sizeof(Type));
  t->source = 0;
  t->tag = StringT;
  t->lineno = lineno;
  return t;
}
Type* make_char_type(int lineno) {
  Type* t = malloc(sizeof(Type));
  t->source = 0;
  t->tag = CharT;
  t->lineno = lineno;
  return t;
}
Type* make_bool_type(int lineno) {
  Type* t = malloc(sizeof(Type));
  t->source = 0;
  t->tag = BoolT;
  t->lineno = lineno;
  return t;
}
Type* make_array_type(int lineno, Type* elt) {
  Type* t = malloc(sizeof(Type));
  t->source = 0;
  t->tag = ArrayT;
  t->lineno = lineno;
  t->u.array.elt = elt;
  return t;
}
Type* make_fun_type(int lineno, TypeList* params, Type* ret) {
  Type* t = malloc(sizeof(Type));
  t->source = 0;
  t->tag = FunT;
  t->lineno = lineno;
  t->u.fun.params = params;
  t->u.fun.ret = ret;
  return t;
}
Type* make_record_type(int lineno, TypeEnv* fields) {
  Type* t = malloc(sizeof(Type));
  t->source = 0;
  t->tag = RecordT;
  t->lineno = lineno;
  t->u.record.fields = fields;
  return t;
}
Type* make_variant_type(int lineno, TypeEnv* fields) {
  Type* t = malloc(sizeof(Type));
  t->source = 0;
  t->tag = VariantT;
  t->lineno = lineno;
  t->u.record.fields = fields;
  return t;
}
Type* make_handler_type(int lineno, TypeEnv* fields, Type* ret) {
  Type* t = malloc(sizeof(Type));
  t->source = 0;
  t->tag = HandlerT;
  t->lineno = lineno;
  t->u.handler.fields = fields;
  t->u.handler.ret = ret;
  return t;
}
Type* make_recursive_type(int lineno, char* var, Type* body) {
  Type* t = malloc(sizeof(Type));
  t->source = 0;
  t->lineno = lineno;
  t->tag = RecursiveT;
  t->u.rec.var = var;
  t->u.rec.body = body;
  return t;
}
Type* make_all_type(int lineno, char* var, Type* body) {
  Type* t = malloc(sizeof(Type));
  t->source = 0;
  t->lineno = lineno;
  t->tag = AllT;
  t->u.all.var = var;
  t->u.all.body = body;
  return t;
}
TypeEnv* make_type_binding(char* var, Type* type, 
			   TypeEnv* rest) {
  TypeEnv* l = malloc(sizeof(TypeEnv));
  l->var = var;
  l->type = type;
  l->rest = rest;
  return l;
}

/* Term Constructors */

TermList* insert_term(Term* t, TermList* next) {
  TermList* l = malloc(sizeof(TermList));
  l->term = t;
  l->next = next;
  return l;
}

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

Term* make_index(int lineno, Term* array, Term* i) {
  Term* t = malloc(sizeof(Term));
  t->tag = Index; 
  t->lineno = lineno;
  t->u.index.array = array;
  t->u.index.i = i;
  return t;
}

Term* make_var(int lineno, char* x) {
  Term* t = malloc(sizeof(Term));
  t->tag = Var; 
  t->lineno = lineno;
  t->u.var = x;
  return t;
}

Term* make_binop(int lineno, enum BinopKind tag, Term* left, Term* right) {
  Term* t = malloc(sizeof(Term));
  t->tag = BinOp; 
  t->lineno = lineno;
  t->u.binop.tag = tag;
  t->u.binop.left = left;
  t->u.binop.right = right;
  return t;
}

Term* make_uniop(int lineno, enum UniopKind tag, Term* expr) {
  Term* t = malloc(sizeof(Term));
  t->tag = UniOp; 
  t->lineno = lineno;
  t->u.uniop.tag = tag;
  t->u.uniop.expr = expr;
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

Term* make_lambda(int lineno, TypeEnv* params, Term* body) {
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

Term* make_array(int lineno, TermList* inits) {
  Term* t = malloc(sizeof(Term));
  t->tag = Array; 
  t->lineno = lineno;
  t->u.array.inits = inits;
  return t;
}

Term* make_array_comp(int lineno, Term* size, Term* init_fun) {
  Term* t = malloc(sizeof(Term));
  t->tag = ArrayComp; 
  t->lineno = lineno;
  t->u.array_comp.size = size;
  t->u.array_comp.init_fun = init_fun;
  return t;
}

Term* make_recursive(int lineno, char* var, Type* ty, Term* body) {
  Term* t = malloc(sizeof(Term));
  t->tag = Recursive; 
  t->lineno = lineno;
  t->u.rec.var = var;
  t->u.rec.type = ty;
  t->u.rec.body = body;
  return t;
}

Term* make_generic(int lineno, char* var, Term* body) {
  Term* t = malloc(sizeof(Term));
  t->tag = Generic; 
  t->lineno = lineno;
  t->u.generic.var = var;
  t->u.generic.body = body;
  return t;
}

Term* make_inst(int lineno, Term* poly, Type* type) {
  Term* t = malloc(sizeof(Term));
  t->tag = Inst; 
  t->lineno = lineno;
  t->u.inst.poly = poly;
  t->u.inst.type = type;
  return t;
}

Term* make_ascribe(int lineno, Term* term, Type* type) {
  Term* t = malloc(sizeof(Term));
  t->tag = Ascribe; 
  t->lineno = lineno;
  t->u.ascribe.term = term;
  t->u.ascribe.type = type;
  return t;
}

Term* make_trace(int lineno, char* label, TermList* inputs, Term* term) {
  Term* t = malloc(sizeof(Term));
  t->tag = Trace; 
  t->lineno = lineno;
  t->u.trace.label = label;
  t->u.trace.inputs = inputs;
  t->u.trace.term = term;
  return t;
}

Term* make_fold(int lineno, Type* type, Term* body) {
  Term* t = malloc(sizeof(Term));
  t->tag = Fold; 
  t->lineno = lineno;
  t->u.fold.type = type;
  t->u.fold.body = body;
  return t;
}

Term* make_unfold(int lineno, Type* type, Term* body) {
  Term* t = malloc(sizeof(Term));
  t->tag = Unfold; 
  t->lineno = lineno;
  t->u.fold.type = type;
  t->u.fold.body = body;
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

Term* make_alias(int lineno, char* var, Type* type, Term* body) {
  Term* t = malloc(sizeof(Term));
  t->tag = Alias; 
  t->lineno = lineno;
  t->u.alias.var = var;
  t->u.alias.type = type;
  t->u.alias.body = body;
  return t;
}

TermBindingList* make_binding(char* field, Term* init, TermBindingList* rest) {
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

Term* make_handler_term(int lineno, char* name, Type* type, Term* body) {
  Term* t = malloc(sizeof(Term));
  t->tag = HandlerTerm; 
  t->lineno = lineno;
  t->u.handler.name = name;
  t->u.handler.type = type;
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
    print_type_env(e->u.lam.params);
    printf(") {...}");
    break;
  default:
    printf("<unknown term>");
  }
}

void print_type_env(TypeEnv* env) {
  if (env != 0) {
    printf("%s : ", env->var);
    print_type(env->type);
    if (env->rest != 0) {
      printf(", ");
    }
    print_type_env(env->rest);
  }
}

void print_type_env_abbrv(TypeEnv* env) {
  if (env != 0) {
    printf("%s", env->var);
    if (env->rest != 0) {
      printf(", ");
    }
    print_type_env_abbrv(env->rest);
  }
}

void print_type_list(TypeList* t) {
  while (t != 0) {
    print_type(t->type);
    if (t->next) {
      printf(", ");
    }
    t = t->next;
  }
}

void print_type(Type* t) {
  if (t->source) {
    print_type(t->source);
  } else {

    switch (t->tag) {
    case ArrayT:
      printf("[");
      print_type(t->u.array.elt);
      printf("]");
      break;
    case StringT:
      printf("string");
      break;
    case CharT:
      printf("char");
      break;
    case VarT:
      printf("`%s", t->u.var);
      break;
    case UnitT:
      printf("unit");
      break;
    case IntT:
      printf("int");
      break;
    case BoolT:
      printf("bool");
      break;
    case FunT: {
      printf("(");
      TypeList* p = t->u.fun.params;
      while (p != 0) {
	print_type(p->type);
	if (p->next != 0) {
	  printf(", ");
	}
	p = p->next;
      }
      printf(") -> ");
      print_type(t->u.fun.ret);
      break;
    }
    case RecordT:
      printf("(group ");
      print_type_env(t->u.record.fields);
      printf(")");
      break;
    case VariantT:
      printf("(one of ");
      print_type_env(t->u.variant.fields);
      printf(")");
      break;
    case HandlerT:
      printf("with tag ");
      print_type_env(t->u.handler.fields);
      printf(" => ");
      print_type(t->u.handler.ret);
      break;
    case RecursiveT:
      printf("(rec %s. ", t->u.rec.var);
      print_type(t->u.rec.body);
      printf(")");
      break;
    case AllT:
      printf("(all %s. ", t->u.all.var);
      print_type(t->u.all.body);
      printf(")");
      break;
    case LamT:
      printf("(fun (%s) ", t->u.tylam.var);
      print_type(t->u.tylam.body);
      printf(")");
      break;
    case AppT:
      print_type(t->u.tyapp.rator);
      printf("(");
      print_type(t->u.tyapp.rand);
      printf(")");
      break;
    }
  }
}

static char* binop_names[] = { "=", "+", "-", "*", "/", "and", "or", "<", "mod" };

char* binop_to_string(enum BinopKind binop) {
  return binop_names[binop];
}

static char* uniop_names[] = { "len", "not", "-" };

char* uniop_to_string(enum UniopKind op) {
  return uniop_names[op];
}

int type_list_len(TypeList* l) {
  if (l == 0) {
    return 0;
  } else {
    return 1 + type_list_len(l->next);
  }
}

Type* type_list_nth(TypeList* l, int i) {
  if (i == 0) {
    return l->type;
  } else {
    return type_list_nth(l->next, i - 1);
  }
}

int term_list_len(TermList* l) {
  if (l == 0) {
    return 0;
  } else {
    return 1 + term_list_len(l->next);
  }
}

Term* term_list_nth(TermList* l, int i) {
  if (i == 0) {
    return l->term;
  } else {
    return term_list_nth(l->next, i - 1);
  }
}

