#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <string.h>
#include "scanner.h"

// Error codes compatible with IFJ25 spec
#ifndef COMPILER_ERROR
#define COMPILER_ERROR 99
#endif

typedef enum nonterminals{
    PROGRAM,
    PROLOG,
    //class enum already exists
    CLASS_NT,
    FUNCLIST,
    FUNCDECL,
    FUNCHEAD,
    PARAMLISTOPT,
    BLOCK,
    PARAMLIST,
    PARAMTAIL,
    SEQUENCEBLOCK,
    SEQUENCE,
    ASSIGN,
    FUNCNAME,
    ARGLISTOPT,
    ARGLIST,
    ARGTAIL,
    TERM,
    NaN // Not a Nonterminal
} Nonterminal_type;

typedef struct node{
    struct node *parent;
    struct node *left_child;
    struct node *right_child; // last child convenience pointer
    struct node *left_sibling;
    struct node *right_sibling;
    Nonterminal_type nonterminal;
    Token *token; // optional: filled on leafs/identifiers
} Node;

void init_tree(Node *tree);
int insert_child(Node *tree, Nonterminal_type nonterminal, Token *token);

#endif // AST_H
