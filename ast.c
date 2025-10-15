#include <stdlib.h>
#include "ast.h"

void init_tree(Node *tree){
    tree->parent = NULL;
    tree->left_child = NULL;
    tree->right_child = NULL;
    tree->left_sibling = NULL;
    tree->right_sibling = NULL;
    tree->nonterminal = NaN;
    tree->token = NULL;
}

int insert_child(Node *tree, Nonterminal_type nonterminal, Token *token){
    if (tree == NULL) return COMPILER_ERROR;
    Node *child = (Node*)malloc(sizeof(Node));
    if(child == NULL)
        return COMPILER_ERROR;
    init_tree(child);
    child->nonterminal = nonterminal;
    child->token = token;
    child->parent = tree;

    if(tree->left_child != NULL){
        // Append as last sibling
        Node *last = tree->left_child;
        while(last->right_sibling != NULL){
            last = last->right_sibling;
        }
        last->right_sibling = child;
        child->left_sibling = last;
        tree->right_child = child;
        return 0;
    }
    tree->left_child = child;
    tree->right_child = child;
    return 0;
}
