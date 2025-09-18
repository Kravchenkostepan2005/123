#include "asn1.h"

#include <stdexcept>
#include <cstring>

namespace asn1 {

static uint8_t makeTagByte(TagClass cls, bool constructed, uint8_t number) {
	uint8_t b = (static_cast<uint8_t>(cls) & 0x03) << 6;
	if (constructed) b |= 0x20;
	b |= (number & 0x1F);
	return b;
}

std::vector<uint8_t> encodeTag(const Tag &tag) {
	if (tag.number >= 31) {
		throw std::runtime_error("High-tag-number form not supported in encoder");
	}
	return { makeTagByte(tag.cls, tag.constructed, static_cast<uint8_t>(tag.number)) };
}

std::vector<uint8_t> encodeLength(size_t length) {
	if (length < 128) {
		return { static_cast<uint8_t>(length) };
	}
	std::vector<uint8_t> bytes;
	size_t tmp = length;
	while (tmp > 0) {
		bytes.insert(bytes.begin(), static_cast<uint8_t>(tmp & 0xFF));
		tmp >>= 8;
	}
	std::vector<uint8_t> out;
	out.push_back(0x80 | static_cast<uint8_t>(bytes.size()));
	out.insert(out.end(), bytes.begin(), bytes.end());
	return out;
}

std::vector<uint8_t> encodeInteger(int64_t value) {
	std::vector<uint8_t> bytes;
	bool negative = value < 0;
	uint64_t u = static_cast<uint64_t>(value);
	if (negative) {
		// two's complement bytes; serialize until sign bit preserved
		// We'll build from least significant byte
		while (true) {
			uint8_t b = static_cast<uint8_t>(u & 0xFF);
			bytes.insert(bytes.begin(), b);
			u >>= 8;
			bool signBit = (b & 0x80) != 0;
			if (u == UINT64_C(0xFFFFFFFFFFFFFFFF) || (u == UINT64_C(0xFFFFFFFFFFFFFFFF) && signBit)) break;
			if (u == 0xFFFFFFFFFFFFFFFFULL) {
				if ((bytes[0] & 0x80) == 0) break; // sign preserved
			}
			if (u == 0xFFFFFFFFFFFFFFFFULL) break;
			if ((u >> 8) == 0xFFFFFFFFFFFFFFFFULL && (bytes[0] & 0x80)) break;
			if ((u >> 8) == 0xFFFFFFFFFFFFFFFFULL) break;
			if (u == 0xFFFFFFFFFFFFFFFFULL) break;
			if ((bytes.size() > 8)) break; // guard
		}
		// Trim redundant 0xFF prefixes
		while (bytes.size() > 1 && bytes[0] == 0xFF && (bytes[1] & 0x80)) {
			bytes.erase(bytes.begin());
		}
	} else {
		uint64_t t = u;
		do {
			bytes.insert(bytes.begin(), static_cast<uint8_t>(t & 0xFF));
			t >>= 8;
		} while (t != 0);
		// Ensure positive sign
		if (bytes.size() > 0 && (bytes[0] & 0x80)) {
			bytes.insert(bytes.begin(), 0x00);
		}
	}

	std::vector<uint8_t> out;
	// Tag INTEGER (0x02)
	out.push_back(makeTagByte(TagClass::Universal, false, 2));
	auto len = encodeLength(bytes.size());
	out.insert(out.end(), len.begin(), len.end());
	out.insert(out.end(), bytes.begin(), bytes.end());
	return out;
}

std::vector<uint8_t> encodeOctetString(const std::vector<uint8_t> &bytes) {
	std::vector<uint8_t> out;
	out.push_back(makeTagByte(TagClass::Universal, false, 4));
	auto len = encodeLength(bytes.size());
	out.insert(out.end(), len.begin(), len.end());
	out.insert(out.end(), bytes.begin(), bytes.end());
	return out;
}

std::vector<uint8_t> encodeObjectIdentifier(const std::vector<uint32_t> &oid) {
	if (oid.size() < 2) throw std::runtime_error("OID needs at least 2 arcs");
	std::vector<uint8_t> body;
	uint32_t first = oid[0];
	uint32_t second = oid[1];
	if (first > 2 || (first < 2 && second >= 40)) throw std::runtime_error("Invalid first two OID arcs");
	body.push_back(static_cast<uint8_t>(first * 40 + second));
	for (size_t i = 2; i < oid.size(); ++i) {
		uint32_t arc = oid[i];
		std::vector<uint8_t> tmp;
		do {
			tmp.insert(tmp.begin(), static_cast<uint8_t>(arc & 0x7F));
			arc >>= 7;
		} while (arc > 0);
		for (size_t j = 0; j < tmp.size(); ++j) {
			uint8_t b = tmp[j];
			if (j + 1 < tmp.size()) b |= 0x80;
			body.push_back(b);
		}
	}
	std::vector<uint8_t> out;
	out.push_back(makeTagByte(TagClass::Universal, false, 6));
	auto len = encodeLength(body.size());
	out.insert(out.end(), len.begin(), len.end());
	out.insert(out.end(), body.begin(), body.end());
	return out;
}

std::vector<uint8_t> encodeNull() {
	return { makeTagByte(TagClass::Universal, false, 5), 0x00 };
}

std::vector<uint8_t> encodeSequence(const std::vector<uint8_t> &content) {
	std::vector<uint8_t> out;
	out.push_back(makeTagByte(TagClass::Universal, true, 16));
	auto len = encodeLength(content.size());
	out.insert(out.end(), len.begin(), len.end());
	out.insert(out.end(), content.begin(), content.end());
	return out;
}

bool readTag(Reader &r, Tag &out) {
	if (r.pos >= r.size) return false;
	uint8_t b = r.data[r.pos++];
	out.cls = static_cast<TagClass>((b >> 6) & 0x03);
	out.constructed = (b & 0x20) != 0;
	uint8_t num = b & 0x1F;
	if (num == 0x1F) return false; // high-tag-number unsupported
	out.number = num;
	return true;
}

bool readLength(Reader &r, size_t &length) {
	if (r.pos >= r.size) return false;
	uint8_t b = r.data[r.pos++];
	if ((b & 0x80) == 0) {
		length = b;
		return true;
	}
	uint8_t numBytes = b & 0x7F;
	if (numBytes == 0 || numBytes > 8) return false;
	if (r.pos + numBytes > r.size) return false;
	size_t l = 0;
	for (uint8_t i = 0; i < numBytes; ++i) {
		l = (l << 8) | r.data[r.pos++];
	}
	length = l;
	return true;
}

bool readValue(Reader &r, size_t length, std::vector<uint8_t> &out) {
	if (r.pos + length > r.size) return false;
	out.assign(r.data + r.pos, r.data + r.pos + length);
	r.pos += length;
	return true;
}

bool decodeInteger(const std::vector<uint8_t> &buf, int64_t &out) {
	if (buf.empty()) return false;
	// two's complement
	bool negative = (buf[0] & 0x80) != 0;
	int64_t val = negative ? -1 : 0;
	for (uint8_t b : buf) {
		val = (val << 8) | b;
	}
	out = val;
	return true;
}

bool decodeOctetString(const std::vector<uint8_t> &buf, std::vector<uint8_t> &out) {
	out = buf;
	return true;
}

bool decodeObjectIdentifier(const std::vector<uint8_t> &buf, std::vector<uint32_t> &out) {
	if (buf.empty()) return false;
	out.clear();
	uint8_t first = buf[0];
	uint32_t x = first / 40;
	uint32_t y = first % 40;
	out.push_back(x);
	out.push_back(y);
	uint32_t acc = 0;
	for (size_t i = 1; i < buf.size(); ++i) {
		uint8_t b = buf[i];
		acc = (acc << 7) | (b & 0x7F);
		if ((b & 0x80) == 0) {
			out.push_back(acc);
			acc = 0;
		}
	}
	if ((buf.back() & 0x80) != 0) return false; // truncated base-128
	return true;
}

std::string tagToString(const Tag &tag) {
	std::string s;
	s += (tag.cls == TagClass::Universal ? "U" : tag.cls == TagClass::Application ? "A" : tag.cls == TagClass::ContextSpecific ? "C" : "P");
	s += tag.constructed ? "C" : "P";
	s += ":" + std::to_string(tag.number);
	return s;
}

}

