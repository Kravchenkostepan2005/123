#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
#define LOG_INFO(...)  do { if (g_verbose) fprintf(stderr, __VA_ARGS__); } while (0)
extern int g_verbose;

char *read_text_file(const char *path, size_t *out_len);
char **read_oids_file(const char *path, size_t *out_count);
void free_string_list(char **list, size_t count);
