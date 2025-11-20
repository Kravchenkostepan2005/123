#include "semantic.h"
#include "ast.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ScopeFrame {
    symtable_t        *table;
    struct ScopeFrame *parent;
} ScopeFrame;

static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1U;
    char *d = (char *)malloc(n);
    if (!d) return NULL;
    memcpy(d, s, n);
    return d;
}

static int push_scope_frame(SemanticContext *context, symtable_t *table) {
    ScopeFrame *frame = malloc(sizeof(ScopeFrame));
    if (!frame) {
        return INTERNAL_ERROR;
    }
    frame->table  = table;
    frame->parent = context->scope_stack;
    context->scope_stack = frame;
    context->current_symtable = table;
    return 0;
}

static void pop_scope_frame(SemanticContext *context, bool keep_global) {
    if (!context || !context->scope_stack) {
        return;
    }

    ScopeFrame *frame = context->scope_stack;
    if (!frame->parent && keep_global) {
        return;
    }

    context->scope_stack = frame->parent;
    context->current_symtable = context->scope_stack ? context->scope_stack->table : NULL;

    if (frame->table) {
        symtable_destroy(frame->table);
    }
    free(frame);
}

static void destroy_scope_stack(SemanticContext *context) {
    while (context->scope_stack) {
        ScopeFrame *next = context->scope_stack->parent;
        if (context->scope_stack->table) {
            symtable_destroy(context->scope_stack->table);
        }
        free(context->scope_stack);
        context->scope_stack = next;
    }
    context->current_symtable = NULL;
}

static Symbol *lookup_scope_chain(ScopeFrame *frame, const char *name) {
    for (ScopeFrame *current = frame; current; current = current->parent) {
        Symbol *symbol = symtable_find(current->table, name);
        if (symbol) {
            return symbol;
        }
    }
    return NULL;
}

static bool subtree_has_token(Node *node, Token_type type) {
    if (!node) {
        return false;
    }
    if (node->token && node->token->type == type) {
        return true;
    }
    if (subtree_has_token(node->left_child, type)) {
        return true;
    }
    return subtree_has_token(node->right_sibling, type);
}

static int validate_prolog(Node *prolog) {
    if (!prolog) {
        return SYNTACTICAL_ERROR;
    }

    bool import_kw = false;
    bool string_ok = false;
    bool for_kw    = false;
    bool ifj_kw    = false;

    for (Node *child = prolog->left_child; child; child = child->right_sibling) {
        if (!child->token) {
            continue;
        }
        if (child->token->type == KEYWORD) {
            switch ((Keyword_type)child->token->data.integer) {
                case IMPORT: import_kw = true; break;
                case FOR:    for_kw    = true; break;
                case IFJ:    ifj_kw    = true; break;
                default: break;
            }
        } else if (child->token->type == STRING_TOKEN && child->token->data.characters) {
            if (strcmp(child->token->data.characters, "ifj25") == 0) {
                string_ok = true;
            }
        }
    }

    return (import_kw && for_kw && ifj_kw && string_ok) ? 0 : SYNTACTICAL_ERROR;
}

static int analyze_block_node(SemanticContext *context, Node *block, bool create_scope);
static int analyze_node_impl(SemanticContext *context, Node *node);

/* ---------- Lifecycle ---------- */

SemanticContext *semantic_init(void) {
    SemanticContext *context = malloc(sizeof(SemanticContext));
    if (!context) {
        return NULL;
    }

    context->global_symtable = symtable_create();
    if (!context->global_symtable) {
        free(context);
        return NULL;
    }

    context->current_symtable = NULL;
    context->class_symtable   = NULL;
    context->current_class    = NULL;
    context->in_class         = false;
    context->in_function      = false;
    context->error_count      = 0;
    context->scope_stack      = NULL;

    if (push_scope_frame(context, context->global_symtable) != 0) {
        symtable_destroy(context->global_symtable);
        free(context);
        return NULL;
    }

    return context;
}

void semantic_cleanup(SemanticContext *context) {
    if (!context) {
        return;
    }

    if (context->class_symtable) {
        symtable_destroy(context->class_symtable);
        context->class_symtable = NULL;
    }

    destroy_scope_stack(context);

    if (context->current_class) {
        free(context->current_class);
        context->current_class = NULL;
    }

    free(context);
}

/* ---------- Scope helpers ---------- */

int enter_scope(SemanticContext *context) {
    if (!context) {
        return INTERNAL_ERROR;
    }

    symtable_t *sym = symtable_create();
    if (!sym) {
        return INTERNAL_ERROR;
    }

    if (push_scope_frame(context, sym) != 0) {
        symtable_destroy(sym);
        return INTERNAL_ERROR;
    }

    return 0;
}

void exit_scope(SemanticContext *context) {
    pop_scope_frame(context, true);
}

int enter_class_scope(SemanticContext *context, const char *class_name) {
    if (!context || !class_name) {
        return INTERNAL_ERROR;
    }

    context->class_symtable = symtable_create();
    if (!context->class_symtable) {
        return INTERNAL_ERROR;
    }

    context->current_class = xstrdup(class_name);
    if (!context->current_class) {
        symtable_destroy(context->class_symtable);
        context->class_symtable = NULL;
        return INTERNAL_ERROR;
    }

    context->in_class = true;
    return 0;
}

void exit_class_scope(SemanticContext *context) {
    if (!context) {
        return;
    }

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

/* ---------- Symbol helpers ---------- */

static int insert_class_member_symbol(SemanticContext *context,
                                      const char *name,
                                      SymbolType type,
                                      int param_count,
                                      const char *class_name) {
    if (!context || !context->class_symtable || !name || !class_name) {
        return INTERNAL_ERROR;
    }

    Symbol *symbol = malloc(sizeof(Symbol));
    if (!symbol) {
        return INTERNAL_ERROR;
    }

    symbol->name       = xstrdup(name);
    symbol->type       = type;
    symbol->data_type  = TYPE_DYNAMIC;
    symbol->param_count = param_count;
    symbol->is_defined = 1;
    symbol->class_name = xstrdup(class_name);
    symbol->members    = NULL;

    if (!symbol->name || !symbol->class_name) {
        free(symbol->name);
        free(symbol->class_name);
        free(symbol);
        return INTERNAL_ERROR;
    }

    if (symtable_insert(context->class_symtable, name, symbol) != 0) {
        free(symbol->name);
        free(symbol->class_name);
        free(symbol);
        return INTERNAL_ERROR;
    }

    return 0;
}

Symbol *find_symbol(SemanticContext *context, const char *name) {
    if (!context || !name) {
        return NULL;
    }

    Symbol *symbol = lookup_scope_chain(context->scope_stack, name);
    if (symbol) {
        return symbol;
    }

    if (context->in_class && context->class_symtable) {
        symbol = symtable_find(context->class_symtable, name);
        if (symbol) {
            return symbol;
        }
    }

    return NULL;
}

int add_symbol(SemanticContext *context,
               const char *name,
               SymbolType type,
               int param_count,
               const char *class_name) {
    if (!context || !name || !context->scope_stack) {
        return INTERNAL_ERROR;
    }

    symtable_t *current_table = context->scope_stack->table;
    Symbol *existing = symtable_find(current_table, name);
    if (existing) {
        if (type == SYMBOL_VARIABLE && existing->type == SYMBOL_VARIABLE) {
            return SEMANTIC_REDEFINITION_ERROR;
        }

        if ((type == SYMBOL_FUNCTION || type == SYMBOL_METHOD) &&
            (existing->type == SYMBOL_FUNCTION || existing->type == SYMBOL_METHOD) &&
            existing->param_count == param_count) {
            return SEMANTIC_REDEFINITION_ERROR;
        }

        if ((type == SYMBOL_GETTER && existing->type == SYMBOL_GETTER) ||
            (type == SYMBOL_SETTER && existing->type == SYMBOL_SETTER)) {
            return SEMANTIC_REDEFINITION_ERROR;
        }
    }

    Symbol *symbol = malloc(sizeof(Symbol));
    if (!symbol) {
        return INTERNAL_ERROR;
    }

    symbol->name       = xstrdup(name);
    symbol->type       = type;
    symbol->data_type  = TYPE_DYNAMIC;
    symbol->param_count = param_count;
    symbol->is_defined = 1;
    symbol->class_name = class_name ? xstrdup(class_name) : NULL;
    symbol->members    = NULL;

    if (!symbol->name || (class_name && !symbol->class_name)) {
        free(symbol->name);
        free(symbol->class_name);
        free(symbol);
        return INTERNAL_ERROR;
    }

    if (type == SYMBOL_CLASS) {
        symbol->members = symtable_create();
        if (!symbol->members) {
            free(symbol->name);
            free(symbol->class_name);
            free(symbol);
            return INTERNAL_ERROR;
        }
    }

    if (symtable_insert(current_table, name, symbol) != 0) {
        free(symbol->name);
        free(symbol->class_name);
        if (symbol->members) {
            symtable_destroy(symbol->members);
        }
        free(symbol);
        return INTERNAL_ERROR;
    }

    return 0;
}

/* ---------- Utility predicates ---------- */

int is_global_variable(const char *name) {
    return (name && strlen(name) >= 2 && name[0] == '_' && name[1] == '_');
}

int is_builtin_function(const char *name) {
    return (name && strncmp(name, "Ifj.", 4) == 0);
}

int validate_builtin_function(const char *func_name, int param_count) {
    if (!func_name) {
        return 0;
    }

    if (!strcmp(func_name, "Ifj.read_str") || !strcmp(func_name, "Ifj.read_num")) {
        return (param_count == 0) ? 0 : SEMANTIC_PARAM_COUNT_ERROR;
    }

    if (!strcmp(func_name, "Ifj.write")  ||
        !strcmp(func_name, "Ifj.str")    ||
        !strcmp(func_name, "Ifj.floor")  ||
        !strcmp(func_name, "Ifj.length") ||
        !strcmp(func_name, "Ifj.chr")) {
        return (param_count == 1) ? 0 : SEMANTIC_PARAM_COUNT_ERROR;
    }

    if (!strcmp(func_name, "Ifj.strcmp") || !strcmp(func_name, "Ifj.ord")) {
        return (param_count == 2) ? 0 : SEMANTIC_PARAM_COUNT_ERROR;
    }

    if (!strcmp(func_name, "Ifj.substring")) {
        return (param_count == 3) ? 0 : SEMANTIC_PARAM_COUNT_ERROR;
    }

    return 0;
}

/* ---------- Counters ---------- */

static int count_identifier_parameters(Node *node) {
    int count = 0;
    if (!node) {
        return 0;
    }
    for (Node *current = node->left_child; current; current = current->right_sibling) {
        if (current->token && current->token->type == IDENTIFIER) {
            count++;
        }
    }
    return count;
}

static int count_arguments(Node *node) {
    int count = 0;
    for (Node *current = node ? node->left_child : NULL; current; current = current->right_sibling) {
        count++;
    }
    return count;
}

/* ---------- Type helpers ---------- */

DataType get_token_data_type(Token *token) {
    if (!token) {
        return TYPE_DYNAMIC;
    }

    switch (token->type) {
        case INT:
        case FLOAT:
            return TYPE_NUM;
        case STRING_TOKEN:
        case ML_STRING_TOKEN:
            return TYPE_STRING;
        case KEYWORD:
            if (token->data.integer == NULL_LOWERCASE) {
                return TYPE_NULL;
            }
            return TYPE_DYNAMIC;
        default:
            return TYPE_DYNAMIC;
    }
}

int check_literal_type_compatibility(Token_type operation,
                                     DataType left_type,
                                     DataType right_type) {
    switch (operation) {
        case ADDITION:
            if ((left_type == TYPE_STRING && right_type == TYPE_STRING) ||
                (left_type == TYPE_NUM && right_type == TYPE_NUM)) {
                return 0;
            }
            if ((left_type == TYPE_STRING && right_type == TYPE_NUM) ||
                (left_type == TYPE_NUM && right_type == TYPE_STRING)) {
                return SEMANTIC_TYPE_ERROR;
            }
            break;

        case SUBTRACTION:
        case MULTIPLICATION:
        case DIVISION:
            if (left_type != TYPE_NUM || right_type != TYPE_NUM) {
                return SEMANTIC_TYPE_ERROR;
            }
            break;

        case LESSER_THAN:
        case LESSER_OR_EQUAL:
        case GREATER_THAN:
        case GREATER_OR_EQUAL:
            if (left_type != TYPE_NUM || right_type != TYPE_NUM) {
                return SEMANTIC_TYPE_ERROR;
            }
            break;

        case EQUAL:
        case NOT_EQUAL:
            return 0;

        default:
            break;
    }

    return SEMANTIC_TYPE_ERROR;
}

/* ---------- AST helpers ---------- */

static Node *find_first_nonterminal(Node *node, int target_nt) {
    if (!node) {
        return NULL;
    }
    if (node->nonterminal == (Nonterminal_type)target_nt) {
        return node;
    }
    Node *res = find_first_nonterminal(node->left_child, target_nt);
    if (res) {
        return res;
    }
    return find_first_nonterminal(node->right_sibling, target_nt);
}

static Node *find_first_identifier(Node *node) {
    if (!node) {
        return NULL;
    }
    if (node->token && node->token->type == IDENTIFIER) {
        return node;
    }

    Node *res = find_first_identifier(node->left_child);
    if (res) {
        return res;
    }
    return find_first_identifier(node->right_sibling);
}

static Node *find_direct_identifier_child(Node *node) {
    for (Node *child = node ? node->left_child : NULL; child; child = child->right_sibling) {
        if (child->token && child->token->type == IDENTIFIER) {
            return child;
        }
    }
    return NULL;
}

static Node *find_direct_nonterminal_child(Node *node, int target_nt) {
    for (Node *child = node ? node->left_child : NULL; child; child = child->right_sibling) {
        if (child->nonterminal == (Nonterminal_type)target_nt) {
            return child;
        }
    }
    return NULL;
}

Node *find_function_parameters(Node *funcdecl) {
    if (!funcdecl) {
        return NULL;
    }

    Node *head = NULL;
    if (funcdecl->left_child && funcdecl->left_child->nonterminal == FUNCHEAD) {
        head = funcdecl->left_child;
    } else {
        head = find_first_nonterminal(funcdecl, FUNCHEAD);
    }

    return head ? head : NULL;
}

static int extract_function_signature(Node *funcdecl,
                                      const char **out_name,
                                      Node **out_params) {
    if (!funcdecl || !out_name || !out_params) {
        return SEMANTIC_OTHER_ERROR;
    }

    const char *name = NULL;
    Node *params = NULL;

    Node *head = NULL;
    if (funcdecl->left_child && funcdecl->left_child->nonterminal == FUNCHEAD) {
        head = funcdecl->left_child;
    } else {
        head = find_first_nonterminal(funcdecl, FUNCHEAD);
    }
    if (head) {
        if (head->left_sibling == NULL) {
            name = NULL;
        } else {
            name = head->left_sibling->token->data.characters;
        }
        params = head->left_child;
    }
    if (!name) {
        return SEMANTIC_OTHER_ERROR;
    }

    *out_name = name;
    *out_params = params;
    return 0;
}

/* ---------- Core analyzers ---------- */

int analyze_variable_reference(SemanticContext *context, const char *var_name) {
    if (!var_name) {
        return SEMANTIC_OTHER_ERROR;
    }

    if (is_global_variable(var_name)) {
        return 0;
    }

    Symbol *symbol = find_symbol(context, var_name);
    if (!symbol || symbol->type != SYMBOL_VARIABLE) {
        return SEMANTIC_UNDEFINED_ERROR;
    }

    return 0;
}

int analyze_function_call(SemanticContext *context,
                          const char *func_name,
                          Node *args_node) {
    if (!func_name) {
        return SEMANTIC_OTHER_ERROR;
    }

    Symbol *func_symbol = find_symbol(context, func_name);
    if (!func_symbol) {
        if (!is_builtin_function(func_name)) {
            return SEMANTIC_UNDEFINED_ERROR;
        }
    }

    int actual_count = count_arguments(args_node);

    if (func_symbol) {
        if (func_symbol->param_count != actual_count) {
            return SEMANTIC_PARAM_COUNT_ERROR;
        }
    } else {
        int builtin_check = validate_builtin_function(func_name, actual_count);
        if (builtin_check != 0) {
            return builtin_check;
        }
    }

    return 0;
}

static int check_function_calls_in_subtree(SemanticContext *context, Node *node) {
    if (!node) return 0;

    int result = 0;

    if (node->nonterminal == FUNCHEAD) {
        Node *id_node = find_first_identifier(node);
        if (id_node && id_node->token) {
            const char *func_name = id_node->token->data.characters;
            Node *args_node = node->right_child;
            result = analyze_function_call(context, func_name, args_node);
            if (result != 0) return result;
        }
    }

    if (node->token && node->token->type == IDENTIFIER) {
        if (node->left_child) {
            const char *func_name = node->token->data.characters;
            result = analyze_function_call(context, func_name, node->left_child);
            if (result != 0) return result;
        }
    }

    if (node->left_child) {
        result = check_function_calls_in_subtree(context, node->left_child);
        if (result != 0) return result;
    }

    if (node->right_sibling) {
        result = check_function_calls_in_subtree(context, node->right_sibling);
        if (result != 0) return result;
    }

    return 0;
}

int analyze_expression(SemanticContext *context, Node *node) {
    if (!node) {
        return 0;
    }

    int result = 0;

    if (node->token && node->token->type == IDENTIFIER) {
        const char *var_name = node->token->data.characters;
        Symbol *symbol = find_symbol(context, var_name);
        if (symbol && symbol->type == SYMBOL_GETTER) {
            return 0;
        }

        result = analyze_variable_reference(context, var_name);
        if (result != 0) {
            return result;
        }
    }

    if (node->nonterminal == FUNCHEAD) {
        Node *id_node = node->left_child;
        while (id_node && (!id_node->token || id_node->token->type != IDENTIFIER)) {
            id_node = id_node->left_child;
        }

        if (id_node && id_node->token) {
            result = analyze_function_call(context,
                                           id_node->token->data.characters,
                                           node->right_child);
            if (result != 0) {
                return result;
            }
        }
    }


    if (node->token && node->token->type == IDENTIFIER && node->left_child) {
        result = analyze_function_call(context, node->token->data.characters, node->left_child);
        if (result != 0) {
            return result;
        }
    }

    if (node->left_child && node->right_child && node->token) {
        DataType left_type = node->left_child->token
                                 ? get_token_data_type(node->left_child->token)
                                 : TYPE_DYNAMIC;
        DataType right_type = node->right_child->token
                                  ? get_token_data_type(node->right_child->token)
                                  : TYPE_DYNAMIC;

        if (left_type != TYPE_DYNAMIC && right_type != TYPE_DYNAMIC) {
            result = check_literal_type_compatibility(node->token->type,
                                                      left_type,
                                                      right_type);
            if (result != 0) {
                return result;
            }
        }
    }

    if (node->token && node->token->type == COMPARISON) {
        if (node->right_child && node->right_child->token) {
            Token *right = node->right_child->token;
            if (right->type != KEYWORD ||
                (right->data.integer != STRING_TYPE &&
                 right->data.integer != NUM_TYPE &&
                 right->data.integer != NULL_UPPERCASE)) {
                return SEMANTIC_TYPE_ERROR;
            }
        }
    }

    if (node->left_child) {
        result = analyze_expression(context, node->left_child);
        if (result != 0) {
            return result;
        }
    }

    if (node->right_child) {
        result = analyze_expression(context, node->right_child);
        if (result != 0) {
            return result;
        }
    }

    return 0;
}

int analyze_function(SemanticContext *context, Node *node) {
    if (!context || !node) {
        return SEMANTIC_OTHER_ERROR;
    }

    const char *func_name = NULL;
    Node *params_node = NULL;
    int result = extract_function_signature(node, &func_name, &params_node);
    if (result != 0) {
        return result;
    }

    int param_count = count_identifier_parameters(params_node);
    result = add_symbol(context, func_name, SYMBOL_FUNCTION, param_count, NULL);
    if (result != 0) {
        return result;
    }

    result = enter_scope(context);
    if (result != 0) {
        return result;
    }

    bool prev_in_function = context->in_function;
    context->in_function = true;

    for (Node *param = params_node; param; param = param->right_child) {
        if (param->token && param->token->type == IDENTIFIER) {
            result = add_symbol(context,
                                param->token->data.characters,
                                SYMBOL_VARIABLE,
                                0,
                                NULL);
            if (result != 0) {
                break;
            }
        }
    }

    if (result == 0) {
        Node *body = find_direct_nonterminal_child(node, BLOCK);
        if (body) {
            result = analyze_block_node(context, body, false);
            if (result == 0) {
                result = check_function_calls_in_subtree(context, body);
            }
        }
    }

    context->in_function = prev_in_function;
    exit_scope(context);
    return result;
}

bool is_getter(Node *node) {
    if (!node || node->nonterminal != FUNCDECL) {
        return false;
    }
    Node *params = find_function_parameters(node);
    if (params == NULL) {
        return true;
    }
    return false;
}

bool is_setter(Node *node) {
    if (!node || node->nonterminal != FUNCDECL) {
        return false;
    }

    Node *params = find_function_parameters(node);
    if (params == NULL) {
        return false;
    }
    if (count_identifier_parameters(params) == 2) {
        if (params->left_child && params->left_child->token) {
            if (params->left_child->token->data.integer == EQUAL) {
                return true;
            }
        }
    }
    return false;
}

int analyze_method(SemanticContext *context, Node *node, const char *class_name) {
    if (!context || !node || !class_name) {
        return SEMANTIC_OTHER_ERROR;
    }

    const char *method_name = NULL;
    Node *params_node = NULL;
    int result = extract_function_signature(node, &method_name, &params_node);
    if (result != 0) {
        return result;
    }

    int param_count = count_identifier_parameters(params_node);
    result = insert_class_member_symbol(context,
                                        method_name,
                                        SYMBOL_METHOD,
                                        param_count,
                                        class_name);
    if (result != 0) {
        return result;
    }

    result = enter_scope(context);
    if (result != 0) {
        return result;
    }

    bool prev_in_function = context->in_function;
    context->in_function = true;

    Node *param = params_node ? params_node->left_child : NULL;
    while (param) {
        if (param->token && param->token->type == IDENTIFIER) {
            result = add_symbol(context,
                                param->token->data.characters,
                                SYMBOL_VARIABLE,
                                0,
                                NULL);
            if (result != 0) {
                break;
            }
        }
        param = param->right_sibling;
    }

    if (result == 0) {
        Node *body = find_direct_nonterminal_child(node, BLOCK);
        if (body) {
            result = analyze_block_node(context, body, false);
            if (result == 0) {
                result = check_function_calls_in_subtree(context, body);
            }
        }
    }

    context->in_function = prev_in_function;
    exit_scope(context);
    return result;
}

int analyze_getter(SemanticContext *context, Node *node, const char *class_name) {
    if (!context || !node || !class_name) {
        return SEMANTIC_OTHER_ERROR;
    }

    const char *getter_name = NULL;
    Node *params_node = NULL;
    int result = extract_function_signature(node, &getter_name, &params_node);
    if (result != 0) {
        return result;
    }

    if (count_identifier_parameters(params_node) != 0) {
        return SEMANTIC_OTHER_ERROR;
    }

    result = insert_class_member_symbol(context,
                                        getter_name,
                                        SYMBOL_GETTER,
                                        0,
                                        class_name);
    if (result != 0) {
        return result;
    }

    result = enter_scope(context);
    if (result != 0) {
        return result;
    }

    bool prev_in_function = context->in_function;
    context->in_function = true;

    Node *body = find_direct_nonterminal_child(node, BLOCK);
    if (body) {
        result = analyze_block_node(context, body, false);
        // ДОПОЛНИТЕЛЬНАЯ ПРОВЕРКА: вызовы функций в теле
        if (result == 0) {
            result = check_function_calls_in_subtree(context, body);
        }
    }

    context->in_function = prev_in_function;
    exit_scope(context);
    return result;
}

int analyze_setter(SemanticContext *context, Node *node, const char *class_name) {
    if (!context || !node || !class_name) {
        return SEMANTIC_OTHER_ERROR;
    }

    const char *setter_name = NULL;
    Node *params_node = NULL;
    int result = extract_function_signature(node, &setter_name, &params_node);
    if (result != 0) {
        return result;
    }

    if (count_identifier_parameters(params_node) != 1) {
        return SEMANTIC_OTHER_ERROR;
    }

    result = insert_class_member_symbol(context,
                                        setter_name,
                                        SYMBOL_SETTER,
                                        1,
                                        class_name);
    if (result != 0) {
        return result;
    }

    result = enter_scope(context);
    if (result != 0) {
        return result;
    }

    bool prev_in_function = context->in_function;
    context->in_function = true;

    for (Node *param = params_node; param; param = param->right_child) {
        if (param->token && param->token->type == IDENTIFIER) {
            result = add_symbol(context,
                                param->token->data.characters,
                                SYMBOL_VARIABLE,
                                0,
                                NULL);
            break;
        }
    }

    if (result == 0) {
        Node *body = find_direct_nonterminal_child(node, BLOCK);
        if (body) {
            result = analyze_block_node(context, body, false);
            // ДОПОЛНИТЕЛЬНАЯ ПРОВЕРКА: вызовы функций в теле
            if (result == 0) {
                result = check_function_calls_in_subtree(context, body);
            }
        }
    }

    context->in_function = prev_in_function;
    exit_scope(context);
    return result;
}

static int analyze_class_body(SemanticContext *context,
                              Node *body_node,
                              const char *class_name) {
    if (!body_node) {
        return 0;
    }

    int result = 0;
    Node *current = body_node;

    while (current && result == 0) {
        Node *candidate = current;

        if (candidate->nonterminal == FUNCLIST) {
            candidate = candidate->left_child;
        }

        if (candidate && candidate->nonterminal == FUNCDECL) {
            if (is_getter(candidate)) {
                result = analyze_getter(context, candidate, class_name);
            } else if (is_setter(candidate)) {
                result = analyze_setter(context, candidate, class_name);
            } else {
                result = analyze_method(context, candidate, class_name);
            }
        }

        current = current->right_child;
    }

    return result;
}

int analyze_class(SemanticContext *context, Node *node) {
    if (!context || !node) {
        return SEMANTIC_OTHER_ERROR;
    }

    Node *id_node = find_direct_identifier_child(node);
    if (!id_node) {
        id_node = find_first_identifier(node);
    }

    if (!id_node || !id_node->token) {
        return SEMANTIC_OTHER_ERROR;
    }

    const char *class_name = id_node->token->data.characters;
    if (strcmp(class_name, "Program") != 0) {
        return SEMANTIC_OTHER_ERROR;
    }

    int result = add_symbol(context, class_name, SYMBOL_CLASS, 0, NULL);
    if (result != 0) {
        return result;
    }

    result = enter_class_scope(context, class_name);
    if (result != 0) {
        return result;
    }

    Node *body = find_direct_nonterminal_child(node, FUNCLIST);
    if (body) {
        result = analyze_class_body(context, body, class_name);
    }

    exit_class_scope(context);
    return result;
}

/* ---------- Statements ---------- */

int analyze_var_declaration(SemanticContext *context, Node *node) {
    if (!context || !node || !node->token) {
        return SEMANTIC_OTHER_ERROR;
    }

    const char *name = node->token->data.characters;
    Symbol *existing = symtable_find(context->current_symtable, name);
    if (existing && existing->type == SYMBOL_VARIABLE) {
        return SEMANTIC_REDEFINITION_ERROR;
    }

    return add_symbol(context, name, SYMBOL_VARIABLE, 0, NULL);
}

int analyze_assignment(SemanticContext *context, Node *node) {
    if (!context || !node || !node->left_child || !node->left_child->token) {
        return SEMANTIC_OTHER_ERROR;
    }

    const char *var_name = node->left_child->token->data.characters;
    int result = 0;

    if (!is_global_variable(var_name)) {
        Symbol *symbol = find_symbol(context, var_name);
        if (symbol && symbol->type == SYMBOL_SETTER) {
        } else {
            result = analyze_variable_reference(context, var_name);
            if (result != 0) {
                return result;
            }
        }
    }

    if (node->right_child) {
        result = analyze_expression(context, node->right_child);
    }

    return result;
}

int analyze_if_statement(SemanticContext *context, Node *node) {
    if (!node) {
        return SEMANTIC_OTHER_ERROR;
    }

    int result = 0;

    if (node->left_child) {
        result = analyze_expression(context, node->left_child);
        if (result != 0) {
            return result;
        }
    }

    if (node->right_child) {
        Node *then_branch = node->right_child->left_child;
        Node *else_branch = node->right_child->right_child;

        if (then_branch) {
            result = analyze_node(context, then_branch);
            if (result != 0) {
                return result;
            }
        }

        if (else_branch) {
            result = analyze_node(context, else_branch);
        }
    }

    return result;
}

int analyze_while_statement(SemanticContext *context, Node *node) {
    if (!node) {
        return SEMANTIC_OTHER_ERROR;
    }

    int result = 0;

    if (node->left_child) {
        result = analyze_expression(context, node->left_child);
        if (result != 0) {
            return result;
        }
    }

    if (node->right_child) {
        result = analyze_node(context, node->right_child);
    }

    return result;
}

int analyze_return_statement(SemanticContext *context, Node *node) {
    if (!context || !node) {
        return SEMANTIC_OTHER_ERROR;
    }

    if (!context->in_function) {
        return SEMANTIC_OTHER_ERROR;
    }

    if (node->left_child) {
        return analyze_expression(context, node->left_child);
    }

    return 0;
}

/* ---------- Program-level checks ---------- */

int check_program_structure(SemanticContext *context, Node *node) {
    (void)context;

    if (!node || node->nonterminal != PROGRAM) {
        return SYNTACTICAL_ERROR;
    }

    Node *prolog = NULL;
    Node *class_node = NULL;

    for (Node *child = node->left_child; child; child = child->right_child) {
        if (child->nonterminal == PROLOG && !prolog) {
            prolog = child;
        }
        if (child->nonterminal == CLASS_NT && !class_node) {
            class_node = child;
        }
    }

    if (!prolog || !class_node) {
        return SYNTACTICAL_ERROR;
    }

    if (validate_prolog(prolog) != 0) {
        return SYNTACTICAL_ERROR;
    }

    Node *identifier = find_direct_identifier_child(class_node);
    if (!identifier || !identifier->token ||
        strcmp(identifier->token->data.characters, "Program") != 0) {
        return SYNTACTICAL_ERROR;
    }

    if (!subtree_has_token(class_node, RIGHT_CURLY_BRACKET)) {
        return SYNTACTICAL_ERROR;
    }

    return 0;
}

int check_main_function(SemanticContext *context) {
    Symbol *main_func = symtable_find(context->global_symtable, "main");
    if (!main_func || main_func->type != SYMBOL_FUNCTION) {
        return SEMANTIC_UNDEFINED_ERROR;
    }

    if (main_func->param_count != 0) {
        return SEMANTIC_PARAM_COUNT_ERROR;
    }

    return 0;
}

/* ---------- Block handling ---------- */

static int analyze_block_node(SemanticContext *context, Node *block, bool create_scope) {
    if (!block) {
        return 0;
    }

    int result = 0;
    if (create_scope) {
        result = enter_scope(context);
        if (result != 0) {
            return result;
        }
    }

    result = analyze_node_impl(context, block->left_child);

    if (result == 0) {
        result = check_function_calls_in_subtree(context, block);
    }

    if (create_scope) {
        exit_scope(context);
    }

    return result;
}

/* ---------- Traversal ---------- */

static int analyze_single_node(SemanticContext *context, Node *node) {
    if (!node) {
        return 0;
    }

    switch (node->nonterminal) {
        case PROLOG:
            return validate_prolog(node);
        case CLASS_NT:
            return analyze_class(context, node);
        case FUNCDECL:
            if (context->current_class) {
                if (is_getter(node)) {
                    return analyze_getter(context, node, context->current_class);
                }
                if (is_setter(node)) {
                    return analyze_setter(context, node, context->current_class);
                }
                return analyze_method(context, node, context->current_class);
            }
            return analyze_function(context, node);
        case SEQUENCE:
            return analyze_node_impl(context, node->left_child);
        case ASSIGN:
            return analyze_assignment(context, node);
        case TERM:
        case EXPR:
        case LOGICOR:
        case LOGICAND:
        case EQUALITY:
        case RELATIONAL:
        case ADDITIVE:
        case MULTIPLICATIVE:
        case UNARY:
        case PRIMARY:
            return analyze_expression(context, node);
        case BLOCK:
            return analyze_block_node(context, node, true);
        default:
            return analyze_node_impl(context, node->left_child);
    }
}

static int analyze_node_impl(SemanticContext *context, Node *node) {
    while (node) {
        int result = analyze_single_node(context, node);
        if (result != 0) {
            return result;
        }
        node = node->right_sibling;
    }
    return 0;
}

int analyze_node(SemanticContext *context, Node *node) {
    return analyze_node_impl(context, node);
}

/* ---------- Entry point ---------- */

int semantic_analyze(Node *ast) {
    if (!ast) {
        return SEMANTIC_OTHER_ERROR;
    }

    SemanticContext *context = semantic_init();
    if (!context) {
        return INTERNAL_ERROR;
    }

    int result = analyze_node(context, ast);
    if (result == 0) {
        result = check_function_calls_in_subtree(context, ast);
    }
    if (result == 0) {
        result = check_program_structure(context, ast);
    }
    if (result == 0) {
        result = check_main_function(context);
    }

    semantic_cleanup(context);
    return result;
}
