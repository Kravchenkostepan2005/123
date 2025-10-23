#pragma once
#include <stdbool.h>
#include <stddef.h>

#include "types.h"
#include "symtable.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum AnalyzerNodeKind {
    AN_NODE_PROGRAM = 1,
    AN_NODE_PROLOG,
    AN_NODE_FUNCLIST,
    AN_NODE_FUNCDECL, // function declaration or definition
    AN_NODE_PARAMLIST,
    AN_NODE_PARAM,
    AN_NODE_BLOCK,
    AN_NODE_VARDECL,
    AN_NODE_ASSIGN,
    AN_NODE_RETURN,
    AN_NODE_EXPR,
    AN_NODE_IDENT,
    AN_NODE_TYPE,
    AN_NODE_CALL,
    AN_NODE_ARG_LIST,
    AN_NODE_CLASS,
    AN_NODE_UNKNOWN = 999
} AnalyzerNodeKind;

typedef struct AstAdapter {
    int  (*get_node_kind)(const void* node);
    const void* (*first_child)(const void* node);
    const void* (*next_sibling)(const void* node);

    // Extractors used by analyzer
    const char* (*get_identifier)(const void* ident_node);
    Type (*get_type_annotation)(const void* type_node);

    // Function decl helpers
    const char* (*get_function_name)(const void* func_node);
    const void* (*get_function_params_node)(const void* func_node);
    const void* (*get_function_return_type_node)(const void* func_node);
    const void* (*get_function_body_block_node)(const void* func_node);

    // Var decl helpers
    const char* (*get_vardecl_name)(const void* vardecl_node);
    const void* (*get_vardecl_type_node)(const void* vardecl_node);
    const void* (*get_vardecl_init_expr_node)(const void* vardecl_node);

    // Assign helpers
    const char* (*get_assign_target_name)(const void* assign_node);
    const void* (*get_assign_value_expr_node)(const void* assign_node);

    // Return helpers
    const void* (*get_return_expr_node)(const void* return_node);

    // Call helpers
    const char* (*get_call_function_name)(const void* call_node);
    const void* (*get_call_args_node)(const void* call_node);

    // Expression typing callback (optional but recommended)
    bool (*infer_expression_type)(const void* expr_node, struct SymbolTable* symbols, Type* out_type, char* errbuf, size_t errbuf_size);
} AstAdapter;

#ifdef __cplusplus
}
#endif
