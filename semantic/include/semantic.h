#pragma once
#include <stdbool.h>

// Forward declarations
typedef struct Node Node;
typedef struct Token Token;

typedef enum {
    INTERNAL_ERROR = 99,
    SEMANTIC_OTHER_ERROR = 1,
    SEMANTIC_UNDEFINED_ERROR = 2,
    SEMANTIC_REDEFINITION_ERROR = 3,
    SEMANTIC_PARAM_COUNT_ERROR = 4,
    SEMANTIC_TYPE_ERROR = 5
} SemanticError;

typedef enum {
    TYPE_DYNAMIC,
    TYPE_NUM,
    TYPE_STRING,
    TYPE_NULL
} DataType;

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_CLASS,
    SYMBOL_METHOD,
    SYMBOL_GETTER,
    SYMBOL_SETTER
} SymbolType;

typedef struct symtable symtable_t;

typedef struct Symbol {
    char *name;
    SymbolType type;
    DataType data_type;
    int param_count;
    int is_defined;
    char *class_name; // for methods/getters/setters
    symtable_t *members; // for class members
} Symbol;

typedef struct SemanticContext {
    symtable_t *global_symtable;
    symtable_t *current_symtable;
    symtable_t *class_symtable;
    char *current_class;
    bool in_class;
    bool in_function;
    int error_count;
} SemanticContext;

// Token and AST support types

typedef enum {
    IDENTIFIER,
    NUM_TYPE,
    STRING_TYPE,
    KEYWORD,
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

typedef enum {
    NULL_LOWERCASE = 0,
} KeywordType;

typedef union {
    double number;
    char *characters;
    int integer;
} TokenData;

struct Token {
    Token_type type;
    TokenData data;
};

// Simplified AST node with binary children

typedef enum {
    PROGRAM,
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

struct Node {
    NonTerminal nonterminal;
    Token *token; // optional associated token
    Node *left_child;
    Node *right_child;
};

// Symtable API
symtable_t *symtable_create(void);
int symtable_insert(symtable_t *table, const char *key, Symbol *symbol);
Symbol *symtable_find(symtable_t *table, const char *key);
void symtable_destroy(symtable_t *table);

// Semantic API (implemented in src/semantic.c)
SemanticContext *semantic_init(void);
void semantic_cleanup(SemanticContext *context);
int enter_scope(SemanticContext *context);
void exit_scope(SemanticContext *context);
int enter_class_scope(SemanticContext *context, const char *class_name);
void exit_class_scope(SemanticContext *context);
Symbol *find_symbol(SemanticContext *context, const char *name);
int add_symbol(SemanticContext *context, const char *name, SymbolType type,
               int param_count, const char *class_name);
int is_global_variable(const char *name);
int is_builtin_function(const char *name);
int analyze_variable_reference(SemanticContext *context, const char *var_name);
int count_parameters(Node *node);
int analyze_parameter_count(const char *func_name, int expected_count, int actual_count);
DataType get_token_data_type(Token *token);
int check_literal_type_compatibility(Token_type operation, DataType left_type, DataType right_type);
int analyze_function_call(SemanticContext *context, const char *func_name, Node *args_node);
int analyze_expression(SemanticContext *context, Node *node);
int analyze_class(SemanticContext *context, Node *node);
int analyze_method(SemanticContext *context, Node *node, const char *class_name);
int analyze_getter(SemanticContext *context, Node *node, const char *class_name);
int analyze_setter(SemanticContext *context, Node *node, const char *class_name);
int analyze_node(SemanticContext *context, Node *node);
int semantic_analyze(Node *ast);
