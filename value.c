#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "value.h"

Value* head(Value* ls) {
  switch (ls->tag) {
  case ListV:
    return ls->u.list.head;
  default:
    printf("error in head, expected list, not ");
    print_value(ls);
    printf("\n");
    exit(-1);
  }
}

Value* tail(Value* ls) {
  switch (ls->tag) {
  case ListV:
    return ls->u.list.tail;
  default:
    printf("error in tail, expected list, not ");
    print_value(ls);
    printf("\n");
    exit(-1);
  }
}

void set_tail(Value* ls, Value* v) {
  switch (ls->tag) {
  case ListV:
    ls->u.list.tail = v;
    break;
  default:
    printf("error in set_tail, expected list, not ");
    print_value(ls);
    printf("\n");
    exit(-1);
  }
}

int is_unit(Value* v) { return v->tag == UnitV; }

int value_list_len(Value* l) {
  if (is_unit(l))
    return 0;
  else
    return 1 + value_list_len(tail(l));
}

Value* get_nth(Value* vs, int i) {
  assert(vs && vs->tag == ListV);
  if (i == 0)
    return head(vs);
  else
    return get_nth(tail(vs), i - 1);
}

Value* make_list(Value* head, Value* tail) {
  Value* t = malloc(sizeof(Value));
  t->tag = ListV;
  t->u.list.head = head;
  t->u.list.tail = tail;
  return t;
}
int is_list(Value* v) { return v->tag == ListV; }

Value* make_unit() {
  Value* t = malloc(sizeof(Value));
  t->tag = UnitV; 
  return t;
}

Value* make_int(int n) {
  Value* t = malloc(sizeof(Value));
  t->tag = IntV; 
  t->u._int = n;
  return t;
}

int is_int(Value* v) { return v->tag == IntV; }

int get_int(Value* v) {
  switch (v->tag) {
  case IntV:
    return v->u._int;
  default:
    printf("error in get_int, expected an integer\n");
    exit(-1);
  }
}  


Value* make_string(char* str) {
  Value* t = malloc(sizeof(Value));
  t->tag = StringV; 
  t->u.str = str;
  return t;
}

int is_string(Value* v) { return v->tag == StringV; }

char* get_cstring(Value* v) {
  switch (v->tag) {
  case StringV:
    return v->u.str;
  default:
    printf("error in get_cstring, expected a string, not\n");
    print_value(v);
    printf("\n");
    exit(-1);
  }
}

Value* make_char(char c) {
  Value* t = malloc(sizeof(Value));
  t->tag = CharV; 
  t->u._char = c;
  return t;
}

int is_char(Value* v) { return v->tag == CharV; }

char get_char(Value* v) {
  switch (v->tag) {
  case CharV:
    return v->u._char;
  default:
    printf("error in get_char, expected a character\n");
    exit(-1);
  }
}  

Value* make_bool(int b) {
  Value* t = malloc(sizeof(Value));
  t->tag = BoolV; 
  t->u._bool = b;
  return t;
}

int is_bool(Value* v) { return v->tag == BoolV; }

int get_bool(Value* v) {
  switch (v->tag) {
  case BoolV:
    return v->u._bool;
  default:
    printf("error in get_bool, expected a boolean\n");
    exit(-1);
  }
}  

Value* make_procedure(Value* params, Term* body, Value* env) {
  Value* t = malloc(sizeof(Value));
  t->tag = ProcV; 
  t->u.proc.params = params;
  t->u.proc.body = body;
  t->u.proc.env = env;
  return t;
}

Value* make_record() {
  Value* t = malloc(sizeof(Value));
  t->tag = RecordV; 
  t->u.record.fields = make_unit();
  return t;
}

Value* record_set(char* field, Value* value, Value* record) {
  switch (record->tag) {
  case RecordV: {
    Value* kv = make_list(make_string(field), value);
    Value* t = malloc(sizeof(Value));
    t->tag = RecordV; 
    t->u.record.fields = make_list(kv, record->u.record.fields);
    return t;
  }
  default:
    printf("error in record set, expected record\n");
    exit(-1);
  }
}

Value* record_get(char* field, Value* record) {
  switch (record->tag) {
  case RecordV: {
    Value* fields = record->u.record.fields;
    while (! is_unit(fields)) {
      if (0 == strcmp(field, get_cstring(head(head(fields)))))
	return tail(head(fields));
      else
	fields = tail(fields);
    }
    printf("error in record get, could not find field %s\n", field);
    exit(-1);
  }
  default:
    printf("error in record get, expected record, not \n");
    print_value(record);
    exit(-1);
  }
}

int is_record(Value* v) { return v->tag == RecordV; }

Value* make_variant(char* name, Value* val) {
  Value* t = malloc(sizeof(Value));
  t->tag = VariantV; 
  t->u.variant.name = name;
  t->u.variant.val = val;
  return t;
}

char* variant_name(Value* variant) {
  switch (variant->tag) {
  case VariantV:
    return variant->u.variant.name;
  default:
    printf("error, expected a variant in variant_name\n");
    exit(-1);
  }
}

Value* variant_value(Value* variant) {
  switch (variant->tag) {
  case VariantV:
    return variant->u.variant.val;
  default:
    printf("error, expected a variant in variant_value\n");
    exit(-1);
  }
}

Value* make_handler(Value* handlers) {
  Value* t = malloc(sizeof(Value));
  t->tag = HandlerV; 
  t->u.handler = handlers;
  return t;
}

int is_handler(Value* v) { return v->tag == HandlerV; }

Value* get_handler(Value* v) {
  switch (v->tag) {
  case HandlerV:
    return v->u.handler;
  default:
    printf("error in get_handler, expected a handler\n");
    exit(-1);
  }
}


Value* make_fix(Value* fun) {
  Value* t = malloc(sizeof(Value));
  t->tag = FixV; 
  t->u.fix.fun = fun;
  return t;
}

Value* make_env(char* var, Value* val, Value* rest) {
  Value* variable = make_string(var);
  return make_list(make_list(variable, val), rest);
}

void print_env(Env* env) {
  if (! is_unit(env)) {
    printf("%s: ", get_cstring(head(head(env))));
    print_value(tail(head(env)));
    if (! is_unit(tail(env))) {
      printf(", ");
      print_env(tail(env));
    }
  }
}

Value* lookup(char* var, Value* env) {
  if (is_unit(env))
    return 0;
  else if (0 == strcmp(var, get_cstring(head(head(env)))))
    return tail(head(env));
  else
    return lookup(var, tail(env));
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
    printf("<function>");
    break;
  case FixV:
    printf("<recursive>");
    break;
  case RecordV: {
    printf("record {");
    print_env(v->u.record.fields);
    printf("}");
    break;
  }
  case HandlerV:
    printf("handler[");
    print_value(v->u.handler);
    printf("]");
    break;
  case VariantV:
    printf("(tag ");
    print_value(v->u.variant.val);
    printf(" as %s", v->u.variant.name);
    printf(")");
    break;
  case ListV:
    printf("(");    
    print_value(v->u.list.head);
    printf(" :: ");
    print_value(v->u.list.tail);
    printf(")");
    break;
  }
}

