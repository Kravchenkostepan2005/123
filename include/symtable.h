#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stddef.h>

/* Opaque type for symbol table */
typedef struct symtable_t symtable_t;

/* Public API */
unsigned int symtable_hash_function(const char *str);

symtable_t *symtable_create(void);
void        symtable_destroy(symtable_t *st);

int         symtable_insert(symtable_t *st, const char *key, void *value);
void       *symtable_find(symtable_t *st, const char *key);

void        symtable_foreach(symtable_t *st,
                             void (*func)(const char *key, void *value, void *ud),
                             void *ud);

#endif /* SYMTABLE_H */

