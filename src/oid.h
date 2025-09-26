#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint32_t *arcs;
	size_t count;
} Oid;

bool oid_parse_numeric(const char *s, Oid *out);
void oid_free(Oid *oid);
bool oid_is_scalar_instance(const Oid *oid);
// Format to numeric dotted string. Returns bytes written (excluding NUL) or -1.
int oid_format(const Oid *oid, char *buf, size_t buf_size);

