#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"

static char* xstrdup(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1U;
    char* out = (char*)malloc(n);
    if (out) memcpy(out, s, n);
    return out;
}

static Token* tok_ident(const char* s) {
    Token* t = (Token*)malloc(sizeof(Token));
    t->type = IDENTIFIER;
    t->data.characters = xstrdup(s);
    return t;
}

static Token* tok_string(const char* s) {
    Token* t = (Token*)malloc(sizeof(Token));
    t->type = STRING_LITERAL;
    t->data.characters = xstrdup(s);
    return t;
}

static Token* tok_int(int v) {
    Token* t = (Token*)malloc(sizeof(Token));
    t->type = INTEGER_LITERAL;
    t->data.integer = v;
    return t;
}

static Token* tok_kw_null(void) {
    Token* t = (Token*)malloc(sizeof(Token));
    t->type = KEYWORD;
    t->data.integer = NULL_KW;
    return t;
}

static Token* tok_op(Token_type op) {
    Token* t = (Token*)malloc(sizeof(Token));
    t->type = op;
    t->data.integer = 0;
    return t;
}

static Node* node(NonTerminal nt, Token* tok, Node* l, Node* r) {
    Node* n = (Node*)malloc(sizeof(Node));
    n->nonterminal = nt;
    n->token = tok;
    n->left_child = l;
    n->right_child = r;
    return n;
}

static Node* chain_params(const char** names, int count) {
    Node* head = NULL;
    for (int i = count - 1; i >= 0; --i) {
        head = node(TERM, tok_ident(names[i]), NULL, head);
    }
    return head;
}

static Node* funcdecl(const char* name, const char** params, int param_count, Node* body) {
    Node* id = node(TERM, tok_string(name), NULL, NULL); // use string token to avoid var-ref check
    Node* head = node(TERM, NULL, id, chain_params(params, param_count));
    return node(FUNCDECL, NULL, head, body);
}

static Node* func_call_head(const char* name, Node* args) {
    Node* id = node(TERM, tok_string(name), NULL, NULL); // avoid var-ref on function name
    return node(FUNCHEAD, NULL, id, args);
}

static Node* chain_args_literal_ints(int count) {
    Node* head = NULL;
    for (int i = count - 1; i >= 0; --i) {
        (void)i;
        head = node(TERM, tok_int(1), NULL, head);
    }
    return head;
}

static Node* seq2(Node* a, Node* b) {
    return node(SEQUENCE, NULL, a, node(SEQUENCE, NULL, b, NULL));
}

// ---------- Tests ----------

static void test_missing_main(void) {
    Node* prolog = node(PROLOG, NULL, node(TERM, tok_string("ifj25"), NULL, NULL), NULL);
    Node* program = node(PROGRAM, NULL, prolog, NULL);
    int rc = semantic_analyze(program);
    assert(rc == SEMANTIC_UNDEFINED_ERROR);
}

static void test_main_no_params_ok(void) {
    Node* prolog = node(PROLOG, NULL, node(TERM, tok_string("ifj25"), NULL, NULL), NULL);
    Node* body = NULL; // empty body
    const char* no_params[] = {};
    Node* main_decl = funcdecl("main", no_params, 0, body);
    Node* program = node(PROGRAM, NULL, prolog, node(PROGRAM, NULL, main_decl, NULL));
    int rc = semantic_analyze(program);
    assert(rc == 0);
}

static void test_main_with_param_fails(void) {
    Node* prolog = node(PROLOG, NULL, node(TERM, tok_string("ifj25"), NULL, NULL), NULL);
    const char* p1[] = {"x"};
    Node* main_decl = funcdecl("main", p1, 1, NULL);
    Node* program = node(PROGRAM, NULL, prolog, node(PROGRAM, NULL, main_decl, NULL));
    int rc = semantic_analyze(program);
    assert(rc == SEMANTIC_PARAM_COUNT_ERROR);
}

static void test_function_call_param_mismatch(void) {
    // f(a,b) declared, called with 1 arg -> error
    Node* prolog = node(PROLOG, NULL, node(TERM, tok_string("ifj25"), NULL, NULL), NULL);
    const char* fparams[] = {"a", "b"};
    Node* call = node(TERM, NULL, func_call_head("f", chain_args_literal_ints(1)), NULL);
    Node* f_decl = funcdecl("f", fparams, 2, NULL);
    Node* main_decl = funcdecl("main", NULL, 0, node(SEQUENCE, NULL, call, NULL));
    Node* program = node(PROGRAM, NULL, prolog,
                    node(PROGRAM, NULL, f_decl,
                    node(PROGRAM, NULL, main_decl, NULL)));
    int rc = semantic_analyze(program);
    assert(rc == SEMANTIC_PARAM_COUNT_ERROR);
}

static void test_builtin_call_ok(void) {
    Node* prolog = node(PROLOG, NULL, node(TERM, tok_string("ifj25"), NULL, NULL), NULL);
    Node* call = node(TERM, NULL, func_call_head("Ifj.print", chain_args_literal_ints(3)), NULL);
    Node* main_decl = funcdecl("main", NULL, 0, node(SEQUENCE, NULL, call, NULL));
    Node* program = node(PROGRAM, NULL, prolog, node(PROGRAM, NULL, main_decl, NULL));
    int rc = semantic_analyze(program);
    assert(rc == 0);
}

static void test_literal_type_mismatch_add_int_string(void) {
    // 1 + "a" should be type error
    Node* prolog = node(PROLOG, NULL, node(TERM, tok_string("ifj25"), NULL, NULL), NULL);
    Node* expr = node(TERM, tok_op(PLUS), node(TERM, tok_int(1), NULL, NULL), node(TERM, tok_string("a"), NULL, NULL));
    Node* main_decl = funcdecl("main", NULL, 0, node(SEQUENCE, NULL, expr, NULL));
    Node* program = node(PROGRAM, NULL, prolog, node(PROGRAM, NULL, main_decl, NULL));
    int rc = semantic_analyze(program);
    assert(rc == SEMANTIC_TYPE_ERROR);
}

static void test_equals_always_ok(void) {
    // 1 == "a" returns ok
    Node* prolog = node(PROLOG, NULL, node(TERM, tok_string("ifj25"), NULL, NULL), NULL);
    Node* expr = node(TERM, tok_op(EQUALS), node(TERM, tok_int(1), NULL, NULL), node(TERM, tok_string("a"), NULL, NULL));
    Node* main_decl = funcdecl("main", NULL, 0, node(SEQUENCE, NULL, expr, NULL));
    Node* program = node(PROGRAM, NULL, prolog, node(PROGRAM, NULL, main_decl, NULL));
    int rc = semantic_analyze(program);
    assert(rc == 0);
}

static void test_global_variable_reference_ok(void) {
    // reference __x without declaration is OK
    Node* prolog = node(PROLOG, NULL, node(TERM, tok_string("ifj25"), NULL, NULL), NULL);
    Node* varref = node(TERM, NULL, node(TERM, tok_ident("__x"), NULL, NULL), NULL);
    Node* main_decl = funcdecl("main", NULL, 0, node(SEQUENCE, NULL, varref, NULL));
    Node* program = node(PROGRAM, NULL, prolog, node(PROGRAM, NULL, main_decl, NULL));
    int rc = semantic_analyze(program);
    assert(rc == 0);
}

static void test_assign_creates_variable_then_use_ok(void) {
    Node* prolog = node(PROLOG, NULL, node(TERM, tok_string("ifj25"), NULL, NULL), NULL);
    Node* assign = node(ASSIGN, NULL, node(TERM, tok_ident("x"), NULL, NULL), node(TERM, tok_int(42), NULL, NULL));
    Node* use_x = node(TERM, NULL, node(TERM, tok_ident("x"), NULL, NULL), NULL);
    Node* body = seq2(assign, use_x);
    Node* main_decl = funcdecl("main", NULL, 0, body);
    Node* program = node(PROGRAM, NULL, prolog, node(PROGRAM, NULL, main_decl, NULL));
    int rc = semantic_analyze(program);
    assert(rc == 0);
}

int main(void) {
    test_missing_main();
    test_main_no_params_ok();
    test_main_with_param_fails();
    test_function_call_param_mismatch();
    test_builtin_call_ok();
    test_literal_type_mismatch_add_int_string();
    test_equals_always_ok();
    test_global_variable_reference_ok();
    test_assign_creates_variable_then_use_ok();
    printf("All semantic tests passed.\n");
    return 0;
}
