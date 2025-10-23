#include "demo_adapter.h"
#include <stdlib.h>

static int get_node_kind(const void* node) {
    const DemoNode* n = (const DemoNode*)node;
    return n->kind;
}

static const void* first_child(const void* node) {
    const DemoNode* n = (const DemoNode*)node;
    return n->first_child;
}

static const void* next_sibling(const void* node) {
    const DemoNode* n = (const DemoNode*)node;
    return n->next_sibling;
}

static const char* get_identifier(const void* ident_node) {
    const DemoNode* n = (const DemoNode*)ident_node;
    return n->name;
}

static Type get_type_annotation(const void* type_node) {
    const DemoNode* n = (const DemoNode*)type_node;
    return n->type;
}

static const char* get_function_name(const void* func_node) {
    const DemoNode* n = (const DemoNode*)func_node;
    return n->name;
}

static const void* get_function_params_node(const void* func_node) {
    const DemoNode* n = (const DemoNode*)func_node;
    // first child is params, second is return type, third is body (if any)
    return n->first_child;
}

static const void* get_function_return_type_node(const void* func_node) {
    const DemoNode* n = (const DemoNode*)func_node;
    const DemoNode* params = n->first_child;
    return params ? params->next_sibling : NULL;
}

static const void* get_function_body_block_node(const void* func_node) {
    const DemoNode* n = (const DemoNode*)func_node;
    const DemoNode* params = n->first_child;
    const DemoNode* ret = params ? params->next_sibling : NULL;
    return ret ? ret->next_sibling : NULL;
}

static const char* get_vardecl_name(const void* vardecl_node) {
    const DemoNode* n = (const DemoNode*)vardecl_node;
    return n->name;
}

static const void* get_vardecl_type_node(const void* vardecl_node) {
    const DemoNode* n = (const DemoNode*)vardecl_node;
    return n->first_child; // first child is type
}

static const void* get_vardecl_init_expr_node(const void* vardecl_node) {
    const DemoNode* n = (const DemoNode*)vardecl_node;
    const DemoNode* type_node = n->first_child;
    return type_node ? type_node->next_sibling : NULL;
}

static const char* get_assign_target_name(const void* assign_node) {
    const DemoNode* n = (const DemoNode*)assign_node;
    return n->name;
}

static const void* get_assign_value_expr_node(const void* assign_node) {
    const DemoNode* n = (const DemoNode*)assign_node;
    return n->first_child;
}

static const void* get_return_expr_node(const void* return_node) {
    const DemoNode* n = (const DemoNode*)return_node;
    return n->first_child;
}

static bool infer_expression_type(const void* expr_node, struct SymbolTable* symbols, Type* out_type, char* errbuf, size_t errbuf_size) {
    (void)symbols; (void)errbuf; (void)errbuf_size;
    const DemoNode* n = (const DemoNode*)expr_node;
    if (n->kind == AN_NODE_EXPR || n->kind == AN_NODE_TYPE) {
        *out_type = n->type;
        return true;
    }
    if (n->kind == AN_NODE_IDENT) {
        // in demo assume identifier type is encoded in type field
        *out_type = n->type;
        return true;
    }
    // fallback unknown
    *out_type = make_type(TYPE_UNKNOWN);
    return true;
}

const AstAdapter DEMO_ADAPTER = {
    .get_node_kind = get_node_kind,
    .first_child = first_child,
    .next_sibling = next_sibling,
    .get_identifier = get_identifier,
    .get_type_annotation = get_type_annotation,
    .get_function_name = get_function_name,
    .get_function_params_node = get_function_params_node,
    .get_function_return_type_node = get_function_return_type_node,
    .get_function_body_block_node = get_function_body_block_node,
    .get_vardecl_name = get_vardecl_name,
    .get_vardecl_type_node = get_vardecl_type_node,
    .get_vardecl_init_expr_node = get_vardecl_init_expr_node,
    .get_assign_target_name = get_assign_target_name,
    .get_assign_value_expr_node = get_assign_value_expr_node,
    .get_return_expr_node = get_return_expr_node,
    .get_call_function_name = NULL,
    .get_call_args_node = NULL,
    .infer_expression_type = infer_expression_type
};

void demo_link(DemoNode* parent, DemoNode* child) {
    if (!parent || !child) return;
    if (!parent->first_child) {
        parent->first_child = child;
    } else {
        DemoNode* it = parent->first_child;
        while (it->next_sibling) it = it->next_sibling;
        it->next_sibling = child;
    }
}
