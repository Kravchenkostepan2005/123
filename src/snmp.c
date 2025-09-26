#include "snmp.h"
#include "ber.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>

static uint32_t rand32(void)
{
	uint32_t v;
	FILE *f = fopen("/dev/urandom", "rb");
	if (f) {
		if (fread(&v, 1, sizeof(v), f) != sizeof(v)) v = (uint32_t)time(NULL);
		fclose(f);
	} else {
		v = (uint32_t)time(NULL);
	}
	return v;
}

int snmp_client_init(snmp_client_t *c, const char *target, int port, const char *community, int timeout_ms, int retries, int verbose)
{
	memset(c, 0, sizeof(*c));
	strncpy(c->target, target ? target : "", sizeof(c->target)-1);
	strncpy(c->community, community ? community : "public", sizeof(c->community)-1);
	c->port = port;
	c->timeout_ms = timeout_ms;
	c->retries = retries;
	c->verbose = verbose;
	c->sockfd = -1;

	struct addrinfo hints; memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	char portstr[16]; snprintf(portstr, sizeof(portstr), "%d", port);
	struct addrinfo *res = NULL;
	int rc = getaddrinfo(c->target, portstr, &hints, &res);
	if (rc != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
		return -1;
	}
	int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sock < 0) { perror("socket"); freeaddrinfo(res); return -1; }

	// set timeout
	struct timeval tv; tv.tv_sec = c->timeout_ms / 1000; tv.tv_usec = (c->timeout_ms % 1000) * 1000;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	// store connected address for sendto each time
	// We store ai in static buffer
	c->sockfd = sock;
	// We will reuse res on each send by resolving again inside snmp_get to keep code simple
	freeaddrinfo(res);
	return 0;
}

void snmp_client_close(snmp_client_t *c)
{
	if (c->sockfd >= 0) close(c->sockfd);
	c->sockfd = -1;
}

static int send_and_recv(snmp_client_t *c, const uint8_t *pkt, size_t pkt_len, uint8_t *resp, size_t resp_cap)
{
	struct addrinfo hints; memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	char portstr[16]; snprintf(portstr, sizeof(portstr), "%d", c->port);
	struct addrinfo *res = NULL;
	int rc = getaddrinfo(c->target, portstr, &hints, &res);
	if (rc != 0) { fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc)); return -1; }

	ssize_t s = sendto(c->sockfd, pkt, pkt_len, 0, res->ai_addr, res->ai_addrlen);
	if (s < 0) { perror("sendto"); freeaddrinfo(res); return -1; }

	ssize_t r = recvfrom(c->sockfd, resp, resp_cap, 0, NULL, NULL);
	if (r < 0) { freeaddrinfo(res); return -1; }
	freeaddrinfo(res);
	return (int)r;
}

int snmp_get(snmp_client_t *c, const uint32_t *oid, size_t oid_len, snmp_varbind_t *vb_out)
{
	uint8_t buf[1500];
	ber_buf_t b; ber_buf_init(&b, buf, sizeof(buf));
	// SNMPv2c Message = SEQUENCE { version INTEGER(1), community OCTET STRING, data GetRequest-PDU }

	// Outer sequence start
	size_t outer_len_pos; if (ber_start_sequence(&b, &outer_len_pos) != 0) return -1;
	// version = 1
	if (ber_encode_integer(&b, 1) != 0) return -1;
	// community
	if (ber_encode_octet_string(&b, (const uint8_t*)c->community, strlen(c->community)) != 0) return -1;
	// GetRequest-PDU [0] IMPLICIT
	uint8_t pdu_tag = 0xA0; // GetRequest
	uint8_t pdu_tmp[1024]; ber_buf_t pb; ber_buf_init(&pb, pdu_tmp, sizeof(pdu_tmp));
	size_t pdu_len_pos; if (ber_start_sequence(&pb, &pdu_len_pos) != 0) return -1;
	// request-id INTEGER, error-status INTEGER, error-index INTEGER
	int32_t req_id = (int32_t)(rand32() & 0x7FFFFFFF);
	if (ber_encode_integer(&pb, req_id) != 0) return -1;
	if (ber_encode_integer(&pb, 0) != 0) return -1;
	if (ber_encode_integer(&pb, 0) != 0) return -1;
	// varbind list
	uint8_t vbl_tmp[1024]; ber_buf_t vbl; ber_buf_init(&vbl, vbl_tmp, sizeof(vbl_tmp));
	size_t vbl_len_pos; if (ber_start_sequence(&vbl, &vbl_len_pos) != 0) return -1;
	// single varbind: SEQUENCE { name OID, value NULL }
	uint8_t vb_tmp[512]; ber_buf_t vb; ber_buf_init(&vb, vb_tmp, sizeof(vb_tmp));
	size_t vb_len_pos; if (ber_start_sequence(&vb, &vb_len_pos) != 0) return -1;
	if (ber_encode_oid(&vb, oid, oid_len) != 0) return -1;
	uint8_t null_tag = 0x05; uint8_t null_len = 0x00;
	if (ber_buf_write(&vb, &null_tag, 1) != 1) return -1;
	if (ber_buf_write(&vb, &null_len, 1) != 1) return -1;
	if (ber_end_sequence(&vb, vb_len_pos) != 0) return -1;
	if (ber_encode_tlv(&vbl, 0x30, vb.data + (vb_len_pos - 1), vb.len - (vb_len_pos - 1)) != 0) return -1; // Wrap vb as SEQUENCE already has tag; reuse content
	if (ber_end_sequence(&vbl, vbl_len_pos) != 0) return -1;
	// append varbindlist to pdu (as SEQUENCE TLV)
	if (ber_encode_tlv(&pb, 0x30, vbl.data + (vbl_len_pos - 1), vbl.len - (vbl_len_pos - 1)) != 0) return -1;
	if (ber_end_sequence(&pb, pdu_len_pos) != 0) return -1;
	// Now write PDU with context tag 0xA0
	if (ber_encode_tlv(&b, pdu_tag, pb.data + (pdu_len_pos - 1), pb.len - (pdu_len_pos - 1)) != 0) return -1;
	if (ber_end_sequence(&b, outer_len_pos) != 0) return -1;

	if (c->verbose) fprintf(stderr, "SNMP GET send len=%zu\n", b.len);
	uint8_t resp[1500];
	int attempt = 0;
	for (; attempt <= c->retries; ++attempt) {
		int r = send_and_recv(c, b.data, b.len, resp, sizeof(resp));
		if (r < 0) {
			if (c->verbose) fprintf(stderr, "timeout or recv error, attempt %d/%d\n", attempt+1, c->retries+1);
			continue;
		}
		// parse response
		size_t off = 0; uint8_t tag; const uint8_t *val; size_t vlen;
		if (ber_decode_tlv(resp, (size_t)r, &off, &tag, &val, &vlen) != 0 || tag != 0x30) { fprintf(stderr, "invalid SNMP message\n"); return -1; }
		// version
		int64_t ver; if (ber_decode_integer(resp, (size_t)r, &off, &ver) != 0 || ver != 1) { fprintf(stderr, "SNMP version mismatch\n"); return -1; }
		// community
		const uint8_t *comm; size_t comm_len; if (ber_decode_octet_string(resp, (size_t)r, &off, &comm, &comm_len) != 0) { fprintf(stderr, "SNMP community missing\n"); return -1; }
		// PDU should be GetResponse 0xA2
		if (off >= (size_t)r || resp[off] != 0xA2) { fprintf(stderr, "unexpected PDU tag\n"); return -1; }
		uint8_t ptag; const uint8_t *pval; size_t plen; if (ber_decode_tlv(resp, (size_t)r, &off, &ptag, &pval, &plen) != 0) return -1;
		size_t poff = (size_t)(pval - resp);
		int64_t rid, errst, erridx; if (ber_decode_integer(resp, (size_t)r, &poff, &rid) != 0) return -1;
		if (rid != req_id) { if (c->verbose) fprintf(stderr, "request-id mismatch\n"); continue; }
		if (ber_decode_integer(resp, (size_t)r, &poff, &errst) != 0) return -1;
		if (ber_decode_integer(resp, (size_t)r, &poff, &erridx) != 0) return -1;
		// varbind list
		uint8_t ltag; const uint8_t *lval; size_t llen; if (ber_decode_tlv(resp, (size_t)r, &poff, &ltag, &lval, &llen) != 0 || ltag != 0x30) return -1;
		// first vb
		uint8_t vtag; const uint8_t *vval; size_t vlen2; size_t l_off = (size_t)(lval - resp);
		if (ber_decode_tlv(resp, (size_t)r, &l_off, &vtag, &vval, &vlen2) != 0 || vtag != 0x30) return -1;
		// name
		size_t vb_off = (size_t)(vval - resp);
		uint32_t roid[64]; size_t roid_len;
		if (ber_decode_oid(resp, (size_t)r, &vb_off, roid, &roid_len, 64) != 0) return -1;
		// value
		uint8_t vtype; const uint8_t *vv; size_t vvl; if (ber_decode_tlv(resp, (size_t)r, &vb_off, &vtype, &vv, &vvl) != 0) return -1;
		vb_out->oid_len = roid_len; for (size_t i = 0; i < roid_len; ++i) vb_out->oid[i] = roid[i];
		vb_out->type = vtype;
		vb_out->str_value = vv; vb_out->str_len = vvl; vb_out->int_value = 0;
		if (vtype == 0x02 || vtype == 0x43) { // INTEGER or TimeTicks
			size_t toff = (size_t)(vv - resp) - 2; // move to tag location? Instead decode using integer helper on a local window
			size_t off2 = (size_t)(vv - resp) - 2; (void)toff; (void)off2;
			size_t offx = (size_t)(vval - resp) + (vb_off - (size_t)(vval - resp)) - vvl - 2; (void)offx;
			int64_t ival = 0; // decode directly from value bytes
			if (vvl > 0 && vvl <= 8) {
				int64_t iv = (vv[0] & 0x80) ? -1 : 0;
				for (size_t k = 0; k < vvl; ++k) iv = (iv << 8) | vv[k];
				ival = iv;
			}
			vb_out->int_value = ival;
		}
		return 0;
	}
	return -1;
}

