#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdbool.h>
#include "symtable.h"
#include "ast.h"
#include "errors.h"

// Semantic analyzer context
typedef struct SemanticContext {
    symtable_t *global_symtable;
    symtable_t *current_symtable;
    symtable_t *class_symtable;
    char *current_class;
    bool in_class;
    bool in_function;
    int error_count;
} SemanticContext;

// API
SemanticContext *semantic_init(void);
void semantic_cleanup(SemanticContext *context);
int semantic_analyze(struct Node *ast);

// (Optional) expose for tests if needed
// struct Symbol *find_symbol(SemanticContext *context, const char *name);

#endif // SEMANTIC_H
