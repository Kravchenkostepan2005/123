#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include "semantic.h"

typedef struct symtable_entry {
    char* key;
    Symbol* value;
    struct symtable_entry* next;
} symtable_entry_t;

struct symtable {
    symtable_entry_t* head;
};

static char* st_strdup(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char* out = (char*)malloc(n);
    if (out) memcpy(out, s, n);
    return out;
}

symtable_t* symtable_create(void) {
    symtable_t* t = (symtable_t*)malloc(sizeof(symtable_t));
    if (!t) return NULL;
    t->head = NULL;
    return t;
}

static void destroy_symbol(Symbol* sym) {
    if (!sym) return;
    if (sym->members) {
        symtable_destroy(sym->members);
        sym->members = NULL;
    }
    free(sym->name);
    free(sym->class_name);
    free(sym);
}

void symtable_destroy(symtable_t* table) {
    if (!table) return;
    symtable_entry_t* cur = table->head;
    while (cur) {
        symtable_entry_t* next = cur->next;
        destroy_symbol(cur->value);
        free(cur->key);
        free(cur);
        cur = next;
    }
    free(table);
}

int symtable_insert(symtable_t* table, const char* key, Symbol* value) {
    if (!table || !key) return 1;
    // Reject duplicate keys
    symtable_entry_t* e = table->head;
    while (e) {
        if (strcmp(e->key, key) == 0) {
            return 1; // already present
        }
        e = e->next;
    }
    symtable_entry_t* node = (symtable_entry_t*)malloc(sizeof(symtable_entry_t));
    if (!node) return 1;
    node->key = st_strdup(key);
    if (!node->key) { free(node); return 1; }
    node->value = value;
    node->next = table->head;
    table->head = node;
    return 0;
}

Symbol* symtable_find(symtable_t* table, const char* key) {
    if (!table || !key) return NULL;
    symtable_entry_t* e = table->head;
    while (e) {
        if (strcmp(e->key, key) == 0) {
            return e->value;
        }
        e = e->next;
    }
    return NULL;
}
