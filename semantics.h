#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stdio.h>
#include "ast.h"

// SEMANTIC-ONLY error codes for the semantics module (subset of IFJ25):
//  0 – OK (no errors)
//  3 – Semantic error – use of undefined function/variable
//  4 – Semantic error – redefinition of function/variable
//  5 – Static semantic error – wrong arg count or wrong builtin param type
//  6 – Static semantic error – type compatibility in arithmetic/string/relational expressions
// 10 – Other semantic errors (fallback for semantics module)

typedef enum ifj_sem_error_code {
    IFJ_SEM_OK = 0,
    IFJ_SEM_UNDEFINED = 3,
    IFJ_SEM_REDEFINITION = 4,
    IFJ_SEM_BAD_CALL_OR_BUILTIN_PARAM = 5,
    IFJ_SEM_TYPE_COMPAT = 6,
    IFJ_SEM_OTHER = 10
} IfjSemErrorCode;

// Backward-compatible alias
typedef IfjSemErrorCode SemError;

// Human-readable name for semantic error code
const char *ifj_sem_error_name(IfjSemErrorCode code);

// Reasons for semantic failures related to prolog and top-level rules
// (These are NOT exit codes; they map to IFJ_SEM_OTHER when failing.)
typedef enum ifj_sem_prolog_rule_reason {
    IFJ_SEM_PR_OK = 0,                // no error
    IFJ_SEM_PR_PROLOG_COUNT,          // missing or multiple PROLOG
    IFJ_SEM_PR_PROLOG_NOT_FIRST,      // PROLOG is not the first child of PROGRAM
    IFJ_SEM_PR_CLASS_COUNT,           // missing or multiple CLASS_NT
    IFJ_SEM_PR_CLASS_NAME_NOT_PROGRAM // class name is not 'Program'
} IfjSemPrologRuleReason;

// Human-readable name for the prolog/rules reason
const char *ifj_sem_prolog_rule_reason_name(IfjSemPrologRuleReason reason);

// Runtime semantic error codes for the generated program (for completeness)
typedef enum ifj_sem_runtime_error_code {
    IFJ_RT_BAD_BUILTIN_PARAM = 25,
    IFJ_RT_TYPE_COMPAT = 26
} IfjSemRuntimeErrorCode;

// Human-readable name for runtime semantic error code
const char *ifj_sem_runtime_error_name(IfjSemRuntimeErrorCode code);

// Validates high-level semantics: prolog and top-level rules
// Returns IFJ_SEM_OK or a semantic error code above (never lexical/syntax/runtime).
// Extended API that also provides the specific reason for failure/success
SemError check_prolog_and_rules_ex(Node *program_root, FILE *errout, IfjSemPrologRuleReason *reason_out);

// Backward-compatible wrapper: ignores the detailed reason
SemError check_prolog_and_rules(Node *program_root, FILE *errout);

#endif // SEMANTICS_H
