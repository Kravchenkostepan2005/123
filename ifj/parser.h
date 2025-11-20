#include <stdio.h>
#include <stdbool.h>
#include "scanner.h"
#include "ast.h"
#ifndef parser_h
#define parser_h
void write_buffer(Token **token_buffer, Token *address);
Token *read_buffer(Token **token_buffer);
int check_correct_rule(Node *tree, Token_type *terminals, Nonterminal_type *nonterminals,Keyword_type *check_keywords,Token **token_buffer);
int parse_topDown(Node *tree);
#endif
