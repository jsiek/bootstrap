#ifndef VALUE_H
#define VALUE_H

/***** Values and Environments *****/

enum ValueKind {
  UnitV, IntV, BoolV, StringV, CharV, ListV,
  ProcV, FixV, RecordV, VariantV, HandlerV
};
struct ValueS;
typedef struct ValueS Value;
typedef Value Env;

/*struct TermS;
typedef struct TermS Term;
*/

typedef Value Term;
  
struct ValueS {
  enum ValueKind tag;
  union {
    int _int;
    int _bool;
    char _char;
    char* str;
    struct { Value* head; Value* tail; } list;
    struct { Value* params; Term* body; Env* env; } proc;
    struct { Value* fields; } record;
    struct { char* name; struct ValueS* val; } variant;
    Value* handler; // an association list of variant names and functions
    struct { struct ValueS* left; struct ValueS* right; } join;
    struct { struct ValueS* fun; } fix;
  } u;
};

Value* make_unit();
int is_unit(Value* v);

Value* make_int(int n);
int is_int(Value* v);
int get_int(Value* v);

Value* make_bool(int b);
int is_bool(Value* v);
int get_bool(Value* v);

Value* make_string(char* str);
int is_string(Value*);
char* get_cstring(Value* v);

Value* make_char(char str);
int is_char(Value*);
char get_char(Value*);

Value* make_list(Value* head, Value* rest);
Value* head(Value* ls);
Value* tail(Value* ls);
void set_tail(Value* ls, Value* v);
int is_list(Value* v);

Value* make_procedure(Value* params, Term* body, Env* env);

Value* make_record();
Value* record_set(char* field, Value* value, Value* record);
Value* record_get(char* field, Value* record);
int is_record(Value*);

Value* make_variant(char* name, Value* val);
char* variant_name(Value* variant);
Value* variant_value(Value* variant);

Value* make_handler(Value* handlers);
int is_handler(Value* v);
Value* get_handler(Value* v);

Value* make_fix(Value* h);

Value* record_set(char* field, Value* value, Value* record);
Value* record_get(char* field, Value* record);

void print_value(Value* value);

void print_value_list(Value*);
int value_list_len(Value* l);
Value* get_nth(Value* vs, int i);

Value* make_env(char* var, Value* val, Env* env);
void print_env(Value*);
Value* lookup(char* var, Env* env);


#endif /* VALUE_H */
