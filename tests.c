#include "semantic.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char *dup_cstr(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char*)malloc(n);
    if (!p) return NULL;
    memcpy(p, s, n);
    return p;
}

static Token *id(const char *s) {
    Token *t = (Token*)malloc(sizeof(Token));
    t->type = IDENTIFIER;
    t->data.characters = dup_cstr(s);
    return t;
}

static Token *str(const char *s) {
    Token *t = (Token*)malloc(sizeof(Token));
    t->type = STRING_TYPE;
    t->data.characters = dup_cstr(s);
    return t;
}

static Token *num(void) {
    Token *t = (Token*)malloc(sizeof(Token));
    t->type = NUM_TYPE;
    t->data.integer = 0;
    return t;
}

static Node *node(NonTerminal nt, Token *tok, Node *l, Node *r) {
    Node *n = (Node*)calloc(1, sizeof(Node));
    n->nonterminal = nt;
    n->token = tok;
    n->left_child = l;
    n->right_child = r;
    return n;
}

static void free_token(Token *t) { if (!t) return; if (t->type == IDENTIFIER || t->type == STRING_TYPE) free(t->data.characters); free(t); }
static void free_tree(Node *n) { if (!n) return; free_tree(n->left_child); free_tree(n->right_child); free_token(n->token); free(n); }

static void test_init_and_symbols() {
    SemanticContext *ctx = semantic_init();
    assert(ctx && ctx->global_symtable && ctx->current_symtable == ctx->global_symtable);

    int rc = add_symbol(ctx, "x", SYMBOL_VARIABLE, 0, NULL);
    assert(rc == 0);
    Symbol *sx = find_symbol(ctx, "x");
    assert(sx && sx->type == SYMBOL_VARIABLE);

    rc = add_symbol(ctx, "x", SYMBOL_VARIABLE, 0, NULL);
    assert(rc == SEMANTIC_REDEFINITION_ERROR);

    semantic_cleanup(ctx);
}

static void test_functions_and_params() {
    SemanticContext *ctx = semantic_init();
    assert(ctx);

    int rc = add_symbol(ctx, "f", SYMBOL_FUNCTION, 2, NULL);
    assert(rc == 0);

    Node a2 = { .nonterminal = FUNCHEAD, .token = NULL, .left_child = NULL, .right_child = NULL };
    Node a1 = { .nonterminal = FUNCHEAD, .token = NULL, .left_child = NULL, .right_child = &a2 };

    rc = analyze_function_call(ctx, "f", &a1);
    assert(rc == 0);

    rc = analyze_function_call(ctx, "f", NULL);
    assert(rc == SEMANTIC_PARAM_COUNT_ERROR);

    rc = analyze_function_call(ctx, "Ifj.print", NULL);
    assert(rc == 0);

    rc = analyze_function_call(ctx, "g", NULL);
    assert(rc == SEMANTIC_UNDEFINED_ERROR);

    semantic_cleanup(ctx);
}

static void test_expression_types() {
    // "+" valid for string+string and num+num, others invalid
    SemanticContext *ctx = semantic_init();

    Node ln = { .nonterminal = TERM, .token = num(), .left_child = NULL, .right_child = NULL };
    Node rn = { .nonterminal = TERM, .token = num(), .left_child = NULL, .right_child = NULL };
    Node addnum = { .nonterminal = TERM, .token = &(Token){ .type = PLUS }, .left_child = &ln, .right_child = &rn };
    int rc = analyze_expression(ctx, &addnum);
    assert(rc == 0);

    Node ls = { .nonterminal = TERM, .token = str("a"), .left_child = NULL, .right_child = NULL };
    Node rs = { .nonterminal = TERM, .token = str("b"), .left_child = NULL, .right_child = NULL };
    Node addstr = { .nonterminal = TERM, .token = &(Token){ .type = PLUS }, .left_child = &ls, .right_child = &rs };
    rc = analyze_expression(ctx, &addstr);
    assert(rc == 0);

    Node bad = { .nonterminal = TERM, .token = &(Token){ .type = MULTIPLY }, .left_child = &ls, .right_child = &rn };
    rc = analyze_expression(ctx, &bad);
    assert(rc == SEMANTIC_TYPE_ERROR);

    semantic_cleanup(ctx);
    free_token(ln.token); free_token(rn.token); free_token(ls.token); free_token(rs.token);
}

static void test_variable_reference_rules() {
    SemanticContext *ctx = semantic_init();

    int rc = analyze_variable_reference(ctx, "__env");
    assert(rc == 0);

    rc = analyze_variable_reference(ctx, "x");
    assert(rc == SEMANTIC_UNDEFINED_ERROR);

    add_symbol(ctx, "x", SYMBOL_VARIABLE, 0, NULL);
    rc = analyze_variable_reference(ctx, "x");
    assert(rc == 0);

    semantic_cleanup(ctx);
}

static void test_analyze_node_program_and_main_checks() {
    // Build a small AST with a function declaration of main() with zero params
    // We need to mirror the expectations from analyze_node for FUNCDECL shape:
    // node(FUNCDECL, NULL, left: node(FUNCHEAD, NULL, left: node(FUNCNAME, id("main"), NULL, NULL), right: params-list), right: body)

    Node *funcname = node(FUNCNAME, id("main"), NULL, NULL);
    Node *funchead = node(FUNCHEAD, NULL, funcname, NULL); // no params list
    Node *funcdecl = node(FUNCDECL, NULL, funchead, NULL); // empty body

    Node *program = node(PROGRAM, NULL, funcdecl, NULL);

    int rc = semantic_analyze(program);
    assert(rc == 0);

    // Now expect failure when main has a parameter
    Node *p1 = node(TERM, id("p"), NULL, NULL);
    funchead->right_child = p1; // one param

    rc = semantic_analyze(program);
    assert(rc == SEMANTIC_PARAM_COUNT_ERROR);

    free_tree(program); // frees all tokens allocated above
}

int main(void) {
    test_init_and_symbols();
    test_functions_and_params();
    test_expression_types();
    test_variable_reference_rules();
    test_analyze_node_program_and_main_checks();
    printf("All tests passed.\n");
    return 0;
}
