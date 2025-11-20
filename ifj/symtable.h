#ifndef SYMTABLE_H
#define SYMTABLE_H


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

    /* Find value by key. Returns stored value or NULL if not found.
       (If you store NULL as a legitimate value, you may need separate semantics.) */
    void *symtable_find(symtable_t *st, const char *key);

    /* Iterate over all items. The order is implementation-dependent.
       func is called for each (key, value) pair with given user data pointer ud.
       func must not modify the table structure (no insert/remove during iteration). */
    void symtable_foreach(symtable_t *st, void (*func)(const char *key, void *value, void *ud), void *ud);

    /* Hash function for keys (required by assignment). */
    unsigned int symtable_hash_function(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* SYMTABLE_H */
