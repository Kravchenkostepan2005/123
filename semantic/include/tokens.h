#ifndef TOKENS_H
#define TOKENS_H

#include <stddef.h>

// Token types used by the analyzer
// Only a minimal set necessary for tests is provided
typedef enum {
    IDENTIFIER = 1,
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    KEYWORD,

    // Operators used inside expressions
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    EQUALS,
    NOT_EQUALS
} TokenType;

// Minimal keyword set, extended as needed
typedef enum {
    NULL_KW = 0
} Keyword;

typedef union TokenData {
    int integer;        // For KEYWORD or integer literal
    double floating;    // For float literal if ever needed
    char *characters;   // For identifiers/strings
} TokenData;

typedef struct Token {
    TokenType type;
    TokenData data;
} Token;

// Convenience constructors
Token *token_identifier(const char *name);
Token *token_string(const char *s);
Token *token_int(int value);
Token *token_keyword(Keyword kw);
Token *token_operator(TokenType op);

#endif // TOKENS_H
