#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <optional>

namespace asn1 {

enum class TagClass : uint8_t {
	Universal = 0,
	Application = 1,
	ContextSpecific = 2,
	Private = 3
};

struct Tag {
	TagClass cls;
	bool constructed;
	uint32_t number; // supports only low-tag-number form in encoder
};

struct Tlv {
	Tag tag;
	std::vector<uint8_t> value;
};

// Encoding utilities
std::vector<uint8_t> encodeTag(const Tag &tag);
std::vector<uint8_t> encodeLength(size_t length);
std::vector<uint8_t> encodeInteger(int64_t value);
std::vector<uint8_t> encodeOctetString(const std::vector<uint8_t> &bytes);
std::vector<uint8_t> encodeObjectIdentifier(const std::vector<uint32_t> &oid);
std::vector<uint8_t> encodeNull();
std::vector<uint8_t> encodeSequence(const std::vector<uint8_t> &content);

// Decoding helpers
struct Reader {
	const uint8_t *data;
	size_t size;
	size_t pos;
	Reader(const uint8_t *d, size_t s) : data(d), size(s), pos(0) {}
};

bool readTag(Reader &r, Tag &out);
bool readLength(Reader &r, size_t &length);
bool readValue(Reader &r, size_t length, std::vector<uint8_t> &out);

bool decodeInteger(const std::vector<uint8_t> &buf, int64_t &out);
bool decodeOctetString(const std::vector<uint8_t> &buf, std::vector<uint8_t> &out);
bool decodeObjectIdentifier(const std::vector<uint8_t> &buf, std::vector<uint32_t> &out);

std::string tagToString(const Tag &tag);

}

