#include "semantic.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * Initialize semantic analyzer context
 */
SemanticContext* semantic_init(void) {
    SemanticContext *context = malloc(sizeof(SemanticContext));
    if (!context) return NULL;

    context->global_symtable = symtable_create();
    context->current_symtable = context->global_symtable;
    context->class_symtable = NULL;
    context->current_class = NULL;
    context->in_class = false;
    context->in_function = false;
    context->error_count = 0;

    if (!context->global_symtable) {
        free(context);
        return NULL;
    }

    return context;
}

/**
 * Clean up semantic analyzer resources
 */
void semantic_cleanup(SemanticContext *context) {
    if (!context) return;

    if (context->global_symtable) {
        // Free all symbols in global table
        symtable_destroy(context->global_symtable);
    }

    if (context->class_symtable) {
        symtable_destroy(context->class_symtable);
    }

    if (context->current_class) free(context->current_class);

    free(context);
}

/**
 * Enter a new scope
 */
static int enter_scope(SemanticContext *context) {
    symtable_t *new_symtable = symtable_create();
    if (!new_symtable) return INTERNAL_ERROR;

    context->current_symtable = new_symtable;
    return 0;
}

/**
 * Exit current scope
 */
static void exit_scope(SemanticContext *context) {
    if (context->current_symtable != context->global_symtable) {
        symtable_destroy(context->current_symtable);
        context->current_symtable = context->global_symtable;
    }
}

/**
 * Enter class scope
 */
static int enter_class_scope(SemanticContext *context, const char *class_name) {
    context->class_symtable = symtable_create();
    if (!context->class_symtable) return INTERNAL_ERROR;

    context->current_class = strdup(class_name);
    context->in_class = true;
    return 0;
}

/**
 * Exit class scope
 */
static void exit_class_scope(SemanticContext *context) {
    if (context->class_symtable) {
        symtable_destroy(context->class_symtable);
        context->class_symtable = NULL;
    }

    if (context->current_class) {
        free(context->current_class);
        context->current_class = NULL;
    }

    context->in_class = false;
}

/**
 * Find symbol in current scope, class scope, or global scope
 */
static Symbol* find_symbol(SemanticContext *context, const char *name) {
    if (!context || !name) return NULL;

    // Search in current scope
    Symbol *symbol = symtable_find(context->current_symtable, name);
    if (symbol) return symbol;

    // Search in class scope
    if (context->in_class && context->class_symtable) {
        symbol = symtable_find(context->class_symtable, name);
        if (symbol) return symbol;
    }

    // Search in global scope
    return symtable_find(context->global_symtable, name);
}

/**
 * Add symbol to current symbol table
 */
static int add_symbol(SemanticContext *context, const char *name, SymbolType type,
               int param_count, const char *class_name) {
    if (!context || !name) return INTERNAL_ERROR;

    // Check for redefinition in current scope
    Symbol *existing = symtable_find(context->current_symtable, name);
    if (existing) {
        if (type == SYMBOL_VARIABLE && existing->type == SYMBOL_VARIABLE) {
            return SEMANTIC_REDEFINITION_ERROR;
        }
        // Check function/method overloading
        if ((type == SYMBOL_FUNCTION || type == SYMBOL_METHOD) &&
            (existing->type == SYMBOL_FUNCTION || existing->type == SYMBOL_METHOD)) {
            if (existing->param_count == param_count) {
                return SEMANTIC_REDEFINITION_ERROR;
            }
        }
    }

    Symbol *symbol = malloc(sizeof(Symbol));
    if (!symbol) return INTERNAL_ERROR;

    symbol->name = strdup(name);
    symbol->type = type;
    symbol->data_type = TYPE_DYNAMIC;
    symbol->param_count = param_count;
    symbol->is_defined = 1;
    symbol->class_name = class_name ? strdup(class_name) : NULL;
    symbol->members = NULL;

    if (type == SYMBOL_CLASS) {
        symbol->members = symtable_create();
        if (!symbol->members) {
            free(symbol->name);
            free(symbol);
            return INTERNAL_ERROR;
        }
    }

    if (symtable_insert(context->current_symtable, name, symbol) != 0) {
        free(symbol->name);
        if (symbol->class_name) free(symbol->class_name);
        if (symbol->members) symtable_destroy(symbol->members);
        free(symbol);
        return INTERNAL_ERROR;
    }

    return 0;
}

/**
 * Check if variable is global
 */
static int is_global_variable(const char *name) {
    return (name && strlen(name) >= 2 && name[0] == '_' && name[1] == '_');
}

/**
 * Check if function is built-in
 */
static int is_builtin_function(const char *name) {
    return (name && strlen(name) >= 4 && strncmp(name, "Ifj.", 4) == 0);
}

/**
 * Analyze variable reference
 */
static int analyze_variable_reference(SemanticContext *context, const char *var_name) {
    if (!var_name) return SEMANTIC_OTHER_ERROR;

    // Global variables are always available
    if (is_global_variable(var_name)) {
        return 0;
    }

    // Check if variable is declared
    Symbol *symbol = find_symbol(context, var_name);
    if (!symbol || (symbol->type != SYMBOL_VARIABLE && symbol->type != SYMBOL_GETTER)) {
        return SEMANTIC_UNDEFINED_ERROR;
    }

    return 0;
}

/**
 * Count parameters in parameter list
 */
static int count_parameters(struct Node *node) {
    if (!node) return 0;

    int count = 0;
    struct Node *current = node;
    while (current) {
        count++;
        current = current->right_child;
    }

    return count;
}

/**
 * Analyze parameter count
 */
static int analyze_parameter_count(const char *func_name, int expected_count, int actual_count) {
    (void)func_name; // unused for now
    if (expected_count != actual_count) {
        return SEMANTIC_PARAM_COUNT_ERROR;
    }
    return 0;
}

/**
 * Get data type from token
 */
static DataType get_token_data_type(struct Token *token) {
    if (!token) return TYPE_DYNAMIC;

    switch (token->type) {
        case INTEGER_LITERAL:
        case FLOAT_LITERAL:
            return TYPE_NUM;
        case STRING_LITERAL:
            return TYPE_STRING;
        case KEYWORD:
            if (token->data.integer == NULL_KW) return TYPE_NULL;
            // Note: Add TRUE_KW and FALSE_KW if BOOLTHEN extension is implemented
            return TYPE_DYNAMIC;
        default:
            return TYPE_DYNAMIC;
    }
}

/**
 * Check literal type compatibility
 */
static int check_literal_type_compatibility(TokenType operation, DataType left_type, DataType right_type) {
    if (left_type == TYPE_DYNAMIC || right_type == TYPE_DYNAMIC) {
        return 0;
    }

    switch (operation) {
        case PLUS:
            if ((left_type == TYPE_STRING && right_type == TYPE_STRING) ||
                (left_type == TYPE_NUM && right_type == TYPE_NUM)) return 0;
            break;
        case MINUS:
        case MULTIPLY:
        case DIVIDE:
            if (left_type == TYPE_NUM && right_type == TYPE_NUM) return 0;
            break;
        case LESS:
        case LESS_EQUAL:
        case GREATER:
        case GREATER_EQUAL:
            if (left_type == TYPE_NUM && right_type == TYPE_NUM) return 0;
            break;
        case EQUALS:
        case NOT_EQUALS:
            return 0; // Always compatible
        default:
            return 0;
    }

    return SEMANTIC_TYPE_ERROR;
}

/**
 * Forward declaration
 */
static int analyze_node(SemanticContext *context, struct Node *node);

/**
 * Analyze function call
 */
static int analyze_function_call(SemanticContext *context, const char *func_name, struct Node *args_node) {
    if (!func_name) return SEMANTIC_OTHER_ERROR;

    // Built-in functions are always available
    if (is_builtin_function(func_name)) {
        return 0;
    }

    // Check if function exists
    Symbol *symbol = find_symbol(context, func_name);
    if (!symbol || (symbol->type != SYMBOL_FUNCTION && symbol->type != SYMBOL_METHOD)) {
        return SEMANTIC_UNDEFINED_ERROR;
    }

    // Check parameter count
    int actual_param_count = count_parameters(args_node);
    return analyze_parameter_count(func_name, symbol->param_count, actual_param_count);
}

/**
 * Analyze expression
 */
static int analyze_expression(SemanticContext *context, struct Node *node) {
    if (!node) return 0;

    int result = 0;

    // Check variable references
    if (node->token && node->token->type == IDENTIFIER) {
        result = analyze_variable_reference(context, node->token->data.characters);
        if (result != 0) return result;
    }

    // Check function calls
    if (node->nonterminal == FUNCHEAD) {
        struct Node *id_node = node->left_child;
        if (id_node && id_node->token) {
            result = analyze_function_call(context, id_node->token->data.characters, node->right_child);
            if (result != 0) return result;
        }
    }

    // Check literal type compatibility
    if (node->left_child && node->right_child && node->token) {
        DataType left_type = get_token_data_type(node->left_child->token);
        DataType right_type = get_token_data_type(node->right_child->token);

        if (left_type != TYPE_DYNAMIC && right_type != TYPE_DYNAMIC) {
            result = check_literal_type_compatibility(node->token->type, left_type, right_type);
            if (result != 0) return result;
        }
    }

    // Recursively analyze sub-expressions
    if (node->left_child) {
        result = analyze_expression(context, node->left_child);
        if (result != 0) return result;
    }

    if (node->right_child) {
        result = analyze_expression(context, node->right_child);
    }

    return result;
}

/**
 * Analyze class definition
 */
static int analyze_class(SemanticContext *context, struct Node *node) {
    if (!node || !node->token) return SEMANTIC_OTHER_ERROR;

    char *class_name = node->token->data.characters;

    // Add class to global symbol table
    int result = add_symbol(context, class_name, SYMBOL_CLASS, 0, NULL);
    if (result != 0) return result;

    // Enter class scope
    result = enter_class_scope(context, class_name);
    if (result != 0) return result;

    // Analyze class body (methods, getters, setters)
    struct Node *body_node = node->right_child;
    if (body_node) {
        result = analyze_node(context, body_node);
    }

    exit_class_scope(context);
    return result;
}

/**
 * Analyze method definition
 */
static int analyze_method(SemanticContext *context, struct Node *node, const char *class_name) {
    if (!node || !node->token) return SEMANTIC_OTHER_ERROR;

    char *method_name = node->token->data.characters;
    int param_count = count_parameters(node->left_child);

    // Add method to class symbol table
    Symbol *symbol = malloc(sizeof(Symbol));
    if (!symbol) return INTERNAL_ERROR;

    symbol->name = strdup(method_name);
    symbol->type = SYMBOL_METHOD;
    symbol->data_type = TYPE_DYNAMIC;
    symbol->param_count = param_count;
    symbol->is_defined = 1;
    symbol->class_name = strdup(class_name);
    symbol->members = NULL;

    if (symtable_insert(context->class_symtable, method_name, symbol) != 0) {
        free(symbol->name);
        free(symbol->class_name);
        free(symbol);
        return INTERNAL_ERROR;
    }

    // Analyze method body
    int result = enter_scope(context);
    if (result != 0) return result;

    // Add parameters to scope
    struct Node *param_node = node->left_child;
    while (param_node) {
        if (param_node->token) {
            result = add_symbol(context, param_node->token->data.characters, SYMBOL_VARIABLE, 0, NULL);
            if (result != 0) {
                exit_scope(context);
                return result;
            }
        }
        param_node = param_node->right_child;
    }

    // Analyze method body
    struct Node *body_node = node->right_child;
    if (body_node) {
        result = analyze_node(context, body_node);
    }

    exit_scope(context);
    return result;
}

/**
 * Analyze getter definition
 */
static int analyze_getter(SemanticContext *context, struct Node *node, const char *class_name) {
    if (!node || !node->token) return SEMANTIC_OTHER_ERROR;

    char *getter_name = node->token->data.characters;

    // Add getter to class symbol table
    Symbol *symbol = malloc(sizeof(Symbol));
    if (!symbol) return INTERNAL_ERROR;

    symbol->name = strdup(getter_name);
    symbol->type = SYMBOL_GETTER;
    symbol->data_type = TYPE_DYNAMIC;
    symbol->param_count = 0; // Getters have no parameters
    symbol->is_defined = 1;
    symbol->class_name = strdup(class_name);
    symbol->members = NULL;

    if (symtable_insert(context->class_symtable, getter_name, symbol) != 0) {
        free(symbol->name);
        free(symbol->class_name);
        free(symbol);
        return INTERNAL_ERROR;
    }

    // Analyze getter body
    int result = enter_scope(context);
    if (result != 0) return result;

    struct Node *body_node = node->right_child;
    if (body_node) {
        result = analyze_node(context, body_node);
    }

    exit_scope(context);
    return result;
}

/**
 * Analyze setter definition
 */
static int analyze_setter(SemanticContext *context, struct Node *node, const char *class_name) {
    if (!node || !node->left_child || !node->left_child->token) return SEMANTIC_OTHER_ERROR;

    char *setter_name = node->left_child->token->data.characters;

    // Add setter to class symbol table
    Symbol *symbol = malloc(sizeof(Symbol));
    if (!symbol) return INTERNAL_ERROR;

    symbol->name = strdup(setter_name);
    symbol->type = SYMBOL_SETTER;
    symbol->data_type = TYPE_DYNAMIC;
    symbol->param_count = 1; // Setters have one parameter
    symbol->is_defined = 1;
    symbol->class_name = strdup(class_name);
    symbol->members = NULL;

    if (symtable_insert(context->class_symtable, setter_name, symbol) != 0) {
        free(symbol->name);
        free(symbol->class_name);
        free(symbol);
        return INTERNAL_ERROR;
    }

    // Analyze setter body
    int result = enter_scope(context);
    if (result != 0) return result;

    // Add parameter to scope
    if (node->right_child && node->right_child->token) {
        result = add_symbol(context, node->right_child->token->data.characters, SYMBOL_VARIABLE, 0, NULL);
        if (result != 0) {
            exit_scope(context);
            return result;
        }
    }

    struct Node *body_node = node->right_child ? node->right_child->right_child : NULL;
    if (body_node) {
        result = analyze_node(context, body_node);
    }

    exit_scope(context);
    return result;
}

/**
 * Analyze AST node recursively
 */
static int analyze_node(SemanticContext *context, struct Node *node) {
    if (!node) return 0;

    int result = 0;

    switch (node->nonterminal) {

        case PROGRAM:
            result = analyze_node(context, node->left_child);
            if (result == 0) {
                result = analyze_node(context, node->right_child);
            }
            break;

        case PROLOG:
            if (node->left_child && node->left_child->token) {
                if (strcmp(node->left_child->token->data.characters, "ifj25") != 0) {
                    return SEMANTIC_OTHER_ERROR;
                }
            }
            break;

        case CLASS_NT:
            result = analyze_class(context, node);
            break;

        case FUNCDECL:
            if (context->current_class) {
                result = analyze_method(context, node, context->current_class);
            } else {
                {
                    struct Node *id_node = node->left_child ? node->left_child->left_child : NULL;
                    if (id_node && id_node->token) {
                        char *func_name = id_node->token->data.characters;
                        int param_count = count_parameters(node->left_child ? node->left_child->right_child : NULL);

                        result = add_symbol(context, func_name, SYMBOL_FUNCTION, param_count, NULL);
                        if (result != 0) break;

                        result = enter_scope(context);
                        if (result != 0) break;

                        // Add parameters
                        struct Node *param_node = node->left_child ? node->left_child->right_child : NULL;
                        while (param_node) {
                            if (param_node->token) {
                                result = add_symbol(context, param_node->token->data.characters, SYMBOL_VARIABLE, 0, NULL);
                                if (result != 0) {
                                    exit_scope(context);
                                    break;
                                }
                            }
                            param_node = param_node->right_child;
                        }

                        if (result == 0) {
                            result = analyze_node(context, node->right_child);
                        }

                        exit_scope(context);
                    }
                }
            }
            break;

        case SEQUENCE:
            if (context->current_class) {
                struct Node *current = node;
                while (current) {
                    result = analyze_node(context, current->left_child);
                    if (result != 0) break;
                    current = current->right_child;
                }
            } else {
              struct Node *current = node;
                while (current) {
                    result = analyze_node(context, current->left_child);
                    if (result != 0) break;
                    current = current->right_child;
                }
            }
            break;

        case ASSIGN:
            if (node->left_child && node->left_child->token) {
                char *var_name = node->left_child->token->data.characters;

                Symbol *symbol = find_symbol(context, var_name);
                if (!symbol) {
                    result = add_symbol(context, var_name, SYMBOL_VARIABLE, 0, NULL);
                    if (result != 0) break;
                }

                if (node->right_child) {
                    result = analyze_expression(context, node->right_child);
                }
            }
            break;

        case FUNCNAME:
            if (node->token) {
                result = analyze_function_call(context, node->token->data.characters, NULL);
            }
            break;

        case TERM:
            result = analyze_expression(context, node);
            break;

        case BLOCK:
            result = enter_scope(context);
            if (result != 0) break;

            if (node->left_child) {
                result = analyze_node(context, node->left_child);
            }

            exit_scope(context);
            break;

        default:
            if (node->left_child) {
                result = analyze_node(context, node->left_child);
            }
            if (result == 0 && node->right_child) {
                result = analyze_node(context, node->right_child);
            }
            break;
    }

    return result;
}

/**
 * Main semantic analysis function
 */
int semantic_analyze(struct Node *ast) {
    SemanticContext *context = semantic_init();
    if (!context) return INTERNAL_ERROR;

    int result = analyze_node(context, ast);

    // Check for main function
    Symbol *main_func = NULL; // We cannot access find_symbol directly from here (static), so re-implement search
    // Search in global symtable for "main"
    main_func = symtable_find(context->global_symtable, "main");
    if (!main_func || main_func->type != SYMBOL_FUNCTION) {
        semantic_cleanup(context);
        return SEMANTIC_UNDEFINED_ERROR;
    }

    if (main_func->param_count != 0) {
        semantic_cleanup(context);
        return SEMANTIC_PARAM_COUNT_ERROR;
    }

    semantic_cleanup(context);
    return result;
}
