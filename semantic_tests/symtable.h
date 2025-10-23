#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct symtable symtable_t;
// Forward-declare Symbol to avoid circular includes
typedef struct Symbol Symbol;

symtable_t* symtable_create(void);
void symtable_destroy(symtable_t* table);
int symtable_insert(symtable_t* table, const char* key, Symbol* value);
Symbol* symtable_find(symtable_t* table, const char* key);

#ifdef __cplusplus
}
#endif
