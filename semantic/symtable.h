#pragma once
#include <stdbool.h>
#include <stddef.h>

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION
} SymbolKind;

typedef struct ParamSpec {
    char* name;        // optional; used for diagnostics
    Type type;
    struct ParamSpec* next;
} ParamSpec;

typedef struct FunctionType {
    Type return_type;
    ParamSpec* params;
    int param_count;
    bool is_defined;
} FunctionType;

typedef struct Symbol {
    char* name;
    SymbolKind kind;
    union {
        Type variable_type;
        FunctionType* function_type;
    } as;
    struct Symbol* next_in_bucket;
} Symbol;

typedef struct Scope {
    Symbol** buckets;
    size_t bucket_count;
    struct Scope* parent;
} Scope;

typedef struct SymbolTable {
    Scope* current;
} SymbolTable;

void symbol_table_init(SymbolTable* table);
void symbol_table_destroy(SymbolTable* table);

void symbol_table_enter_scope(SymbolTable* table);
void symbol_table_leave_scope(SymbolTable* table);

// Returns NULL on redeclaration in the same scope
Symbol* symbol_table_define_variable(SymbolTable* table, const char* name, Type type);

// Returns NULL on duplicate declaration in the same scope with incompatible signature
Symbol* symbol_table_declare_function(SymbolTable* table, const char* name, FunctionType* signature);

// Mark function as defined; returns false on duplicate definition
bool symbol_table_define_function(SymbolTable* table, const char* name);

// Lookup in current and parent scopes
Symbol* symbol_table_lookup(SymbolTable* table, const char* name);
// Lookup only in current scope
Symbol* symbol_table_lookup_current(SymbolTable* table, const char* name);

// Iterate all symbols in all visible scopes (from current up to root)
void symbol_table_for_each(SymbolTable* table, void (*cb)(Symbol* sym, void* user_data), void* user_data);

// Helpers
FunctionType* function_type_create(Type return_type);
void function_type_add_param(FunctionType* fn, const char* name, Type type);
void function_type_free(FunctionType* fn);

#ifdef __cplusplus
}
#endif
