#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace BER {

enum class Class : uint8_t { Universal = 0x00, Application = 0x40, Context = 0x80, Private = 0xC0 };

struct Tlv {
    uint8_t tag;
    std::vector<uint8_t> value;
};

// Encoding helpers
void encode_length(std::vector<uint8_t>& out, size_t len);
void encode_tlv(std::vector<uint8_t>& out, uint8_t tag, const std::vector<uint8_t>& value);
void encode_integer(std::vector<uint8_t>& out, int64_t value);
void encode_octet_string(std::vector<uint8_t>& out, const std::vector<uint8_t>& bytes);
void encode_null(std::vector<uint8_t>& out);
void encode_oid(std::vector<uint8_t>& out, const std::string& oid);
void encode_sequence(std::vector<uint8_t>& out, const std::vector<uint8_t>& content);

// Decoding helpers
bool decode_tlv(const std::vector<uint8_t>& buf, size_t& offset, uint8_t& tag, std::vector<uint8_t>& value);
bool decode_integer(const std::vector<uint8_t>& bytes, int64_t& value);
bool decode_unsigned(const std::vector<uint8_t>& bytes, uint64_t& value);
bool decode_oid(const std::vector<uint8_t>& bytes, std::string& oid);

}

