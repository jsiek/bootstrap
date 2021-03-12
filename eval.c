#include <string.h>
#include <stdio.h>
#include "eval.h"

extern char* input_filename;

ValueList* insert_value(Value* v, ValueList* next) {
  ValueList* l = malloc(sizeof(ValueList));
  l->value = v;
  l->next = next;
  return l;
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

Value* make_array_value(ValueList* inits) {
  Value* t = malloc(sizeof(Value));
  t->tag = ArrayV; 
  int n = value_list_len(inits);
  t->u.array.len = n;
  t->u.array.data = malloc(n * sizeof(Value));
  for (int i = 0; i != n; ++i) {
    t->u.array.data[i] = inits->value;
    inits = inits->next;
  }
  return t;
}

Value* make_procedure(TypeEnv* params, Term* body, Env* env) {
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
    TypeEnv* params = rator->u.proc.params;
    Env* env = rator->u.proc.env;
    while (params && rands) {
      env = make_env(params->var, rands->value, env);
      params = params->rest;
      rands = rands->next;
    }
    return eval(rator->u.proc.body, env, depth);
  }
  case FixV: {
    return apply(apply(rator->u.fix.fun, insert_value(rator, 0), depth), rands, depth);
  }
  default:
    printf("Error, attempt to call something that was not a function\n");
    print_value(rator); exit(-1); return 0;
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
  case Ascribe: {
    return eval(e->u.ascribe.term, env, depth);
  }
  case Trace: {
    print_spaces(2 * depth);
    printf("start %s(", e->u.trace.label);
    ValueList* l = eval_list(e->u.trace.inputs, env, depth);
    print_value_list(l);
    printf("):\n");
    Value* v = eval(e->u.trace.term, env, depth + 1);
    /*
    printf("return %s: ", e->u.trace.label);
    print_value(v);
    printf("\n");
    */
    print_spaces(2 * depth);
    printf("end %s\n", e->u.trace.label); 
    return v;
  }
  case Array: {
    ValueList* vs = eval_list(e->u.array.inits, env, depth);
    return make_array_value(vs);
  }
  case ArrayComp: {
    Value* size = eval(e->u.array_comp.size, env, depth);
    Value* init_fun = eval(e->u.array_comp.init_fun, env, depth);
    if (size->tag == IntT) {
      int n = size->u._int;
      Value* array = malloc(sizeof(Value));
      array->tag = ArrayV; 
      array->u.array.len = n;
      array->u.array.data = malloc(n * sizeof(Value));
      for (int i = 0; i != n; ++i) {
	Value* index = make_int_value(i);
	array->u.array.data[i] = apply(init_fun, insert_value(index, 0), depth);
      }
      return array;
    } else {
      printf("Error, expected integer size for array comprehension\n");
      exit(-1);
    }

  }
  case Index: {
    Value* array = eval(e->u.index.array, env, depth);
    Value* ind = eval(e->u.index.i, env, depth);
    switch (array->tag) {
    case ArrayV: {
      switch (ind->tag) {
      case IntV: {
	int n = array->u.array.len;
	int i = ind->u._int;
	if (i < n) {
	  return array->u.array.data[i];
	} else {
	  fprintf(stdout, "%s:%d:Error, attempt to index %d beyond end of array %d\n", 
		  input_filename, e->lineno, i, n);
	}
      }
      default:
	printf("Error, expected integer index in index expression\n");
	exit(-1);
      } /* switch (ind->tag) */
    }
    case StringV: {
      switch (ind->tag) {
      case IntV: {
	int n = strlen(array->u.str);
	int i = ind->u._int;
	if (i < n) {
	  return make_char_value(array->u.str[i]);
	} else {
	  printf("Error, attempt to index beyond end of string\n");
	  exit(-1);
	}
      }
      default:
	printf("Error, expected integer index in index expression\n");
      } /* switch (ind->tag) */
    }
    default:
      fprintf(stdout, "%s:%d: Error, expected a string or array in index expression\n",
	     input_filename, e->lineno);
      exit(-1);
    }
  }
  case Generic:
    /*return make_procedure(0, e->u.generic.body, env);*/
    return eval(e->u.generic.body, env, depth);
  case Inst: {
    Value* poly = eval(e->u.inst.poly, env, depth);
    /*return apply(poly, 0);*/
    return poly;
  }
  case Fold:
    return eval(e->u.fold.body, env, depth);
  case Unfold:
    return eval(e->u.unfold.body, env, depth);
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
  case Alias: {
    return eval(e->u.alias.body, env, depth);
  }
  case UniOp: {
    Value* v = eval(e->u.uniop.expr, env, depth);
    if (v->tag == IntV && e->u.uniop.tag == Neg) {
      return make_int_value(- v->u._int);
    } else if (v->tag == StringV && e->u.uniop.tag == Len) {
      return make_int_value(strlen(v->u.str));
    } else if (v->tag == ArrayV && e->u.uniop.tag == Len) {
      return make_int_value(v->u.array.len);
    } else if (v->tag == BoolV && e->u.uniop.tag == Not) {
      return make_bool_value(! v->u._bool);
    } else {
      fprintf(stderr, "%s:%d: Error: operator %s is not applicable to ",
	      input_filename, e->lineno, uniop_to_string(e->u.uniop.tag));
      print_value(v); printf("\n");
    }
    return 0;
  }
  case BinOp: {
    Value* left = eval(e->u.binop.left, env, depth);
    Value* right = eval(e->u.binop.right, env, depth);
    if (left->tag == BoolV && right->tag == BoolV) {
      switch (e->u.binop.tag) {
      case And:
	return make_bool_value(left->u._bool && right->u._bool);
      case Or:
	return make_bool_value(left->u._bool || right->u._bool);
      case Equal:
	return make_bool_value(left->u._bool == right->u._bool);
      default:
	printf("Error, operator '%s' not applicable to Booleans\n",
	       binop_to_string(e->u.binop.tag));
	exit(-1);
      }
    } else if (left->tag == IntV && right->tag == IntV) {
      switch (e->u.binop.tag) {
      case Add:
	return make_int_value(left->u._int + right->u._int);
      case Sub:
	return make_int_value(left->u._int - right->u._int);
      case Mul:
	return make_int_value(left->u._int * right->u._int);
      case Div:
	return make_int_value(left->u._int / right->u._int);
      case Mod:
	return make_int_value(left->u._int % right->u._int);
      case Equal:
	return make_bool_value(left->u._int == right->u._int);
      case Less:
	return make_bool_value(left->u._int < right->u._int);
      default:
	printf("Error, operator '%s' not applicable to integers\n",
	       binop_to_string(e->u.binop.tag));
	exit(-1);
      } /* end switch */
    } else if (left->tag == CharV && right->tag == CharV) {
      switch (e->u.binop.tag) {
      case Equal:
	return make_bool_value(left->u._char == right->u._char);
      case Add: {
	char* s = malloc(3 * sizeof(char));
	s[0] = left->u._char;
	s[1] = right->u._char;
	s[2] = 0;
	return make_string_value(s);
      }
      default:
	printf("Error, operator not applicable to characters\n");
	exit(-1);
      }
    } else if (left->tag == StringV && right->tag == StringV) {
      switch (e->u.binop.tag) {
      case Equal:
	return make_bool_value(strcmp(left->u.str, right->u.str) == 0);
      case Add: {
	int ln = strlen(left->u.str);
	int rn = strlen(right->u.str);
	char* lr = malloc((ln + rn + 1) * sizeof(char));
	strcpy(lr, left->u.str);
	strcpy(lr + ln, right->u.str);
	return make_string_value(lr);
      }
      default:
	printf("Error, operator not applicable to strings\n");
	exit(-1);
      } /* end switch */

    } else if (left->tag == CharV && right->tag == StringV) {
      switch (e->u.binop.tag) {
      case Add: {
	int rn = strlen(right->u.str);
	char* lr = malloc((1 + rn + 1) * sizeof(char));
	lr[0] = left->u._char;
	strcpy(lr + 1, right->u.str);
	return make_string_value(lr);
      }
      default:
	printf("Error, operator not applicable to strings\n");
	exit(-1);
      } /* end switch */

    } else if (left->tag == StringV && right->tag == CharV) {
      switch (e->u.binop.tag) {
      case Equal:
	return make_bool_value(strcmp(left->u.str, right->u.str) == 0);
      case Add: {
	int ln = strlen(left->u.str);
	int rn = 1;
	char* lr = malloc((ln + rn + 1) * sizeof(char));
	strcpy(lr, left->u.str);
	lr[ln] = right->u._char;
	lr[ln+1] = 0;
	return make_string_value(lr);
      }
      default:
	printf("Error, operator not applicable to strings\n");
	exit(-1);
      } /* end switch */

    } else if (left->tag == HandlerV && right->tag == HandlerV) {
      switch (e->u.binop.tag) {
      case Or: {
	Handler* r = right->u.handler;
	Handler* l = left->u.handler;
	while (l != 0) {
	  Handler* c = l;
	  l = l->next;
	  r = make_handler(c->name, c->fun, r);
	}
	return make_handler_value(r);
      }
      default:
	fprintf(stdout, "Error, operator not applicable to handlers\n");
	exit(-1);
      } /* end switch */

    } else {
      printf("Error, unexpected input values to operator '%s'\n",
	     binop_to_string(e->u.binop.tag));
      exit(-1); return 0;
    } /* end if */

  } /* end case BinOp */
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
    return make_handler_value(make_handler(e->u.handler.name, fun, 0));
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
  case ArrayV:
    printf("[");
    for (int i = 0; i != v->u.array.len; ++i) {
      print_value(v->u.array.data[i]);
      if (i + 1 != v->u.array.len) {
	printf(", ");
      }
    }
    printf("]");
    break;
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
    printf("(group ");
    print_env(v->u.record.fields);
    printf(")");
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
