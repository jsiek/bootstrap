#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "eval.h"

extern char* input_filename;

int trace = 0;

Value* find_handler(char* name, Value* h) {
  if (is_unit(h)) {
    return 0;
  } else if (strcmp(name, get_cstring(head(head(h)))) == 0) {
    return tail(head(h));
  } else {
    return find_handler(name, tail(h));
  }
}

Value* apply(Value* rator, Value* rands, int depth) {
  switch (rator->tag) {
  case ProcV: {
    Value* params = rator->u.proc.params;
    Env* env = rator->u.proc.env;
    while (params && rands) {
      env = make_env(get_cstring(head(params)), head(rands), env);
      params = tail(params);
      rands = tail(rands);
    }
    return eval(rator->u.proc.body, env, depth + 1);
  }
  case FixV: {
    return apply(apply(rator->u.fix.fun, make_list(rator, make_unit()), depth), rands, depth);
  }
  default:
    printf("Error, attempt to call something that was not a function\n");
    print_value(rator);
    printf("\n");
    exit(-1); return 0;
  }
}

static char* dummy_var = "_";

Value* eval_list(Value* e, Env* env, int depth) {
  if (is_unit(e)) {
    return make_unit();
  } else {
    return make_list(eval(head(e), env, depth + 1),
		     eval_list(tail(e), env, depth));
  }
}

Env* eval_term_bindings(Value* l, Env* env, int depth) {
  if (is_unit(l)) {
    return make_unit();
  } else {
    Value* rhs = eval(tail(head(l)), env, depth + 1);
    Env* rest = eval_term_bindings(tail(l), env, depth);
    return make_env(get_cstring(head(head(l))), rhs, rest);
  }
}

void print_spaces(int n) {
  for (int i = 0; i != n; ++i) {
    printf(" ");
  }
}

void indent(int depth) {
  while (depth--)
    printf(" ");
}

void write_value(Value* v) {
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
    printf("\n");
    exit(-1);
  }
}

Value* eval_op(Term* e, Env* env, int depth) {
  Value* arg_vals = 0;
  for (Value* args = record_get("arguments", variant_value(e)); ! is_unit(args); args = tail(args)) {
    Value* v = eval(head(args), env, depth + 1);
    arg_vals = make_list(v, arg_vals);
  }
  char* operator = get_cstring(record_get("operator", variant_value(e)));
  if (strcmp("read", operator)) {
    return make_char(getchar());
  } else if (strcmp("write", operator)) {
    Value* v = get_nth(arg_vals, 0);
    write_value(v);
    return make_unit();
  } else if (strcmp("neg", operator)) {
    Value* v = get_nth(arg_vals, 0);
    assert (value_list_len(arg_vals) == 1 && is_int(v));
    return make_int(- get_int(v));
  } else if (strcmp("length", operator)) {
    Value* v = get_nth(arg_vals, 0);
    assert (value_list_len(arg_vals) == 1 && is_string(v));
    return make_int(strlen(get_cstring(v)));
  } else if (strcmp("not", operator)) {
    Value* v = get_nth(arg_vals, 0);
    assert (value_list_len(arg_vals) == 1 && is_bool(v));
    return make_bool(! v->u._bool);
  } else if (strcmp("and", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    assert(is_bool(left) && is_bool(right));
    return make_bool(get_bool(left) && get_bool(right));
  } else if (strcmp("or", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    if (is_bool(left) && is_bool(right)) {
      return make_bool(get_bool(left) || get_bool(right));
    } else if (is_handler(left) && is_handler(right)) {
      Value* r = get_handler(right);
      Value* l = get_handler(left);
      while (! is_unit(l)) {
	Value* c = l;
	l = tail(l);
	r = make_list(head(c), r);
      }
      return make_handler(r);
    }
  } else if (strcmp("equal", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    if (is_bool(left) && is_bool(right)) {      
      return make_bool(get_bool(left) == get_bool(right));
    } else if (is_int(left) && is_int(right)) {
      return make_bool(get_int(left) == get_int(right));
    } else if (is_char(left) && is_char(right)) {
      return make_bool(get_char(left) == get_char(right));
    } else if (is_string(left) && is_string(right)) {
      return make_bool(strcmp(get_cstring(left), get_cstring(right)) == 0);
    }
  } else if (strcmp("add", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    if (is_int(left) && is_int(right)) {      
      return make_int(is_int(left) + is_int(right));
    } else if (is_char(left) && is_char(right)) {
      char* s = malloc(3 * sizeof(char));
      s[0] = get_char(left);
      s[1] = get_char(right);
      s[2] = 0;
      return make_string(s);
    } else if (is_string(left) && is_string(right)) {
      int ln = strlen(get_cstring(left));
      int rn = strlen(get_cstring(right));
      char* lr = malloc((ln + rn + 1) * sizeof(char));
      strcpy(lr, left->u.str);
      strcpy(lr + ln, right->u.str);
      return make_string(lr);
    } else if (is_char(left) && is_string(right)) {
      int rn = strlen(right->u.str);
      char* lr = malloc((1 + rn + 1) * sizeof(char));
      lr[0] = get_char(left);
      strcpy(lr + 1, get_cstring(right));
      return make_string(lr);
    } else if (is_string(left) && is_char(right)) {
      int ln = strlen(left->u.str);
      int rn = 1;
      char* lr = malloc((ln + rn + 1) * sizeof(char));
      strcpy(lr, get_cstring(left));
      lr[ln] = get_char(right);
      lr[ln+1] = 0;
      return make_string(lr);
    } else {
      printf("error in addition\n");
      exit(-1);
    }
  } else if (strcmp("sub", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    if (is_int(left) && is_int(right)) {      
      return make_int(get_int(left) - get_int(right));
    } else {
      printf("error in subtraction\n");
      exit(-1);
    }
  } else if (strcmp("mul", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    if (is_int(left) && is_int(right)) {      
      return make_int(get_int(left) * get_int(right));
    } else {
      printf("error in multiplication\n");
      exit(-1);
    }
  } else if (strcmp("div", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    if (is_int(left) && is_int(right)) {      
      return make_int(get_int(left) / get_int(right));
    } else {
      printf("error in division\n");
      exit(-1);
    }
  } else if (strcmp("mod", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    if (is_int(left) && is_int(right)) {
      return make_int(get_int(left) % get_int(right));
    } else {
      printf("error in modulo\n");
      exit(-1);
    }
  } else if (strcmp("less", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    if (is_int(left) && is_int(right)) {
      return make_bool(get_int(left) < get_int(right));
    } else {
      printf("error in less than\n");
      exit(-1);
    }
  } else if (strcmp("head", operator)) {
    return head(get_nth(arg_vals, 0));
  } else if (strcmp("tail", operator)) {
    return tail(get_nth(arg_vals, 0));
  } else if (strcmp("make_list", operator)) {
    return make_list(get_nth(arg_vals, 0), get_nth(arg_vals, 1));
  } else {
    fprintf(stderr, "%s:%d: Error: operator %s is not applicable to ",
	    input_filename, get_int(record_get("line", variant_value(e))), operator);
    print_value_list(arg_vals); printf("\n");
    exit(-1);
  }
  assert(0);
  return 0;
}

Value* eval(Term* e, Env* env, int depth) {
  if (trace) {
    indent(depth); printf("eval "); print_term(e); printf("\n");
  }
  Value* result = make_unit();
  if (depth > 10000) {
    fprintf(stdout, "procedure call stack too large, terminating\n");
    exit(-1);
  }
  if (is_char(e)) {
    result = e;
  } else if (is_string(e)) {
    result = e;
  } else if (is_unit(e)) {
    result = e;
  } else if (is_int(e)) {
    result = e;
  } else if (is_bool(e)) {
    result = e;
  } else if (strcmp("var", variant_name(e))) {
    char* name = get_cstring(variant_value(e));
    Value* val = lookup(name, env);
    if (val == 0) {
      printf("Undefined variable: %s\n", name);
      exit(-1);
    } else {
      result = val;
    }
  } else if (strcmp("lamba", variant_name(e))) {
    result = make_procedure(record_get("params", variant_value(e)),
			    record_get("body", variant_value(e)),
			    env);
  } else if (strcmp("application", variant_name(e))) {
    Value* rator = eval(record_get("rator", variant_value(e)), env, depth + 1);
    Value* rands = eval_list(record_get("rands", variant_value(e)), env, depth + 1);
    result = apply(rator, rands, depth);
  } else if (strcmp("recursive", variant_name(e))) {
    Env* env2 = make_env(get_cstring(record_get("var", variant_value(e))), 0, env);
    Value* v = eval(record_get("body", variant_value(e)), env2, depth + 1);
    set_tail(head(env2), v);
    result = v;
  } else if (strcmp("let", variant_name(e))) {
    Value* rhs = eval(record_get("rhs", variant_value(e)), env, depth + 1);
    Env* env2 = make_env(get_cstring(record_get("var", variant_value(e))), rhs, env);
    result = eval(record_get("body", variant_value(e)), env2, depth + 1);
  } else if (strcmp("op", variant_name(e))) {
    result = eval_op(e, env, depth);
  } else if (strcmp("ifthen", variant_name(e))) {
    Value* cond = eval(record_get("cond", variant_value(e)), env, depth + 1);
    if (is_bool(cond)) {
      if (get_bool(cond)) {
	result = eval(record_get("then", variant_value(e)), env, depth + 1);
      } else {
	result = eval(record_get("else", variant_value(e)), env, depth + 1);
      }
    } else {
      printf("Error, expected a Boolean in test of if-then-else, not:\n");
      print_value(cond);
      printf("\n");
      exit(-1);
    }
  } else if (strcmp("record", variant_name(e))) {
    Value* l = record_get("fields", variant_value(e));
    Env* fields = eval_term_bindings(l, env, depth);
    result = make_record(fields);
  } else if (strcmp("get_field", variant_name(e))) {
    Value* rec = eval(record_get("record", variant_value(e)), env, depth + 1);
    if (is_record(rec)) {
      char* field = get_cstring(record_get("field", variant_value(e)));
      Value* val = lookup(field, rec->u.record.fields);
      if (val == 0) {
	printf("field %s is not present in record:\n", field);
	print_value(rec);
	printf("\n");
	exit(-1);
      } else {
	result = val;
      }
    } else {
      printf("expected a record on left side of period in field access expression, not:\n");
      print_value(rec);
      printf("\n");
      exit(-1); 
    }
  } else if (strcmp("set_field", variant_name(e))) {
    Value* rec = eval(record_get("record", variant_value(e)), env, depth + 1);
    Value* replacement = eval(record_get("replacement", variant_value(e)), env, depth + 1);
    if (is_record(rec)) {
      char* field = get_cstring(record_get("field", variant_value(e)));
      Env* fields = make_env(field, replacement, rec->u.record.fields);
      result = make_record(fields);
    } else {
      printf("expected a record on left side of period in field access expression, not:\n");
      print_value(rec);
      printf("\n");
      exit(-1); 
    }
  } else if (strcmp("variant", variant_name(e))) {
    Value* val = eval(record_get("init", variant_value(e)), env, depth + 1);
    char* name = get_cstring(record_get("name", variant_value(e)));
    result = make_variant(name, val);
  } else if (strcmp("handler", variant_name(e))) {
    Value* fun = eval(record_get("body", variant_value(e)), env, depth + 1);
    result = make_handler(make_list(make_list(record_get("name", variant_value(e)), fun),
				    make_unit()));
  } else if (strcmp("case", variant_name(e))) {
    Value* descr = eval(record_get("descr", variant_value(e)), env, depth + 1);
    Value* handler = eval(record_get("handler", variant_value(e)), env, depth + 1);
    switch (descr->tag) {
    case VariantV:
      switch (handler->tag) {
      case HandlerV: {
	Value* h = find_handler(descr->u.variant.name, handler->u.handler);
	if (h == 0) {
	  printf("Error, could not find handler for %s\n", descr->u.variant.name);
	  exit(-1);
	}
	result = apply(h, make_list(descr->u.variant.val, make_unit()), depth);
	break;
      }
      default:
	printf("Error, expected handler for case expression, not:\n");
	print_value(descr);
	printf("\n");
	exit(-1); 
      }
      break;
    default:
      printf("%s:%d: Error, expected variant in case expression, not:\n",
	     input_filename, get_int(record_get("line", variant_value(e))));
      print_value(descr);
      printf("\n");
      exit(-1); 
    }
  } else {
    printf("eval unrecognized expression\n");
    print_term(e);
    exit(-1);
  }
  if (trace) {
    indent(depth);
    printf("=> ");
    print_value(result);
    printf("\n");
  }
  return result;
} /* eval */


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

