#include <string.h>
#include <stdlib.h>
#include "ber.h"

void ber_init(BerBuf *b, uint8_t *external, size_t cap) {
	b->data = external;
	b->len = 0;
	b->cap = cap;
}

size_t ber_length_of_len(size_t len) {
	if (len < 128) return 1;
	if (len <= 0xFF) return 2;
	if (len <= 0xFFFF) return 3;
	if (len <= 0xFFFFFF) return 4;
	return 5;
}

uint8_t *ber_reserve(BerBuf *b, size_t n) {
	if (b->len + n > b->cap) return NULL;
	uint8_t *p = b->data + b->len;
	b->len += n;
	return p;
}

bool ber_put_tag(BerBuf *b, uint8_t tag) {
	uint8_t *p = ber_reserve(b, 1);
	if (!p) return false;
	*p = tag;
	return true;
}

bool ber_put_length(BerBuf *b, size_t len) {
	if (len < 128) {
		uint8_t *p = ber_reserve(b, 1);
		if (!p) return false;
		*p = (uint8_t)len;
		return true;
	}
	// long form
	uint8_t bytes[4];
	int n = 0;
	if (len <= 0xFF) { bytes[0] = (uint8_t)len; n = 1; }
	else if (len <= 0xFFFF) { bytes[0] = (uint8_t)(len >> 8); bytes[1] = (uint8_t)len; n = 2; }
	else if (len <= 0xFFFFFF) { bytes[0] = (uint8_t)(len >> 16); bytes[1] = (uint8_t)(len >> 8); bytes[2] = (uint8_t)len; n = 3; }
	else { bytes[0] = (uint8_t)(len >> 24); bytes[1] = (uint8_t)(len >> 16); bytes[2] = (uint8_t)(len >> 8); bytes[3] = (uint8_t)len; n = 4; }
	uint8_t *p = ber_reserve(b, 1 + (size_t)n);
	if (!p) return false;
	*p++ = (uint8_t)(0x80 | n);
	memcpy(p, bytes, (size_t)n);
	return true;
}

bool ber_put_tlv(BerBuf *b, uint8_t tag, const uint8_t *value, size_t vlen) {
	return ber_put_tag(b, tag) && ber_put_length(b, vlen) &&
		(value == NULL || (memcpy(ber_reserve(b, vlen), value, vlen), true));
}

bool ber_put_integer(BerBuf *b, int64_t val) {
	// Minimal two's complement encoding without unnecessary leading 0x00/0xFF
	uint8_t tmp[9];
	int n = 0;
	int64_t v = val;
	uint8_t sign = (v < 0) ? 0xFF : 0x00;
	do {
		tmp[8 - n++] = (uint8_t)(v & 0xFF);
		v >>= 8;
	} while (v != 0 && v != -1 && n < 8);
	// Ensure the highest bit indicates correct sign
	uint8_t *bytes = &tmp[9 - n];
	if ((bytes[0] & 0x80) != (sign & 0x80)) {
		bytes = &tmp[8 - n];
		tmp[8 - n] = sign;
		n++;
	}
	return ber_put_tag(b, BER_TAG_INTEGER) && ber_put_length(b, (size_t)n) &&
		(memcpy(ber_reserve(b, (size_t)n), bytes, (size_t)n), true);
}

bool ber_put_null(BerBuf *b) {
	return ber_put_tag(b, BER_TAG_NULL) && ber_put_length(b, 0);
}

bool ber_put_octet_string(BerBuf *b, const uint8_t *s, size_t n) {
	return ber_put_tag(b, BER_TAG_OCTET_STR) && ber_put_length(b, n) &&
		(memcpy(ber_reserve(b, n), s, n), true);
}

bool ber_put_oid(BerBuf *b, const uint32_t *arcs, size_t count) {
	if (count < 2) return false;
	uint8_t buf[128];
	size_t len = 0;
	buf[len++] = (uint8_t)(arcs[0] * 40 + arcs[1]);
	for (size_t i = 2; i < count; ++i) {
		uint32_t v = arcs[i];
		uint8_t tmp[5];
		int n = 0;
		do { tmp[n++] = (uint8_t)(v & 0x7F); v >>= 7; } while (v && n < 5);
		for (int j = n - 1; j >= 0; --j) {
			uint8_t byte = tmp[j];
			if (j != 0) byte |= 0x80;
			if (len >= sizeof(buf)) return false;
			buf[len++] = byte;
		}
	}
	return ber_put_tag(b, BER_TAG_OID) && ber_put_length(b, len) &&
		(memcpy(ber_reserve(b, len), buf, len), true);
}

bool ber_begin_sequence(BerBuf *b, uint8_t tag, size_t *len_pos) {
	if (!ber_put_tag(b, tag)) return false;
	*len_pos = b->len;
	// Reserve max length field (5 bytes), we'll compact in end
	uint8_t *p = ber_reserve(b, 5);
	return p != NULL;
}

bool ber_end_sequence(BerBuf *b, size_t len_pos) {
	// Determine content length after the reserved length field
	size_t content_start = len_pos + 5; // worst case
	if (content_start > b->len) return false;
	size_t content_len = b->len - content_start;
	// Compute actual length-of-length and move content if needed
	uint8_t tmp[5];
	BerBuf tb = { .data = tmp, .len = 0, .cap = sizeof(tmp) };
	if (!ber_put_length(&tb, content_len)) return false;
	size_t lol = tb.len;
	// Move content forward/backward to make room of lol instead of 5
	size_t from = content_start;
	size_t to = len_pos + lol;
	if (to != from) memmove(b->data + to, b->data + from, content_len);
	b->len = to + content_len;
	memcpy(b->data + len_pos, tmp, lol);
	return true;
}

static bool get_length(const uint8_t *buf, size_t buflen, size_t *offset, size_t *out_len) {
	if (*offset >= buflen) return false;
	uint8_t b = buf[*offset];
	(*offset)++;
	if ((b & 0x80) == 0) { *out_len = b; return true; }
	int n = b & 0x7F;
	if (n < 1 || n > 4) return false;
	if (*offset + (size_t)n > buflen) return false;
	size_t len = 0;
	for (int i = 0; i < n; ++i) len = (len << 8) | buf[(*offset)++];
	*out_len = len;
	return true;
}

bool ber_get_tlv(const uint8_t *buf, size_t buflen, size_t *offset,
			uint8_t *out_tag, const uint8_t **out_val, size_t *out_len) {
	if (*offset + 2 > buflen) return false;
	uint8_t tag = buf[(*offset)++];
	size_t len = 0;
	if (!get_length(buf, buflen, offset, &len)) return false;
	if (*offset + len > buflen) return false;
	*out_tag = tag;
	*out_val = &buf[*offset];
	*out_len = len;
	*offset += len;
	return true;
}

bool ber_get_integer(const uint8_t *buf, size_t buflen, size_t *offset, int64_t *out) {
	uint8_t tag; const uint8_t *val; size_t len;
	if (!ber_get_tlv(buf, buflen, offset, &tag, &val, &len)) return false;
	if (tag != BER_TAG_INTEGER) return false;
	if (len < 1 || len > 8) return false;
	int64_t v = (val[0] & 0x80) ? -1 : 0; // sign extend
	for (size_t i = 0; i < len; ++i) v = (v << 8) | val[i];
	*out = v;
	return true;
}

bool ber_get_octet_string(const uint8_t *buf, size_t buflen, size_t *offset,
			const uint8_t **out, size_t *out_len) {
	uint8_t tag; const uint8_t *val; size_t len;
	if (!ber_get_tlv(buf, buflen, offset, &tag, &val, &len)) return false;
	if (tag != BER_TAG_OCTET_STR) return false;
	*out = val; *out_len = len; return true;
}

bool ber_get_null(const uint8_t *buf, size_t buflen, size_t *offset) {
	uint8_t tag; const uint8_t *val; size_t len;
	if (!ber_get_tlv(buf, buflen, offset, &tag, &val, &len)) return false;
	return tag == BER_TAG_NULL && len == 0;
}

bool ber_get_oid(const uint8_t *buf, size_t buflen, size_t *offset, uint32_t *arcs, size_t *inout_count) {
	uint8_t tag; const uint8_t *val; size_t len;
	if (!ber_get_tlv(buf, buflen, offset, &tag, &val, &len)) return false;
	if (tag != BER_TAG_OID) return false;
	if (len < 1) return false;
	if (*inout_count < 2) return false;
	uint32_t a0 = val[0] / 40;
	uint32_t a1 = val[0] % 40;
	arcs[0] = a0; arcs[1] = a1;
	size_t idx = 2;
	uint32_t v = 0;
	for (size_t i = 1; i < len; ++i) {
		v = (v << 7) | (val[i] & 0x7F);
		if ((val[i] & 0x80) == 0) {
			if (idx >= *inout_count) return false;
			arcs[idx++] = v;
			v = 0;
		}
	}
	*inout_count = idx;
	return true;
}

