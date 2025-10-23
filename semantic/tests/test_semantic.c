#include <stdio.h>
#include <assert.h>
#include "semantic.h"
#include "ast.h"
#include "tokens.h"
#include "errors.h"

static Node *make_prolog_ifj25(void) {
    return node_new(PROLOG, NULL, node_new(TERM, token_string("ifj25"), NULL, NULL), NULL);
}

static Node *make_params_chain(int count, const char *names[]) {
    Node *head = NULL;
    Node *tail = NULL;
    for (int i = 0; i < count; ++i) {
        Node *n = node_new(TERM, token_identifier(names[i]), NULL, NULL);
        if (!head) {
            head = tail = n;
        } else {
            tail->right_child = n;
            tail = n;
        }
    }
    return head;
}

static Node *make_funcdecl(const char *name, int param_count, const char *param_names[], Node *body) {
    Node *id = node_new(TERM, token_identifier(name), NULL, NULL);
    Node *params = make_params_chain(param_count, param_names);
    Node *head = node_new(FUNCHEAD, NULL, id, params);
    Node *decl = node_new(FUNCDECL, NULL, head, body);
    return decl;
}

static Node *make_program(Node *left, Node *right) {
    return node_new(PROGRAM, NULL, left, right);
}

static Node *make_sequence1(Node *stmt) {
    return node_new(SEQUENCE, NULL, stmt, NULL);
}

static void test_ok_main_no_params_simple(void) {
    Node *prolog = make_prolog_ifj25();
    Node *main_decl = make_funcdecl("main", 0, NULL, NULL);
    Node *program = make_program(prolog, main_decl);

    int rc = semantic_analyze(program);
    assert(rc == 0);
}

static void test_param_count_error_main_with_1_param(void) {
    const char *params[] = {"a"};
    Node *prolog = make_prolog_ifj25();
    Node *main_decl = make_funcdecl("main", 1, params, NULL);
    Node *program = make_program(prolog, main_decl);

    int rc = semantic_analyze(program);
    assert(rc == SEMANTIC_PARAM_COUNT_ERROR);
}

static void test_undefined_variable_in_main_body(void) {
    Node *prolog = make_prolog_ifj25();
    Node *use_x = node_new(TERM, token_identifier("x"), NULL, NULL);
    Node *body = make_sequence1(use_x);
    Node *main_decl = make_funcdecl("main", 0, NULL, body);
    Node *program = make_program(prolog, main_decl);

    int rc = semantic_analyze(program);
    assert(rc == SEMANTIC_UNDEFINED_ERROR);
}

static void test_type_error_plus_string_num(void) {
    Node *prolog = make_prolog_ifj25();
    Node *op = node_new(TERM, token_operator(PLUS),
                        node_new(TERM, token_string("a"), NULL, NULL),
                        node_new(TERM, token_int(1), NULL, NULL));
    Node *assign = node_new(ASSIGN,
                            NULL,
                            node_new(TERM, token_identifier("v"), NULL, NULL),
                            op);
    Node *body = make_sequence1(assign);
    Node *main_decl = make_funcdecl("main", 0, NULL, body);
    Node *program = make_program(prolog, main_decl);

    int rc = semantic_analyze(program);
    assert(rc == SEMANTIC_TYPE_ERROR);
}

static void test_builtin_function_call_ok(void) {
    Node *prolog = make_prolog_ifj25();
    Node *call_builtin = node_new(FUNCNAME, token_identifier("Ifj.print"), NULL, NULL);
    Node *body = make_sequence1(call_builtin);
    Node *main_decl = make_funcdecl("main", 0, NULL, body);
    Node *program = make_program(prolog, main_decl);

    int rc = semantic_analyze(program);
    assert(rc == 0);
}

int main(void) {
    test_ok_main_no_params_simple();
    test_param_count_error_main_with_1_param();
    test_undefined_variable_in_main_body();
    test_type_error_plus_string_num();
    test_builtin_function_call_ok();
    printf("All semantic analyzer tests passed.\n");
    return 0;
}
