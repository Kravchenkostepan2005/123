#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	const char *url; // e.g., http://host:port/path
} HttpConfig;

typedef struct {
	int status_code;
	const char *body; // points into internal buffer
	size_t body_len;
} HttpResponse;

// Simple blocking HTTP/1.1 POST with JSON body. Returns 0 on success.
int http_post_json(const char *url, const char *body, size_t body_len, HttpResponse *resp_out);

