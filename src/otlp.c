#include "otlp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>

int parse_http_url(const char *url, http_url_t *out)
{
	memset(out, 0, sizeof(*out));
	const char *p = strstr(url, "://");
	if (!p) return -1;
	size_t slen = (size_t)(p - url);
	if (slen >= sizeof(out->scheme)) return -1;
	memcpy(out->scheme, url, slen); out->scheme[slen] = '\0';
	p += 3;
	const char *slash = strchr(p, '/');
	const char *hostport_end = slash ? slash : p + strlen(p);
	const char *colon = NULL;
	for (const char *q = p; q < hostport_end; ++q) if (*q == ':') { colon = q; break; }
	if (colon) {
		size_t hlen = (size_t)(colon - p); if (hlen >= sizeof(out->host)) return -1;
		memcpy(out->host, p, hlen); out->host[hlen] = '\0';
		int port = atoi(colon + 1);
		if (port <= 0 || port > 65535) return -1;
		out->port = port;
	} else {
		size_t hlen = (size_t)(hostport_end - p); if (hlen >= sizeof(out->host)) return -1;
		memcpy(out->host, p, hlen); out->host[hlen] = '\0';
		out->port = (strcmp(out->scheme, "https") == 0) ? 443 : 80;
	}
	if (slash) {
		size_t plen = strlen(slash);
		if (plen >= sizeof(out->path)) return -1;
		memcpy(out->path, slash, plen + 1);
	} else {
		strcpy(out->path, "/");
	}
	return 0;
}

static int http_post_json(const http_url_t *u, const char *json, size_t json_len, int verbose)
{
	struct addrinfo hints; memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	char portstr[16]; snprintf(portstr, sizeof(portstr), "%d", u->port);
	struct addrinfo *res = NULL;
	int rc = getaddrinfo(u->host, portstr, &hints, &res);
	if (rc != 0) { fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc)); return -1; }
	int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sock < 0) { perror("socket"); freeaddrinfo(res); return -1; }
	if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) { perror("connect"); close(sock); freeaddrinfo(res); return -1; }
	freeaddrinfo(res);
	char header[1024];
	int n = snprintf(header, sizeof(header),
		"POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
		u->path, u->host, json_len);
	if (n <= 0 || (size_t)n >= sizeof(header)) { close(sock); return -1; }
	ssize_t w = send(sock, header, (size_t)n, 0);
	if (w < 0) { perror("send header"); close(sock); return -1; }
	w = send(sock, json, json_len, 0);
	if (w < 0) { perror("send body"); close(sock); return -1; }
	char resp[1024]; ssize_t r = recv(sock, resp, sizeof(resp)-1, 0);
	if (r <= 0) { close(sock); return -1; }
	resp[r] = '\0';
	if (verbose) fprintf(stderr, "OTLP resp: %.*s\n", (int)r, resp);
	// Check HTTP status
	if (strncmp(resp, "HTTP/1.1 200", 12) != 0 && strncmp(resp, "HTTP/1.1 20", 11) != 0) {
		// non-2xx
		close(sock);
		return -1;
	}
	close(sock);
	return 0;
}

int otlp_export_gauge(const http_url_t *endpoint, const char *metric_name, const char *unit, double value, int verbose)
{
	char json[1024];
	// Minimal OTLP/HTTP JSON for metrics
	// Resource-less single gauge point with time unix nanos now
	struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
	unsigned long long tn = (unsigned long long)ts.tv_sec * 1000000000ULL + (unsigned long long)ts.tv_nsec;
	int n = snprintf(json, sizeof(json),
		"{\"resourceMetrics\":[{\"scopeMetrics\":[{\"metrics\":[{\"name\":\"%s\",\"unit\":\"%s\",\"gauge\":{\"dataPoints\":[{\"asDouble\":%.10g,\"timeUnixNano\":\"%llu\"}]}}]}]}]}",
		metric_name, unit ? unit : "", value, tn);
	if (n <= 0 || (size_t)n >= sizeof(json)) return -1;
	return http_post_json(endpoint, json, (size_t)n, verbose);
}

