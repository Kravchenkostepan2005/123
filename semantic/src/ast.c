#include <stdlib.h>
#include <string.h>
#include "ast.h"

Node *node_new(Nonterminal nt, Token *tok, Node *left, Node *right) {
    Node *n = (Node *)malloc(sizeof(Node));
    if (!n) return NULL;
    n->nonterminal = nt;
    n->token = tok;
    n->left_child = left;
    n->right_child = right;
    return n;
}

// --- Token constructors ---
#include "tokens.h"

static Token *token_new(TokenType type) {
    Token *t = (Token *)malloc(sizeof(Token));
    if (!t) return NULL;
    t->type = type;
    memset(&t->data, 0, sizeof(t->data));
    return t;
}

Token *token_identifier(const char *name) {
    Token *t = token_new(IDENTIFIER);
    if (!t) return NULL;
    t->data.characters = name ? strdup(name) : NULL;
    return t;
}

Token *token_string(const char *s) {
    Token *t = token_new(STRING_LITERAL);
    if (!t) return NULL;
    t->data.characters = s ? strdup(s) : NULL;
    return t;
}

Token *token_int(int value) {
    Token *t = token_new(INTEGER_LITERAL);
    if (!t) return NULL;
    t->data.integer = value;
    return t;
}

Token *token_keyword(Keyword kw) {
    Token *t = token_new(KEYWORD);
    if (!t) return NULL;
    t->data.integer = (int)kw;
    return t;
}

Token *token_operator(TokenType op) {
    Token *t = token_new(op);
    return t;
}
