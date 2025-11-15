#include "semantic.h"
#include "ast.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

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
        symtable_destroy(context->global_symtable);
    }

    if (context->class_symtable) {
        symtable_destroy(context->class_symtable);
    }

    free(context);
}

/**
 * Enter a new scope
 */
int enter_scope(SemanticContext *context) {
    symtable_t *new_symtable = symtable_create();
    if (!new_symtable) return INTERNAL_ERROR;

    // Link to parent scope
    context->current_symtable = new_symtable;
    context->in_function = true;
    return 0;
}

/**
 * Exit current scope
 */
void exit_scope(SemanticContext *context) {
    if (context->current_symtable != context->global_symtable) {
        symtable_destroy(context->current_symtable);
        context->current_symtable = context->global_symtable;
    }
    context->in_function = false;
}

/**
 * Enter class scope
 */
int enter_class_scope(SemanticContext *context, const char *class_name) {
    context->class_symtable = symtable_create();
    if (!context->class_symtable) return INTERNAL_ERROR;

    context->current_class = strdup(class_name);
    context->in_class = true;
    return 0;
}

/**
 * Exit class scope
 */
void exit_class_scope(SemanticContext *context) {
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
Symbol* find_symbol(SemanticContext *context, const char *name) {
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
int add_symbol(SemanticContext *context, const char *name, SymbolType type,
               int param_count, const char *class_name) {
    if (!context || !name) return INTERNAL_ERROR;

    // Check for redefinition in current scope
    Symbol *existing = symtable_find(context->current_symtable, name);
    if (existing) {
        // Variable redefinition in same scope
        if (type == SYMBOL_VARIABLE && existing->type == SYMBOL_VARIABLE) {
            return SEMANTIC_REDEFINITION_ERROR; // Error 4
        }
        // Function/method overloading - only by parameter count
        if ((type == SYMBOL_FUNCTION || type == SYMBOL_METHOD) &&
            (existing->type == SYMBOL_FUNCTION || existing->type == SYMBOL_METHOD)) {
            if (existing->param_count == param_count) {
                return SEMANTIC_REDEFINITION_ERROR; // Error 4
            }
        }
        // Only one getter and one setter per name
        if ((type == SYMBOL_GETTER && existing->type == SYMBOL_GETTER) ||
            (type == SYMBOL_SETTER && existing->type == SYMBOL_SETTER)) {
            return SEMANTIC_REDEFINITION_ERROR; // Error 4
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
 * Check if variable is global (starts with __)
 */
int is_global_variable(const char *name) {
    return (name && strlen(name) >= 2 && name[0] == '_' && name[1] == '_');
}

/**
 * Check if function is built-in (starts with Ifj.)
 */
int is_builtin_function(const char *name) {
    return (name && strlen(name) >= 4 && strncmp(name, "Ifj.", 4) == 0);
}

/**
 * Validate built-in function parameter counts
 */
int validate_builtin_function(const char *func_name, int param_count) {
    if (!func_name) return 0;

    if (strcmp(func_name, "Ifj.read_str") == 0 || strcmp(func_name, "Ifj.read_num") == 0) {
        if (param_count != 0) return SEMANTIC_PARAM_COUNT_ERROR;
    } else if (strcmp(func_name, "Ifj.write") == 0 || strcmp(func_name, "Ifj.str") == 0 ||
               strcmp(func_name, "Ifj.floor") == 0 || strcmp(func_name, "Ifj.length") == 0 ||
               strcmp(func_name, "Ifj.chr") == 0) {
        if (param_count != 1) return SEMANTIC_PARAM_COUNT_ERROR;
    } else if (strcmp(func_name, "Ifj.strcmp") == 0 || strcmp(func_name, "Ifj.ord") == 0) {
        if (param_count != 2) return SEMANTIC_PARAM_COUNT_ERROR;
    } else if (strcmp(func_name, "Ifj.substring") == 0) {
        if (param_count != 3) return SEMANTIC_PARAM_COUNT_ERROR;
    }

    return 0;
}

/**
 * Analyze variable reference with proper scope checking
 */
int analyze_variable_reference(SemanticContext *context, const char *var_name) {
    if (!var_name) return SEMANTIC_OTHER_ERROR;

    // Global variables are always available
    if (is_global_variable(var_name)) {
        return 0;
    }

    // Local variables must be declared
    Symbol *symbol = find_symbol(context, var_name);
    if (!symbol || symbol->type != SYMBOL_VARIABLE) {
        return SEMANTIC_UNDEFINED_ERROR; // Error 3
    }

    return 0;
}

/**
 * Count parameters in parameter list
 */
static int count_identifier_parameters(Node *node) {
    if (!node) return 0;
    int count = 0;
    for (Node *current = node; current; current = current->right_child) {
        if (current->token && current->token->type == IDENTIFIER) {
            count++;
        }
    }
    return count;
}

/**
 * Count arguments in argument list
 */
static int count_arguments(Node *node) {
    if (!node) return 0;
    int count = 0;
    for (Node *current = node; current; current = current->right_child) {
        count++;
    }
    return count;
}

/**
 * Get data type from token
 */
DataType get_token_data_type(Token *token) {
    if (!token) return TYPE_DYNAMIC;

    switch (token->type) {
        case INT:
        case FLOAT:
            return TYPE_NUM;
        case STRING_TOKEN:
        case ML_STRING_TOKEN:
            return TYPE_STRING;
        case KEYWORD:
            if (token->data.integer == NULL_LOWERCASE)
                return TYPE_NULL;
            return TYPE_DYNAMIC;
        default:
            return TYPE_DYNAMIC;
    }
}

/**
 * Enhanced type compatibility check according to IFJ25 specification
 */
int check_literal_type_compatibility(Token_type operation, DataType left_type, DataType right_type) {
    switch (operation) {
        case ADDITION:
            // String concatenation OR numeric addition
            if ((left_type == TYPE_STRING && right_type == TYPE_STRING) ||
                (left_type == TYPE_NUM && right_type == TYPE_NUM)) {
                return 0;
            }
            // Mixed string and number is error
            if ((left_type == TYPE_STRING && right_type == TYPE_NUM) ||
                (left_type == TYPE_NUM && right_type == TYPE_STRING)) {
                return SEMANTIC_TYPE_ERROR; // Error 6
            }
            break;

        case SUBTRACTION:
        case MULTIPLICATION:
        case DIVISION:
            // Only numeric operations
            if (left_type != TYPE_NUM || right_type != TYPE_NUM) {
                return SEMANTIC_TYPE_ERROR; // Error 6
            }
            break;

        case LESSER_THAN:
        case LESSER_OR_EQUAL:
        case GREATER_THAN:
        case GREATER_OR_EQUAL:
            // Only numeric comparisons
            if (left_type != TYPE_NUM || right_type != TYPE_NUM) {
                return SEMANTIC_TYPE_ERROR; // Error 6
            }
            break;

        case EQUAL:
        case NOT_EQUAL:
            // Any types can be compared for equality
            return 0;

        default:
            break;
    }

    return SEMANTIC_TYPE_ERROR; // Error 6
}

/**
 * Analyze function call with parameter validation
 */
int analyze_function_call(SemanticContext *context, const char *func_name, Node *args_node) {
    if (!func_name) return SEMANTIC_OTHER_ERROR;

    // Check if function exists
    Symbol *func_symbol = find_symbol(context, func_name);
    if (!func_symbol) {
        // Built-in functions are always available
        if (!is_builtin_function(func_name)) {
            return SEMANTIC_UNDEFINED_ERROR; // Error 3
        }
    }

    // Count actual arguments
    int actual_count = count_arguments(args_node);

    // Validate parameter count
    if (func_symbol) {
        if (func_symbol->param_count != actual_count) {
            return SEMANTIC_PARAM_COUNT_ERROR; // Error 5
        }
    } else if (is_builtin_function(func_name)) {
        // Validate built-in function parameter counts
        int result = validate_builtin_function(func_name, actual_count);
        if (result != 0) return result;
    }

    return 0;
}

/**
 * Analyze expression with full type checking
 */
int analyze_expression(SemanticContext *context, Node *node) {
    if (!node) return 0;

    int result = 0;

    // Check variable references
    if (node->token && node->token->type == IDENTIFIER) {
        char *var_name = node->token->data.characters;

        // Check if it's a getter call (no parentheses)
        Symbol *symbol = find_symbol(context, var_name);
        if (symbol && symbol->type == SYMBOL_GETTER) {
            // Getter call - no parameters allowed
            return 0;
        }

        // Regular variable reference
        result = analyze_variable_reference(context, var_name);
        if (result != 0) return result;
    }

    // Check function calls
    if (node->nonterminal == FUNCHEAD) {
        Node *id_node = node->left_child;
        if (id_node && id_node->token) {
            result = analyze_function_call(context, id_node->token->data.characters, node->right_child);
            if (result != 0) return result;
        }
    }

    // Check binary operations for type compatibility
    if (node->left_child && node->right_child && node->token) {
        DataType left_type = TYPE_DYNAMIC;
        DataType right_type = TYPE_DYNAMIC;

        // Get left operand type from literal if available
        if (node->left_child->token) {
            left_type = get_token_data_type(node->left_child->token);
        }

        // Get right operand type from literal if available
        if (node->right_child->token) {
            right_type = get_token_data_type(node->right_child->token);
        }

        // Check type compatibility for known types
        if (left_type != TYPE_DYNAMIC && right_type != TYPE_DYNAMIC) {
            result = check_literal_type_compatibility(node->token->type, left_type, right_type);
            if (result != 0) return result;
        }
    }

    // Handle is operator specially
    if (node->token && node->token->type == COMPARISON) {
        // Right side must be String, Num, or Null type keyword
        if (node->right_child && node->right_child->token) {
            Token *right_token = node->right_child->token;
            if (right_token->type != KEYWORD ||
            (right_token->data.integer != STRING_TYPE &&
                 right_token->data.integer != NUM_TYPE &&
                 right_token->data.integer != NULL_UPPERCASE))
            {
                return SEMANTIC_TYPE_ERROR; // Error 6
            }
        }
    }

    // Recursively analyze sub-expressions
    if (node->left_child) {
        result = analyze_expression(context, node->left_child);
        if (result != 0) return result;
    }

    if (node->right_child) {
        result = analyze_expression(context, node->right_child);
        if (result != 0) return result;
    }

    return result;
}

/**
 * Helpers to find nodes in AST
 */
static Node* find_first_nonterminal(Node *node, int target_nt) {
    if (!node) return NULL;
    if (node->nonterminal == target_nt) return node;
    Node *res = find_first_nonterminal(node->left_child, target_nt);
    if (res) return res;
    return find_first_nonterminal(node->right_child, target_nt);
}

static Node* find_first_identifier(Node *node) {
    if (!node) return NULL;
    if (node->token && node->token->type == IDENTIFIER) return node;
    Node *res = find_first_identifier(node->left_child);
    if (res) return res;
    return find_first_identifier(node->right_child);
}

/**
 * Find function parameters in AST
 */
Node* find_function_parameters(Node *funcdecl) {
    if (!funcdecl) return NULL;

    Node *head = NULL;
    if (funcdecl->left_child && funcdecl->left_child->nonterminal == FUNCHEAD) {
        head = funcdecl->left_child;
    } else {
        head = find_first_nonterminal(funcdecl, FUNCHEAD);
    }

    if (head) return head->right_child;
    return NULL;
}

/**
 * Analyze function definition
 */
int analyze_function(SemanticContext *context, Node *node) {
    if (!context || !node) {
        return SEMANTIC_OTHER_ERROR;
    }

    // Find function head
    Node *head = NULL;
    if (node->left_child && node->left_child->nonterminal == FUNCHEAD) {
        head = node->left_child;
    } else {
        head = find_first_nonterminal(node, FUNCHEAD);
    }

    const char *func_name = NULL;
    Node *params_node = NULL;

    if (head) {
        Node *id_node = head->left_child;
        if (id_node && id_node->token == NULL && id_node->left_child && id_node->left_child->token) {
            id_node = id_node->left_child;
        }
        if (id_node && id_node->token) {
            func_name = id_node->token->data.characters;
        }
        params_node = head->right_child;
    }

    // Fallback for function name
    if (!func_name) {
        if (node->token && node->token->type == IDENTIFIER) {
            func_name = node->token->data.characters;
        } else {
            Node *id = find_first_identifier(node);
            if (id && id->token) {
                func_name = id->token->data.characters;
            }
        }
    }

    if (!func_name) {
        return SEMANTIC_OTHER_ERROR;
    }

    // Determine parameters
    if (!params_node) {
        params_node = find_function_parameters(node);
    }
    int param_count = count_identifier_parameters(params_node);

    // Add function to global symbol table
    int result = add_symbol(context, func_name, SYMBOL_FUNCTION, param_count, NULL);
    if (result != 0) {
        return result;
    }

    // Enter function scope
    result = enter_scope(context);
    if (result != 0) return result;

    // Add parameters to scope as local variables
    Node *param_node = params_node;
    while (param_node) {
        if (param_node->token && param_node->token->type == IDENTIFIER) {
            result = add_symbol(context, param_node->token->data.characters, SYMBOL_VARIABLE, 0, NULL);
            if (result != 0) {
                exit_scope(context);
                return result;
            }
        }
        param_node = param_node->right_child;
    }

    // Analyze function body
    Node *body = node->right_child ? node->right_child : NULL;
    if (body) {
        result = analyze_node(context, body);
    }

    exit_scope(context);
    return result;
}

/**
 * Check if node is a getter definition
 */
bool is_getter(Node *node) {
    if (!node || node->nonterminal != FUNCDECL) return false;

    Node *head = node->left_child;
    if (!head || head->nonterminal != FUNCHEAD) return false;

    // Getters have no parameters
    Node *params = head->right_child;
    if (params && count_identifier_parameters(params) > 0) return false;

    return true;
}

/**
 * Check if node is a setter definition
 */
bool is_setter(Node *node) {
    if (!node || node->nonterminal != FUNCDECL) return false;

    Node *head = node->left_child;
    if (!head || head->nonterminal != FUNCHEAD) return false;

    // Setters have exactly one parameter
    Node *params = head->right_child;
    if (!params || count_identifier_parameters(params) != 1) return false;

    return true;
}

/**
 * Analyze class body - methods, getters, setters
 */
int analyze_class_body(SemanticContext *context, Node *body_node, const char *class_name) {
    if (!body_node) return 0;

    int result = 0;
    Node *current = body_node;

    while (current) {
        Node *element = current->left_child;
        if (element && element->nonterminal == FUNCDECL) {
            if (is_getter(element)) {
                result = analyze_getter(context, element, class_name);
            } else if (is_setter(element)) {
                result = analyze_setter(context, element, class_name);
            } else {
                result = analyze_method(context, element, class_name);
            }

            if (result != 0) break;
        }
        current = current->right_child;
    }

    return result;
}

/**
 * Analyze class definition with getters and setters
 */
int analyze_class(SemanticContext *context, Node *node) {
    if (!node || !node->token) return SEMANTIC_OTHER_ERROR;

    char *class_name = node->token->data.characters;

    // Verify it's class Program
    if (strcmp(class_name, "Program") != 0) {
        return SEMANTIC_OTHER_ERROR;
    }

    // Add class to global symbol table
    int result = add_symbol(context, class_name, SYMBOL_CLASS, 0, NULL);
    if (result != 0) return result;

    // Enter class scope
    result = enter_class_scope(context, class_name);
    if (result != 0) return result;

    // Analyze class body
    Node *body_node = node->right_child;
    if (body_node) {
        result = analyze_class_body(context, body_node, class_name);
    }

    exit_class_scope(context);
    return result;
}

/**
 * Analyze method definition
 */
int analyze_method(SemanticContext *context, Node *node, const char *class_name) {
    if (!node || !node->token) return SEMANTIC_OTHER_ERROR;

    char *method_name = node->token->data.characters;
    int param_count = count_identifier_parameters(node->left_child);

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
    Node *param_node = node->left_child;
    while (param_node) {
        if (param_node->token && param_node->token->type == IDENTIFIER) {
            result = add_symbol(context, param_node->token->data.characters, SYMBOL_VARIABLE, 0, NULL);
            if (result != 0) {
                exit_scope(context);
                return result;
            }
        }
        param_node = param_node->right_child;
    }

    Node *body_node = node->right_child;
    if (body_node) {
        result = analyze_node(context, body_node);
    }

    exit_scope(context);
    return result;
}

/**
 * Analyze getter definition
 */
int analyze_getter(SemanticContext *context, Node *node, const char *class_name) {
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

    Node *body_node = node->right_child;
    if (body_node) {
        result = analyze_node(context, body_node);
    }

    exit_scope(context);
    return result;
}

/**
 * Analyze setter definition
 */
int analyze_setter(SemanticContext *context, Node *node, const char *class_name) {
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

    Node *body_node = node->right_child ? node->right_child->right_child : NULL;
    if (body_node) {
        result = analyze_node(context, body_node);
    }

    exit_scope(context);
    return result;
}

/**
 * Analyze variable declaration
 */
int analyze_var_declaration(SemanticContext *context, Node *node) {
    if (!node || !node->token) return SEMANTIC_OTHER_ERROR;

    char *var_name = node->token->data.characters;

    // Check if variable is already declared in current scope
    Symbol *existing = symtable_find(context->current_symtable, var_name);
    if (existing && existing->type == SYMBOL_VARIABLE) {
        return SEMANTIC_REDEFINITION_ERROR; // Error 4
    }

    // Add variable to symbol table
    return add_symbol(context, var_name, SYMBOL_VARIABLE, 0, NULL);
}

/**
 * Analyze assignment statement
 */
int analyze_assignment(SemanticContext *context, Node *node) {
    if (!node || !node->left_child || !node->left_child->token) {
        return SEMANTIC_OTHER_ERROR;
    }

    char *var_name = node->left_child->token->data.characters;
    int result = 0;

    // Check left side validity
    if (is_global_variable(var_name)) {
        // Global variable - always OK
    } else {
        Symbol *symbol = find_symbol(context, var_name);
        if (symbol && symbol->type == SYMBOL_SETTER) {
            // Setter call - OK
        } else {
            // Local variable - must be declared
            result = analyze_variable_reference(context, var_name);
            if (result != 0) return result;
        }
    }

    // Analyze expression being assigned
    if (node->right_child) {
        result = analyze_expression(context, node->right_child);
    }

    return result;
}

/**
 * Analyze if statement
 */
int analyze_if_statement(SemanticContext *context, Node *node) {
    if (!node) return SEMANTIC_OTHER_ERROR;

    int result = 0;

    // Analyze condition
    if (node->left_child) {
        result = analyze_expression(context, node->left_child);
        if (result != 0) return result;
    }

    // Analyze then and else branches
    if (node->right_child) {
        Node *then_branch = node->right_child->left_child;
        if (then_branch) {
            result = analyze_node(context, then_branch);
            if (result != 0) return result;
        }

        Node *else_branch = node->right_child->right_child;
        if (else_branch) {
            result = analyze_node(context, else_branch);
        }
    }

    return result;
}

/**
 * Analyze while statement
 */
int analyze_while_statement(SemanticContext *context, Node *node) {
    if (!node) return SEMANTIC_OTHER_ERROR;

    int result = 0;

    // Analyze condition
    if (node->left_child) {
        result = analyze_expression(context, node->left_child);
        if (result != 0) return result;
    }

    // Analyze body
    if (node->right_child) {
        result = analyze_node(context, node->right_child);
    }

    return result;
}

/**
 * Analyze return statement
 */
int analyze_return_statement(SemanticContext *context, Node *node) {
    if (!node) return SEMANTIC_OTHER_ERROR;

    // Return must be inside a function
    if (!context->in_function) {
        return SEMANTIC_OTHER_ERROR;
    }

    // Analyze return expression
    if (node->left_child) {
        return analyze_expression(context, node->left_child);
    }

    return 0;
}

/**
 * Check program structure (prolog + class Program)
 */
int check_program_structure(SemanticContext *context, Node *node) {
    if (!node || node->nonterminal != PROGRAM) {
        return SEMANTIC_OTHER_ERROR;
    }

    // Check we have both children
    if (!node->left_child || !node->right_child) {
        return SEMANTIC_OTHER_ERROR;
    }

    // Left child should be prolog, right child should be class
    if (node->left_child->nonterminal != PROLOG || node->right_child->nonterminal != CLASS_NT) {
        return SEMANTIC_OTHER_ERROR;
    }

    // Class should be named "Program"
    if (!node->right_child->token || strcmp(node->right_child->token->data.characters, "Program") != 0) {
        return SEMANTIC_OTHER_ERROR;
    }

    return 0;
}

/**
 * Check if main function exists and is valid
 */
int check_main_function(SemanticContext *context) {
    // Check for main function in global scope
    Symbol *main_func = symtable_find(context->global_symtable, "main");
    if (!main_func || main_func->type != SYMBOL_FUNCTION) {
        return SEMANTIC_UNDEFINED_ERROR; // Error 3
    }

    // Main must have exactly 0 parameters
    if (main_func->param_count != 0) {
        return SEMANTIC_PARAM_COUNT_ERROR; // Error 5
    }

    return 0;
}

/**
 * Analyze AST node recursively - main dispatch function
 */
int analyze_node(SemanticContext *context, Node *node) {
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
            // Already validated in structure check
            break;

        case CLASS_NT:
            result = analyze_class(context, node);
            break;

        case FUNCDECL:
            if (context->current_class) {
                // In class context - method, getter, or setter
                if (is_getter(node)) {
                    result = analyze_getter(context, node, context->current_class);
                } else if (is_setter(node)) {
                    result = analyze_setter(context, node, context->current_class);
                } else {
                    result = analyze_method(context, node, context->current_class);
                }
            } else {
                // Global function
                result = analyze_function(context, node);
            }
            break;

        case SEQUENCE:
        {
            Node *current = node;
            while (current) {
                result = analyze_node(context, current->left_child);
                if (result != 0) break;
                current = current->right_child;
            }
        }
        break;

        case ASSIGN:
            result = analyze_assignment(context, node);
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

        // Handle other expression nonterminals
        case EXPR:
        case LOGICOR:
        case LOGICAND:
        case EQUALITY:
        case RELATIONAL:
        case ADDITIVE:
        case MULTIPLICATIVE:
        case UNARY:
        case PRIMARY:
            result = analyze_expression(context, node);
            break;

        default:
            // Recursively analyze children
            if (node->left_child) {
                result = analyze_node(context, node->left_child);
                if (result != 0) return result;
            }
            if (node->right_child) {
                result = analyze_node(context, node->right_child);
            }
            break;
    }

    return result;
}

/**
 * Main semantic analysis function
 */
int semantic_analyze(Node *ast) {
    // For now, always return 0 to test if basic structure works
    return 0;
}
