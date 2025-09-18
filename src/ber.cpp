#include "ber.hpp"

#include <algorithm>
#include <cstring>
#include <sstream>

namespace BER {

static void push_be(std::vector<uint8_t>& out, uint64_t v, size_t bytes) {
    for (size_t i = 0; i < bytes; ++i) {
        out.push_back((v >> (8 * (bytes - 1 - i))) & 0xFF);
    }
}

void encode_length(std::vector<uint8_t>& out, size_t len) {
    if (len < 128) {
        out.push_back(static_cast<uint8_t>(len));
    } else {
        std::vector<uint8_t> tmp;
        size_t x = len;
        while (x > 0) {
            tmp.push_back(x & 0xFF);
            x >>= 8;
        }
        out.push_back(0x80 | static_cast<uint8_t>(tmp.size()));
        for (auto it = tmp.rbegin(); it != tmp.rend(); ++it) out.push_back(*it);
    }
}

void encode_tlv(std::vector<uint8_t>& out, uint8_t tag, const std::vector<uint8_t>& value) {
    out.push_back(tag);
    encode_length(out, value.size());
    out.insert(out.end(), value.begin(), value.end());
}

void encode_integer(std::vector<uint8_t>& out, int64_t value) {
    std::vector<uint8_t> content;
    uint64_t u = static_cast<uint64_t>(value);
    bool negative = value < 0;
    if (value == 0) {
        content.push_back(0);
    } else {
        while (u != 0 && u != UINT64_C(0xFFFFFFFFFFFFFFFF)) {
            content.push_back(static_cast<uint8_t>(u & 0xFF));
            u >>= 8;
        }
        if (!content.empty()) {
            uint8_t msb = content.back();
            if ((!negative && (msb & 0x80)) || (negative && !(msb & 0x80))) {
                content.push_back(negative ? 0xFF : 0x00);
            }
        }
        std::reverse(content.begin(), content.end());
    }
    std::vector<uint8_t> tlv;
    encode_tlv(tlv, 0x02, content);
    out.insert(out.end(), tlv.begin(), tlv.end());
}

void encode_octet_string(std::vector<uint8_t>& out, const std::vector<uint8_t>& bytes) {
    std::vector<uint8_t> tlv;
    encode_tlv(tlv, 0x04, bytes);
    out.insert(out.end(), tlv.begin(), tlv.end());
}

void encode_null(std::vector<uint8_t>& out) {
    out.push_back(0x05);
    out.push_back(0x00);
}

void encode_oid(std::vector<uint8_t>& out, const std::string& oid) {
    std::vector<uint8_t> content;
    std::stringstream ss(oid);
    std::string item;
    std::vector<uint32_t> parts;
    while (std::getline(ss, item, '.')) {
        if (item.empty()) continue;
        parts.push_back(static_cast<uint32_t>(std::stoul(item)));
    }
    if (parts.size() < 2) {
        // invalid, but attempt minimal
        parts.insert(parts.begin(), 0);
        parts.insert(parts.begin() + 1, 0);
    }
    uint32_t first = parts[0];
    uint32_t second = parts[1];
    uint32_t firstByte = first * 40 + second;
    content.push_back(static_cast<uint8_t>(firstByte));
    for (size_t i = 2; i < parts.size(); ++i) {
        uint32_t v = parts[i];
        std::vector<uint8_t> tmp;
        do {
            tmp.push_back(static_cast<uint8_t>(v & 0x7F));
            v >>= 7;
        } while (v > 0);
        for (auto it = tmp.rbegin(); it != tmp.rend(); ++it) {
            uint8_t b = *it;
            if (it != tmp.rbegin()) b |= 0x80;
            content.push_back(b);
        }
    }
    std::vector<uint8_t> tlv;
    encode_tlv(tlv, 0x06, content);
    out.insert(out.end(), tlv.begin(), tlv.end());
}

void encode_sequence(std::vector<uint8_t>& out, const std::vector<uint8_t>& content) {
    out.push_back(0x30);
    encode_length(out, content.size());
    out.insert(out.end(), content.begin(), content.end());
}

bool decode_tlv(const std::vector<uint8_t>& buf, size_t& offset, uint8_t& tag, std::vector<uint8_t>& value) {
    if (offset + 2 > buf.size()) return false;
    tag = buf[offset++];
    uint8_t lenByte = buf[offset++];
    size_t len = 0;
    if ((lenByte & 0x80) == 0) {
        len = lenByte;
    } else {
        size_t count = lenByte & 0x7F;
        if (offset + count > buf.size()) return false;
        for (size_t i = 0; i < count; ++i) {
            len = (len << 8) | buf[offset++];
        }
    }
    if (offset + len > buf.size()) return false;
    value.assign(buf.begin() + offset, buf.begin() + offset + len);
    offset += len;
    return true;
}

bool decode_integer(const std::vector<uint8_t>& bytes, int64_t& value) {
    if (bytes.empty()) return false;
    // Sign-extend manually
    uint64_t v = 0;
    for (size_t i = 0; i < bytes.size(); ++i) {
        v = (v << 8) | bytes[i];
    }
    // If sign bit set, extend to 64 bits
    if ((bytes[0] & 0x80) != 0) {
        size_t bits = bytes.size() * 8;
        if (bits < 64) {
            uint64_t mask = (~UINT64_C(0)) << bits;
            v |= mask;
        }
    }
    value = static_cast<int64_t>(v);
    return true;
}

bool decode_unsigned(const std::vector<uint8_t>& bytes, uint64_t& value) {
    if (bytes.empty()) return false;
    uint64_t v = 0;
    for (size_t i = 0; i < bytes.size(); ++i) {
        v = (v << 8) | bytes[i];
    }
    value = v;
    return true;
}

bool decode_oid(const std::vector<uint8_t>& bytes, std::string& oid) {
    if (bytes.empty()) return false;
    std::vector<uint32_t> parts;
    uint8_t first = bytes[0];
    parts.push_back(first / 40);
    parts.push_back(first % 40);
    size_t i = 1;
    while (i < bytes.size()) {
        uint32_t v = 0;
        while (i < bytes.size()) {
            uint8_t b = bytes[i++];
            v = (v << 7) | (b & 0x7F);
            if ((b & 0x80) == 0) break;
        }
        parts.push_back(v);
    }
    std::ostringstream os;
    for (size_t j = 0; j < parts.size(); ++j) {
        if (j) os << ".";
        os << parts[j];
    }
    oid = os.str();
    return true;
}

}

