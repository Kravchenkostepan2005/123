#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "oid.h"

static int parse_uint32(const char *s, size_t len, uint32_t *out) {
	if (len == 0) return -1;
	uint64_t v = 0;
	for (size_t i = 0; i < len; ++i) {
		if (!isdigit((unsigned char)s[i])) return -1;
		v = v * 10 + (uint64_t)(s[i] - '0');
		if (v > 0xFFFFFFFFu) return -1;
	}
	*out = (uint32_t)v;
	return 0;
}

bool oid_parse_numeric(const char *s, Oid *out) {
	memset(out, 0, sizeof(*out));
	if (!s || *s == '\0') return false;
	// Skip optional leading dot
	if (*s == '.') s++;
	// Count arcs
	size_t count = 1;
	for (const char *p = s; *p; ++p) if (*p == '.') count++;
	uint32_t *arcs = (uint32_t*)calloc(count, sizeof(uint32_t));
	if (!arcs) return false;
	const char *seg = s;
	size_t seg_len = 0;
	size_t idx = 0;
	for (const char *p = s; ; ++p) {
		if (*p == '.' || *p == '\0') {
			if (parse_uint32(seg, seg_len, &arcs[idx]) != 0) { free(arcs); return false; }
			idx++;
			if (*p == '\0') break;
			seg = p + 1;
			seg_len = 0;
		} else {
			seg_len++;
		}
	}
	out->arcs = arcs;
	out->count = idx;
	return true;
}

void oid_free(Oid *oid) {
	if (!oid) return;
	free(oid->arcs);
	oid->arcs = NULL;
	oid->count = 0;
}

bool oid_is_scalar_instance(const Oid *oid) {
	if (!oid || oid->count == 0) return false;
	return oid->arcs[oid->count - 1] == 0;
}

int oid_format(const Oid *oid, char *buf, size_t buf_size) {
	if (!oid || !buf || buf_size == 0) return -1;
	size_t pos = 0;
	for (size_t i = 0; i < oid->count; ++i) {
		int n = snprintf(buf + pos, buf_size - pos, i == 0 ? "%u" : ".%u", oid->arcs[i]);
		if (n < 0) return -1;
		pos += (size_t)n;
		if (pos >= buf_size) return -1;
	}
	return (int)pos;
}

