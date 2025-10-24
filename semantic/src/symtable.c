#include "semantic.h"
#include <stdlib.h>
#include <string.h>

// A very small chained-hash symbol table implementation suitable for tests

#define INITIAL_BUCKETS 64

typedef struct SymEntry {
    char *key;
    Symbol *symbol;
    struct SymEntry *next;
} SymEntry;

struct symtable {
    size_t bucket_count;
    SymEntry **buckets;
};

static unsigned long hash_str(const char *s) {
    unsigned long h = 5381;
    int c;
    while ((c = *s++)) {
        h = ((h << 5) + h) + (unsigned long)c;
    }
    return h;
}

symtable_t *symtable_create(void) {
    symtable_t *t = (symtable_t *)malloc(sizeof(symtable_t));
    if (!t) return NULL;
    t->bucket_count = INITIAL_BUCKETS;
    t->buckets = (SymEntry **)calloc(t->bucket_count, sizeof(SymEntry *));
    if (!t->buckets) {
        free(t);
        return NULL;
    }
    return t;
}

static void symentry_free(SymEntry *e) {
    while (e) {
        SymEntry *next = e->next;
        free(e->key);
        // Free symbol content
        if (e->symbol) {
            free(e->symbol->name);
            if (e->symbol->class_name) free(e->symbol->class_name);
            if (e->symbol->members) symtable_destroy(e->symbol->members);
            free(e->symbol);
        }
        free(e);
        e = next;
    }
}

void symtable_destroy(symtable_t *table) {
    if (!table) return;
    for (size_t i = 0; i < table->bucket_count; ++i) {
        if (table->buckets[i]) symentry_free(table->buckets[i]);
    }
    free(table->buckets);
    free(table);
}

int symtable_insert(symtable_t *table, const char *key, Symbol *symbol) {
    if (!table || !key || !symbol) return -1;
    unsigned long h = hash_str(key) % table->bucket_count;
    SymEntry *head = table->buckets[h];
    for (SymEntry *e = head; e; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            // Allow overloading resolution at a higher level; here disallow exact duplicate keys
            return -1;
        }
    }
    SymEntry *ne = (SymEntry *)malloc(sizeof(SymEntry));
    if (!ne) return -1;
    ne->key = strdup(key);
    ne->symbol = symbol;
    ne->next = head;
    table->buckets[h] = ne;
    return 0;
}

Symbol *symtable_find(symtable_t *table, const char *key) {
    if (!table || !key) return NULL;
    unsigned long h = hash_str(key) % table->bucket_count;
    for (SymEntry *e = table->buckets[h]; e; e = e->next) {
        if (strcmp(e->key, key) == 0) return e->symbol;
    }
    return NULL;
}
