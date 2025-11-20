#include <stdio.h>
#include "scanner.h"

#ifndef ast_h
#define ast_h
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
    SEQUENCELIST,
    ASSIGN,
    FUNCNAME,
    ARGLISTOPT,
    ARGLIST,
    ARGTAIL,
    TERM,
    EXPR,
    LOGICOR,
    LOGICORTAIL,
    LOGICAND,
    LOGICANDTAIL,
    EQUALITY,
    EQUALITYTAIL,
    RELATIONAL,
    RELATIONALTAIL,
    ADDITIVE,
    ADDITIVETAIL,
    MULTIPLICATIVE,
    MULTIPLICATIVETAIL,
    UNARY,
    PRIMARY,
    PRIMARYIDENTTAIL,
    NaN // Not a Nonterminal
} Nonterminal_type;

typedef struct node{
    struct node *parent;
    struct node *left_child;
    struct node *right_child;
    struct node *left_sibling;
    struct node *right_sibling;
    Nonterminal_type nonterminal;
    Token *token;
} Node;

char *print_nonterminals(Nonterminal_type nonterminal);
//void print_tree(Node *root);
void print_ast_tree(Node *root,int indent);
void init_tree(Node *tree);
int insert_child(Node *tree, Nonterminal_type nonterminal, Token *token);
void convert_ast_tree(Node *tree,Node *ast_tree);
#endif
