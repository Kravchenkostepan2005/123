#include "parser.h"
#include "semantic.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

static void free_tree(Node *node) {
    if (!node) return;
    free_tree(node->left_child);
    free_tree(node->right_sibling);
    if (node->token) {
        free_token(node->token);
    }
    free(node);
}

int main(void) {
    Node parse_root;
    init_tree(&parse_root);
    parse_root.nonterminal = PROGRAM;

    int parse_result = parse_topDown(&parse_root);
    if (parse_result != 0) {
        fprintf(stderr, "parse error %d\n", parse_result);
        return parse_result;
    }

    Node ast_root;
    init_tree(&ast_root);
    ast_root.nonterminal = PROGRAM;
    convert_ast_tree(parse_root.left_child, &ast_root);

    int semantic_result = semantic_analyze(&ast_root);
    return semantic_result;
}
