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
#include <sys/time.h>

#include "snmp.h"
#include "ber.h"
#include "common.h"

// SNMP PDU types
#define SNMP_TAG_MESSAGE   0x30
#define SNMP_TAG_PDU_GET   0xA0

static bool encode_snmp_get(uint8_t *buf, size_t cap, const char *community, const Oid *oids, size_t oid_count, size_t *out_len, int32_t request_id) {
	BerBuf b; ber_init(&b, buf, cap);

	// Message SEQUENCE
	size_t msg_len_pos;
	if (!ber_begin_sequence(&b, SNMP_TAG_MESSAGE, &msg_len_pos)) return false;

	// version INTEGER = 1 (SNMPv2c)
	if (!ber_put_integer(&b, 1)) return false;
	// community OCTET STRING
	if (!ber_put_octet_string(&b, (const uint8_t*)community, strlen(community))) return false;

	// GET PDU
	size_t pdu_len_pos;
	if (!ber_begin_sequence(&b, SNMP_TAG_PDU_GET, &pdu_len_pos)) return false;
	// request-id, error-status, error-index
	if (!ber_put_integer(&b, request_id)) return false;
	if (!ber_put_integer(&b, 0)) return false;
	if (!ber_put_integer(&b, 0)) return false;

	// VarBindList: SEQUENCE OF VarBind
	size_t vbl_len_pos;
	if (!ber_begin_sequence(&b, BER_TAG_SEQUENCE, &vbl_len_pos)) return false;
	for (size_t i = 0; i < oid_count; ++i) {
		size_t vb_len_pos;
		if (!ber_begin_sequence(&b, BER_TAG_SEQUENCE, &vb_len_pos)) return false;
		if (!ber_put_oid(&b, oids[i].arcs, oids[i].count)) return false;
		if (!ber_put_null(&b)) return false; // value is NULL in GET request
		if (!ber_end_sequence(&b, vb_len_pos)) return false;
	}
	if (!ber_end_sequence(&b, vbl_len_pos)) return false;

	if (!ber_end_sequence(&b, pdu_len_pos)) return false;
	if (!ber_end_sequence(&b, msg_len_pos)) return false;

	*out_len = b.len;
	return true;
}

static bool decode_varbind_value(uint8_t tag, const uint8_t *val, size_t len, VarBind *vb) {
	// Handle a subset sufficient for gauges/counters/integers and octets
	if (tag == BER_TAG_INTEGER) {
		int64_t v = (val[0] & 0x80) ? -1 : 0;
		for (size_t i = 0; i < len; ++i) v = (v << 8) | val[i];
		vb->type = SNMP_TYPE_INTEGER;
		vb->value.integer = v;
		return true;
	}
	if (tag == BER_TAG_OCTET_STR) {
		vb->type = SNMP_TYPE_OCTET_STRING;
		vb->value.octets.ptr = val;
		vb->value.octets.len = len;
		return true;
	}
	if (tag == BER_TAG_NULL) {
		vb->type = SNMP_TYPE_NULL;
		return true;
	}
	if (tag == BER_TAG_OID) {
		uint32_t tmp[64]; size_t n = 64;
		size_t off = 0;
		BerBuf dummy; // unused
		(void)dummy;
		if (!ber_get_oid(val, len, &off, tmp, &n)) return false;
		vb->type = SNMP_TYPE_OID;
		vb->value.oid_val.arcs = (uint32_t*)malloc(n * sizeof(uint32_t));
		if (!vb->value.oid_val.arcs) return false;
		memcpy(vb->value.oid_val.arcs, tmp, n * sizeof(uint32_t));
		vb->value.oid_val.count = n;
		return true;
	}
	if (tag == BER_TAG_GAUGE32 || tag == BER_TAG_COUNTER32 || tag == BER_TAG_TIMETICKS) {
		uint64_t v = 0;
		for (size_t i = 0; i < len; ++i) v = (v << 8) | val[i];
		vb->type = (tag == BER_TAG_GAUGE32) ? SNMP_TYPE_GAUGE32 : (tag == BER_TAG_COUNTER32 ? SNMP_TYPE_COUNTER32 : SNMP_TYPE_TIMETICKS);
		vb->value.u64 = v;
		return true;
	}
	if (tag == 0x46) { // Counter64
		uint64_t v = 0;
		for (size_t i = 0; i < len; ++i) v = (v << 8) | val[i];
		vb->type = SNMP_TYPE_COUNTER64;
		vb->value.u64 = v;
		return true;
	}
	return false;
}

static int parse_response(const uint8_t *buf, size_t buflen, VarBind *out_vbs, size_t max_vbs) {
	size_t off = 0; uint8_t tag; const uint8_t *val; size_t len;
	// Message
	if (!ber_get_tlv(buf, buflen, &off, &tag, &val, &len) || tag != BER_TAG_SEQUENCE) return -1;
	size_t msg_end = off - len + len; // unused
	(void)msg_end;
	// version
	int64_t version;
	if (!ber_get_integer(buf, buflen, &off, &version)) return -1;
	// community
	const uint8_t *comm; size_t comm_len;
	if (!ber_get_octet_string(buf, buflen, &off, &comm, &comm_len)) return -1;
	// PDU (GetResponse is 0xA2)
	if (!ber_get_tlv(buf, buflen, &off, &tag, &val, &len)) return -1;
	if (tag != 0xA2) return -1;
	// Inside PDU: request-id, error-status, error-index
	int64_t req_id, err_status, err_index;
	if (!ber_get_integer(buf, buflen, &off, &req_id)) return -1;
	if (!ber_get_integer(buf, buflen, &off, &err_status)) return -1;
	if (!ber_get_integer(buf, buflen, &off, &err_index)) return -1;
	// VarBindList
	if (!ber_get_tlv(buf, buflen, &off, &tag, &val, &len) || tag != BER_TAG_SEQUENCE) return -1;
	size_t vbl_end = off - len + len; // not used directly
	(void)vbl_end;

	int vb_index = 0;
	// Iterate varbinds by peeking into val area
	size_t vboff = off - len;
	while (vboff < off) {
		uint8_t vtag; const uint8_t *vval; size_t vlen;
		if (!ber_get_tlv(buf, buflen, &vboff, &vtag, &vval, &vlen)) break;
		if (vtag != BER_TAG_SEQUENCE) break;
		// VarBind SEQUENCE content
		size_t vboff2 = vboff - vlen; // start of VB content
		// Actually decode inside using a fresh cursor over vval
		size_t inner_off = vboff - vlen;
		(void)inner_off;
		// Re-parse: OID
		size_t tmp_off = vboff - vlen;
		uint8_t itag; const uint8_t *ival; size_t ilen;
		if (!ber_get_tlv(buf, buflen, &tmp_off, &itag, &ival, &ilen) || itag != BER_TAG_OID) break;
		uint32_t arcs[64]; size_t n = 64; size_t ival_off = 0;
		if (!ber_get_oid(ival, ilen, &ival_off, arcs, &n)) break;
		if (!ber_get_tlv(buf, buflen, &tmp_off, &itag, &ival, &ilen)) break;
		if (vb_index < (int)max_vbs) {
			VarBind *vb = &out_vbs[vb_index];
			vb->oid.arcs = (uint32_t*)malloc(n * sizeof(uint32_t));
			if (!vb->oid.arcs) break;
			memcpy(vb->oid.arcs, arcs, n * sizeof(uint32_t));
			vb->oid.count = n;
			if (!decode_varbind_value(itag, ival, ilen, vb)) { varbind_free(vb); break; }
			vb_index++;
		}
		vboff = tmp_off;
	}
	return vb_index;
}

static int udp_send_recv(const char *host, uint16_t port, const uint8_t *req, size_t req_len,
			uint8_t *resp, size_t resp_cap, uint32_t timeout_ms) {
	int sock = -1;
	int rc = -1;
	struct addrinfo hints; memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	char portstr[16]; snprintf(portstr, sizeof(portstr), "%u", port);
	struct addrinfo *res = NULL;
	if (getaddrinfo(host, portstr, &hints, &res) != 0) return -1;
	for (struct addrinfo *ai = res; ai; ai = ai->ai_next) {
		sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (sock < 0) continue;
		struct timeval tv; tv.tv_sec = timeout_ms / 1000; tv.tv_usec = (timeout_ms % 1000) * 1000;
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		ssize_t sent = sendto(sock, req, req_len, 0, ai->ai_addr, ai->ai_addrlen);
		if (sent != (ssize_t)req_len) { close(sock); sock = -1; continue; }
		rc = (int)recvfrom(sock, resp, resp_cap, 0, NULL, NULL);
		close(sock);
		sock = -1;
		if (rc > 0) break;
	}
	freeaddrinfo(res);
	return rc;
}

int snmp_get(const SnmpConfig *cfg, const Oid *oids, size_t oid_count, VarBind *out_vbs, size_t max_vbs) {
	uint8_t req[1024]; size_t req_len = 0;
	int32_t request_id = (int32_t)(rand() & 0x7FFFFFFF);
	if (!encode_snmp_get(req, sizeof(req), cfg->community, oids, oid_count, &req_len, request_id)) {
		return -1;
	}
	uint8_t resp[1500];
	int attempts = cfg->retries + 1;
	while (attempts-- > 0) {
		int n = udp_send_recv(cfg->target_host, cfg->target_port, req, req_len, resp, sizeof(resp), cfg->timeout_ms);
		if (n < 0) {
			if (attempts == 0) return -2; // timeout
			continue;
		}
		int vbcount = parse_response(resp, (size_t)n, out_vbs, max_vbs);
		if (vbcount >= 0) return vbcount;
		if (attempts == 0) return -1;
	}
	return -1;
}

void varbind_free(VarBind *vb) {
	if (!vb) return;
	free(vb->oid.arcs);
	vb->oid.arcs = NULL; vb->oid.count = 0;
	if (vb->type == SNMP_TYPE_OID) {
		free(vb->value.oid_val.arcs);
		vb->value.oid_val.arcs = NULL;
		vb->value.oid_val.count = 0;
	}
}

