#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"

static char *str_dup_portable(const char *s) {
	size_t n = strlen(s) + 1;
	char *d = (char*)malloc(n);
	if (!d) return NULL;
	memcpy(d, s, n);
	return d;
}

int g_verbose = 0;

static char *trim(char *s) {
	char *end;
	while (*s && isspace((unsigned char)*s)) s++;
	if (*s == 0) return s;
	end = s + strlen(s) - 1;
	while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
	return s;
}

char *read_text_file(const char *path, size_t *out_len) {
	FILE *f = fopen(path, "rb");
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	if (sz < 0) { fclose(f); return NULL; }
	rewind(f);
	char *buf = (char*)malloc((size_t)sz + 1);
	if (!buf) { fclose(f); return NULL; }
	size_t n = fread(buf, 1, (size_t)sz, f);
	fclose(f);
	buf[n] = '\0';
	if (out_len) *out_len = n;
	return buf;
}

char **read_oids_file(const char *path, size_t *out_count) {
	*out_count = 0;
	size_t len = 0;
	char *content = read_text_file(path, &len);
	if (!content) return NULL;

	size_t cap = 8;
	char **list = (char**)malloc(cap * sizeof(char*));
	if (!list) { free(content); return NULL; }

	char *line = strtok(content, "\n\r");
	while (line) {
		char *t = trim(line);
		if (*t && *t != '#') {
			if (*out_count == cap) {
				cap *= 2;
				char **tmp = (char**)realloc(list, cap * sizeof(char*));
				if (!tmp) break;
				list = tmp;
			}
			list[*out_count] = str_dup_portable(t);
			(*out_count)++;
		}
		line = strtok(NULL, "\n\r");
	}
	free(content);
	return list;
}

void free_string_list(char **list, size_t count) {
	if (!list) return;
	for (size_t i = 0; i < count; ++i) free(list[i]);
	free(list);
}
