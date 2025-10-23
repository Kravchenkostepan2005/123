#include "semantics.h"
#include <stdlib.h>
#include <string.h>

// --- Small helpers ---
static unsigned sem_hash(const char *s) {
    unsigned h = 2166136261u;
    for (; *s; ++s) {
        h ^= (unsigned char)(*s);
        h *= 16777619u;
    }
    return h;
}

static SemScope *sem_scope_new(SemScope *parent) {
    SemScope *s = (SemScope *)calloc(1, sizeof(SemScope));
    if (!s) return NULL;
    s->parent = parent;
    return s;
}

static void sem_symbol_free_chain(SemSymbol *sym) {
    while (sym) {
        SemSymbol *n = sym->next;
        // names and arrays assumed to be owned externally or persist for program duration
        free(sym);
        sym = n;
    }
}

static void sem_scope_free(SemScope *s) {
    if (!s) return;
    for (size_t i = 0; i < SEM_SCOPE_BUCKETS; ++i) {
        sem_symbol_free_chain(s->buckets[i]);
    }
    free(s);
}

static SemSymbol *sem_sym_lookup_in(SemScope *scope, const char *name) {
    if (!scope || !name) return NULL;
    unsigned h = sem_hash(name) % SEM_SCOPE_BUCKETS;
    for (SemSymbol *p = scope->buckets[h]; p; p = p->next) {
        if (p->kind == SYM_VAR && p->u.var.name && strcmp(p->u.var.name, name) == 0) return p;
        if (p->kind == SYM_FUNC && p->u.func.name && strcmp(p->u.func.name, name) == 0) return p;
    }
    return NULL;
}

static SemSymbol *sem_sym_lookup(SemScope *scope, const char *name) {
    for (SemScope *s = scope; s; s = s->parent) {
        SemSymbol *sym = sem_sym_lookup_in(s, name);
        if (sym) return sym;
    }
    return NULL;
}

static int sem_sym_insert(SemScope *scope, SemSymbol *sym, const char *name) {
    if (!scope || !sym || !name) return SEM_ERR_OTHER;
    if (sem_sym_lookup_in(scope, name)) return SEM_ERR_OTHER; // duplicate in same scope
    unsigned h = sem_hash(name) % SEM_SCOPE_BUCKETS;
    sym->next = scope->buckets[h];
    scope->buckets[h] = sym;
    return SEM_OK;
}

SemType sem_unify_numeric(SemType a, SemType b) {
    if (a == TYPE_UNKNOWN) return b;
    if (b == TYPE_UNKNOWN) return a;
    if (a == b) return a;
    if ((a == TYPE_INT && b == TYPE_NUMBER) || (a == TYPE_NUMBER && b == TYPE_INT)) return TYPE_NUMBER;
    return TYPE_UNKNOWN;
}

bool sem_is_truthy_type(SemType t) {
    switch (t) {
        case TYPE_BOOL:
        case TYPE_INT:
        case TYPE_NUMBER:
        case TYPE_STRING:
            return true;
        case TYPE_NIL:
        case TYPE_VOID:
        case TYPE_UNKNOWN:
        default:
            return false;
    }
}

int sem_init(SemAnalyzer *an) {
    if (!an) return SEM_ERR_OTHER;
    an->current = sem_scope_new(NULL);
    an->prolog_ok = false;
    if (!an->current) return SEM_ERR_OTHER;
    return sem_install_builtins(an);
}

void sem_dispose(SemAnalyzer *an) {
    if (!an) return;
    while (an->current) {
        SemScope *p = an->current->parent;
        sem_scope_free(an->current);
        an->current = p;
    }
}

int sem_scope_push(SemAnalyzer *an) {
    if (!an) return SEM_ERR_OTHER;
    SemScope *s = sem_scope_new(an->current);
    if (!s) return SEM_ERR_OTHER;
    an->current = s;
    return SEM_OK;
}

void sem_scope_pop(SemAnalyzer *an) {
    if (!an || !an->current) return;
    SemScope *p = an->current->parent;
    sem_scope_free(an->current);
    an->current = p;
}

int sem_on_prolog(SemAnalyzer *an, const char *import_name) {
    if (!an) return SEM_ERR_OTHER;
    if (!import_name) return SEM_ERR_UNDEFINED;
    // For IFJ/IFJ23 tasks: expect exact module name like "ifj23"
    if (strcmp(import_name, "ifj23") != 0 && strcmp(import_name, "IFJ") != 0 && strcmp(import_name, "IFJ23") != 0) {
        return SEM_ERR_UNDEFINED;
    }
    an->prolog_ok = true;
    return SEM_OK;
}

int sem_on_var_decl(SemAnalyzer *an, const char *name, SemType type, bool is_const) {
    if (!an || !an->current || !name) return SEM_ERR_OTHER;
    SemSymbol *sym = (SemSymbol *)calloc(1, sizeof(SemSymbol));
    if (!sym) return SEM_ERR_OTHER;
    sym->kind = SYM_VAR;
    sym->u.var.name = name;
    sym->u.var.type = type;
    sym->u.var.is_const = is_const;
    sym->u.var.is_defined = false;
    int rc = sem_sym_insert(an->current, sym, name);
    if (rc != SEM_OK) { free(sym); return rc; }
    return SEM_OK;
}

int sem_on_var_define(SemAnalyzer *an, const char *name, SemType init_type) {
    if (!an || !an->current || !name) return SEM_ERR_OTHER;
    SemSymbol *sym = sem_sym_lookup(an->current, name);
    if (!sym || sym->kind != SYM_VAR) return SEM_ERR_UNDEFINED;
    if (sym->u.var.is_defined) return SEM_ERR_OTHER; // redefinition
    // Type check: if declared type known and init incompatible -> type error
    if (sym->u.var.type != TYPE_UNKNOWN && init_type != TYPE_UNKNOWN) {
        if (sym->u.var.type == TYPE_INT && init_type == TYPE_NUMBER) {
            // allow number -> int? typically no; require explicit conversion, so type error
            return SEM_ERR_TYPE;
        }
        if (sym->u.var.type != init_type) {
            // allow int vs number unification? Strict: mismatch is error
            return SEM_ERR_TYPE;
        }
    }
    if (sym->u.var.type == TYPE_UNKNOWN) {
        sym->u.var.type = init_type;
    }
    sym->u.var.is_defined = true;
    return SEM_OK;
}

int sem_on_func_decl(SemAnalyzer *an, const char *name, const SemParam *params, size_t param_count, SemType ret_type) {
    if (!an || !an->current || !name) return SEM_ERR_OTHER;
    SemSymbol *existing = sem_sym_lookup_in(an->current, name);
    if (existing) {
        if (existing->kind != SYM_FUNC) return SEM_ERR_OTHER; // name clash with var
        // Check signature compatibility (same params and return type)
        if (existing->u.func.param_count != param_count || existing->u.func.return_type != ret_type) return SEM_ERR_TYPE;
        for (size_t i = 0; i < param_count; ++i) {
            if (existing->u.func.params[i].type != params[i].type) return SEM_ERR_TYPE;
        }
        return SEM_OK; // duplicate declaration with same signature is OK
    }
    SemSymbol *sym = (SemSymbol *)calloc(1, sizeof(SemSymbol));
    if (!sym) return SEM_ERR_OTHER;
    sym->kind = SYM_FUNC;
    sym->u.func.name = name;
    sym->u.func.return_type = ret_type;
    sym->u.func.param_count = param_count;
    // copy params into a persistent array
    if (param_count > 0) {
        SemParam *copy = (SemParam *)calloc(param_count, sizeof(SemParam));
        if (!copy) { free(sym); return SEM_ERR_OTHER; }
        for (size_t i = 0; i < param_count; ++i) copy[i] = params[i];
        sym->u.func.params = copy;
    }
    sym->u.func.is_defined = false;
    sym->u.func.is_builtin = false;
    int rc = sem_sym_insert(an->current, sym, name);
    if (rc != SEM_OK) {
        if (sym->u.func.params) free((void *)sym->u.func.params);
        free(sym);
        return rc;
    }
    return SEM_OK;
}

int sem_on_func_define_begin(SemAnalyzer *an, const char *name, const SemParam *params, size_t param_count, SemType ret_type) {
    if (!an || !an->current || !name) return SEM_ERR_OTHER;
    SemSymbol *sym = sem_sym_lookup_in(an->current, name);
    if (!sym) {
        // Implicit declaration via definition
        int rc = sem_on_func_decl(an, name, params, param_count, ret_type);
        if (rc != SEM_OK) return rc;
        sym = sem_sym_lookup_in(an->current, name);
    }
    if (!sym || sym->kind != SYM_FUNC) return SEM_ERR_OTHER;
    if (sym->u.func.is_defined) return SEM_ERR_OTHER; // redefinition
    // Verify signature matches any previous decl
    if (sym->u.func.param_count != param_count || sym->u.func.return_type != ret_type) return SEM_ERR_TYPE;
    for (size_t i = 0; i < param_count; ++i) {
        if (sym->u.func.params[i].type != params[i].type) return SEM_ERR_TYPE;
    }
    sym->u.func.is_defined = true;
    // Enter function scope
    int rc = sem_scope_push(an);
    if (rc != SEM_OK) return rc;
    // Install parameters as defined variables in function scope
    for (size_t i = 0; i < param_count; ++i) {
        const char *pname = params[i].name ? params[i].name : "_";
        rc = sem_on_var_decl(an, pname, params[i].type, false);
        if (rc != SEM_OK) return rc;
        rc = sem_on_var_define(an, pname, params[i].type);
        if (rc != SEM_OK) return rc;
    }
    return SEM_OK;
}

int sem_on_func_define_end(SemAnalyzer *an, const char *name) {
    (void)name; // name can be used to cross-validate context if desired
    if (!an) return SEM_ERR_OTHER;
    sem_scope_pop(an);
    return SEM_OK;
}

int sem_on_assign(SemAnalyzer *an, const char *name, SemType expr_type) {
    if (!an || !name) return SEM_ERR_OTHER;
    SemSymbol *sym = sem_sym_lookup(an->current, name);
    if (!sym || sym->kind != SYM_VAR) return SEM_ERR_UNDEFINED; // variable not found
    if (sym->u.var.is_const && sym->u.var.is_defined) return SEM_ERR_OTHER; // const reassignment
    if (sym->u.var.type == TYPE_UNKNOWN) {
        sym->u.var.type = expr_type; // infer
    } else if (expr_type != TYPE_UNKNOWN && sym->u.var.type != expr_type) {
        // numeric unify optional: keep strict
        return SEM_ERR_TYPE;
    }
    sym->u.var.is_defined = true;
    return SEM_OK;
}

int sem_on_return(SemAnalyzer *an, const char *func_name, SemType expr_type) {
    if (!an || !func_name) return SEM_ERR_OTHER;
    SemSymbol *sym = sem_sym_lookup_in(an->current ? an->current->parent : NULL, func_name);
    // Note: function symbol is in parent scope when inside function scope
    if (!sym || sym->kind != SYM_FUNC) return SEM_ERR_OTHER;
    SemType expected = sym->u.func.return_type;
    if (expected == TYPE_VOID) {
        // returning value from void is an error unless expr is TYPE_VOID/NIL allowed by grammar
        if (expr_type != TYPE_VOID && expr_type != TYPE_NIL) return SEM_ERR_TYPE;
        return SEM_OK;
    }
    if (expr_type == TYPE_UNKNOWN) return SEM_ERR_TYPE; // cannot verify
    if (expr_type != expected) return SEM_ERR_TYPE;
    return SEM_OK;
}

int sem_on_call(SemAnalyzer *an, const char *name, const SemType *arg_types, size_t arg_count, SemType *out_return_type) {
    if (!an || !name) return SEM_ERR_OTHER;
    SemSymbol *sym = sem_sym_lookup(an->current, name);
    if (!sym || sym->kind != SYM_FUNC) return SEM_ERR_UNDEFINED;
    const SemFuncSig *fn = &sym->u.func;
    if (fn->param_count != arg_count) return SEM_ERR_TYPE;
    for (size_t i = 0; i < arg_count; ++i) {
        SemType pt = fn->params[i].type;
        SemType at = arg_types ? arg_types[i] : TYPE_UNKNOWN;
        if (pt != at) {
            // allow int->number
            if (!(pt == TYPE_NUMBER && at == TYPE_INT)) return SEM_ERR_TYPE;
        }
    }
    if (out_return_type) *out_return_type = fn->return_type;
    return SEM_OK;
}

// --- Built-in installation ---

static int install_builtin(SemAnalyzer *an, const char *name, SemType ret, const SemParam *params, size_t n) {
    if (!an || !name) return SEM_ERR_OTHER;
    SemSymbol *sym = (SemSymbol *)calloc(1, sizeof(SemSymbol));
    if (!sym) return SEM_ERR_OTHER;
    sym->kind = SYM_FUNC;
    sym->u.func.name = name;
    sym->u.func.return_type = ret;
    sym->u.func.param_count = n;
    if (n > 0) {
        SemParam *copy = (SemParam *)calloc(n, sizeof(SemParam));
        if (!copy) { free(sym); return SEM_ERR_OTHER; }
        for (size_t i = 0; i < n; ++i) copy[i] = params[i];
        sym->u.func.params = copy;
    }
    sym->u.func.is_defined = true;
    sym->u.func.is_builtin = true;
    int rc = sem_sym_insert(an->current, sym, name);
    if (rc != SEM_OK) { if (sym->u.func.params) free((void *)sym->u.func.params); free(sym); }
    return rc;
}

int sem_install_builtins(SemAnalyzer *an) {
    if (!an) return SEM_ERR_OTHER;
    // Example builtins per IFJ23 typical tasks
    // print(any ...) -> void (we'll accept string/number/int/bool/ nil)
    SemParam p_any = { .name = "x", .type = TYPE_STRING, .is_variadic = true };
    (void)p_any; // simplified: we'll just model a few variants

    SemParam p_s1[] = { { .name = "s", .type = TYPE_STRING } };
    SemParam p_n1[] = { { .name = "n", .type = TYPE_NUMBER } };
    SemParam p_i1[] = { { .name = "i", .type = TYPE_INT } };

    // readString() -> string
    install_builtin(an, "readString", TYPE_STRING, NULL, 0);
    // readInt() -> int
    install_builtin(an, "readInt", TYPE_INT, NULL, 0);
    // readNumber() -> number
    install_builtin(an, "readNumber", TYPE_NUMBER, NULL, 0);
    // toInteger(number) -> int
    install_builtin(an, "toInteger", TYPE_INT, p_n1, 1);
    // toNumber(int) -> number
    install_builtin(an, "toNumber", TYPE_NUMBER, p_i1, 1);
    // length(string) -> int
    install_builtin(an, "length", TYPE_INT, p_s1, 1);

    return SEM_OK;
}
