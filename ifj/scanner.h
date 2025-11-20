#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

                    // TODO: Add states as they pop up.

typedef union {
    unsigned int integer;
    float floating_point;
    char *characters;
} Token_content;


enum State {
    START,                    // 0
    CHECK_COMMENT,                // 1
    END_COMMENT,                // 2
    EXTENSION,                // 3
    IDENTIFIER_OR_KEYWORD,            // 4
    INBUILT_FUNCTION,            // 5
    INTEGER_CHECK,                // 6
    INTEGER_HEXADECIMAL,            // 7
    FLOAT_CHECK_DECIMAL,            // 8
    FLOAT_CHECK_EXPONENT,            // 9
    SPACE_SKIP,                // 10
    ERROR_STATE,                // 11
    QUOTES_STATE,                // 12
    STRING_STATE,                // 13
    ML_STRING_STATE,            // 14
    TWO_QUOTES_STATE,            // 15
    ML_STRING_QUOTE,            // 16
    ML_STRING_WHITESPACE_SKIP,        // 17
    ML_STRING_WHITESPACE_SKIP_QUOTE,    // 18
    ESCAPE_HEXADECIMAL,            // 19
    ESCAPE_SEQUENCE,            // 20
    UNDERSCORE_STATE,            // 21
    GLOBAL_IDENTIFIER_STATE,        // 22
    ML_STRING_WHITESPACE_SKIP_AFTER_QUOTES    // 23
};

typedef enum{
    EMPTY,                                    // so that a starting token is not undefined, and its type can be checked when freeing memory
    UNDECLARED,                                // same as above, but with some characters inside
    IDENTIFIER,
    KEYWORD,
    GLOBAL_IDENTIFIER,
    INT,
    FLOAT,
    WHITESPACE,
    //for parser, can be whitespace but doesnt have to
    WHITESPACE_EPSILON,
    EOL,
    EOL_EPSILON,
    EOF_TOKEN,            // to avoid clashing with preexisting EOF defines
    LEFT_BRACKET,
    RIGHT_BRACKET,
    LEFT_CURLY_BRACKET,
    RIGHT_CURLY_BRACKET,
    COMMA,
    EQUAL,
    NOT_EQUAL,
    GREATER_THAN,
    LESSER_THAN,
    GREATER_OR_EQUAL,
    LESSER_OR_EQUAL,
    MULTIPLICATION,
    DIVISION,
    ADDITION,
    SUBTRACTION,
    ML_COMMENT_START, //ML = MULTILINE
    ML_COMMENT_END,
    COMMENT,
    COMPARISON,
    DOT,
    STRING_TOKEN,
    ML_STRING_TOKEN,
    ERROR,
    NaT //Not a terminal, easier implementation
} Token_type;

typedef struct token{
    Token_type type;
    Token_content data;
} Token;

typedef enum Keyword {        //chybilo IMPORT FOR
    IFJ,
    IMPORT,
    FOR,
    CLASS,
    IF,
    ELSE,
    IS,
    NULL_LOWERCASE,
    RETURN,
    VAR,
    WHILE,
    STATIC,
    TRUE,
    FALSE,
    NUM_TYPE,
    STRING_TYPE,
    NULL_UPPERCASE
} Keyword_type;


enum Error_output{
    LEXICAL_ERROR = 1,
    SYNTACTICAL_ERROR,
    COMPILER_ERROR = 99
};

enum Inbuilt_functions{
    READ_STR, READ_NUM, WRITE, FLOOR, STR, LENGTH, SUBSTRING, STRCMP, ORD, CHR
};

Token *get_token(void);
void free_token(Token *token);
#endif
