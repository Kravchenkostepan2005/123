#include "htab.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct htab_item {
    htab_pair_t        pair;
    struct htab_item  *next;
};

struct htab {
    size_t        size;          /* number of pairs */
    size_t        bucket_count;  /* number of buckets */
    htab_item_t **buckets;       /* array of list heads */
};

static char *xstrdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1u;
    char *d = (char *)malloc(n);
    if (!d) return NULL;
    memcpy(d, s, n);
    return d;
}

size_t htab_hash_function(const char *str) {
    unsigned int h = 0u;
    const unsigned char *p = (const unsigned char *)str;
    while (p && *p) h = 65599u * h + *p++;
    return (size_t)h;
}

htab_t *htab_init(size_t bucket_count) {
    if (bucket_count == 0u) bucket_count = 1u;
    htab_t *t = (htab_t *)malloc(sizeof(htab_t));
    if (!t) return NULL;
    t->size = 0u;
    t->bucket_count = bucket_count;
    t->buckets = (htab_item_t **)calloc(bucket_count, sizeof(htab_item_t *));
    if (!t->buckets) {
        free(t);
        return NULL;
    }
    return t;
}

void htab_clear(htab_t *t) {
    if (!t) return;
    for (size_t i = 0; i < t->bucket_count; ++i) {
        htab_item_t *node = t->buckets[i];
        while (node) {
            htab_item_t *next = node->next;
            free((char *)node->pair.key);
            free(node);
            node = next;
        }
        t->buckets[i] = NULL;
    }
    t->size = 0u;
}

void htab_free(htab_t *t) {
    if (!t) return;
    htab_clear(t);
    free(t->buckets);
    free(t);
}

size_t htab_size(const htab_t *t) {
    return t ? t->size : 0u;
}

size_t htab_bucket_count(const htab_t *t) {
    return t ? t->bucket_count : 0u;
}

static size_t bucket_index(const htab_t *t, const char *key) {
    assert(t && key);
    size_t h = htab_hash_function(key);
    return (t->bucket_count > 0u) ? (h % t->bucket_count) : 0u;
}

htab_iterator_t htab_end(const htab_t *t) {
    htab_iterator_t it;
    it.t = (htab_t *)t;
    it.idx = t ? t->bucket_count : 0u;
    it.ptr = NULL;
    return it;
}

htab_iterator_t htab_begin(const htab_t *t) {
    if (!t) return htab_end(t);
    for (size_t i = 0; i < t->bucket_count; ++i) {
        if (t->buckets[i]) {
            htab_iterator_t it = { (htab_t *)t, i, t->buckets[i] };
            return it;
        }
    }
    return htab_end(t);
}

htab_iterator_t htab_find(const htab_t *t, htab_key_t key) {
    if (!t || !key) return htab_end(t);
    size_t idx = bucket_index(t, key);
    htab_item_t *node = t->buckets[idx];
    while (node) {
        if (strcmp(node->pair.key, key) == 0) {
            htab_iterator_t it = { (htab_t *)t, idx, node };
            return it;
        }
        node = node->next;
    }
    return htab_end(t);
}

htab_iterator_t htab_lookup_add(htab_t *t, htab_key_t key) {
    if (!t || !key) return htab_end(t);
    size_t idx = bucket_index(t, key);
    htab_item_t *node = t->buckets[idx];
    while (node) {
        if (strcmp(node->pair.key, key) == 0) {
            htab_iterator_t it = { t, idx, node };
            return it;
        }
        node = node->next;
    }
    /* not found -> insert at front */
    htab_item_t *new_node = (htab_item_t *)malloc(sizeof(htab_item_t));
    if (!new_node) return htab_end(t);
    char *key_copy = xstrdup(key);
    if (!key_copy) {
        free(new_node);
        return htab_end(t);
    }
    new_node->pair.key = key_copy;
    new_node->pair.value = 0u;
    new_node->next = t->buckets[idx];
    t->buckets[idx] = new_node;
    t->size++;
    htab_iterator_t it = { t, idx, new_node };
    return it;
}

bool htab_erase(htab_t *t, htab_key_t key) {
    if (!t || !key) return false;
    size_t idx = bucket_index(t, key);
    htab_item_t *prev = NULL;
    htab_item_t *node = t->buckets[idx];
    while (node) {
        if (strcmp(node->pair.key, key) == 0) {
            if (prev) prev->next = node->next; else t->buckets[idx] = node->next;
            free((char *)node->pair.key);
            free(node);
            if (t->size > 0u) t->size--;
            return true;
        }
        prev = node;
        node = node->next;
    }
    return false;
}

htab_iterator_t htab_iterator_next(htab_iterator_t it) {
    if (!it.t) return it;
    if (it.ptr && it.ptr->next) {
        it.ptr = it.ptr->next;
        return it;
    }
    /* move to next non-empty bucket */
    size_t i = it.idx + 1u;
    for (; i < it.t->bucket_count; ++i) {
        if (it.t->buckets[i]) {
            it.idx = i;
            it.ptr = it.t->buckets[i];
            return it;
        }
    }
    return htab_end(it.t);
}

htab_key_t htab_iterator_get_key(htab_iterator_t it) {
    return it.ptr ? it.ptr->pair.key : NULL;
}

htab_value_t *htab_iterator_get_value(htab_iterator_t it) {
    return it.ptr ? &it.ptr->pair.value : NULL;
}

bool htab_iterator_valid(htab_iterator_t it) {
    return it.ptr != NULL;
}

bool htab_iterator_equal(htab_iterator_t it1, htab_iterator_t it2) {
    return it1.t == it2.t && it1.ptr == it2.ptr && it1.idx == it2.idx;
}

void htab_for_each(const htab_t *t, void (*f)(htab_pair_t *pair, void *ud), void *ud) {
    if (!t || !f) return;
    for (size_t i = 0; i < t->bucket_count; ++i) {
        for (htab_item_t *node = t->buckets[i]; node; node = node->next) {
            f(&node->pair, ud);
        }
    }
}
