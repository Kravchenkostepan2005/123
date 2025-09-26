#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "oid.h"

typedef enum {
	SNMP_OK = 0,
	SNMP_TIMEOUT,
	SNMP_ERR
} SnmpResult;

typedef enum {
	SNMP_TYPE_INTEGER,
	SNMP_TYPE_OCTET_STRING,
	SNMP_TYPE_OID,
	SNMP_TYPE_NULL,
	SNMP_TYPE_COUNTER32,
	SNMP_TYPE_GAUGE32,
	SNMP_TYPE_TIMETICKS,
	SNMP_TYPE_COUNTER64
} SnmpType;

typedef struct {
	Oid oid;
	SnmpType type;
	union {
		int64_t integer;
		struct { const uint8_t *ptr; size_t len; } octets;
		Oid oid_val;
		uint64_t u64;
	} value;
} VarBind;

typedef struct {
	const char *target_host;
	uint16_t target_port;
	const char *community;
	uint32_t timeout_ms;
	int retries;
} SnmpConfig;

// Perform SNMPv2c GET for given scalar OIDs. Returns number of successful varbinds filled.
int snmp_get(const SnmpConfig *cfg, const Oid *oids, size_t oid_count, VarBind *out_vbs, size_t max_vbs);

void varbind_free(VarBind *vb);

