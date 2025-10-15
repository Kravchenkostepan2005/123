#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stdio.h>
#include "ast.h"

// Error codes aligned to IFJ25 spec
// 2 - syntax error, 3 - undefined, 4 - redefinition, 99 - internal

typedef enum sem_error {
    SEM_OK = 0,
    SEM_SYNTAX = 2,
    SEM_UNDEF = 3,
    SEM_REDEF = 4,
    SEM_INTERNAL = 99
} SemError;

// Validates high-level semantics: prolog and top-level rules
// - Exactly one PROLOG and it must be the first child of PROGRAM
// - Exactly one CLASS_NT under PROGRAM (class Program skeleton)
// - Optionally checks class name token equals "Program" if present
// Returns SEM_OK or one of the error codes above.
SemError check_prolog_and_rules(Node *program_root, FILE *errout);

#endif // SEMANTICS_H
