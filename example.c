#include <stdio.h>
#include <stdlib.h>
#include "semantics.h"

static Token make_ident(const char *s){
    Token t; t.type = TOK_IDENTIFIER; t.lexeme = s; t.length = 0; t.line = 1; t.column = 1; return t;
}

static Node* new_node(Nonterminal_type nt, Token *tok){
    Node *n = (Node*)malloc(sizeof(Node));
    init_tree(n);
    n->nonterminal = nt; n->token = tok; return n;
}

int main(){
    // Build minimal PROGRAM -> PROLOG, CLASS_NT("Program") tree
    Node *root = new_node(PROGRAM, NULL);
    Token prologTok = make_ident("import \"ifj25\" for Ifj");
    insert_child(root, PROLOG, &prologTok);
    Token classTok = make_ident("Program");
    insert_child(root, CLASS_NT, &classTok);

    IfjSemPrologRuleReason reason;
    SemError err = check_prolog_and_rules_ex(root, stderr, &reason);
    if (err != IFJ_SEM_OK) {
        fprintf(stderr, "Failed with code %d (%s), reason %s\n", err, ifj_sem_error_name(err), ifj_sem_prolog_rule_reason_name(reason));
        return err;
    }
    puts("OK");
    return 0;
}
