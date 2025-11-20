#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser.h"
#include "symtable.h"
#include <stdbool.h>

/* Error codes */
#define LEXICAL_ERROR            1
#define SYNTACTICAL_ERROR        2
#define SEMANTIC_UNDEFINED_ERROR 3
#define SEMANTIC_REDEFINITION_ERROR 4
#define SEMANTIC_PARAM_COUNT_ERROR 5
#define SEMANTIC_TYPE_ERROR      6
#define SEMANTIC_OTHER_ERROR     10
#define INTERNAL_ERROR           99

/* Runtime error codes */
#define RUNTIME_TYPE_ERROR       25
#define RUNTIME_TYPE_COMPAT_ERROR 26

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_BUILTIN_FUNCTION,
    SYMBOL_CLASS,
    SYMBOL_METHOD,
    SYMBOL_GETTER,
    SYMBOL_SETTER
} SymbolType;

typedef enum {
    TYPE_DYNAMIC,
    TYPE_NULL,
    TYPE_NUM,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_CLASS
} DataType;

typedef struct Symbol {
    char       *name;
    SymbolType  type;
    DataType    data_type;
    int         param_count;
    int         is_defined;
    char       *class_name;
    symtable_t *members;
} Symbol;

typedef struct ScopeFrame ScopeFrame;

typedef struct SemanticContext {
    symtable_t *global_symtable;
    symtable_t *current_symtable;
    symtable_t *class_symtable;
    ScopeFrame *scope_stack;
    char       *current_class;
    bool        in_class;
    bool        in_function;
    int         error_count;
} SemanticContext;

SemanticContext *semantic_init(void);
void semantic_cleanup(SemanticContext *context);

int enter_scope(SemanticContext *context);
void exit_scope(SemanticContext *context);
int enter_class_scope(SemanticContext *context, const char *class_name);
void exit_class_scope(SemanticContext *context);

Symbol *find_symbol(SemanticContext *context, const char *name);
int add_symbol(SemanticContext *context,
               const char *name,
               SymbolType type,
               int param_count,
               const char *class_name);

int is_global_variable(const char *name);
int is_builtin_function(const char *name);
int validate_builtin_function(const char *func_name, int param_count);

int analyze_variable_reference(SemanticContext *context, const char *var_name);
int analyze_function_call(SemanticContext *context,
                          const char *func_name,
                          Node *args_node);

DataType get_token_data_type(Token *token);
int check_literal_type_compatibility(Token_type operation,
                                     DataType left_type,
                                     DataType right_type);

int analyze_expression(SemanticContext *context, Node *node);
int analyze_var_declaration(SemanticContext *context, Node *node);
int analyze_assignment(SemanticContext *context, Node *node);
int analyze_if_statement(SemanticContext *context, Node *node);
int analyze_while_statement(SemanticContext *context, Node *node);
int analyze_return_statement(SemanticContext *context, Node *node);

int analyze_class(SemanticContext *context, Node *node);
int analyze_function(SemanticContext *context, Node *node);
int analyze_method(SemanticContext *context, Node *node, const char *class_name);
int analyze_getter(SemanticContext *context, Node *node, const char *class_name);
int analyze_setter(SemanticContext *context, Node *node, const char *class_name);

int analyze_node(SemanticContext *context, Node *node);
int semantic_analyze(Node *ast);

int check_program_structure(SemanticContext *context, Node *node);
int check_main_function(SemanticContext *context);

#endif /* SEMANTIC_H */
