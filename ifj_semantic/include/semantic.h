#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdio.h>
#include "scanner.h"
#include "ast.h"

// Error codes aligned with IFJ25 subset for semantics
typedef enum ifj_sem_error_code {
    IFJ_SEM_OK = 0,
    IFJ_SEM_UNDEFINED = 3,
    IFJ_SEM_REDEFINITION = 4,
    IFJ_SEM_BAD_CALL_OR_BUILTIN_PARAM = 5,
    IFJ_SEM_TYPE_COMPAT = 6,
    IFJ_SEM_OTHER = 10
} IfjSemErrorCode;

typedef IfjSemErrorCode SemError;

// Reasons for prolog/top-level validation
typedef enum ifj_sem_prolog_rule_reason {
    IFJ_SEM_PR_OK = 0,
    IFJ_SEM_PR_PROLOG_COUNT,
    IFJ_SEM_PR_PROLOG_NOT_FIRST,
    IFJ_SEM_PR_CLASS_COUNT,
    IFJ_SEM_PR_CLASS_NAME_NOT_PROGRAM
} IfjSemPrologRuleReason;

const char *ifj_sem_error_name(IfjSemErrorCode code);
const char *ifj_sem_prolog_rule_reason_name(IfjSemPrologRuleReason reason);

// Top-level semantic checks (works on your AST/Token types)
SemError check_prolog_and_rules_ex(Node *program_root, FILE *errout, IfjSemPrologRuleReason *reason_out);
SemError check_prolog_and_rules(Node *program_root, FILE *errout);

#endif // SEMANTIC_H
