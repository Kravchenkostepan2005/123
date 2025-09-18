#include "../src/ber.hpp"
#include <iostream>
#include <vector>

int main() {
    std::vector<uint8_t> out;
    BER::encode_oid(out, "1.3.6.1.2.1.1.3.0");
    // Should start with tag 0x06
    if (out.empty() || out[0] != 0x06) { std::cerr << "Bad OID tag" << std::endl; return 1; }
    // Decode TLV
    size_t off = 0; uint8_t tag; std::vector<uint8_t> val;
    if (!BER::decode_tlv(out, off, tag, val)) { std::cerr << "Decode TLV failed" << std::endl; return 1; }
    if (tag != 0x06) { std::cerr << "Tag mismatch" << std::endl; return 1; }
    std::string oid;
    if (!BER::decode_oid(val, oid)) { std::cerr << "Decode OID failed" << std::endl; return 1; }
    if (oid != "1.3.6.1.2.1.1.3.0") { std::cerr << "OID roundtrip mismatch: " << oid << std::endl; return 1; }
    return 0;
}

