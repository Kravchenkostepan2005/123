/*
 * symtable.c
 * TRP (implicit chaining in array) implementation for symbol table.
 *
 * - Buckets array holds heads of chains (indices into entries[] or SIZE_MAX)
 * - Each occupied entry stores next index in its chain (SIZE_MAX for end)
 * - Free entries are managed via a free-list using the same next field
 * - Dynamic resizing with full rehash/move of nodes (keys are not duplicated on rehash)
 *
 * Keys are duplicated with strdup on insert; values are stored as void* (not freed on destroy).
 */

#include "symtable.h"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* ---- configuration ---- */
#ifndef INITIAL_CAPACITY
#define INITIAL_CAPACITY 16
#endif

#ifndef MAX_LOAD_FACTOR
#define MAX_LOAD_FACTOR 0.6
#endif

/* ---- helpers ---- */
static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1U;
    char *d = (char *)malloc(n);
    if (!d) return NULL;
    memcpy(d, s, n);
    return d;
}

/* Official-like hash function for keys (65599) */
unsigned int symtable_hash_function(const char *str) {
    unsigned int h = 0U;
    const unsigned char *p = (const unsigned char *)str;
    while (*p) {
        h = 65599U * h + (unsigned int)(*p++);
    }
    return h;
}

/* ---- table internals ---- */
typedef struct entry_t {
    char *key;        /* NULL => entry is free */
    void *value;
    size_t next;      /* chain link (or next free index when key==NULL); SIZE_MAX for none */
} entry_t;

struct symtable_t {
    entry_t *entries;     /* array of capacity entries */
    size_t  *bucket_head; /* array of capacity bucket heads (indices into entries or SIZE_MAX) */
    size_t   capacity;    /* number of slots in entries and buckets */
    size_t   count;       /* number of occupied entries */
    size_t   free_head;   /* head index of free-list (SIZE_MAX if none) */
};

static const size_t INVALID_INDEX = (size_t)~0ULL; /* SIZE_MAX */

static int symtable_rehash(symtable_t *st, size_t new_capacity);

/* Acquire a free entry index from free-list */
static size_t symtable_alloc_entry(symtable_t *st) {
    if (st->free_head == INVALID_INDEX) return INVALID_INDEX;
    size_t idx = st->free_head;
    st->free_head = st->entries[idx].next;
    st->entries[idx].next = INVALID_INDEX;
    return idx;
}

/* Return an entry back to free-list (expects key already NULL) */
static void symtable_free_entry(symtable_t *st, size_t idx) {
    st->entries[idx].next = st->free_head;
    st->free_head = idx;
}

/* ---- create / destroy ---- */
symtable_t *symtable_create(void) {
    symtable_t *st = (symtable_t *)malloc(sizeof(symtable_t));
    if (!st) return NULL;

    st->capacity = (INITIAL_CAPACITY < 4U) ? 4U : (size_t)INITIAL_CAPACITY;
    st->count = 0U;

    st->entries = (entry_t *)calloc(st->capacity, sizeof(entry_t));
    if (!st->entries) { free(st); return NULL; }

    st->bucket_head = (size_t *)malloc(st->capacity * sizeof(size_t));
    if (!st->bucket_head) { free(st->entries); free(st); return NULL; }

    for (size_t i = 0; i < st->capacity; ++i) {
        st->bucket_head[i] = INVALID_INDEX;
    }

    /* build free-list 0->1->2->... */
    for (size_t i = 0; i < st->capacity - 1U; ++i) {
        st->entries[i].key = NULL;
        st->entries[i].value = NULL;
        st->entries[i].next = i + 1U;
    }
    st->entries[st->capacity - 1U].key = NULL;
    st->entries[st->capacity - 1U].value = NULL;
    st->entries[st->capacity - 1U].next = INVALID_INDEX;
    st->free_head = 0U;

    return st;
}

void symtable_destroy(symtable_t *st) {
    if (!st) return;
    if (st->entries) {
        for (size_t i = 0; i < st->capacity; ++i) {
            if (st->entries[i].key != NULL) {
                free(st->entries[i].key);
            }
        }
        free(st->entries);
    }
    free(st->bucket_head);
    free(st);
}

/* ---- internal: ensure capacity for one more element by rehashing if needed ---- */
static int symtable_ensure_capacity(symtable_t *st) {
    double load = (st->capacity == 0U) ? 1.0 : ((double)(st->count + 1U) / (double)st->capacity);
    if (load > MAX_LOAD_FACTOR || st->free_head == INVALID_INDEX) {
        size_t new_cap = st->capacity * 2U;
        if (new_cap < 4U) new_cap = 4U;
        return symtable_rehash(st, new_cap);
    }
    return 0;
}

/* ---- rehash ---- */
static int symtable_rehash(symtable_t *st, size_t new_capacity) {
    entry_t *new_entries = (entry_t *)calloc(new_capacity, sizeof(entry_t));
    if (!new_entries) return -1;
    size_t *new_bucket_head = (size_t *)malloc(new_capacity * sizeof(size_t));
    if (!new_bucket_head) { free(new_entries); return -1; }

    for (size_t i = 0; i < new_capacity; ++i) new_bucket_head[i] = INVALID_INDEX;

    /* Build free-list for new entries */
    for (size_t i = 0; i < new_capacity - 1U; ++i) {
        new_entries[i].key = NULL;
        new_entries[i].value = NULL;
        new_entries[i].next = i + 1U;
    }
    new_entries[new_capacity - 1U].key = NULL;
    new_entries[new_capacity - 1U].value = NULL;
    new_entries[new_capacity - 1U].next = INVALID_INDEX;
    size_t new_free_head = 0U;

    /* Move all existing nodes without duplicating keys */
    for (size_t i = 0; i < st->capacity; ++i) {
        if (st->entries[i].key != NULL) {
            /* allocate new slot */
            if (new_free_head == INVALID_INDEX) {
                /* shouldn't happen because new_capacity > old count */
                free(new_bucket_head);
                free(new_entries);
                return -1;
            }
            size_t ni = new_free_head;
            new_free_head = new_entries[ni].next;

            /* place the node */
            new_entries[ni].key = st->entries[i].key;   /* transfer ownership */
            new_entries[ni].value = st->entries[i].value;
            new_entries[ni].next = INVALID_INDEX;

            unsigned int h = symtable_hash_function(new_entries[ni].key);
            size_t b = (size_t)(h % (unsigned int)new_capacity);
            new_entries[ni].next = new_bucket_head[b];
            new_bucket_head[b] = ni;
        }
    }

    /* replace old */
    free(st->bucket_head);
    free(st->entries);
    st->entries = new_entries;
    st->bucket_head = new_bucket_head;
    st->free_head = new_free_head;
    st->capacity = new_capacity;
    /* st->count remains the same */
    return 0;
}

/* ---- insert ---- */
int symtable_insert(symtable_t *st, const char *key, void *value) {
    if (!st || !key) return -1;

    if (symtable_ensure_capacity(st) != 0) return -1;

    unsigned int h = symtable_hash_function(key);
    size_t b = (size_t)(h % (unsigned int)st->capacity);

    /* search in chain for existing key */
    size_t cur = st->bucket_head[b];
    while (cur != INVALID_INDEX) {
        entry_t *e = &st->entries[cur];
        if (e->key && strcmp(e->key, key) == 0) {
            e->value = value;
            return 1; /* updated */
        }
        cur = e->next;
    }

    /* allocate new entry */
    size_t idx = symtable_alloc_entry(st);
    if (idx == INVALID_INDEX) {
        /* try one more rehash if free-list unexpectedly empty */
        if (symtable_rehash(st, st->capacity * 2U) != 0) return -1;
        idx = symtable_alloc_entry(st);
        if (idx == INVALID_INDEX) return -1;
    }

    char *kcopy = xstrdup(key);
    if (!kcopy) { symtable_free_entry(st, idx); return -1; }

    entry_t *ne = &st->entries[idx];
    ne->key = kcopy;
    ne->value = value;
    ne->next = st->bucket_head[b];
    st->bucket_head[b] = idx;
    st->count++;
    return 0; /* inserted */
}

/* ---- find ---- */
void *symtable_find(symtable_t *st, const char *key) {
    if (!st || !key) return NULL;
    unsigned int h = symtable_hash_function(key);
    size_t b = (size_t)(h % (unsigned int)st->capacity);
    size_t cur = st->bucket_head[b];
    while (cur != INVALID_INDEX) {
        entry_t *e = &st->entries[cur];
        if (e->key && strcmp(e->key, key) == 0) {
            return e->value;
        }
        cur = e->next;
    }
    return NULL;
}

/* ---- foreach ---- */
void symtable_foreach(symtable_t *st, void (*func)(const char *key, void *value, void *ud), void *ud) {
    if (!st || !func) return;
    for (size_t i = 0; i < st->capacity; ++i) {
        if (st->entries[i].key != NULL) {
            func(st->entries[i].key, st->entries[i].value, ud);
        }
    }
}

