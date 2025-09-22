#include "symtable.h"
int main(){symtable_t *st=symtable_create(); symtable_insert(st, "a", (void*)1); symtable_insert(st, "b", (void*)2); symtable_insert(st, "a", (void*)3); symtable_destroy(st); return 0;}
