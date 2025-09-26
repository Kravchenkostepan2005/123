#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "snmp.h"

typedef struct {
	const char *name;
	const char *unit; // optional
} MetricMeta;

// Build OTLP/HTTP JSON body for a set of gauge-like varbinds mapped to names.
// 'names' array length equals vb_count; if names[i] is NULL, derive from OID.
// Returns allocated string (must free), or NULL on error.
char *otel_build_gauges_json(const char *resource_attr, const VarBind *vbs, const MetricMeta *metas, size_t vb_count);

