#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "otel.h"
#include "oid.h"

static void json_escape(const char *s, char *out, size_t out_sz) {
	size_t pos = 0;
	for (; *s && pos + 2 < out_sz; ++s) {
		unsigned char c = (unsigned char)*s;
		if (c == '"' || c == '\\') { if (pos + 2 >= out_sz) break; out[pos++]='\\'; out[pos++]=c; }
		else if (c >= 0x20) { out[pos++] = c; }
		else { if (pos + 6 >= out_sz) break; pos += (size_t)snprintf(out+pos, out_sz-pos, "\\u%04x", c); }
	}
	out[pos] = '\0';
}

static char *derive_metric_name(const Oid *oid) {
	char buf[256];
	if (oid_format(oid, buf, sizeof(buf)) < 0) return NULL;
	// replace dots with underscores and prefix
	for (char *p = buf; *p; ++p) if (*p == '.') *p = '_';
	char *name = (char*)malloc(strlen("snmp.") + strlen(buf) + 1);
	if (!name) return NULL;
	strcpy(name, "snmp.");
	strcat(name, buf);
	return name;
}

char *otel_build_gauges_json(const char *resource_attr, const VarBind *vbs, const MetricMeta *metas, size_t vb_count) {
	// Construct minimal OTLP JSON for metrics per spec
	// { "resourceMetrics": [ { "resource": {"attributes": [...]}, "scopeMetrics": [ { "metrics": [ ... ] } ] } ] }

	// Timestamp in nanoseconds
	struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
	long long ts_nanos = (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;

	// Buffer assembly via dynamic string
	size_t cap = 8192; size_t len = 0; char *json = (char*)malloc(cap);
	if (!json) return NULL;
	#define APPEND_FMT(...) do { \
		int _n = snprintf(json + len, cap - len, __VA_ARGS__); \
		if (_n < 0) { free(json); return NULL; } \
		if ((size_t)_n >= cap - len) { \
			cap = (cap + (size_t)_n + 4096); \
			char *tmp = (char*)realloc(json, cap); if (!tmp) { free(json); return NULL; } \
			json = tmp; \
			_n = snprintf(json + len, cap - len, __VA_ARGS__); \
		} \
		len += (size_t)_n; \
	} while (0)

	APPEND_FMT("{\"resourceMetrics\":[{\"resource\":{\"attributes\":[");
	if (resource_attr && *resource_attr) {
		char esc[256]; json_escape(resource_attr, esc, sizeof(esc));
		APPEND_FMT("{\"key\":\"service.name\",\"value\":{\"stringValue\":\"%s\"}}", esc);
	}
	APPEND_FMT("]},\"scopeMetrics\":[{\"metrics\":[");
	for (size_t i = 0; i < vb_count; ++i) {
		const VarBind *vb = &vbs[i];
		const MetricMeta *meta = metas ? &metas[i] : NULL;
		// Map value to double for gauge
		double value = 0.0; bool ok = true;
		if (vb->type == SNMP_TYPE_INTEGER) value = (double)vb->value.integer;
		else if (vb->type == SNMP_TYPE_GAUGE32 || vb->type == SNMP_TYPE_COUNTER32 || vb->type == SNMP_TYPE_TIMETICKS || vb->type == SNMP_TYPE_COUNTER64) value = (double)vb->value.u64;
		else { ok = false; }
		if (!ok) continue;
		char *name_owned = NULL; const char *name = NULL;
		if (meta && meta->name) name = meta->name; else { name_owned = derive_metric_name(&vb->oid); name = name_owned ? name_owned : "snmp.unknown"; }
		char esc_name[512]; json_escape(name, esc_name, sizeof(esc_name));
		const char *unit = (meta && meta->unit) ? meta->unit : "";
		char esc_unit[128]; json_escape(unit, esc_unit, sizeof(esc_unit));
		APPEND_FMT("%s{\"name\":\"%s\",\"unit\":\"%s\",\"gauge\":{\"dataPoints\":[{\"asDouble\":%.6f,\"timeUnixNano\":%lld}]} }",
			i==0?"":",", esc_name, esc_unit, value, ts_nanos);
		free(name_owned);
	}
	APPEND_FMT("]}]}]}");
	#undef APPEND_FMT
	return json;
}

