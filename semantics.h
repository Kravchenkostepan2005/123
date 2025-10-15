#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stdio.h>
#include "ast.h"

// IFJ/IAL project error codes (aligned to IFJ25 spec)
// Names below reflect the specification categories.
// Return codes (compiler exit codes):
//  0 – OK (no errors)
//  1 – Lexical analysis error
//  2 – Syntactic analysis error (e.g., missing skeleton)
//  3 – Semantic error – use of undefined function/variable
//  4 – Semantic error – redefinition of function/variable
//  5 – Static semantic error – wrong arg count or wrong builtin param type
//  6 – Static semantic error – type compatibility in arithmetic/string/relational expressions
// 10 – Other semantic errors
// 99 – Internal compiler error (not influenced by input)
// Runtime-only (for generated code):
// 25 – Runtime semantic error – wrong builtin param type
// 26 – Runtime semantic error – type compatibility in expressions

typedef enum ifj_error_code {
    // Success
    IFJ_OK = 0,
    // Analysis phases
    IFJ_LEX_ERROR = 1,
    IFJ_SYNTACTIC_ERROR = 2,
    IFJ_SEM_UNDEFINED = 3,
    IFJ_SEM_REDEFINITION = 4,
    IFJ_SEM_BAD_CALL_OR_BUILTIN_PARAM = 5,
    IFJ_SEM_TYPE_COMPAT = 6,
    IFJ_SEM_OTHER = 10,
    IFJ_INTERNAL_ERROR = 99,
    // Runtime (interpreter) codes produced by generated program
    IFJ_RT_BAD_BUILTIN_PARAM = 25,
    IFJ_RT_TYPE_COMPAT = 26,

    // Backward-compatible aliases used by this semantics module
    SEM_OK = IFJ_OK,
    SEM_SYNTAX = IFJ_SYNTACTIC_ERROR,
    SEM_UNDEF = IFJ_SEM_UNDEFINED,
    SEM_REDEF = IFJ_SEM_REDEFINITION,
    SEM_INTERNAL = IFJ_INTERNAL_ERROR
} IfjErrorCode;

// Keep legacy name for convenience
typedef IfjErrorCode SemError;

// Human-readable name for given IFJ error code (short label)
const char *ifj_error_name(IfjErrorCode code);

// Validates high-level semantics: prolog and top-level rules
// - Exactly one PROLOG and it must be the first child of PROGRAM
// - Exactly one CLASS_NT under PROGRAM (class Program skeleton)
// - Optionally checks class name token equals "Program" if present
// Returns IFJ_OK or one of the IFJ_* error codes above.
SemError check_prolog_and_rules(Node *program_root, FILE *errout);

#endif // SEMANTICS_H
