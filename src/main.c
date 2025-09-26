#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>

#include "common.h"
#include "oid.h"
#include "snmp.h"
#include "otel.h"
#include "http.h"

typedef struct {
	char *name;
	char *unit;
} MapEntry;

static int load_mapping_json(const char *path, const char **oids, size_t count, MapEntry *out) {
	if (!path) return 0;
	size_t len = 0; char *txt = read_text_file(path, &len);
	if (!txt) return -1;
	for (size_t i = 0; i < count; ++i) { out[i].name = NULL; out[i].unit = NULL; }
	// Very naive JSON lookup per OID string
	for (size_t i = 0; i < count; ++i) {
		char key[512]; snprintf(key, sizeof(key), "\"%s\"", oids[i]);
		char *p = strstr(txt, key);
		if (!p) continue;
		char *sec = strchr(p, '{'); if (!sec) continue;
		char *end = strchr(sec, '}'); if (!end) continue;
		char *name_pos = strstr(sec, "\"name\"");
		if (name_pos && name_pos < end) {
			char *q = strchr(name_pos, '"'); if (q) q = strchr(q+1, '"'); if (q) { char *q2 = strchr(q+1, '"'); if (q2) { size_t n = (size_t)(q2 - (q+1)); out[i].name = (char*)malloc(n+1); memcpy(out[i].name, q+1, n); out[i].name[n]='\0'; } }
		}
		char *unit_pos = strstr(sec, "\"unit\"");
		if (unit_pos && unit_pos < end) {
			char *q = strchr(unit_pos, '"'); if (q) q = strchr(q+1, '"'); if (q) { char *q2 = strchr(q+1, '"'); if (q2) { size_t n = (size_t)(q2 - (q+1)); out[i].unit = (char*)malloc(n+1); memcpy(out[i].unit, q+1, n); out[i].unit[n]='\0'; } }
		}
	}
	free(txt);
	return 0;
}

static void free_mapping(MapEntry *m, size_t count) { for (size_t i=0;i<count;++i){ free(m[i].name); free(m[i].unit);} }

static void usage(void) {
	fprintf(stderr, "Usage: snmp2otel -t target [-C community] -o oids_file -e endpoint [-i interval] [-r retries] [-T timeout] [-p port] [-v] [-m mapping.json]\n");
}

int main(int argc, char **argv) {
	const char *target = NULL;
	const char *community = "public";
	const char *oids_file = NULL;
	const char *endpoint = NULL;
	const char *mapping_file = NULL;
	int interval = 10;
	int retries = 2;
	int timeout_ms = 1000;
	int port = 161;

	int opt;
	while ((opt = getopt(argc, argv, "t:C:o:e:i:r:T:p:vm:")) != -1) {
		switch (opt) {
			case 't': target = optarg; break;
			case 'C': community = optarg; break;
			case 'o': oids_file = optarg; break;
			case 'e': endpoint = optarg; break;
			case 'i': interval = atoi(optarg); break;
			case 'r': retries = atoi(optarg); break;
			case 'T': timeout_ms = atoi(optarg); break;
			case 'p': port = atoi(optarg); break;
			case 'v': g_verbose = 1; break;
			case 'm': mapping_file = optarg; break;
			default: usage(); return 1;
		}
	}
	if (!target || !oids_file || !endpoint || interval <= 0) { usage(); return 1; }

	size_t oid_count = 0; char **oid_lines = read_oids_file(oids_file, &oid_count);
	if (!oid_lines || oid_count == 0) { LOG_ERROR("Failed to read OIDs file\n"); return 1; }
	Oid *oids = (Oid*)calloc(oid_count, sizeof(Oid));
	if (!oids) { LOG_ERROR("Out of memory\n"); return 1; }
	for (size_t i = 0; i < oid_count; ++i) {
		if (!oid_parse_numeric(oid_lines[i], &oids[i]) || !oid_is_scalar_instance(&oids[i])) {
			LOG_ERROR("Invalid scalar OID: %s\n", oid_lines[i]);
		}
	}

	MapEntry *map = (MapEntry*)calloc(oid_count, sizeof(MapEntry));
	if (load_mapping_json(mapping_file, (const char**)oid_lines, oid_count, map) != 0) {
		LOG_ERROR("Warning: mapping file not loaded\n");
	}

	SnmpConfig cfg = { .target_host = target, .target_port = (uint16_t)port, .community = community, .timeout_ms = (uint32_t)timeout_ms, .retries = retries };
	VarBind *vbs = (VarBind*)calloc(oid_count, sizeof(VarBind));
	MetricMeta *metas = (MetricMeta*)calloc(oid_count, sizeof(MetricMeta));
	for (size_t i = 0; i < oid_count; ++i) { metas[i].name = map[i].name; metas[i].unit = map[i].unit; }

	for (;;) {
		int n = snmp_get(&cfg, oids, oid_count, vbs, oid_count);
		if (n < 0) {
			if (n == -2) LOG_ERROR("SNMP timeout\n"); else LOG_ERROR("SNMP error\n");
			goto sleep_next;
		}
		LOG_INFO("SNMP received %d varbinds\n", n);
		char *json = otel_build_gauges_json(target, vbs, metas, (size_t)n);
		if (!json) { LOG_ERROR("Failed to build OTLP JSON\n"); goto after_send; }
		LOG_INFO("Export JSON: %s\n", json);
		HttpResponse resp;
		if (http_post_json(endpoint, json, strlen(json), &resp) != 0) {
			LOG_ERROR("HTTP POST failed\n");
		} else {
			if (resp.status_code < 200 || resp.status_code >= 300) {
				LOG_ERROR("Export HTTP status: %d\n", resp.status_code);
			}
		}
		free(json);
	after_send:
		for (size_t i = 0; i < (size_t)n; ++i) varbind_free(&vbs[i]);
sleep_next:
		sleep((unsigned int)interval);
	}

	free_string_list(oid_lines, oid_count);
	free(oids); free(vbs); free(metas); free_mapping(map, oid_count); free(map);
	return 0;
}

