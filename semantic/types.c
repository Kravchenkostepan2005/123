#include "types.h"

const char* type_kind_name(TypeKind kind) {
    switch (kind) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_STRING: return "string";
        case TYPE_BOOL: return "bool";
        case TYPE_VOID: return "void";
        case TYPE_NIL: return "nil";
        case TYPE_AUTO: return "auto";
        case TYPE_UNKNOWN: return "<unknown>";
        case TYPE_ERROR: return "<error>";
        default: return "<invalid>";
    }
}

bool type_equals(Type a, Type b) {
    return a.kind == b.kind;
}

bool type_is_assignable(Type target, Type value) {
    if (target.kind == TYPE_AUTO) {
        return true; // target can take any value
    }
    if (target.kind == value.kind) {
        return true;
    }
    // simple numeric promotion: int -> float
    if (target.kind == TYPE_FLOAT && value.kind == TYPE_INT) {
        return true;
    }
    // nil assignable only to nil and string? Keep conservative
    if (target.kind == TYPE_NIL && value.kind == TYPE_NIL) {
        return true;
    }
    return false;
}
