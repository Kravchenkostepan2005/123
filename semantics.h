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
    IFJ_SEM_OTHER = 10,
    IFJ_SEM_INTERNAL = 99
} IfjSemErrorCode;

// Backward-compatible alias
typedef IfjSemErrorCode SemError;

// Human-readable name for semantic error code
const char *ifj_sem_error_name(IfjSemErrorCode code);

// Validates high-level semantics: prolog and top-level rules
// Returns IFJ_SEM_OK or a semantic error code above (never lexical/syntax/runtime).
SemError check_prolog_and_rules(Node *program_root, FILE *errout);

#endif // SEMANTICS_H
