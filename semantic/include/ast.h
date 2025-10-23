#ifndef AST_H
#define AST_H

#include "tokens.h"

// Nonterminals used by semantic analyzer
// Provide values referenced in analyzer logic
typedef enum {
    PROGRAM = 1,
    PROLOG,
    CLASS_NT,
    FUNCDECL,
    SEQUENCE,
    ASSIGN,
    FUNCNAME,
    TERM,
    BLOCK,
    FUNCHEAD
} Nonterminal;

typedef struct Node {
    Nonterminal nonterminal;
    Token *token;              // May be NULL depending on node kind
    struct Node *left_child;   // Left child
    struct Node *right_child;  // Right child / next (e.g., in sequences/params)
} Node;

Node *node_new(Nonterminal nt, Token *tok, Node *left, Node *right);

#endif // AST_H
