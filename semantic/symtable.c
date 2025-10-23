#include "symtable.h"

#include <stdlib.h>
#include <string.h>

#define DEFAULT_BUCKETS 127

// Portable string duplicate to avoid non-standard strdup
static char* str_dup_portable(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* out = (char*)malloc(len);
    if (!out) return NULL;
    memcpy(out, s, len);
    return out;
}

static unsigned long hash_string(const char* s) {
    unsigned long h = 1469598103934665603ull; // FNV-1a 64-bit basis
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 1099511628211ull;
    }
    return h;
}

static Scope* scope_create(size_t bucket_count, Scope* parent) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    scope->bucket_count = bucket_count;
    scope->buckets = (Symbol**)calloc(bucket_count, sizeof(Symbol*));
    scope->parent = parent;
    return scope;
}

static void symbol_free(Symbol* sym) {
    if (!sym) return;
    free(sym->name);
    if (sym->kind == SYMBOL_FUNCTION) {
        function_type_free(sym->as.function_type);
    }
    free(sym);
}

static void scope_free(Scope* scope) {
    if (!scope) return;
    for (size_t i = 0; i < scope->bucket_count; ++i) {
        Symbol* s = scope->buckets[i];
        while (s) {
            Symbol* next = s->next_in_bucket;
            symbol_free(s);
            s = next;
        }
    }
    free(scope->buckets);
    free(scope);
}

void symbol_table_init(SymbolTable* table) {
    table->current = scope_create(DEFAULT_BUCKETS, NULL);
}

void symbol_table_destroy(SymbolTable* table) {
    Scope* s = table->current;
    while (s) {
        Scope* parent = s->parent;
        scope_free(s);
        s = parent;
    }
    table->current = NULL;
}

void symbol_table_enter_scope(SymbolTable* table) {
    table->current = scope_create(DEFAULT_BUCKETS, table->current);
}

void symbol_table_leave_scope(SymbolTable* table) {
    if (!table->current) return;
    Scope* parent = table->current->parent;
    scope_free(table->current);
    table->current = parent;
}

static Symbol* symbol_create(const char* name, SymbolKind kind) {
    Symbol* sym = (Symbol*)malloc(sizeof(Symbol));
    sym->name = str_dup_portable(name);
    sym->kind = kind;
    sym->next_in_bucket = NULL;
    return sym;
}

Symbol* symbol_table_define_variable(SymbolTable* table, const char* name, Type type) {
    Scope* scope = table->current;
    unsigned long h = hash_string(name) % scope->bucket_count;
    for (Symbol* s = scope->buckets[h]; s; s = s->next_in_bucket) {
        if (strcmp(s->name, name) == 0) {
            // Redeclaration in same scope
            return NULL;
        }
    }
    Symbol* sym = symbol_create(name, SYMBOL_VARIABLE);
    sym->as.variable_type = type;
    sym->next_in_bucket = scope->buckets[h];
    scope->buckets[h] = sym;
    return sym;
}

static bool function_signatures_compatible(const FunctionType* a, const FunctionType* b) {
    if (!type_equals(a->return_type, b->return_type)) return false;
    if (a->param_count != b->param_count) return false;
    const ParamSpec* pa = a->params;
    const ParamSpec* pb = b->params;
    while (pa && pb) {
        if (!type_equals(pa->type, pb->type)) return false;
        pa = pa->next;
        pb = pb->next;
    }
    return pa == NULL && pb == NULL;
}

Symbol* symbol_table_declare_function(SymbolTable* table, const char* name, FunctionType* signature) {
    Scope* scope = table->current; // functions live in current scope (usually global)
    unsigned long h = hash_string(name) % scope->bucket_count;
    for (Symbol* s = scope->buckets[h]; s; s = s->next_in_bucket) {
        if (strcmp(s->name, name) == 0) {
            if (s->kind != SYMBOL_FUNCTION) return NULL;
            if (!function_signatures_compatible(s->as.function_type, signature)) {
                return NULL; // Different signature
            }
            // Merge definition flag if any
            if (signature->is_defined) {
                if (s->as.function_type->is_defined) {
                    // duplicate definition
                    return NULL;
                }
                s->as.function_type->is_defined = true;
            }
            return s;
        }
    }
    Symbol* sym = symbol_create(name, SYMBOL_FUNCTION);
    sym->as.function_type = signature; // take ownership
    sym->next_in_bucket = scope->buckets[h];
    scope->buckets[h] = sym;
    return sym;
}

bool symbol_table_define_function(SymbolTable* table, const char* name) {
    Scope* scope = table->current;
    unsigned long h = hash_string(name) % scope->bucket_count;
    for (Symbol* s = scope->buckets[h]; s; s = s->next_in_bucket) {
        if (strcmp(s->name, name) == 0 && s->kind == SYMBOL_FUNCTION) {
            if (s->as.function_type->is_defined) return false;
            s->as.function_type->is_defined = true;
            return true;
        }
    }
    return false;
}

Symbol* symbol_table_lookup(SymbolTable* table, const char* name) {
    for (Scope* scope = table->current; scope; scope = scope->parent) {
        unsigned long h = hash_string(name) % scope->bucket_count;
        for (Symbol* s = scope->buckets[h]; s; s = s->next_in_bucket) {
            if (strcmp(s->name, name) == 0) return s;
        }
    }
    return NULL;
}

Symbol* symbol_table_lookup_current(SymbolTable* table, const char* name) {
    Scope* scope = table->current;
    unsigned long h = hash_string(name) % scope->bucket_count;
    for (Symbol* s = scope->buckets[h]; s; s = s->next_in_bucket) {
        if (strcmp(s->name, name) == 0) return s;
    }
    return NULL;
}

void symbol_table_for_each(SymbolTable* table, void (*cb)(Symbol* sym, void* user_data), void* user_data) {
    if (!cb) return;
    for (Scope* scope = table->current; scope; scope = scope->parent) {
        for (size_t i = 0; i < scope->bucket_count; ++i) {
            for (Symbol* s = scope->buckets[i]; s; s = s->next_in_bucket) {
                cb(s, user_data);
            }
        }
    }
}

FunctionType* function_type_create(Type return_type) {
    FunctionType* fn = (FunctionType*)malloc(sizeof(FunctionType));
    fn->return_type = return_type;
    fn->params = NULL;
    fn->param_count = 0;
    fn->is_defined = false;
    return fn;
}

void function_type_add_param(FunctionType* fn, const char* name, Type type) {
    ParamSpec* p = (ParamSpec*)malloc(sizeof(ParamSpec));
    p->name = name ? str_dup_portable(name) : NULL;
    p->type = type;
    p->next = NULL;
    if (!fn->params) {
        fn->params = p;
    } else {
        ParamSpec* it = fn->params;
        while (it->next) it = it->next;
        it->next = p;
    }
    fn->param_count += 1;
}

void function_type_free(FunctionType* fn) {
    if (!fn) return;
    ParamSpec* p = fn->params;
    while (p) {
        ParamSpec* next = p->next;
        if (p->name) free(p->name);
        free(p);
        p = next;
    }
    free(fn);
}
