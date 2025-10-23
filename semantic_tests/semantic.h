#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "symtable.h"

// Error codes
#define INTERNAL_ERROR 99
#define SEMANTIC_REDEFINITION_ERROR 3
#define SEMANTIC_UNDEFINED_ERROR 1
#define SEMANTIC_PARAM_COUNT_ERROR 5
#define SEMANTIC_TYPE_ERROR 6
#define SEMANTIC_OTHER_ERROR 4

// Data types
typedef enum {
    TYPE_DYNAMIC = 0,
    TYPE_NUM,
    TYPE_STRING,
    TYPE_NULL
} DataType;

// Symbol types
typedef enum {
    SYMBOL_VARIABLE = 0,
    SYMBOL_GETTER,
    SYMBOL_FUNCTION,
    SYMBOL_METHOD,
    SYMBOL_CLASS,
    SYMBOL_SETTER
} SymbolType;

// Token types (operators and literals)
typedef enum {
    INTEGER_LITERAL = 0,
    FLOAT_LITERAL,
    STRING_LITERAL,
    KEYWORD,
    IDENTIFIER,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    EQUALS,
    NOT_EQUALS
} Token_type;

// Keyword kinds
enum { NULL_KW = 0 };

// Nonterminals used in the AST
typedef enum {
    PROGRAM = 0,
    PROLOG,
    CLASS_NT,
    FUNCDECL,
    SEQUENCE,
    ASSIGN,
    FUNCNAME,
    TERM,
    BLOCK,
    FUNCHEAD
} NonTerminal;

// Token structure
typedef struct Token {
    Token_type type;
    union {
        int integer;     // for KEYWORD kind or numeric literals if needed
        char* characters; // for identifiers/strings
    } data;
} Token;

// Forward-declare Node for pointers in itself
struct Node;

// AST node structure
typedef struct Node {
    NonTerminal nonterminal;
    Token* token;             // optional
    struct Node* left_child;  // optional
    struct Node* right_child; // optional
} Node;

// Symbol structure
typedef struct Symbol {
    char* name;
    SymbolType type;
    DataType data_type;
    int param_count;
    int is_defined;
    char* class_name;     // nullable
    symtable_t* members;  // for classes: member table
} Symbol;

// Semantic analyzer context
typedef struct SemanticContext {
    symtable_t* global_symtable;
    symtable_t* current_symtable;
    symtable_t* class_symtable; // active class symbol table
    char* current_class;        // active class name
    bool in_class;
    bool in_function;
    int error_count;
} SemanticContext;

// API
SemanticContext* semantic_init(void);
void semantic_cleanup(SemanticContext* context);
int semantic_analyze(Node* ast);
