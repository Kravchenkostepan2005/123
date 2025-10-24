#include "semantic.h"
#include <stdlib.h>
#include <string.h>

static char *xstrdup2(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char*)malloc(n);
    if (!p) return NULL;
    memcpy(p, s, n);
    return p;
}

// A tiny chained hash table implementation sufficient for tests

typedef struct SymEntry {
    char *key;
    Symbol *value;
    struct SymEntry *next;
} SymEntry;

struct symtable_t {
    size_t bucket_count;
    SymEntry **buckets;
};

static unsigned long djb2(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + (unsigned long)c;
    return hash;
}

symtable_t *symtable_create(void) {
    symtable_t *t = (symtable_t*)malloc(sizeof(symtable_t));
    if (!t) return NULL;
    t->bucket_count = 256;
    t->buckets = (SymEntry**)calloc(t->bucket_count, sizeof(SymEntry*));
    if (!t->buckets) { free(t); return NULL; }
    return t;
}

void symtable_destroy(symtable_t *table) {
    if (!table) return;
    for (size_t i = 0; i < table->bucket_count; ++i) {
        SymEntry *e = table->buckets[i];
        while (e) {
            SymEntry *n = e->next;
            if (e->value) {
                if (e->value->name) free(e->value->name);
                if (e->value->class_name) free(e->value->class_name);
                if (e->value->members) symtable_destroy(e->value->members);
                free(e->value);
            }
            free(e->key);
            free(e);
            e = n;
        }
    }
    free(table->buckets);
    free(table);
}

int symtable_insert(symtable_t *table, const char *key, Symbol *value) {
    if (!table || !key) return -1;
    unsigned long h = djb2(key) % table->bucket_count;
    SymEntry *e = table->buckets[h];
    while (e) {
        if (strcmp(e->key, key) == 0) {
            // allow multiple functions with same name only if param_count differs is handled by caller
            // reject duplicate key here to keep semantics consistent with provided add_symbol logic
            return -1;
        }
        e = e->next;
    }
    e = (SymEntry*)malloc(sizeof(SymEntry));
    if (!e) return -1;
    e->key = xstrdup2(key);
    e->value = value;
    e->next = table->buckets[h];
    table->buckets[h] = e;
    return 0;
}

Symbol *symtable_find(symtable_t *table, const char *key) {
    if (!table || !key) return NULL;
    unsigned long h = djb2(key) % table->bucket_count;
    SymEntry *e = table->buckets[h];
    while (e) {
        if (strcmp(e->key, key) == 0) return e->value;
        e = e->next;
    }
    return NULL;
}
