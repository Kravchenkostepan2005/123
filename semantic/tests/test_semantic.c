#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"

static char *dup_cstr(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    memcpy(p, s, n);
    return p;
}

static Token *tok_ident(const char *name) {
    Token *t = (Token *)malloc(sizeof(Token));
    t->type = IDENTIFIER;
    t->data.characters = dup_cstr(name);
    return t;
}

static Token *tok_num(double v) {
    Token *t = (Token *)malloc(sizeof(Token));
    t->type = NUM_TYPE;
    t->data.number = v;
    return t;
}

static Token *tok_str(const char *s) {
    Token *t = (Token *)malloc(sizeof(Token));
    t->type = STRING_TYPE;
    t->data.characters = dup_cstr(s);
    return t;
}

static Token *tok_op(Token_type op) {
    Token *t = (Token *)malloc(sizeof(Token));
    t->type = op;
    t->data.integer = 0;
    return t;
}

static Node *node_new(NonTerminal nt, Token *tok, Node *l, Node *r) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->nonterminal = nt;
    n->token = tok;
    n->left_child = l;
    n->right_child = r;
    return n;
}

// Build minimal AST representing: function main() { x = 1 + 2 }
static Node *build_valid_program(void) {
    // term: 1 + 2
    Node *one = node_new(TERM, tok_num(1), NULL, NULL);
    Node *two = node_new(TERM, tok_num(2), NULL, NULL);
    Node *plus = node_new(TERM, tok_op(PLUS), one, two);

    // assign: x = (1+2)
    Node *lhs = node_new(TERM, tok_ident("x"), NULL, NULL);
    Node *assign = node_new(ASSIGN, NULL, lhs, plus);

    // block with single statement
    Node *block = node_new(BLOCK, NULL, assign, NULL);

    // function decl: main() -> left is header, right is body
    Node *fn_id = node_new(FUNCNAME, tok_ident("main"), NULL, NULL);
    Node *hdr = node_new(FUNCDECL, NULL, fn_id, NULL); // header: left=id, right=params(NULL)
    Node *fn_decl = node_new(FUNCDECL, NULL, hdr, block);

    // program: left=function, right=NULL
    Node *program = node_new(PROGRAM, NULL, fn_decl, NULL);
    return program;
}

// Build program without main -> should error undefined main
static Node *build_no_main_program(void) {
    Node *fn_id = node_new(FUNCNAME, tok_ident("foo"), NULL, NULL);
    Node *hdr = node_new(FUNCDECL, NULL, fn_id, NULL);
    Node *body = node_new(BLOCK, NULL, NULL, NULL);
    Node *fn_decl = node_new(FUNCDECL, NULL, hdr, body);
    return node_new(PROGRAM, NULL, fn_decl, NULL);
}

// Build main with one parameter -> param count error
static Node *build_main_with_param(void) {
    Node *param = node_new(TERM, tok_ident("a"), NULL, NULL);

    Node *fn_id = node_new(FUNCNAME, tok_ident("main"), NULL, NULL);
    Node *hdr = node_new(FUNCDECL, NULL, fn_id, param); // header: right_child is param list
    Node *fn_decl = node_new(FUNCDECL, NULL, hdr, node_new(BLOCK, NULL, NULL, NULL));

    Node *program = node_new(PROGRAM, NULL, fn_decl, NULL);

    return program;
}

// Expression type mismatch: "a" - 1 -> TYPE_ERROR during analyze_expression
static Node *build_type_error_in_term(void) {
    Node *left = node_new(TERM, tok_str("a"), NULL, NULL);
    Node *right = node_new(TERM, tok_num(1), NULL, NULL);
    Node *minus = node_new(TERM, tok_op(MINUS), left, right);

    Node *assign = node_new(ASSIGN, NULL, node_new(TERM, tok_ident("x"), NULL, NULL), minus);
    Node *block = node_new(BLOCK, NULL, assign, NULL);

    Node *fn_id = node_new(FUNCNAME, tok_ident("main"), NULL, NULL);
    Node *hdr = node_new(FUNCDECL, NULL, fn_id, NULL);
    Node *fn_decl = node_new(FUNCDECL, NULL, hdr, block);

    return node_new(PROGRAM, NULL, fn_decl, NULL);
}

static void free_ast(Node *n) {
    if (!n) return;
    free_ast(n->left_child);
    free_ast(n->right_child);
    if (n->token) {
        if (n->token->type == IDENTIFIER || n->token->type == STRING_TYPE) {
            free(n->token->data.characters);
        }
        free(n->token);
    }
    free(n);
}

int main(void) {
    // Valid program should return 0
    Node *ok = build_valid_program();
    int r = semantic_analyze(ok);
    assert(r == 0);
    free_ast(ok);

    // No main -> undefined error
    Node *no_main = build_no_main_program();
    r = semantic_analyze(no_main);
    assert(r == SEMANTIC_UNDEFINED_ERROR);
    free_ast(no_main);

    // Main with a parameter -> param count error
    Node *main_param = build_main_with_param();
    r = semantic_analyze(main_param);
    assert(r == SEMANTIC_PARAM_COUNT_ERROR);
    free_ast(main_param);

    // Type error in expression
    Node *type_err = build_type_error_in_term();
    r = semantic_analyze(type_err);
    assert(r == SEMANTIC_TYPE_ERROR);
    free_ast(type_err);

    printf("All tests passed.\n");
    return 0;
}
