#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "eval.h"

extern char* input_filename;

ValueList* insert_value(Value* v, ValueList* next) {
  ValueList* l = malloc(sizeof(ValueList));
  l->value = v;
  l->next = next;
  return l;
}

Value* get_nth(ValueList* vs, int i) {
  assert(vs);
  if (i == 0)
    return vs->value;
  else
    return get_nth(vs->next, i - 1);
}

Value* make_unit_value() {
  Value* t = malloc(sizeof(Value));
  t->tag = UnitV; 
  return t;
}

Value* make_int_value(int n) {
  Value* t = malloc(sizeof(Value));
  t->tag = IntV; 
  t->u._int = n;
  return t;
}

Value* make_string_value(char* str) {
  Value* t = malloc(sizeof(Value));
  t->tag = StringV; 
  t->u.str = str;
  return t;
}

Value* make_char_value(char c) {
  Value* t = malloc(sizeof(Value));
  t->tag = CharV; 
  t->u._char = c;
  return t;
}

Value* make_bool_value(int b) {
  Value* t = malloc(sizeof(Value));
  t->tag = BoolV; 
  t->u._bool = b;
  return t;
}

Value* make_procedure(VarList* params, Term* body, Env* env) {
  Value* t = malloc(sizeof(Value));
  t->tag = ProcV; 
  t->u.proc.params = params;
  t->u.proc.body = body;
  t->u.proc.env = env;
  return t;
}

Value* make_record_value(Env* fields) {
  Value* t = malloc(sizeof(Value));
  t->tag = RecordV; 
  t->u.record.fields = fields;
  return t;
}

Value* make_variant_value(char* name, Value* val) {
  Value* t = malloc(sizeof(Value));
  t->tag = VariantV; 
  t->u.variant.name = name;
  t->u.variant.val = val;
  return t;
}

Handler* make_handler(char* name, Value* fun, Handler* rest) {
  Handler* h = malloc(sizeof(Handler));
  h->name = name;
  h->fun = fun;
  h->next = rest;
  return h;
}

Value* make_handler_value(Handler* h) {
  Value* t = malloc(sizeof(Value));
  t->tag = HandlerV; 
  t->u.handler = h;
  return t;
}

Value* make_fix_value(Value* fun) {
  Value* t = malloc(sizeof(Value));
  t->tag = FixV; 
  t->u.fix.fun = fun;
  return t;
}

Env* make_env(char* var, Value* val, Env* rest) {
  Env* env = malloc(sizeof(Env));
  env->var = var;
  env->val = val;
  env->rest = rest;
  return env;
}

static Value* lookup(char* var, Env* env) {
  if (env == 0) {
    return 0;
  } else if (strcmp(var, env->var) == 0) {
    return env->val;
  } else {
    return lookup(var, env->rest);
  }
}

Handler* find_handler(char* name, Handler* h) {
  if (h == 0) {
    return 0;
  } else if (strcmp(name, h->name) == 0) {
    return h;
  } else {
    return find_handler(name, h->next);
  }
}

Value* apply(Value* rator, ValueList* rands, int depth) {
  switch (rator->tag) {
  case ProcV: {
    VarList* params = rator->u.proc.params;
    Env* env = rator->u.proc.env;
    while (params && rands) {
      env = make_env(params->var, rands->value, env);
      params = params->next;
      rands = rands->next;
    }
    return eval(rator->u.proc.body, env, depth);
  }
  case FixV: {
    return apply(apply(rator->u.fix.fun, insert_value(rator, 0), depth), rands, depth);
  }
  default:
    printf("Error, attempt to call something that was not a function\n");
    print_value(rator);
    printf("\n");
    exit(-1); return 0;
  }
}

static char* dummy_var = "_";

ValueList* eval_list(TermList* e, Env* env, int depth) {
  if (e == 0) {
    return 0;
  } else {
    return insert_value(eval(e->term, env, depth),
			eval_list(e->next, env, depth));
  }
}

void print_env(Env* env) {
  if (env != 0) {
    printf("%s is ", env->var);
    print_value(env->val);
    if (env->rest != 0) {
      printf(", ");
      print_env(env->rest);
    }
  }
}

Env* eval_term_bindings(TermBindingList* l, Env* env, int depth) {
  if (l == 0) {
    return 0;
  } else {
    Value* rhs = eval(l->initializer, env, depth);
    Env* rest = eval_term_bindings(l->rest, env, depth);
    return make_env(l->field, rhs, rest);
  }
}

void print_spaces(int n) {
  for (int i = 0; i != n; ++i) {
    printf(" ");
  }
}

Value* eval(Term* e, Env* env, int depth) {
  printf("eval "); print_term(e); printf("\n");
  if (depth > 10000) {
    fprintf(stdout, "procedure call stack too large, terminating\n");
    exit(-1);
  }
  switch (e->tag) {
  case Char:
    return make_char_value(e->u._char);
  case String:
    return make_string_value(e->u.str);
  case UnitTerm:
    return make_unit_value();
  case Int:
    return make_int_value(e->u._int);
  case Bool:
    return make_bool_value(e->u._bool);
  case Var: {
    Value* val = lookup(e->u.var, env);
    if (val == 0) {
      printf("Undefined variable: %s\n", e->u.var);
      exit(-1);
      return 0;
    } else {
      return val;
    }
  }
  case Lam: {
    return make_procedure(e->u.lam.params, e->u.lam.body, env);
  }
  case App: {
    Value* rator = eval(e->u.app.rator, env, depth);
    ValueList* rands = eval_list(e->u.app.rands, env, depth);
    return apply(rator, rands, depth);
  }
  case Recursive: {
    Env* env2 = make_env(e->u.rec.var, 0, env);
    Value* v = eval(e->u.rec.body, env2, depth);
    env2->val = v;
    return v;
  }
  case Let: {
    Value* rhs = eval(e->u.let.rhs, env, depth);
    Env* env2 = make_env(e->u.let.var, rhs, env);
    return eval(e->u.let.body, env2, depth);
  }
  case Op: {
    ValueList* arg_vals = 0;
    for (TermList* args = e->u.op.args; args != 0; args = args->next) {
      Value* v = eval(args->term, env, depth);
      arg_vals = insert_value(v, arg_vals);
    }
    
    switch (e->u.op.tag) {
    case Read: {
      return make_char_value(getchar());
    }
    case Write: {
      Value* v = get_nth(arg_vals, 0);
      switch (v->tag) {
      case CharV:
	putchar(v->u._char);
	break;
      case IntV:
	printf("%d", v->u._int);
	break;
      case StringV:
	printf("%s", v->u.str);
	break;
      case BoolV:
	if (v->u._bool == 0) {
	  printf("false");
	} else {
	  printf("true");
	}
	break;
      default:
	printf("error, can't write ");
	print_value(v);
	exit(-1);
      }
      return make_unit_value();
    }
    case Neg: {
      Value* v = get_nth(arg_vals, 0);
      assert (arg_vals && v->tag == IntV);
      return make_int_value(- v->u._int);
    }
    case Len: {
      Value* v = get_nth(arg_vals, 0);
      assert (arg_vals && v->tag == StringV);
      return make_int_value(strlen(v->u.str));
    }
    case Not: {
      Value* v = get_nth(arg_vals, 0);
      assert (arg_vals && v->tag == BoolV);
      return make_bool_value(! v->u._bool);
    }
    case And: {
      Value* left = get_nth(arg_vals, 0);
      Value* right = get_nth(arg_vals, 1);
      assert(left->tag == BoolV && right->tag == BoolV);
      return make_bool_value(left->u._bool && right->u._bool);
    }
    case Or: {
      Value* left = get_nth(arg_vals, 0);
      Value* right = get_nth(arg_vals, 1);
      if (left->tag == BoolV && right->tag == BoolV) {
	return make_bool_value(left->u._bool || right->u._bool);
      } else if (left->tag == HandlerV && right->tag == HandlerV) {
	Handler* r = right->u.handler;
	Handler* l = left->u.handler;
	while (l != 0) {
	  Handler* c = l;
	  l = l->next;
	  r = make_handler(c->name, c->fun, r);
	}
	return make_handler_value(r);
      }
    }	
    case Equal: {
      Value* left = get_nth(arg_vals, 0);
      Value* right = get_nth(arg_vals, 1);
      if (left->tag == BoolV && right->tag == BoolV) {      
	return make_bool_value(left->u._bool == right->u._bool);
      } else if (left->tag == IntV && right->tag == IntV) {
	return make_bool_value(left->u._int == right->u._int);
      } else if (left->tag == CharV && right->tag == CharV) {
	return make_bool_value(left->u._char == right->u._char);
      } else if (left->tag == StringV && right->tag == StringV) {
	return make_bool_value(strcmp(left->u.str, right->u.str) == 0);
      }
    }
    case Add: {
      Value* left = get_nth(arg_vals, 0);
      Value* right = get_nth(arg_vals, 1);
      if (left->tag == IntV && right->tag == IntV) {      
	return make_int_value(left->u._int + right->u._int);
      } else if (left->tag == CharV && right->tag == CharV) {
	char* s = malloc(3 * sizeof(char));
	s[0] = left->u._char;
	s[1] = right->u._char;
	s[2] = 0;
	return make_string_value(s);
      } else if (left->tag == StringV && right->tag == StringV) {
	int ln = strlen(left->u.str);
	int rn = strlen(right->u.str);
	char* lr = malloc((ln + rn + 1) * sizeof(char));
	strcpy(lr, left->u.str);
	strcpy(lr + ln, right->u.str);
	return make_string_value(lr);
      } else if (left->tag == CharV && right->tag == StringV) {
	int rn = strlen(right->u.str);
	char* lr = malloc((1 + rn + 1) * sizeof(char));
	lr[0] = left->u._char;
	strcpy(lr + 1, right->u.str);
	return make_string_value(lr);
      } else if (left->tag == StringV && right->tag == CharV) {
	int ln = strlen(left->u.str);
	int rn = 1;
	char* lr = malloc((ln + rn + 1) * sizeof(char));
	strcpy(lr, left->u.str);
	lr[ln] = right->u._char;
	lr[ln+1] = 0;
	return make_string_value(lr);
      }
    }
    case Sub: {
      Value* left = get_nth(arg_vals, 0);
      Value* right = get_nth(arg_vals, 1);
      return make_int_value(left->u._int - right->u._int);
    }
    case Mul: {
      Value* left = get_nth(arg_vals, 0);
      Value* right = get_nth(arg_vals, 1);
      return make_int_value(left->u._int * right->u._int);
    }
    case Div: {
      Value* left = get_nth(arg_vals, 0);
      Value* right = get_nth(arg_vals, 1);
      return make_int_value(left->u._int / right->u._int);
    }
    case Mod: {
      Value* left = get_nth(arg_vals, 0);
      Value* right = get_nth(arg_vals, 1);
      return make_int_value(left->u._int % right->u._int);
    }
    case Less: {
      Value* left = get_nth(arg_vals, 0);
      Value* right = get_nth(arg_vals, 1);
      return make_bool_value(left->u._int < right->u._int);
    }
    default:
      fprintf(stderr, "%s:%d: Error: operator %s is not applicable to ",
	      input_filename, e->lineno, op_to_string(e->u.op.tag));
      print_value_list(arg_vals); printf("\n");
    }
    return 0;
  } // case Op

  case IfThen: {
    Value* cond = eval(e->u.ifthen.cond, env, depth);
    switch (cond->tag) {
    case BoolV:
      if (cond->u._bool == 0) {
	return eval(e->u.ifthen.els, env, depth);
      } else {
	return eval(e->u.ifthen.thn, env, depth);
      }
    default:
      printf("Error, expected a Boolean in test of if-then-else, not:\n");
      print_value(cond); exit(-1); return 0;
    } /* end case cond-tag */
  } /* end case IfThen */
  case Record: {
    TermBindingList* l = e->u.record.fields;
    Env* fields = eval_term_bindings(l, env, depth);
    return make_record_value(fields);
  } /* end Rec */
  case Field: {
    Value* rec = eval(e->u.field_access.rec, env, depth);
    if (rec->tag == RecordV) {
      Value* val = lookup(e->u.field_access.field, rec->u.record.fields);
      if (val == 0) {
	printf("field %s is not present in record:\n", e->u.field_access.field);
	print_value(rec); exit(-1); return 0;
      } else {
	return val;
      }
    } else {
      printf("expected a record on left side of period in field access expression, not:\n");
      print_value(rec); exit(-1); return 0;
    }
  } /* end Field */
  case FieldUpdate: {
    Value* rec = eval(e->u.field_update.rec, env, depth);
    Value* replacement = eval(e->u.field_update.replacement, env, depth);
    if (rec->tag == RecordV) {
      Env* fields = make_env(e->u.field_update.field, replacement, rec->u.record.fields);
      return make_record_value(fields);
    } else {
      printf("expected a record on left side of period in field access expression, not:\n");
      print_value(rec); exit(-1); return 0;
    }
  } /* end FieldUpdate */
  case Variant: {
    Value* val = eval(e->u.variant.init, env, depth);
    return make_variant_value(e->u.variant.name, val);
  }
  case HandlerTerm: {
    Value* fun = eval(e->u.handler.body, env, depth);
    return make_handler_value(make_handler(e->u.handler.var, fun, 0));
  }
  case Case: {
    Value* descr = eval(e->u._case.descr, env, depth);
    Value* handler = eval(e->u._case.handler, env, depth);
    switch (descr->tag) {
    case VariantV:
      switch (handler->tag) {
      case HandlerV: {
	Handler* h = find_handler(descr->u.variant.name, handler->u.handler);
	if (h == 0) {
	  printf("Error, could not find handler for %s\n", descr->u.variant.name);
	  exit(-1); return 0;
	}
	return apply(h->fun, insert_value(descr->u.variant.val, 0), depth);
      }
      default:
	printf("Error, expected handler for case expression, not:\n");
	print_value(descr); exit(-1); return 0;
      }
    default:
      printf("%s:%d: Error, expected variant in case expression, not:\n",
	     input_filename, e->lineno);
      print_value(descr); exit(-1); return 0;
    }
  } /* end Case */
  } /* end case e->tag */
}


void print_value(Value* v) {
  switch (v->tag) {
  case UnitV:
    printf("()");
    break;
  case IntV:
    printf("%d", v->u._int);
    break;
  case StringV:
    printf("\"%s\"", v->u.str);
    break;
  case CharV:
    printf("#%c", v->u._char);
    break;
  case BoolV:
    if (v->u._bool == 0) {
      printf("false");
    } else {
      printf("true");
    }
    break;
  case ProcV:
    printf("function");
    break;
  case FixV:
    printf("recursive function");
    break;
  case RecordV: {
    printf("record {");
    print_env(v->u.record.fields);
    printf("}");
    break;
  }
  case HandlerV:
    printf("handler");
    break;
  case VariantV:
    printf("(tag ");
    print_value(v->u.variant.val);
    printf(" as %s", v->u.variant.name);
    printf(")");
    break;
  default:
    printf("unknown value");
    break;
  }
}

char *read_file(FILE* fp)
{
    char *fcontent = NULL;
    int fsize = 0;

    if(fp) {
        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
        rewind(fp);

        fcontent = (char*) malloc(sizeof(char) * fsize);
        fread(fcontent, 1, fsize, fp);

        fclose(fp);
    }
    return fcontent;
}

int value_list_len(ValueList* l) {
  if (l == 0) {
    return 0;
  } else {
    return 1 + value_list_len(l->next);
  }
}

void print_value_list(ValueList* l) {
  if (l) {
    print_value(l->value);
    if (l->next) {
      printf(", ");
    }
    print_value_list(l->next);
  }
}
