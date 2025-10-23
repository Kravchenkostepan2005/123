#ifndef ERRORS_H
#define ERRORS_H

// Error codes used by the semantic analyzer
// Values chosen to be distinct; tests assert these symbols, not raw ints.
#define INTERNAL_ERROR 99
#define SEMANTIC_REDEFINITION_ERROR 3
#define SEMANTIC_UNDEFINED_ERROR 1
#define SEMANTIC_PARAM_COUNT_ERROR 4
#define SEMANTIC_TYPE_ERROR 5
#define SEMANTIC_OTHER_ERROR 6

#endif // ERRORS_H
