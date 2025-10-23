#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stddef.h>

// Data types tracked for symbols
typedef enum {
    TYPE_DYNAMIC = 0,
    TYPE_NUM,
    TYPE_STRING,
    TYPE_NULL
} DataType;

// Kinds of symbols (variables, functions, class members...)
typedef enum {
    SYMBOL_VARIABLE = 0,
    SYMBOL_FUNCTION,
    SYMBOL_METHOD,
    SYMBOL_CLASS,
    SYMBOL_GETTER,
    SYMBOL_SETTER
} SymbolType;

struct symtable; // forward declaration

// Symbol entry stored in symbol tables
typedef struct Symbol {
    char *name;                 // identifier
    SymbolType type;            // symbol kind
    DataType data_type;         // static/literal type info (if known)
    int param_count;            // number of parameters (functions/methods)
    int is_defined;             // flag for definition state
    char *class_name;           // owning class for methods/getters/setters
    struct symtable *members;   // member table (for classes)
} Symbol;

// Simple linked-list symbol table (sufficient for tests)
typedef struct symtable symtable_t;

symtable_t *symtable_create(void);
void symtable_destroy(symtable_t *table);

// Returns 0 on success, non-zero on failure
int symtable_insert(symtable_t *table, const char *key, Symbol *value);

// Returns pointer to symbol if found, otherwise NULL
Symbol *symtable_find(symtable_t *table, const char *key);

#endif // SYMTABLE_H
