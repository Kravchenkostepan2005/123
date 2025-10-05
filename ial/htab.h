#ifndef IAL_HTAB_H
#define IAL_HTAB_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Commonly used in IAL assignments */
typedef const char *htab_key_t;
typedef size_t htab_value_t;

typedef struct htab htab_t;
typedef struct htab_item htab_item_t;

typedef struct {
    htab_key_t   key;
    htab_value_t value;
} htab_pair_t;

/* Iterator over table items */
typedef struct {
    htab_t      *t;    /* owning table */
    size_t       idx;  /* bucket index */
    htab_item_t *ptr;  /* current item or NULL */
} htab_iterator_t;

/* API */
htab_t *htab_init(size_t bucket_count);
void    htab_free(htab_t *t);
void    htab_clear(htab_t *t);
size_t  htab_size(const htab_t *t);
size_t  htab_bucket_count(const htab_t *t);

size_t  htab_hash_function(const char *str);

/* Lookup/add and find return iterators */
htab_iterator_t htab_lookup_add(htab_t *t, htab_key_t key);
htab_iterator_t htab_find(const htab_t *t, htab_key_t key);
bool            htab_erase(htab_t *t, htab_key_t key);

/* Iteration */
htab_iterator_t htab_begin(const htab_t *t);
htab_iterator_t htab_end(const htab_t *t);
htab_iterator_t htab_iterator_next(htab_iterator_t it);
htab_key_t      htab_iterator_get_key(htab_iterator_t it);
htab_value_t   *htab_iterator_get_value(htab_iterator_t it);
bool            htab_iterator_valid(htab_iterator_t it);
bool            htab_iterator_equal(htab_iterator_t it1, htab_iterator_t it2);

/* Apply a function to each stored pair (implementation-defined order) */
void htab_for_each(const htab_t *t, void (*f)(htab_pair_t *pair, void *ud), void *ud);

#ifdef __cplusplus
}
#endif

#endif /* IAL_HTAB_H */
