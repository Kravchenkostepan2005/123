#ifndef IFJ_SYMTABLE_H
#define IFJ_SYMTABLE_H

#include <stddef.h>   /* size_t */

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque handle to the symbol table */
typedef struct symtable_t symtable_t;

/* Create a new (empty) symbol table */
symtable_t *symtable_create(void);

/* Destroy a symbol table and free associated memory.
   NOTE: stored values (void*) are NOT freed by the table; caller is responsible. */
void symtable_destroy(symtable_t *st);

/* Insert key -> value.
   If key does not exist, inserts and returns 0.
   If key exists, updates stored value and returns 1.
   On allocation failure returns -1.
   Keys are copied (strdup) inside the table. */
int symtable_insert(symtable_t *st, const char *key, void *value);

/* Find value by key. Returns stored value or NULL if not found. */
void *symtable_find(const symtable_t *st, const char *key);

/* Remove a key from the table. Returns 1 if removed, 0 if not found. */
int symtable_remove(symtable_t *st, const char *key);

/* Remove all entries from the table, keeping current capacity. */
void symtable_clear(symtable_t *st);

/* Number of stored entries. */
size_t symtable_size(const symtable_t *st);

/* Iterate over all items. The order is implementation-dependent.
   func is called for each (key, value) pair with given user data pointer ud. */
void symtable_foreach(const symtable_t *st,
                      void (*func)(const char *key, void *value, void *ud),
                      void *ud);

/* Hash function for keys (65599). */
unsigned int symtable_hash_function(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* IFJ_SYMTABLE_H */
