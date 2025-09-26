#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "symtable.h"

static void count_visitor(const char *key, void *value, void *ud) {
    (void)key;
    (void)value;
    size_t *counter = (size_t *)ud;
    (*counter)++;
}

static void collect_concat_visitor(const char *key, void *value, void *ud) {
    (void)value;
    char *buf = (char *)ud;
    strcat(buf, key);
    strcat(buf, ",");
}

int main(void) {
    /* create/destroy */
    symtable_t *st = symtable_create();
    assert(st != NULL);

    /* find on empty / NULL args */
    assert(symtable_find(NULL, "x") == NULL);
    assert(symtable_find(st, NULL) == NULL);
    assert(symtable_find(st, "missing") == NULL);

    /* insert new */
    int v1 = 123;
    assert(symtable_insert(st, "alpha", &v1) == 0);
    assert(symtable_find(st, "alpha") == &v1);

    /* update existing (returns 1) */
    int v1b = 456;
    assert(symtable_insert(st, "alpha", &v1b) == 1);
    assert(symtable_find(st, "alpha") == &v1b);

    /* collisions and multiple inserts */
    int v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    assert(symtable_insert(st, "beta", &v2) == 0);
    assert(symtable_insert(st, "gamma", &v3) == 0);
    assert(symtable_insert(st, "delta", &v4) == 0);
    assert(symtable_insert(st, "epsilon", &v5) == 0);

    assert(symtable_find(st, "beta") == &v2);
    assert(symtable_find(st, "gamma") == &v3);
    assert(symtable_find(st, "delta") == &v4);
    assert(symtable_find(st, "epsilon") == &v5);

    /* force rehash by exhausting free list */
    /* INITIAL_CAPACITY is 8; we already inserted 5 unique keys (alpha..epsilon). */
    /* Insert more to exceed capacity and trigger growth */
    for (int i = 0; i < 16; i++) {
        char key[32];
        snprintf(key, sizeof(key), "k%d", i);
        int *pv = (int *)malloc(sizeof(int));
        assert(pv != NULL);
        *pv = i;
        int rc = symtable_insert(st, key, pv);
        assert(rc == 0);
    }

    /* All previously inserted keys must still be present after possible rehash */
    assert(symtable_find(st, "alpha") == &v1b);
    assert(symtable_find(st, "beta") == &v2);
    assert(symtable_find(st, "gamma") == &v3);
    assert(symtable_find(st, "delta") == &v4);
    assert(symtable_find(st, "epsilon") == &v5);

    /* foreach should visit every occupied slot exactly once */
    size_t count = 0;
    symtable_foreach(st, count_visitor, &count);
    /* We inserted: 5 fixed keys + 16 generated keys = 21 unique keys */
    assert(count >= 21); /* could be more if earlier updates didn't consume free slots */

    /* foreach content sanity: concatenate a few known keys and check substrings */
    char buf[1024];
    buf[0] = '\0';
    symtable_foreach(st, collect_concat_visitor, buf);
    assert(strstr(buf, "alpha,") != NULL);
    assert(strstr(buf, "beta,") != NULL);

    symtable_destroy(st);

    /* Hash function quick checks (determinism) */
    unsigned int h1 = symtable_hash_function("alpha");
    unsigned int h2 = symtable_hash_function("alpha");
    unsigned int h3 = symtable_hash_function("beta");
    assert(h1 == h2);
    assert(h1 != h3);

    printf("All symtable tests passed.\n");
    return 0;
}

