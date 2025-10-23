#include "analyzer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void append_error(char* errbuf, size_t n, const char* msg) {
    if (!errbuf || n == 0) return;
    size_t len = strlen(errbuf);
    if (len >= n - 1) return;
    snprintf(errbuf + len, n - len, "%s", msg);
}

static int type_error(const char* what, Type expected, Type actual, char* errbuf, size_t n) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Type error: %s: expected %s, got %s\n", what, type_kind_name(expected.kind), type_kind_name(actual.kind));
    append_error(errbuf, n, buf);
    return SEM_ERR_TYPE_MISMATCH;
}

static int analyze_block(const void* block_node, const AstAdapter* ad, SymbolTable* sym, Type current_function_return, char* errbuf, size_t n);

static int analyze_statement(const void* node, const AstAdapter* ad, SymbolTable* sym, Type current_function_return, char* errbuf, size_t n) {
    int kind = ad->get_node_kind(node);
    switch (kind) {
        case AN_NODE_VARDECL: {
            const char* name = ad->get_vardecl_name(node);
            const void* type_node = ad->get_vardecl_type_node(node);
            const void* init_expr = ad->get_vardecl_init_expr_node(node);
            Type declared = type_node ? ad->get_type_annotation(type_node) : make_type(TYPE_AUTO);
            if (!symbol_table_define_variable(sym, name, declared)) {
                append_error(errbuf, n, "Redeclaration of variable\n");
                return SEM_ERR_REDECLARATION;
            }
            if (init_expr) {
                Type init_type = make_type(TYPE_UNKNOWN);
                if (ad->infer_expression_type && !ad->infer_expression_type(init_expr, sym, &init_type, errbuf, n)) {
                    return SEM_ERR_TYPE_MISMATCH;
                }
                Symbol* s = symbol_table_lookup_current(sym, name);
                if (s->as.variable_type.kind == TYPE_AUTO) {
                    s->as.variable_type = init_type;
                } else if (!type_is_assignable(s->as.variable_type, init_type)) {
                    return type_error("variable initialization", s->as.variable_type, init_type, errbuf, n);
                }
            }
            return SEM_OK;
        }
        case AN_NODE_ASSIGN: {
            const char* target = ad->get_assign_target_name(node);
            Symbol* s = symbol_table_lookup(sym, target);
            if (!s || s->kind != SYMBOL_VARIABLE) {
                append_error(errbuf, n, "Assignment to undeclared variable\n");
                return SEM_ERR_UNDECLARED_IDENT;
            }
            Type value_type = make_type(TYPE_UNKNOWN);
            const void* expr = ad->get_assign_value_expr_node(node);
            if (ad->infer_expression_type && !ad->infer_expression_type(expr, sym, &value_type, errbuf, n)) {
                return SEM_ERR_TYPE_MISMATCH;
            }
            if (!type_is_assignable(s->as.variable_type, value_type)) {
                return type_error("assignment", s->as.variable_type, value_type, errbuf, n);
            }
            return SEM_OK;
        }
        case AN_NODE_RETURN: {
            Type value_type = make_type(TYPE_VOID);
            const void* expr = ad->get_return_expr_node(node);
            if (expr) {
                if (ad->infer_expression_type && !ad->infer_expression_type(expr, sym, &value_type, errbuf, n)) {
                    return SEM_ERR_TYPE_MISMATCH;
                }
            }
            if (!type_is_assignable(current_function_return, value_type)) {
                return type_error("return", current_function_return, value_type, errbuf, n);
            }
            return SEM_OK;
        }
        case AN_NODE_CALL: {
            const char* fname = ad->get_call_function_name ? ad->get_call_function_name(node) : NULL;
            if (!fname) {
                append_error(errbuf, n, "Invalid call node (missing name)\n");
                return ERR_INTERNAL;
            }
            Symbol* s = symbol_table_lookup(sym, fname);
            if (!s || s->kind != SYMBOL_FUNCTION) {
                append_error(errbuf, n, "Call to undefined function\n");
                return SEM_ERR_UNDEFINED_FUNCTION;
            }
            FunctionType* fn = s->as.function_type;
            int expected = fn->param_count;
            int seen = 0;
            const void* args_node = ad->get_call_args_node ? ad->get_call_args_node(node) : NULL;
            const void* arg = args_node ? ad->first_child(args_node) : NULL;
            ParamSpec* spec = fn->params;
            while (arg && spec) {
                Type argt = make_type(TYPE_UNKNOWN);
                if (ad->infer_expression_type && !ad->infer_expression_type(arg, sym, &argt, errbuf, n)) {
                    return SEM_ERR_TYPE_MISMATCH;
                }
                if (!type_is_assignable(spec->type, argt)) {
                    return type_error("function argument", spec->type, argt, errbuf, n);
                }
                seen += 1;
                arg = ad->next_sibling(arg);
                spec = spec->next;
            }
            if (seen != expected || spec != NULL || arg != NULL) {
                append_error(errbuf, n, "Wrong number of arguments in call\n");
                return SEM_ERR_TYPE_MISMATCH;
            }
            return SEM_OK;
        }
        case AN_NODE_BLOCK: {
            return analyze_block(node, ad, sym, current_function_return, errbuf, n);
        }
        default:
            // Unknown/unsupported statement kinds are ignored by default
            return SEM_OK;
    }
}

static int analyze_block(const void* block_node, const AstAdapter* ad, SymbolTable* sym, Type current_function_return, char* errbuf, size_t n) {
    symbol_table_enter_scope(sym);
    const void* child = ad->first_child(block_node);
    while (child) {
        int rc = analyze_statement(child, ad, sym, current_function_return, errbuf, n);
        if (rc != SEM_OK) {
            symbol_table_leave_scope(sym);
            return rc;
        }
        child = ad->next_sibling(child);
    }
    symbol_table_leave_scope(sym);
    return SEM_OK;
}

static bool block_contains_return(const void* block_node, const AstAdapter* ad) {
    const void* child = ad->first_child(block_node);
    while (child) {
        int k = ad->get_node_kind(child);
        if (k == AN_NODE_RETURN) return true;
        if (k == AN_NODE_BLOCK && block_contains_return(child, ad)) return true;
        child = ad->next_sibling(child);
    }
    return false;
}

static int collect_function_declarations(const void* root, const AstAdapter* ad, SymbolTable* sym, char* errbuf, size_t n) {
    const void* node = ad->first_child(root);
    while (node) {
        int kind = ad->get_node_kind(node);
        if (kind == AN_NODE_FUNCDECL) {
            const char* name = ad->get_function_name(node);
            const void* ret_node = ad->get_function_return_type_node(node);
            Type ret_type = ret_node ? ad->get_type_annotation(ret_node) : make_type(TYPE_VOID);
            FunctionType* fn = function_type_create(ret_type);
            const void* params = ad->get_function_params_node(node);
            const void* p = params ? ad->first_child(params) : NULL;
            while (p) {
                if (ad->get_node_kind(p) == AN_NODE_PARAM) {
                    const char* pname = ad->get_identifier(p);
                    const void* ptype_node = ad->first_child(p); // or adapter method if needed
                    Type ptype = ptype_node ? ad->get_type_annotation(ptype_node) : make_type(TYPE_AUTO);
                    function_type_add_param(fn, pname, ptype);
                }
                p = ad->next_sibling(p);
            }
            const void* body = ad->get_function_body_block_node(node);
            fn->is_defined = body != NULL;
            if (!symbol_table_declare_function(sym, name, fn)) {
                function_type_free(fn);
                append_error(errbuf, n, "Duplicate or incompatible function declaration\n");
                return SEM_ERR_DUPLICATE_FUNCTION;
            }
        }
        node = ad->next_sibling(node);
    }
    return SEM_OK;
}

static int analyze_function_bodies(const void* root, const AstAdapter* ad, SymbolTable* sym, char* errbuf, size_t n) {
    const void* node = ad->first_child(root);
    while (node) {
        if (ad->get_node_kind(node) == AN_NODE_FUNCDECL) {
            const char* name = ad->get_function_name(node);
            Symbol* s = symbol_table_lookup(sym, name);
            if (!s || s->kind != SYMBOL_FUNCTION) {
                append_error(errbuf, n, "Internal: function symbol not found after declaration\n");
                return SEM_ERR_UNDEFINED_FUNCTION;
            }
            FunctionType* fn = s->as.function_type;
            const void* body = ad->get_function_body_block_node(node);
            if (body) {
                symbol_table_enter_scope(sym);
                // bind parameters
                const void* params = ad->get_function_params_node(node);
                const void* p = params ? ad->first_child(params) : NULL;
                ParamSpec* spec = fn->params;
                while (p && spec) {
                    const char* pname = ad->get_identifier(p);
                    if (!symbol_table_define_variable(sym, pname, spec->type)) {
                        append_error(errbuf, n, "Parameter name conflicts with existing symbol\n");
                        symbol_table_leave_scope(sym);
                        return SEM_ERR_REDECLARATION;
                    }
                    p = ad->next_sibling(p);
                    spec = spec->next;
                }
                // analyze body
                int rc = analyze_block(body, ad, sym, fn->return_type, errbuf, n);
                symbol_table_leave_scope(sym);
                if (rc != SEM_OK) return rc;

                // simple check: non-void functions must contain at least one return
                if (fn->return_type.kind != TYPE_VOID && !block_contains_return(body, ad)) {
                    append_error(errbuf, n, "Missing return in non-void function\n");
                    return SEM_ERR_MISSING_RETURN;
                }
            }
        }
        node = ad->next_sibling(node);
    }
    return SEM_OK;
}

int analyze_program(const void* root, const AstAdapter* ad, const AnalyzerConfig* config, char* errbuf, size_t n) {
    if (errbuf && n) errbuf[0] = '\0';
    SymbolTable symbols;
    symbol_table_init(&symbols);

    int rc = collect_function_declarations(root, ad, &symbols, errbuf, n);
    if (rc != SEM_OK) {
        symbol_table_destroy(&symbols);
        return rc;
    }

    rc = analyze_function_bodies(root, ad, &symbols, errbuf, n);
    if (rc != SEM_OK) {
        symbol_table_destroy(&symbols);
        return rc;
    }

    // Optional: enforce main signature
    if (config && config->require_main) {
        Symbol* s = symbol_table_lookup(&symbols, "main");
        if (!s || s->kind != SYMBOL_FUNCTION) {
            append_error(errbuf, n, "Missing main function\n");
            symbol_table_destroy(&symbols);
            return SEM_ERR_UNDEFINED_FUNCTION;
        }
        FunctionType* fn = s->as.function_type;
        if (!type_equals(fn->return_type, config->main_return_type) || fn->param_count != config->main_param_count) {
            append_error(errbuf, n, "Invalid main function signature\n");
            symbol_table_destroy(&symbols);
            return SEM_ERR_TYPE_MISMATCH;
        }
        if (!fn->is_defined) {
            append_error(errbuf, n, "main declared but not defined\n");
            symbol_table_destroy(&symbols);
            return SEM_ERR_UNDEFINED_FUNCTION;
        }
    }

    symbol_table_destroy(&symbols);
    return SEM_OK;
}
