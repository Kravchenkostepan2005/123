#ifndef SCANNER_H
#define SCANNER_H

#include <stddef.h>

// Minimal token interface for semantics module.
// Integrate with your real scanner later.

typedef enum token_type {
    TOK_IDENTIFIER = 1,
    TOK_STRING_LITERAL = 2,
    TOK_OTHER = 100
} TokenType;

typedef struct token {
    TokenType type;
    const char *lexeme;   // not owned; or allocated elsewhere
    size_t length;        // optional
    int line;
    int column;
} Token;

#endif // SCANNER_H
