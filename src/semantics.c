#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>

#include "ast.h"        // Node layout and Nonterminal_type
#include "semantics.h"  // public API

// ----------------------------- Utilities ------------------------------------

static void messages_init(SemanticMessages *messages) {
    messages->items = NULL;
    messages->count = 0;
    messages->capacity = 0;
}

static void messages_push(SemanticMessages *messages, SemanticMessage *msg) {
    if (messages->count == messages->capacity) {
        size_t new_capacity = messages->capacity == 0 ? 8 : messages->capacity * 2;
        SemanticMessage *new_items = (SemanticMessage *)realloc(messages->items, new_capacity * sizeof(SemanticMessage));
        if (!new_items) return; // out of memory: drop message silently; caller sees fewer diagnostics
        messages->items = new_items;
        messages->capacity = new_capacity;
    }
    messages->items[messages->count++] = *msg;
}

static char *dup_cstr(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char *p = (char *)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n + 1);
    return p;
}

static const char *default_get_token_lexeme(const struct Token *tok) {
    (void)tok;
    return NULL;
}

static bool default_get_token_location(const struct Token *tok, int *out_line, int *out_col) {
    (void)tok;
    if (out_line) *out_line = -1;
    if (out_col) *out_col = -1;
    return false;
}

static bool default_is_variable(const struct Token *tok, const char *lexeme) {
    (void)tok;
    if (!lexeme || !lexeme[0]) return false;
    unsigned char c = (unsigned char)lexeme[0];
    return (c == '_') || (c >= 'A' && c <= 'Z');
}

static bool default_is_builtin(const char *name, int arity) {
    if (!name) return false;
    // Minimal builtins set; extend as needed.
    if ((strcmp(name, "=") == 0) && arity == 2) return true;
    if ((strcmp(name, "is") == 0) && arity == 2) return true;
    if ((strcmp(name, "<") == 0 || strcmp(name, ">") == 0 || strcmp(name, "=<") == 0 || strcmp(name, ">=") == 0) && arity == 2) return true;
    return false;
}

// -------------------------- Simple string arena -----------------------------

typedef struct StringSlot {
    char *text;
} StringSlot;

typedef struct StringArena {
    StringSlot *items;
    size_t count;
    size_t capacity;
} StringArena;

static void string_arena_init(StringArena *a) {
    a->items = NULL; a->count = 0; a->capacity = 0;
}

static const char *string_arena_intern(StringArena *a, const char *s) {
    if (!s) return NULL;
    // Not deduplicating for simplicity; could be extended to hash table.
    if (a->count == a->capacity) {
        size_t new_capacity = a->capacity == 0 ? 16 : a->capacity * 2;
        StringSlot *new_items = (StringSlot *)realloc(a->items, new_capacity * sizeof(StringSlot));
        if (!new_items) return NULL;
        a->items = new_items;
        a->capacity = new_capacity;
    }
    char *copy = dup_cstr(s);
    if (!copy) return NULL;
    a->items[a->count++].text = copy;
    return copy;
}

static void string_arena_free(StringArena *a) {
    for (size_t i = 0; i < a->count; ++i) free(a->items[i].text);
    free(a->items);
    a->items = NULL; a->count = 0; a->capacity = 0;
}

// ---------------------------- Symbol tables ---------------------------------

typedef struct PredicateKey {
    const char *name; // interned pointer
    int arity;
} PredicateKey;

typedef struct PredicateSetEntry {
    PredicateKey key;
    bool present;
} PredicateSetEntry;

typedef struct PredicateSet {
    PredicateSetEntry *entries;
    size_t count;
    size_t capacity;
} PredicateSet;

static size_t hash_ptr(const void *p) {
    uintptr_t v = (uintptr_t)p;
    // mix
    v ^= v >> 33; v *= 0xff51afd7ed558ccdULL;
    v ^= v >> 33; v *= 0xc4ceb9fe1a85ec53ULL;
    v ^= v >> 33;
    return (size_t)v;
}

static size_t hash_predicate_key(PredicateKey k) {
    size_t h = hash_ptr(k.name) ^ (size_t)(k.arity * 1315423911u);
    return h ? h : 1469598103934665603ULL;
}

static bool predicate_key_eq(PredicateKey a, PredicateKey b) {
    return a.name == b.name && a.arity == b.arity;
}

static void predicate_set_init(PredicateSet *s) {
    s->entries = NULL; s->count = 0; s->capacity = 0;
}

static bool predicate_set_has(PredicateSet *s, PredicateKey key) {
    if (s->capacity == 0) return false;
    size_t mask = s->capacity - 1;
    size_t idx = hash_predicate_key(key) & mask;
    for (;;) {
        PredicateSetEntry *e = &s->entries[idx];
        if (!e->present) return false;
        if (predicate_key_eq(e->key, key)) return true;
        idx = (idx + 1) & mask;
    }
}

static bool predicate_set_put(PredicateSet *s, PredicateKey key) {
    // ensure capacity: keep load factor <= 0.7 and capacity power of two
    if (s->capacity == 0 || (double)s->count / (double)s->capacity > 0.7) {
        size_t new_capacity = s->capacity == 0 ? 16 : s->capacity * 2;
        PredicateSetEntry *new_entries = (PredicateSetEntry *)calloc(new_capacity, sizeof(PredicateSetEntry));
        if (!new_entries) return false;
        // rehash
        size_t old_capacity = s->capacity;
        PredicateSetEntry *old_entries = s->entries;
        s->entries = new_entries; s->capacity = new_capacity; s->count = 0;
        if (old_entries) {
            for (size_t i = 0; i < old_capacity; ++i) {
                if (old_entries[i].present) {
                    predicate_set_put(s, old_entries[i].key);
                }
            }
            free(old_entries);
        }
    }
    size_t mask = s->capacity - 1;
    size_t idx = hash_predicate_key(key) & mask;
    for (;;) {
        PredicateSetEntry *e = &s->entries[idx];
        if (!e->present) {
            e->present = true;
            e->key = key;
            s->count++;
            return true;
        }
        if (predicate_key_eq(e->key, key)) {
            return true; // already present
        }
        idx = (idx + 1) & mask;
    }
}

static void predicate_set_free(PredicateSet *s) {
    free(s->entries);
    s->entries = NULL; s->count = 0; s->capacity = 0;
}

// Check whether any predicate with the given name exists in the set (ignoring arity)
static bool predicate_set_has_name(PredicateSet *s, const char *name) {
    if (s->capacity == 0) return false;
    for (size_t i = 0; i < s->capacity; ++i) {
        if (s->entries[i].present && s->entries[i].key.name == name) return true;
    }
    return false;
}

// --------------------------- Clause variable bag ----------------------------

typedef struct VarCounter {
    const struct Token *tok;
    const char *name; // interned
    int count;
} VarCounter;

typedef struct VarBag {
    VarCounter *items;
    size_t count;
    size_t capacity;
} VarBag;

static void varbag_init(VarBag *b) { b->items = NULL; b->count = 0; b->capacity = 0; }

static void varbag_free(VarBag *b) { free(b->items); b->items = NULL; b->count = 0; b->capacity = 0; }

static void varbag_add(VarBag *b, const struct Token *tok, const char *name) {
    for (size_t i = 0; i < b->count; ++i) {
        if (b->items[i].name == name) { b->items[i].count++; return; }
    }
    if (b->count == b->capacity) {
        size_t new_capacity = b->capacity == 0 ? 8 : b->capacity * 2;
        VarCounter *new_items = (VarCounter *)realloc(b->items, new_capacity * sizeof(VarCounter));
        if (!new_items) return;
        b->items = new_items; b->capacity = new_capacity;
    }
    b->items[b->count].tok = tok;
    b->items[b->count].name = name;
    b->items[b->count].count = 1;
    b->count++;
}

// --------------------------- AST helpers (generic) --------------------------

static Node *node_first_child(Node *n) { return n ? n->left_child : NULL; }
static Node *node_next_sibling(Node *n) { return n ? n->right_sibling : NULL; }

// Extract identifier lexeme for predicate/function names from a TERM or FUNCNAME node.
// This is grammar-specific; we use token->lexeme via callback, or NULL if absent.
static const char *node_identifier_lexeme(Node *n, const SemanticsConfig *cfg) {
    if (!n) return NULL;
    const struct Token *tok = n->token;
    if (!tok) return NULL;
    const char *lex = NULL;
    if (cfg->get_token_lexeme) {
        lex = cfg->get_token_lexeme(tok);
    } else {
        lex = default_get_token_lexeme(tok);
    }
    return lex;
}

static void token_location(const SemanticsConfig *cfg, const struct Token *tok, int *out_line, int *out_col) {
    if (!tok) { if (out_line) *out_line = -1; if (out_col) *out_col = -1; return; }
    if (cfg->get_token_location && cfg->get_token_location(tok, out_line, out_col)) return;
    default_get_token_location(tok, out_line, out_col);
}

static bool token_is_variable(const SemanticsConfig *cfg, const struct Token *tok, const char *lex) {
    if (cfg->use_lexical_variable_rule || !cfg->is_variable) {
        return default_is_variable(tok, lex);
    }
    return cfg->is_variable(tok, lex);
}

// Count arity by counting TERM children for a FUNCTOR TERM or ARGLIST.
static int count_arity(Node *term_or_args) {
    int arity = 0;
    for (Node *c = node_first_child(term_or_args); c; c = node_next_sibling(c)) {
        if (c->nonterminal == TERM) {
            arity++;
        }
    }
    return arity;
}

// ------------------------------ Analyzer ------------------------------------

typedef struct Analyzer {
    SemanticsConfig cfg;
    StringArena arena;
    PredicateSet defined;  // head functors defined by program
    SemanticMessages *out;
} Analyzer;

static void report(Analyzer *a, SemanticMessageKind kind, const struct Token *tok, const char *predicate_name, int predicate_arity, const char *fmt, ...) {
    int line = -1, col = -1;
    token_location(&a->cfg, tok, &line, &col);

    char buffer[512];
    va_list ap; va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    SemanticMessage m;
    m.kind = kind;
    m.predicate_name = predicate_name ? string_arena_intern(&a->arena, predicate_name) : NULL;
    m.predicate_arity = predicate_arity;
    m.line = line;
    m.column = col;
    m.message = dup_cstr(buffer);
    m.token = tok;
    messages_push(a->out, &m);
}

// Visit a clause head: collect predicate definition name/arity and record in defined set.
static void visit_clause_head(Analyzer *a, Node *head) {
    // Expect grammar like: FUNCNAME ( ARGLISTOPT ) under head or TERM functor
    const char *name = node_identifier_lexeme(head, &a->cfg);
    if (!name) return;
    const char *interned = string_arena_intern(&a->arena, name);
    int arity = 0;

    // Try to count TERM children as arguments
    arity = count_arity(head);

    PredicateKey key = { interned, arity };
    predicate_set_put(&a->defined, key);
}

// Visit a goal call: check that functor is defined or builtin and arity matches a definition if exists.
static void visit_goal_call(Analyzer *a, Node *term) {
    const char *name = node_identifier_lexeme(term, &a->cfg);
    if (!name) return;
    const char *interned = string_arena_intern(&a->arena, name);
    int arity = count_arity(term);

    if (a->cfg.is_builtin_predicate ? a->cfg.is_builtin_predicate(interned, arity) : default_is_builtin(interned, arity)) {
        return; // ok
    }

    PredicateKey key = { interned, arity };
    if (!predicate_set_has(&a->defined, key)) {
        if (predicate_set_has_name(&a->defined, interned)) {
            report(a, SEM_ERROR_PREDICATE_ARITY_MISMATCH, term->token, interned, arity,
                   "Arity mismatch for %s/%d (different arity defined)", interned, arity);
        } else {
            report(a, SEM_ERROR_UNDEFINED_PREDICATE, term->token, interned, arity,
                   "Undefined predicate: %s/%d", interned, arity);
        }
    }
}

// Traverse variables of a clause to collect singletons; this is grammar-dependent.
static void collect_clause_variables(Analyzer *a, Node *node, VarBag *bag) {
    if (!node) return;
    // If node token is a variable per rule, add
    const char *lex = node_identifier_lexeme(node, &a->cfg);
    if (lex) {
        if (token_is_variable(&a->cfg, node->token, lex)) {
            const char *interned = string_arena_intern(&a->arena, lex);
            varbag_add(bag, node->token, interned);
        }
    }
    for (Node *c = node_first_child(node); c; c = node_next_sibling(c)) {
        collect_clause_variables(a, c, bag);
    }
}

static void warn_singletons(Analyzer *a, VarBag *bag) {
    if (!a->cfg.warn_singleton_variables) return;
    for (size_t i = 0; i < bag->count; ++i) {
        if (bag->items[i].count == 1) {
            report(a, SEM_WARN_SINGLETON_VARIABLE, bag->items[i].tok, NULL, -1,
                   "Singleton variable: %s", bag->items[i].name);
        }
    }
}

// Generic traversal based on Nonterminal_type constants provided; adjust as needed.
static void traverse(Analyzer *a, Node *node) {
    if (!node) return;

    switch (node->nonterminal) {
        case FUNCDECL:
        case FUNCHEAD:
            visit_clause_head(a, node);
            break;
        case TERM:
            visit_goal_call(a, node);
            break;
        default:
            break;
    }

    for (Node *c = node_first_child(node); c; c = node_next_sibling(c)) {
        traverse(a, c);
    }
}

// High-level pass: first collect all heads (definitions), then validate calls per clause and warn on variables.
static void collect_definitions(Analyzer *a, Node *node) {
    if (!node) return;
    if (node->nonterminal == FUNCDECL || node->nonterminal == FUNCHEAD) {
        visit_clause_head(a, node);
    }
    for (Node *c = node_first_child(node); c; c = node_next_sibling(c)) {
        collect_definitions(a, c);
    }
}

static void validate_calls_and_vars(Analyzer *a, Node *node) {
    if (!node) return;

    // A simplistic clause boundary heuristic: treat FUNCDECL as a clause.
    if (node->nonterminal == FUNCDECL) {
        VarBag bag; varbag_init(&bag);
        collect_clause_variables(a, node, &bag);
        warn_singletons(a, &bag);
        varbag_free(&bag);
    }

    if (node->nonterminal == TERM) {
        visit_goal_call(a, node);
    }

    for (Node *c = node_first_child(node); c; c = node_next_sibling(c)) {
        validate_calls_and_vars(a, c);
    }
}

int semantics_analyze(Node *root, const SemanticsConfig *config, SemanticMessages *out_messages) {
    if (!out_messages) return -1;
    messages_init(out_messages);

    Analyzer a;
    a.cfg = (SemanticsConfig){0};
    if (config) a.cfg = *config;
    if (!a.cfg.get_token_lexeme) a.cfg.get_token_lexeme = default_get_token_lexeme;
    if (!a.cfg.get_token_location) a.cfg.get_token_location = default_get_token_location;
    if (!a.cfg.is_variable) a.cfg.is_variable = default_is_variable;
    if (!a.cfg.is_builtin_predicate) a.cfg.is_builtin_predicate = default_is_builtin;

    string_arena_init(&a.arena);
    predicate_set_init(&a.defined);
    a.out = out_messages;

    // two-pass: collect definitions, then validate calls and variables
    collect_definitions(&a, root);
    validate_calls_and_vars(&a, root);

    predicate_set_free(&a.defined);
    string_arena_free(&a.arena);
    return 0;
}

void semantics_free_messages(SemanticMessages *messages) {
    if (!messages || !messages->items) return;
    for (size_t i = 0; i < messages->count; ++i) {
        free(messages->items[i].message);
    }
    free(messages->items);
    messages->items = NULL;
    messages->count = 0;
    messages->capacity = 0;
}
