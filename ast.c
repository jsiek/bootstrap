#include <stdio.h>
#include <string.h>
#include "ast.h"


/* Term Constructors */

Term* make_var(int lineno, char* x) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("name", make_string(x), rec);
  return make_variant("var", rec);
}

Term* make_unit_term(int lineno) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  return make_variant("unit", rec);
}

Term* make_int_term(int lineno, int n) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("integer", make_int(n), rec);
  return make_variant("int", rec);
}

Term* make_bool_term(int lineno, int b) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("boolean", make_bool(b), rec);
  return make_variant("bool", rec);
}

Term* make_string_term(int lineno, char* str) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("string", make_string(str), rec);
  return make_variant("string", rec);
}

Term* make_char_term(int lineno, char c) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("char", make_char(c), rec);
  return make_variant("char", rec);
}

Term* make_op(int lineno, char* op, Value* args) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("operator", make_string(op), rec);
  rec = record_set("arguments", args, rec);
  return make_variant("op", rec);
}

Term* make_ifthen(int lineno, Term* cond, Term* thn, Term* els) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("cond", cond, rec);
  rec = record_set("then", thn, rec);
  rec = record_set("else", els, rec);
  return make_variant("ifthen", rec);
}

Term* make_lambda(int lineno, Value* params, Term* body) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("params", params, rec);
  rec = record_set("body", body, rec);
  return make_variant("lambda", rec);
}

Term* make_app(int lineno, Term* rator, Value* rands) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("rator", rator, rec);
  rec = record_set("rands", rands, rec);
  return make_variant("application", rec);
}

Term* make_recursive(int lineno, char* var, Term* body) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("var", make_string(var), rec);
  rec = record_set("body", body, rec);
  return make_variant("recursive", rec);
}

Term* make_let(int lineno, char* var, Term* rhs, Term* body) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("var", make_string(var), rec);
  rec = record_set("rhs", rhs, rec);
  rec = record_set("body", body, rec);
  return make_variant("let", rec);
}

Term* make_record_term(int lineno, Value* fields) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("fields", fields, rec);
  return make_variant("record", rec);
}

Term* make_field_access(int lineno, Term* record, char* field) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("record", record, rec);
  rec = record_set("field", make_string(field), rec);
  return make_variant("get_field", rec);
}

Term* make_field_update(int lineno, Term* record, char* field, Term* replacement) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("record", record, rec);
  rec = record_set("field", make_string(field), rec);
  rec = record_set("replacement", replacement, rec);
  return make_variant("set_field", rec);
}

Term* make_case(int lineno, Term* descr, Term* handler) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("descr", descr, rec);
  rec = record_set("handler", handler, rec);
  return make_variant("case", rec);
}

Term* make_variant_term(int lineno, char* name, Term* init) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("name", make_string(name), rec);
  rec = record_set("init", init, rec);
  return make_variant("variant", rec);
}

Term* make_handler_term(int lineno, char* name, Term* body) {
  Value* rec = make_record();
  rec = record_set("line", make_int(lineno), rec);
  rec = record_set("name", make_string(name), rec);
  rec = record_set("body", body, rec);
  return make_variant("handler", rec);
}

void print_term_list(Term* e) {
  if (is_list(e)) {
    print_term(head(e));
    if (! is_unit(tail(e)))
      printf(", ");
    print_term_list(tail(e));
  }
}

void print_param_list(Term* e) {
  if (is_list(e)) {
    if (is_string(head(e)))
      printf("%s", get_cstring(head(e)));
    if (! is_unit(tail(e)))
      printf(", ");
    print_param_list(tail(e));
  }
}

void print_fields(Term* fs) {
  if (! is_unit(fs)) {
    Value* h = head(fs);
    printf("%s: ", get_cstring(head(h)));
    print_term(tail(h));
    if (! is_unit(tail(fs)))
      printf(" , ");
    print_term_list(tail(fs));
  }
}

void print_term(Term* e) {
  if (0 == strcmp("unit", variant_name(e)))
    printf("()");    
  else if (0 == strcmp("int", variant_name(e)))
    printf("%d", get_int(record_get("integer", variant_value(e))));
  else if (0 == strcmp("bool", variant_name(e)))
    printf("%d", get_bool(record_get("boolean", variant_value(e))));
  else if (0 == strcmp("string", variant_name(e)))
    printf("\"%s\"", get_cstring(record_get("string", variant_value(e))));
  else if (0 == strcmp("char", variant_name(e)))
    printf("#%c", get_char(record_get("char", variant_value(e))));
  else if (0 == strcmp("var", variant_name(e))) {
    printf("`%s", get_cstring(record_get("name", variant_value(e))));
  } else if (0 == strcmp("get_field", variant_name(e))) {
    print_term(record_get("record", variant_value(e)));
    printf(".%s", get_cstring(record_get("field", variant_value(e))));
  } else if (0 == strcmp("application", variant_name(e))) {
    print_term(record_get("rator", variant_value(e)));
    printf("(");
    print_term_list(record_get("rands", variant_value(e)));
    printf(")");
  } else if (0 == strcmp("lambda", variant_name(e))) {
    printf("fun (");
    print_param_list(record_get("params", variant_value(e)));
    printf(") {");
    print_term(record_get("body", variant_value(e)));
    printf("}");
  } else if (0 == strcmp("recursive", variant_name(e))) {
    printf("rec (");
    printf("%s", get_cstring(record_get("var", variant_value(e))));
    printf(") ");
    print_term(record_get("body", variant_value(e)));
  } else if (0 == strcmp("let", variant_name(e))) {
    printf("%s", get_cstring(record_get("var", variant_value(e))));
    printf(": ");
    print_term(record_get("rhs", variant_value(e)));
    printf(";\n");
    print_term(record_get("body", variant_value(e)));
  } else if (0 == strcmp("record", variant_name(e))) {
    printf("record {");
    print_fields(record_get("fields", variant_value(e)));
    printf("}");
  } else if (0 == strcmp("op", variant_name(e))) {
    printf("%s(", get_cstring(record_get("operator", variant_value(e))));
    print_term_list(record_get("arguments", variant_value(e)));
    printf(")");
  } else if (0 == strcmp("set_field", variant_name(e))) {
    print_term(record_get("record", variant_value(e)));
    printf(".%s <- ", get_cstring(record_get("field", variant_value(e))));
    print_term(record_get("replacement", variant_value(e)));
  } else if (0 == strcmp("variant", variant_name(e))) {
    printf("(tag ");
    print_term(record_get("init", variant_value(e)));
    printf(" as %s)", get_cstring(record_get("name", variant_value(e))));
  } else if (0 == strcmp("handler", variant_name(e))) {
    printf("tag %s => ", get_cstring(record_get("name", variant_value(e))));
    print_term(record_get("body", variant_value(e)));
  } else if (0 == strcmp("case", variant_name(e))) {
    printf("case ");
    print_term(record_get("descr", variant_value(e)));
    printf(" of ");
    print_term(record_get("handler", variant_value(e)));
  } else if (0 == strcmp("ifthen", variant_name(e))) {
    printf("if ");
    print_term(record_get("cond", variant_value(e)));
    printf(" then ");
    print_term(record_get("then", variant_value(e)));    
    printf(" else ");
    print_term(record_get("else", variant_value(e)));    
  } else {
    printf("error, unhandled case in print term\n");
    exit(-1);
  }
}

