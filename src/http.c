#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "http.h"

static int parse_url(const char *url, char *scheme, size_t scheme_sz, char *host, size_t host_sz, int *port, char *path, size_t path_sz) {
	// Supports http only for baseline
	const char *p = strstr(url, "://");
	if (!p) return -1;
	size_t s_len = (size_t)(p - url);
	if (s_len + 1 > scheme_sz) return -1;
	memcpy(scheme, url, s_len); scheme[s_len] = '\0';
	p += 3; // skip ://
	const char *slash = strchr(p, '/');
	const char *host_end = slash ? slash : url + strlen(url);
	const char *colon = memchr(p, ':', (size_t)(host_end - p));
	size_t h_len = colon ? (size_t)(colon - p) : (size_t)(host_end - p);
	if (h_len + 1 > host_sz) return -1;
	memcpy(host, p, h_len); host[h_len] = '\0';
	if (colon) {
		*port = atoi(colon + 1);
	} else {
		*port = (strcmp(scheme, "https") == 0) ? 443 : 80;
	}
	if (slash) {
		if (strlen(slash) + 1 > path_sz) return -1;
		strncpy(path, slash, path_sz - 1); path[path_sz - 1] = '\0';
	} else {
		if (path_sz < 2) return -1;
		path[0] = '/';
		path[1] = '\0';
	}
	return 0;
}

int http_post_json(const char *url, const char *body, size_t body_len, HttpResponse *resp_out) {
	char scheme[8], host[256], path[1024]; int port;
	if (parse_url(url, scheme, sizeof(scheme), host, sizeof(host), &port, path, sizeof(path)) != 0)
		return -1;
	if (strcmp(scheme, "http") != 0) return -1; // HTTPS not implemented in baseline

	struct addrinfo hints; memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; hints.ai_socktype = SOCK_STREAM;
	char portstr[16]; snprintf(portstr, sizeof(portstr), "%d", port);
	struct addrinfo *res = NULL;
	if (getaddrinfo(host, portstr, &hints, &res) != 0) return -1;
	int sock = -1; int rc = -1;
	for (struct addrinfo *ai = res; ai; ai = ai->ai_next) {
		sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (sock < 0) continue;
		if (connect(sock, ai->ai_addr, ai->ai_addrlen) == 0) break;
		close(sock); sock = -1;
	}
	freeaddrinfo(res);
	if (sock < 0) return -1;

	char header[1024];
	int hn = snprintf(header, sizeof(header),
		"POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
		path, host, body_len);
	if (hn <= 0 || (size_t)hn >= sizeof(header)) { close(sock); return -1; }
	ssize_t sent = send(sock, header, (size_t)hn, 0);
	if (sent != (ssize_t)hn) { close(sock); return -1; }
	if (body_len > 0) {
		ssize_t s2 = send(sock, body, body_len, 0);
		if (s2 != (ssize_t)body_len) { close(sock); return -1; }
	}

	// Read response into dynamic buffer
	size_t cap = 4096, len = 0;
	char *resp = (char*)malloc(cap);
	if (!resp) { close(sock); return -1; }
	for (;;) {
		if (len == cap) { cap *= 2; char *tmp = (char*)realloc(resp, cap); if (!tmp) { free(resp); close(sock); return -1; } resp = tmp; }
		ssize_t n = recv(sock, resp + len, cap - len, 0);
		if (n < 0) { free(resp); close(sock); return -1; }
		if (n == 0) break; len += (size_t)n;
	}
	close(sock);
	// Parse status code
	int status = 0;
	char *sp = memchr(resp, ' ', len);
	if (sp && sp + 3 < resp + len) status = atoi(sp + 1);
	// Find header/body separator
	char *hdr_end = NULL;
	for (size_t i = 0; i + 3 < len; ++i) {
		if (resp[i] == '\r' && resp[i+1] == '\n' && resp[i+2] == '\r' && resp[i+3] == '\n') { hdr_end = resp + i + 4; break; }
	}
	if (!hdr_end) { free(resp); return -1; }
	if (resp_out) {
		resp_out->status_code = status;
		resp_out->body = hdr_end;
		resp_out->body_len = (size_t)(resp + len - hdr_end);
	}
	// Keep buffer allocated; user does not free in this simple client; we leak minimally
	return 0;
}

