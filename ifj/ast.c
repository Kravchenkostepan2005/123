#include "ast.h"
#include <string.h>
#include <stdlib.h>
void init_tree(Node *tree){
    tree->parent = NULL;
    tree->left_child = NULL;
    tree->right_child = NULL;
    tree->left_sibling = NULL;
    tree->right_sibling = NULL;
    tree->nonterminal = NaN;
    tree->token = NULL;
}
int insert_child(Node *tree,Nonterminal_type nonterminal,Token *token){
    Node *child = malloc(sizeof(Node));
    if(child == NULL)
        return COMPILER_ERROR;
    init_tree(child);
    child->nonterminal = nonterminal;
    child->token = token;
    if(tree->left_child != NULL){
        tree->right_child = child;
        Node *tmp = tree->left_child;
        while(tmp->right_sibling != NULL){
            tmp = tmp->right_sibling;
        }
        tmp->right_sibling = child;
        child->left_sibling = tmp;
        return 0;
    }
    tree->left_child = child;
    tree->right_child = child;
    return 0;
}

char *print_nonterminals(Nonterminal_type nonterminal){
    switch(nonterminal){
        case PROGRAM: return "PROGRAM";
        case PROLOG: return "PROLOG";
        case CLASS_NT: return "CLASS_NT";
        case FUNCLIST: return "FUNCLIST";
        case FUNCDECL: return "FUNCDECL";
        case FUNCHEAD: return "FUNCHEAD";
        case BLOCK: return "BLOCK";
        case PARAMLISTOPT: return "PARAMLISTOPT";
        case PARAMLIST: return "PARAMLIST";
//        case PARAMTAIL: return "PARAMTAIL";
        case SEQUENCEBLOCK: return "SEQUENCEBLOCK";
        case SEQUENCE: return "SEQUENCE";
        case FUNCNAME: return "FUNCNAME";
        case ARGLISTOPT: return "ARGLISTOPT";
        case ARGLIST: return "ARGLIST";
//        case ARGTAIL: return "ARGTAIL";
        case TERM: return "TERM";
//        case LOGICOR: return "LOGICOR";
//        case LOGICORTAIL: return "LOGICORTAIL";
//        case LOGICAND: return "LOGICAND";
//        case LOGICANDTAIL: return "LOGICANDTAIL";
//        case EQUALITY: return "EQUALITY";
//        case EQUALITYTAIL: return "EQUALITYTAIL";
//        case RELATIONAL: return "RELATIONAL";
//        case RELATIONALTAIL: return "RELATIONALTAIL";
//        case ADDITIVE: return "ADDITIVE";
//        case ADDITIVETAIL: return "ADDITIVETAIL";
//        case MULTIPLICATIVE: return "MULTIPLICATIVE";
//        case MULTIPLICATIVETAIL: return "MULTIPLICATIVETAIL";
//        case UNARY: return "UNARY";
//        case PRIMARY: return "PRIMARY";
//        case PRIMARYIDENTTAIL: return "PRIMARYIDENTTAIL";
        case EXPR: return "EXPR";
        case NaN: return "NaN";
        default: return "REST";
    }
}
char *print_tokens(Token_type token) {
    switch (token) {
        case EMPTY: return "EMPTY";
        case UNDECLARED: return "UNDECLARED";
        case IDENTIFIER: return "IDENTIFIER";
        case KEYWORD: return "KEYWORD";
        case GLOBAL_IDENTIFIER: return "GLOBAL_IDENTIFIER";
        case INT: return "INT";
        case FLOAT: return "FLOAT";
        case WHITESPACE: return "WHITESPACE";
        case EOL: return "EOL";
        case EOF_TOKEN: return "EOF_TOKEN";
//        case LEFT_BRACKET: return "LEFT_BRACKET";
//        case RIGHT_BRACKET: return "RIGHT_BRACKET";
//        case LEFT_CURLY_BRACKET: return "LEFT_CURLY_BRACKET";
//        case RIGHT_CURLY_BRACKET: return "RIGHT_CURLY_BRACKET";
//        case COMMA: return "COMMA";
        case EQUAL: return "EQUAL";
        case NOT_EQUAL: return "NOT_EQUAL";
        case GREATER_THAN: return "GREATER_THAN";
        case LESSER_THAN: return "LESSER_THAN";
        case GREATER_OR_EQUAL: return "GREATER_OR_EQUAL";
        case LESSER_OR_EQUAL: return "LESSER_OR_EQUAL";
        case MULTIPLICATION: return "MULTIPLICATION";
        case DIVISION: return "DIVISION";
        case ADDITION: return "ADDITION";
        case SUBTRACTION: return "SUBTRACTION";
        case ML_COMMENT_START: return "ML_COMMENT_START";
        case ML_COMMENT_END: return "ML_COMMENT_END";
        case COMMENT: return "COMMENT";
        case COMPARISON: return "COMPARISON";
//        case DOT: return "DOT";
        case STRING_TOKEN: return "STRING_TOKEN";
        case ML_STRING_TOKEN: return "ML_STRING_TOKEN";
        case ERROR: return "ERROR";
        case NaT: return "NaT";
        default: return "UNKNOWN_TOKEN";
    }
}

void print_ast_tree(Node *root, int depth) {
    if (root == NULL)
        return;
    for (int i = 0; i < depth; i++) {
        printf("│   ");
    }
    if(root->nonterminal == NaN){
        printf("├── %s\n", print_tokens(root->token->type));
    }
    else{
        printf("├── %s\n", print_nonterminals(root->nonterminal));
    }
    if (root->left_child != NULL)
        print_ast_tree(root->left_child, depth + 1);
    if (root->right_sibling != NULL)
        print_ast_tree(root->right_sibling, depth);
}
void convert_ast_tree(Node *tree, Node *ast) {
    if (tree == NULL || ast == NULL)
        return;

    Node *current = tree;
    while (current != NULL) {
        const char *label;
        if (current->nonterminal != NaN)
            label = print_nonterminals(current->nonterminal);
        else
            label = print_tokens(current->token ? current->token->type : NaT);
        if (strcmp(label, "REST") == 0 || strcmp(label, "UNKNOWN_TOKEN") == 0) {
            if (current->left_child != NULL)
                convert_ast_tree(current->left_child, ast);
            current = current->right_sibling;
            continue;
        }
        if (insert_child(ast, current->nonterminal, current->token) != 0)
            return; // allocation error or similar
        Node *new_child = ast->right_child;
        if (current->left_child != NULL)
            convert_ast_tree(current->left_child, new_child);
        current = current->right_sibling;
    }
}
