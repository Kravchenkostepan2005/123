#pragma once
#include <stdbool.h>

typedef enum {
    TYPE_DYNAMIC,
    TYPE_NUM,
    TYPE_STRING,
    TYPE_NULL
} DataType;

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_METHOD,
    SYMBOL_CLASS,
    SYMBOL_GETTER,
    SYMBOL_SETTER
} SymbolType;

typedef struct Symbol Symbol;

typedef struct symtable_t symtable_t;

struct Symbol {
    char *name;
    SymbolType type;
    DataType data_type;
    int param_count;
    int is_defined;
    char *class_name;
    symtable_t *members; // for classes
};

typedef enum {
    // token types
    IDENTIFIER,
    NUM_TYPE,
    STRING_TYPE,
    KEYWORD,
    // operators
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
    // nonterminals used in analyze_node switch
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

typedef enum {
    NULL_LOWERCASE = 0
} KeywordValue;

typedef struct Token {
    Token_type type;
    union {
        long integer; // for KEYWORD mapping or numbers (not used fully)
        char *characters; // for IDENTIFIER/STRING
    } data;
} Token;

typedef struct Node {
    NonTerminal nonterminal;
    Token *token; // optional
    struct Node *left_child;
    struct Node *right_child;
} Node;

// Error codes (subset)
#define INTERNAL_ERROR 99
#define SEMANTIC_REDEFINITION_ERROR 3
#define SEMANTIC_UNDEFINED_ERROR 1
#define SEMANTIC_PARAM_COUNT_ERROR 4
#define SEMANTIC_TYPE_ERROR 5
#define SEMANTIC_OTHER_ERROR 2

// symtable API
symtable_t *symtable_create(void);
void symtable_destroy(symtable_t *table);
int symtable_insert(symtable_t *table, const char *key, Symbol *value);
Symbol *symtable_find(symtable_t *table, const char *key);

// semantic context
typedef struct {
    symtable_t *global_symtable;
    symtable_t *current_symtable;
    symtable_t *class_symtable;
    char *current_class;
    bool in_class;
    bool in_function;
    int error_count;
} SemanticContext;

// public API from provided snippet
SemanticContext* semantic_init(void);
void semantic_cleanup(SemanticContext *context);
int enter_scope(SemanticContext *context);
void exit_scope(SemanticContext *context);
int enter_class_scope(SemanticContext *context, const char *class_name);
void exit_class_scope(SemanticContext *context);
Symbol* find_symbol(SemanticContext *context, const char *name);
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
