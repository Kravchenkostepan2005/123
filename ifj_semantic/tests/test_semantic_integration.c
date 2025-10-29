#include <stdio.h>
#include "scanner.h"
#include "ast.h"
#include "parser.h"
#include "semantic.h"

int main(void) {
    Node root;
    init_tree(&root);

    int parse_rc = parse_topDown(&root);
    if (parse_rc != 0) {
        fprintf(stderr, "Parser failed with code %d\n", parse_rc);
        return parse_rc;
    }

    IfjSemPrologRuleReason reason;
    SemError sem_rc = check_prolog_and_rules_ex(&root, stderr, &reason);
    if (sem_rc != IFJ_SEM_OK) {
        fprintf(stderr, "Semantics failed: %s (reason=%s)\n",
                ifj_sem_error_name(sem_rc),
                ifj_sem_prolog_rule_reason_name(reason));
        return sem_rc;
    }

    puts("OK (lexer+parser+semantics)");
    return 0;
}
