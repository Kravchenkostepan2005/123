#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stddef.h>
#include <stdbool.h>

// Forward declarations to avoid coupling to scanner internals
// The real project should provide these in "scanner.h" and "ast.h".
struct Token; // opaque to the semantic analyzer

// Opaque AST forward declarations; full definitions live in ast.h.
struct node;           // AST node
enum nonterminals;     // Nonterminal tags

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------- Diagnostics ---------------------------------

typedef enum SemanticMessageKind {
    SEM_ERROR_UNDEFINED_PREDICATE,
    SEM_ERROR_PREDICATE_ARITY_MISMATCH,
    SEM_ERROR_REDEFINITION_IN_SAME_CLAUSE,

    SEM_WARN_SINGLETON_VARIABLE,
    SEM_WARN_UNSAFE_VARIABLE,

    SEM_INTERNAL_ERROR
} SemanticMessageKind;

typedef struct SemanticMessage {
    SemanticMessageKind kind;

    // Optional predicate context
    const char *predicate_name;   // not owned; points into analyzer-managed storage
    int predicate_arity;          // -1 if N/A

    // Optional token location (line/column if available via callbacks)
    int line;
    int column;

    // Human-readable message (owned by the analyzer; freed by semantics_free_messages)
    char *message;

    // Original token pointer for clients that want to map back to AST
    const struct Token *token;
} SemanticMessage;

typedef struct SemanticMessages {
    SemanticMessage *items;
    size_t count;
    size_t capacity;
} SemanticMessages;

// ------------------------------ Configuration ------------------------------

typedef struct SemanticsConfig {
    // If true, emit a warning for variables that appear only once in a clause.
    bool warn_singleton_variables;

    // If true, treat variables strictly by first-character rule ([_] or [A-Z]).
    // If false, rely on is_variable callback if provided; otherwise fall back to rule.
    bool use_lexical_variable_rule;

    // Client-provided callbacks (all optional). If NULL, sensible defaults are used.

    // Returns a borrowed, null-terminated string lexeme for the token (identifier text).
    const char *(*get_token_lexeme)(const struct Token *tok);

    // Returns 1-based line and column for the token; return false if unknown.
    bool (*get_token_location)(const struct Token *tok, int *out_line, int *out_col);

    // Classify whether token represents a variable symbol.
    // If not provided, we use lexical rule: first char '_' or uppercase => variable.
    bool (*is_variable)(const struct Token *tok, const char *lexeme);

    // Classify whether the predicate with name+arity is a builtin (e.g., =, is, <, ...).
    // Builtins are not required to be defined by the program.
    bool (*is_builtin_predicate)(const char *name, int arity);
} SemanticsConfig;

// ------------------------------ Public API ---------------------------------

// Analyze the AST rooted at `root` and emit diagnostics into `out_messages`.
// Returns 0 on success, non-zero on fatal internal error.
int semantics_analyze(struct node *root, const SemanticsConfig *config, SemanticMessages *out_messages);

// Release memory owned by the diagnostics vector.
void semantics_free_messages(SemanticMessages *messages);

#ifdef __cplusplus
}
#endif

#endif // SEMANTICS_H
