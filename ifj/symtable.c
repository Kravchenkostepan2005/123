/*
 * symtable.c - Simple symbol table implementation with implicit chaining in array.
 *
 * - Buckets array holds heads of chains (indices into slots[] or NO_INDEX)
 * - Each occupied entry stores next index in its chain (NO_INDEX for end)
 * - Free entries are managed via a free-list using the same next field
 * - Dynamic resizing with full rehash/move of nodes (keys are duplicated on insert)
 *
 * Values are stored as void* (not freed on destroy).
 */
#include "symtable.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Internal slot structure */
typedef struct slot_t {
    char   *key;
    void   *val;
    size_t  next;   /* index to next in chain, or NO_INDEX */
} slot_t;

/* Symbol table itself */
struct symtable_t {
    slot_t *slots;     /* entries */
    size_t *buckets;   /* heads of chains (indices into slots, or NO_INDEX) */
    size_t  capacity;  /* size of slots[] and buckets[] */
    size_t  used;      /* number of occupied slots */
    size_t  free_head; /* head of free-list (index into slots, or NO_INDEX) */
};

#define INITIAL_CAPACITY 8u
#define NO_INDEX ((size_t)-1)

/* Local strdup to avoid portability warnings */
static char *xstrdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1u;
    char *d = (char *)malloc(n);
    if (!d) return NULL;
    memcpy(d, s, n);
    return d;
}

/* Hash function (65599) */
unsigned int symtable_hash_function(const char *str) {
    unsigned int h = 0u;
    const unsigned char *p = (const unsigned char *)str;
    while (p && *p) h = 65599u * h + *p++;
    return h;
}

/* Create table */
symtable_t *symtable_create(void) {
    symtable_t *st = (symtable_t *)malloc(sizeof(symtable_t));
    if (!st) return NULL;

    st->capacity = INITIAL_CAPACITY;

    st->slots = (slot_t *)calloc(st->capacity, sizeof(slot_t));
    st->buckets = (size_t *)malloc(st->capacity * sizeof(size_t));
    if (!st->slots || !st->buckets) {
        free(st->slots);
        free(st->buckets);
        free(st);
        return NULL;
    }

    for (size_t i = 0; i < st->capacity; i++) {
        st->buckets[i] = NO_INDEX;
    }

    for (size_t i = 0; i < st->capacity; i++) {
        st->slots[i].key = NULL;
        st->slots[i].val = NULL;
        st->slots[i].next = (i + 1u < st->capacity) ? (i + 1u) : NO_INDEX;
    }

    st->free_head = 0u;
    st->used = 0u;

    return st;
}

/* Destroy table */
void symtable_destroy(symtable_t *st) {
    if (!st) return;

    for (size_t i = 0; i < st->capacity; i++) {
        free(st->slots[i].key);
        st->slots[i].key = NULL;
        st->slots[i].val = NULL;
    }

    free(st->slots);
    free(st->buckets);
    free(st);
}

/* Rehash into bigger table */
static int symtable_rehash(symtable_t *st, size_t new_capacity) {
    slot_t *old_slots = st->slots;
    size_t  old_capacity = st->capacity;

    slot_t *new_slots = (slot_t *)calloc(new_capacity, sizeof(slot_t));
    size_t *new_buckets = (size_t *)malloc(new_capacity * sizeof(size_t));
    if (!new_slots || !new_buckets) {
        free(new_slots);
        free(new_buckets);
        return -1;
    }

    for (size_t i = 0; i < new_capacity; i++) {
        new_buckets[i] = NO_INDEX;
    }

    /* Move occupied slots (transfer ownership of keys) */
    for (size_t i = 0; i < old_capacity; i++) {
        if (old_slots[i].key != NULL) {
            size_t new_index = i; /* reuse index when possible (i < new_capacity assumed) */
            if (new_index >= new_capacity) {
                /* fallback: linear probe for a free spot within new capacity */
                for (new_index = 0; new_index < new_capacity && new_slots[new_index].key != NULL; new_index++) {
                    /* find free */
                }
                if (new_index == new_capacity) {
                    free(new_slots);
                    free(new_buckets);
                    return -1;
                }
            }

            new_slots[new_index].key = old_slots[i].key;
            new_slots[new_index].val = old_slots[i].val;

            unsigned int h = symtable_hash_function(new_slots[new_index].key);
            size_t bucket = ((size_t)h) % new_capacity;

            new_slots[new_index].next = new_buckets[bucket];
            new_buckets[bucket] = new_index;
        }
    }

    free(st->slots);
    free(st->buckets);

    st->slots = new_slots;
    st->buckets = new_buckets;
    st->capacity = new_capacity;

    /* rebuild free-list */
    st->free_head = NO_INDEX;
    for (size_t i = new_capacity; i-- > 0u;) {
        if (new_slots[i].key == NULL) {
            new_slots[i].next = st->free_head;
            st->free_head = i;
        }
    }

    return 0;
}

/* Insert key/value */
int symtable_insert(symtable_t *st, const char *key, void *value) {
    if (!st || !key) return -1;

    unsigned int h = symtable_hash_function(key);
    size_t bucket = ((size_t)h) % st->capacity;

    /* look for existing */
    size_t idx = st->buckets[bucket];
    while (idx != NO_INDEX) {
        slot_t *slot = &st->slots[idx];
        if (slot->key && strcmp(slot->key, key) == 0) {
            slot->val = value;
            return 1;
        }
        idx = slot->next;
    }

    /* need new slot */
    if (st->free_head == NO_INDEX) {
        if (symtable_rehash(st, st->capacity * 2u) != 0) return -1;
        bucket = ((size_t)h) % st->capacity;
    }

    size_t new_idx = st->free_head;
    slot_t *new_slot = &st->slots[new_idx];
    st->free_head = new_slot->next;

    char *key_copy = xstrdup(key);
    if (!key_copy) {
        /* return slot back to free-list */
        new_slot->next = st->free_head;
        st->free_head = new_idx;
        return -1;
    }

    new_slot->key = key_copy;
    new_slot->val = value;
    new_slot->next = st->buckets[bucket];
    st->buckets[bucket] = new_idx;

    st->used++;
    return 0;
}

/* Find by key */
void *symtable_find(const symtable_t *st, const char *key) {
    if (!st || !key) return NULL;

    unsigned int h = symtable_hash_function(key);
    size_t bucket = ((size_t)h) % st->capacity;

    size_t idx = st->buckets[bucket];
    while (idx != NO_INDEX) {
        const slot_t *slot = &st->slots[idx];
        if (slot->key && strcmp(slot->key, key) == 0) {
            return slot->val;
        }
        idx = slot->next;
    }
    return NULL;
}

/* Remove by key */
int symtable_remove(symtable_t *st, const char *key) {
    if (!st || !key) return 0;

    unsigned int h = symtable_hash_function(key);
    size_t bucket = ((size_t)h) % st->capacity;

    size_t prev = NO_INDEX;
    size_t idx = st->buckets[bucket];

    while (idx != NO_INDEX) {
        slot_t *slot = &st->slots[idx];
        if (slot->key && strcmp(slot->key, key) == 0) {
            /* unlink from chain */
            if (prev == NO_INDEX) {
                st->buckets[bucket] = slot->next;
            } else {
                st->slots[prev].next = slot->next;
            }

            /* free key and push slot to free-list */
            free(slot->key);
            slot->key = NULL;
            slot->val = NULL;

            slot->next = st->free_head;
            st->free_head = idx;

            if (st->used > 0u) st->used--;
            return 1;
        }
        prev = idx;
        idx = slot->next;
    }

    return 0; /* not found */
}

/* Clear all entries */
void symtable_clear(symtable_t *st) {
    if (!st) return;

    /* free all keys */
    for (size_t i = 0; i < st->capacity; i++) {
        if (st->slots[i].key) {
            free(st->slots[i].key);
            st->slots[i].key = NULL;
        }
        st->slots[i].val = NULL;
    }

    /* reset buckets */
    for (size_t i = 0; i < st->capacity; i++) {
        st->buckets[i] = NO_INDEX;
    }

    /* rebuild free-list as a simple chain 0->1->... */
    for (size_t i = 0; i < st->capacity; i++) {
        st->slots[i].next = (i + 1u < st->capacity) ? (i + 1u) : NO_INDEX;
    }
    st->free_head = (st->capacity > 0u) ? 0u : NO_INDEX;

    st->used = 0u;
}

size_t symtable_size(const symtable_t *st) {
    return st ? st->used : 0u;
}

/* Foreach */
void symtable_foreach(const symtable_t *st,
                      void (*func)(const char *key, void *value, void *ud),
                      void *ud) {
    if (!st || !func) return;

    for (size_t i = 0; i < st->capacity; i++) {
        if (st->slots[i].key) {
            func(st->slots[i].key, st->slots[i].val, ud);
        }
    }
}
