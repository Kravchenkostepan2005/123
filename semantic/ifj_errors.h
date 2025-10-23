#pragma once

// Default IFJ-style error codes (can be adjusted to match assignment)
#ifndef ERR_LEXICAL
#define ERR_LEXICAL 1
#endif
#ifndef ERR_SYNTAX
#define ERR_SYNTAX 2
#endif
#ifndef ERR_SEM_UNDEF
#define ERR_SEM_UNDEF 3   // undefined/redeclared identifiers, missing symbols
#endif
#ifndef ERR_SEM_TYPES
#define ERR_SEM_TYPES 4   // type incompatibilities, wrong argument/return types
#endif
#ifndef ERR_SEM_OTHER
#define ERR_SEM_OTHER 5   // other semantic errors (e.g., duplicate definitions, missing return)
#endif
#ifndef ERR_INTERNAL
#define ERR_INTERNAL 99
#endif
