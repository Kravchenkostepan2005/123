#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stdbool.h>
#include <stddef.h>

// --- Public error codes to align with parser/compiler ---
// Match your project's error codes if they already exist.
// 0 = OK; non-zero = error. Suggested codes:
// 1: Lexical error, 2: Syntactic error, 3: Semantic undefined, 4: Semantic type, 6: Other semantic

typedef enum {
    SEM_OK = 0,
    SEM_ERR_UNDEFINED = 3,
    SEM_ERR_TYPE = 4,
    SEM_ERR_OTHER = 6
} SemError;

// --- Language Types ---
// Adjust to the language in the PDF: ints, numbers, strings, nil, void, etc.

typedef enum {
    TYPE_VOID = 0,
    TYPE_INT,
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_NIL,
    TYPE_UNKNOWN
} SemType;

// Function parameter info
typedef struct {
    const char *name;    // optional for checks, can be NULL
    SemType type;
    bool is_variadic;    // if language supports it
} SemParam;

// Function signature
typedef struct {
    const char *name;       // symbol name
    SemType return_type;
    const SemParam *params; // array (owned by symbol)
    size_t param_count;
    bool is_defined;        // definition encountered
    bool is_builtin;        // built-in
} SemFuncSig;

// Variable symbol
typedef struct {
    const char *name;
    SemType type;
    bool is_const; // if supported
    bool is_defined;
} SemVar;

// Kind of symbol in table
typedef enum { SYM_VAR, SYM_FUNC } SymKind;

// Symbol record
typedef struct SemSymbol {
    SymKind kind;
    union {
        SemVar var;
        SemFuncSig func;
    } u;
    struct SemSymbol *next; // chaining within a scope bucket
} SemSymbol;

// Scope structure (simple chained hash table by fixed buckets)
#define SEM_SCOPE_BUCKETS 97

typedef struct SemScope {
    SemSymbol *buckets[SEM_SCOPE_BUCKETS];
    struct SemScope *parent;
} SemScope;

// Analyzer context
typedef struct {
    SemScope *current;  // current scope
    bool prolog_ok;     // validated prolog/import if required
} SemAnalyzer;

// --- Public API ---

// Lifecycle
int sem_init(SemAnalyzer *an);
void sem_dispose(SemAnalyzer *an);

// Scopes
int sem_scope_push(SemAnalyzer *an);
void sem_scope_pop(SemAnalyzer *an);

// Prolog (e.g., "import ifj23")
int sem_on_prolog(SemAnalyzer *an, const char *import_name);

// Declarations/definitions
int sem_on_var_decl(SemAnalyzer *an, const char *name, SemType type, bool is_const);
int sem_on_var_define(SemAnalyzer *an, const char *name, SemType init_type);
int sem_on_func_decl(SemAnalyzer *an, const char *name, const SemParam *params, size_t param_count, SemType ret_type);
int sem_on_func_define_begin(SemAnalyzer *an, const char *name, const SemParam *params, size_t param_count, SemType ret_type);
int sem_on_func_define_end(SemAnalyzer *an, const char *name);

// Statements
int sem_on_assign(SemAnalyzer *an, const char *name, SemType expr_type);
int sem_on_return(SemAnalyzer *an, const char *func_name, SemType expr_type);
int sem_on_call(SemAnalyzer *an, const char *name, const SemType *arg_types, size_t arg_count, SemType *out_return_type);

// Utilities
SemType sem_unify_numeric(SemType a, SemType b);    // int ⊔ number => number
bool sem_is_truthy_type(SemType t);                 // usable in conditions

// Builtins (optional: expose for tests)
int sem_install_builtins(SemAnalyzer *an);

#endif // SEMANTICS_H
