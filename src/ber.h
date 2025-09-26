#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Minimal BER encoder/decoder for SNMPv2c needs

typedef struct {
	uint8_t *data;
	size_t len;
	size_t cap;
} BerBuf;

void ber_init(BerBuf *b, uint8_t *external, size_t cap);
size_t ber_length_of_len(size_t len);
uint8_t *ber_reserve(BerBuf *b, size_t n);
bool ber_put_tlv(BerBuf *b, uint8_t tag, const uint8_t *value, size_t vlen);
bool ber_put_length(BerBuf *b, size_t len);
bool ber_put_tag(BerBuf *b, uint8_t tag);
bool ber_put_integer(BerBuf *b, int64_t val);
bool ber_put_null(BerBuf *b);
bool ber_put_octet_string(BerBuf *b, const uint8_t *s, size_t n);
bool ber_put_oid(BerBuf *b, const uint32_t *arcs, size_t count);
bool ber_begin_sequence(BerBuf *b, uint8_t tag, size_t *len_pos);
bool ber_end_sequence(BerBuf *b, size_t len_pos);

// Decoding helpers
bool ber_get_tlv(const uint8_t *buf, size_t buflen, size_t *offset,
			uint8_t *out_tag, const uint8_t **out_val, size_t *out_len);
bool ber_get_integer(const uint8_t *buf, size_t buflen, size_t *offset, int64_t *out);
bool ber_get_octet_string(const uint8_t *buf, size_t buflen, size_t *offset,
			const uint8_t **out, size_t *out_len);
bool ber_get_null(const uint8_t *buf, size_t buflen, size_t *offset);
bool ber_get_oid(const uint8_t *buf, size_t buflen, size_t *offset, uint32_t *arcs, size_t *inout_count);

// Tags
#define BER_TAG_INTEGER     0x02
#define BER_TAG_OCTET_STR   0x04
#define BER_TAG_NULL        0x05
#define BER_TAG_OID         0x06
#define BER_TAG_SEQUENCE    0x30

// SNMP specific
#define BER_TAG_IPADDRESS   0x40
#define BER_TAG_COUNTER32   0x41
#define BER_TAG_GAUGE32     0x42
#define BER_TAG_TIMETICKS   0x43
#define BER_TAG_OPAQUE      0x44
#define BER_TAG_COUNTER64   0x46

