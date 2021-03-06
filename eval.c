#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "eval.h"
#include "syntax.tab.h"

extern char* input_filename;
extern Term* program;

extern int yylex_init (yyscan_t* scanner);
void yyset_in  ( FILE * _in_str , yyscan_t yyscanner );
int yylex_destroy ( yyscan_t yyscanner );


int trace = 0;

void runtime_error(Term* e, char* message) {
  Value* record = variant_value(e);
  fprintf(stderr, "%s:%d: %s", input_filename, get_int(record_get("line", record)),
	  message);
  abort();
}

void internal_error(Term* e, char* message) {
  Value* record = variant_value(e);
  fprintf(stderr, "%s:%d: [internal] %s", input_filename,
	  get_int(record_get("line", record)),
	  message);
  abort();
}

void indent(int depth) {
  while (depth--)
    printf(" ");
}

Value* find_handler(char* name, Value* h, Term* e) {
  if (is_unit(h)) {
    return 0;
  } else if (is_list(h)) {
    Value* bind = head(h);
    if (is_list(bind)) {
      if (0 == strcmp(name, get_cstring(head(bind)))) {
	return tail(head(h));
      } else {
	return find_handler(name, tail(h), e);
      }
    } else {
      internal_error(e, "in find_handler, expected association list\n");
      return 0;
    }
  } else {
    internal_error(e, "in find_handler, expected association list\n");
    return 0;
  }
}

Value* apply(Value* rator, Value* rands, int depth, Term* e) {
  if (trace) {
    indent(depth); printf("{\n");
  }
  switch (rator->tag) {
  case ProcV: {
    Value* params = rator->u.proc.params;
    Env* env = rator->u.proc.env;
    while (is_list(params) && is_list(rands)) {
      env = make_env(get_cstring(head(params)), head(rands), env);
      params = tail(params);
      rands = tail(rands);
    }
    Value* result = eval(rator->u.proc.body, env, depth + 2);
    if (trace) {
      indent(depth); printf("}\n");
    }
    return result;
  }
  case FixV: {
    return apply(apply(rator->u.fix.fun, make_list(rator, make_unit()), depth, e),
		 rands, depth, e);
  }
  default:
    runtime_error(e, "call expected a function\n");
    return 0;
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

void write_value(Value* v, Term* e) {
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
    print_value(v);
    //internal_error(e, "error in write, unhandled value\n");
  }
}

char* value_to_string(Value* v, Term* e) {
  switch (v->tag) {
  case CharV: {
    char* string = malloc(sizeof(char) * 2);
    string[0] = v->u._char;
    string[1] = 0;
    return string;
  }
  case IntV: {
    char* string = malloc(sizeof(char) * 100);
    snprintf(string, 100, "%d", v->u._int);
    return string;
  }
  case StringV:
    return v->u.str;
  case BoolV: {
    char* string = malloc(sizeof(char) * 100);
    if (v->u._bool == 0) {
      snprintf(string, 100, "false");
    } else {
      snprintf(string, 100, "true");
    }
    return string;
  }
  default:
    internal_error(e, "error in write, unhandled value\n");
    return 0;
  }
}

Value* eval_op(Term* e, Env* env, int depth) {
  Value* arg_vals_rev = make_unit();
  // evaluate the arguments
  for (Value* args = record_get("arguments", variant_value(e)); ! is_unit(args); args = tail(args)) {
    Value* v = eval(head(args), env, depth + 1);
    arg_vals_rev = make_list(v, arg_vals_rev);
  }
  // reverse the argument list back to normal
  Value* arg_vals = make_unit();
  for (Value* args = arg_vals_rev; ! is_unit(args); args = tail(args)) {
    arg_vals = make_list(head(args), arg_vals);
  }
  char* operator = get_cstring(record_get("operator", variant_value(e)));
  if (0 == strcmp("read", operator)) {
    return make_char(getchar());
  } else if (0 == strcmp("write", operator)) {
    Value* argument = get_nth(arg_vals, 0);
    write_value(argument, e);
    return make_unit();
  } else if (0 == strcmp("string_of", operator)) {
    Value* argument = get_nth(arg_vals, 0);
    return make_string(value_to_string(argument, e));
  } else if (0 == strcmp("parse", operator)) {
    yyscan_t scanner;
    yylex_init(&scanner);
    Value* argument = get_nth(arg_vals, 0);
    char* file_name = get_cstring(argument);
    FILE* file = fopen(file_name, "r");
    if (!file) {
      char message[1000];
      snprintf(message, 1000, "could not open file \"%s\"\n", file_name);
      runtime_error(e, message);
    }
    yyset_in(file, scanner);
    yyparse(scanner);
    yylex_destroy(scanner);
    return program;
  } else if (0 == strcmp("neg", operator)) {
    Value* v = get_nth(arg_vals, 0);
    assert (value_list_len(arg_vals) == 1 && is_int(v));
    return make_int(- get_int(v));
  } else if (0 == strcmp("length", operator)) {
    Value* v = get_nth(arg_vals, 0);
    assert (value_list_len(arg_vals) == 1 && is_string(v));
    return make_int(strlen(get_cstring(v)));
  } else if (0 == strcmp("not", operator)) {
    Value* v = get_nth(arg_vals, 0);
    assert (value_list_len(arg_vals) == 1 && is_bool(v));
    return make_bool(! v->u._bool);
 } else if (0 == strcmp("and", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    assert(is_bool(left) && is_bool(right));
    return make_bool(get_bool(left) && get_bool(right));
  } else if (0 == strcmp("or", operator)) {
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
  } else if (0 == strcmp("equal", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    if (is_bool(left) && is_bool(right)) {      
      return make_bool(get_bool(left) == get_bool(right));
    } else if (is_int(left) && is_int(right)) {
      return make_bool(get_int(left) == get_int(right));
    } else if (is_char(left) && is_char(right)) {
      return make_bool(get_char(left) == get_char(right));
    } else if (is_string(left) && is_string(right)) {
      return make_bool(0 == strcmp(get_cstring(left), get_cstring(right)));
    }
  } else if (0 == strcmp("add", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    if (is_int(left) && is_int(right)) {      
      return make_int(get_int(left) + get_int(right));
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
      strcpy(lr, get_cstring(left));
      strcpy(lr + ln, get_cstring(right));
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
      runtime_error(e, "invalid argument in addition\n");
    }
  } else if (0 == strcmp("sub", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    if (is_int(left) && is_int(right)) {      
      return make_int(get_int(left) - get_int(right));
    } else {
      runtime_error(e, "invalid argument in subtraction\n");
    }
  } else if (0 == strcmp("mul", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    if (is_int(left) && is_int(right)) {      
      return make_int(get_int(left) * get_int(right));
    } else {
      runtime_error(e, "invalid argument in multiplication\n");
    }
  } else if (0 == strcmp("div", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    if (is_int(left) && is_int(right)) {      
      return make_int(get_int(left) / get_int(right));
    } else {
      runtime_error(e, "invalid argument in division\n");
    }
  } else if (0 == strcmp("mod", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    if (is_int(left) && is_int(right)) {
      return make_int(get_int(left) % get_int(right));
    } else {
      runtime_error(e, "invalid argument in modulo\n");
    }
  } else if (0 == strcmp("less", operator)) {
    Value* left = get_nth(arg_vals, 0);
    Value* right = get_nth(arg_vals, 1);
    if (is_int(left) && is_int(right)) {
      return make_bool(get_int(left) < get_int(right));
    } else {
      runtime_error(e, "invalid argument in less than\n");
      exit(-1);
    }
  } else if (0 == strcmp("head", operator)) {
    Value* arg = get_nth(arg_vals, 0);
    if (is_list(arg)) {
      return head(arg);
    } else {
      runtime_error(e, "head expected a list argument");
    }
  } else if (0 == strcmp("tail", operator)) {
    Value* arg = get_nth(arg_vals, 0);
    if (is_list(arg)) {
      return tail(arg);
    } else {
      runtime_error(e, "tail expected a list argument");
    }
  } else if (0 == strcmp("make_list", operator)) {
    return make_list(get_nth(arg_vals, 0), get_nth(arg_vals, 1));
  } else {
    char message[1000];
    snprintf(message, 1000, "error in operator %s\n", operator);
    runtime_error(e, message);
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
    runtime_error(e, "procedure call stack too large, terminating\n");
  }
  if (0 == strcmp("char", variant_name(e))) {
    result = record_get("char", variant_value(e));
  } else if (0 == strcmp("string", variant_name(e))) {
    result = record_get("string", variant_value(e));
  } else if (0 == strcmp("unit", variant_name(e))) {
    result = make_unit();
  } else if (0 == strcmp("int", variant_name(e))) {
    result = record_get("integer", variant_value(e));
  } else if (0 == strcmp("bool", variant_name(e))) {
    result = record_get("boolean", variant_value(e));
  } else if (0 == strcmp("var", variant_name(e))) {
    char* name = get_cstring(record_get("name", variant_value(e)));
    if (trace) { indent(depth); printf("variable %s\n", name); }
    Value* val = lookup(name, env);
    if (val) {
      if (trace) { indent(depth); printf("finished lookup\n"); print_value(val); }
      result = val;
    } else {
      if (trace) { indent(depth); printf("error in lookup\n"); }
      char message[1000];
      snprintf(message, 1000, "use of undefined variable: %s\n", name);
      runtime_error(e, message);
    }
  } else if (0 == strcmp("lambda", variant_name(e))) {
    result = make_procedure(record_get("params", variant_value(e)),
			    record_get("body", variant_value(e)),
			    env);
  } else if (0 == strcmp("application", variant_name(e))) {
    if (trace) {
      indent(depth); printf("starting application\n");
    }
    Value* rator = eval(record_get("rator", variant_value(e)), env, depth + 1);
    if (trace) {
      indent(depth); printf("finished rator\n");
    }
    Value* rands = eval_list(record_get("rands", variant_value(e)), env, depth + 1);
    if (trace) {
      indent(depth); printf("finished rands\n");
    }
    result = apply(rator, rands, depth, e);
  } else if (0 == strcmp("recursive", variant_name(e))) {
    Env* env2 = make_env(get_cstring(record_get("var", variant_value(e))), 0, env);
    Value* v = eval(record_get("body", variant_value(e)), env2, depth + 1);
    set_tail(head(env2), v);
    result = v;
  } else if (0 == strcmp("let", variant_name(e))) {
    Value* rhs = eval(record_get("rhs", variant_value(e)), env, depth + 1);
    Env* env2 = make_env(get_cstring(record_get("var", variant_value(e))), rhs, env);
    result = eval(record_get("body", variant_value(e)), env2, depth + 1);
  } else if (0 == strcmp("op", variant_name(e))) {
    result = eval_op(e, env, depth);
  } else if (0 == strcmp("ifthen", variant_name(e))) {
    Value* cond = eval(record_get("cond", variant_value(e)), env, depth + 1);
    if (is_bool(cond)) {
      if (get_bool(cond)) {
	result = eval(record_get("then", variant_value(e)), env, depth + 1);
      } else {
	result = eval(record_get("else", variant_value(e)), env, depth + 1);
      }
    } else {
      runtime_error(e, "expected a Boolean in test of if-then-else\n");
    }
  } else if (0 == strcmp("record", variant_name(e))) {
    Value* l = record_get("fields", variant_value(e));
    Env* fields = eval_term_bindings(l, env, depth);
    result = make_record(fields);
  } else if (0 == strcmp("get_field", variant_name(e))) {
    Value* rec = eval(record_get("record", variant_value(e)), env, depth + 1);
    if (is_record(rec)) {
      char* field = get_cstring(record_get("field", variant_value(e)));
      Value* val = lookup(field, rec->u.record.fields);
      if (val) {
	result = val;
      } else {
	char message[1000];
	snprintf(message, 1000, "field %s is not present in record:\n", field);
	runtime_error(e, message);
      }
    } else {
      runtime_error(e, "expected a record  in field access\n");
    }
  } else if (0 == strcmp("set_field", variant_name(e))) {
    Value* rec = eval(record_get("record", variant_value(e)), env, depth + 1);
    Value* replacement = eval(record_get("replacement", variant_value(e)), env, depth + 1);
    if (is_record(rec)) {
      char* field = get_cstring(record_get("field", variant_value(e)));
      Env* fields = make_env(field, replacement, rec->u.record.fields);
      result = make_record(fields);
    } else {
      runtime_error(e, "expected a record in field access\n");
    }
  } else if (0 == strcmp("variant", variant_name(e))) {
    Value* val = eval(record_get("init", variant_value(e)), env, depth + 1);
    char* name = get_cstring(record_get("name", variant_value(e)));
    result = make_variant(name, val);
  } else if (0 == strcmp("handler", variant_name(e))) {
    Value* fun = eval(record_get("body", variant_value(e)), env, depth + 1);
    result = make_handler(make_list(make_list(record_get("name", variant_value(e)), fun),
				    make_unit()));
  } else if (0 == strcmp("case", variant_name(e))) {
    Value* descr = eval(record_get("descr", variant_value(e)), env, depth + 1);
    Value* handler = eval(record_get("handler", variant_value(e)), env, depth + 1);
    switch (descr->tag) {
    case VariantV:
      switch (handler->tag) {
      case HandlerV: {
	Value* h = find_handler(variant_name(descr), handler->u.handler, e);
	if (h == 0) {
	  char message[1000];
	  snprintf(message, 1000, "could not find handler for %s\n",
		   variant_name(descr));
	  runtime_error(e, message);
	}
	result = apply(h, make_list(variant_value(descr), make_unit()), depth, e);
	break;
      }
      default:
	runtime_error(e, "expected handler in case\n");
      }
      break;
    default:
      runtime_error(e, "expected variant in case\n");
    }
  } else {
    runtime_error(e, "in eval, unrecognized expression\n");
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

