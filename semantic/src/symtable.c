#include <stdlib.h>
#include <string.h>
#include "symtable.h"

typedef struct sym_entry {
    char *key;
    Symbol *value;
    struct sym_entry *next;
} sym_entry_t;

struct symtable {
    sym_entry_t *head;
};

symtable_t *symtable_create(void) {
    symtable_t *t = (symtable_t *)malloc(sizeof(symtable_t));
    if (!t) return NULL;
    t->head = NULL;
    return t;
}

static void free_symbol(Symbol *s) {
    if (!s) return;
    if (s->name) free(s->name);
    if (s->class_name) free(s->class_name);
    if (s->members) {
        symtable_destroy(s->members);
        s->members = NULL;
    }
    free(s);
}

void symtable_destroy(symtable_t *table) {
    if (!table) return;
    sym_entry_t *cur = table->head;
    while (cur) {
        sym_entry_t *next = cur->next;
        if (cur->key) free(cur->key);
        free_symbol(cur->value);
        free(cur);
        cur = next;
    }
    free(table);
}

int symtable_insert(symtable_t *table, const char *key, Symbol *value) {
    if (!table || !key || !value) return -1;
    sym_entry_t *entry = (sym_entry_t *)malloc(sizeof(sym_entry_t));
    if (!entry) return -1;
    entry->key = strdup(key);
    if (!entry->key) { free(entry); return -1; }
    entry->value = value;
    entry->next = table->head;
    table->head = entry;
    return 0;
}

Symbol *symtable_find(symtable_t *table, const char *key) {
    if (!table || !key) return NULL;
    for (sym_entry_t *e = table->head; e; e = e->next) {
        if (e->key && strcmp(e->key, key) == 0) {
            return e->value;
        }
    }
    return NULL;
}
