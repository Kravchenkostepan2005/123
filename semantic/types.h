#pragma once
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_VOID,
    TYPE_NIL,
    TYPE_AUTO,
    TYPE_UNKNOWN,
    TYPE_ERROR
} TypeKind;

typedef struct Type {
    TypeKind kind;
} Type;

static inline Type make_type(TypeKind kind) {
    Type t; t.kind = kind; return t;
}

const char* type_kind_name(TypeKind kind);
bool type_equals(Type a, Type b);
bool type_is_assignable(Type target, Type value);

#ifdef __cplusplus
}
#endif
