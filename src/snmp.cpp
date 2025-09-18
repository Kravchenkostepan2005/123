#include "snmp.hpp"
#include "log.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <random>

using namespace std::chrono;

SnmpClient::SnmpClient(const std::string& targetHost,
                       int targetPort,
                       const std::string& community,
                       int timeoutMs,
                       int retries)
    : host_(targetHost), port_(targetPort), community_(community), timeoutMs_(timeoutMs), retries_(retries) {}

std::vector<uint8_t> SnmpClient::build_get_request(int32_t requestId, const std::string& oid) {
    // Build VarBind: SEQUENCE { name OID, value NULL }
    std::vector<uint8_t> varbind_content;
    BER::encode_oid(varbind_content, oid);
    BER::encode_null(varbind_content);
    std::vector<uint8_t> varbind;
    BER::encode_sequence(varbind, varbind_content);

    // VarBindList: SEQUENCE OF varbind
    std::vector<uint8_t> varbind_list;
    BER::encode_sequence(varbind_list, varbind);

    // PDU: GetRequest-PDU [0] (0xA0)
    std::vector<uint8_t> pdu_content;
    BER::encode_integer(pdu_content, requestId);
    BER::encode_integer(pdu_content, 0); // error-status
    BER::encode_integer(pdu_content, 0); // error-index
    pdu_content.insert(pdu_content.end(), varbind_list.begin(), varbind_list.end());

    std::vector<uint8_t> pdu;
    // Context-specific constructed tag 0 (0xA0)
    BER::encode_tlv(pdu, 0xA0, pdu_content);

    // Message: SEQUENCE { version INTEGER(1), community OCTET STRING, data PDU }
    std::vector<uint8_t> msg_content;
    BER::encode_integer(msg_content, 1); // SNMP v2c
    BER::encode_octet_string(msg_content, std::vector<uint8_t>(community_.begin(), community_.end()));
    msg_content.insert(msg_content.end(), pdu.begin(), pdu.end());

    std::vector<uint8_t> message;
    BER::encode_sequence(message, msg_content);
    return message;
}

bool SnmpClient::send_and_recv(const std::vector<uint8_t>& req, std::vector<uint8_t>& resp) {
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    addrinfo* res = nullptr;
    std::string portStr = std::to_string(port_);
    if (getaddrinfo(host_.c_str(), portStr.c_str(), &hints, &res) != 0) {
        return false;
    }

    int sock = -1;
    for (addrinfo* p = res; p != nullptr; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock < 0) continue;
        // set timeouts
        timeval tv{};
        tv.tv_sec = timeoutMs_ / 1000;
        tv.tv_usec = (timeoutMs_ % 1000) * 1000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        ssize_t sent = sendto(sock, req.data(), req.size(), 0, p->ai_addr, p->ai_addrlen);
        if (sent < 0) {
            close(sock);
            sock = -1;
            continue;
        }

        uint8_t buf[2048];
        ssize_t recvd = recvfrom(sock, buf, sizeof(buf), 0, nullptr, nullptr);
        if (recvd > 0) {
            resp.assign(buf, buf + recvd);
            close(sock);
            freeaddrinfo(res);
            return true;
        }
        close(sock);
        sock = -1;
    }
    if (res) freeaddrinfo(res);
    return false;
}

Result<SnmpValue> SnmpClient::parse_get_response(const std::vector<uint8_t>& buf, int32_t requestId) {
    size_t off = 0;
    uint8_t tag;
    std::vector<uint8_t> val;
    if (!BER::decode_tlv(buf, off, tag, val) || tag != 0x30) return Result<SnmpValue>::failure("Invalid top-level sequence");
    size_t off2 = 0;
    uint8_t t2;
    std::vector<uint8_t> v2;
    // version
    if (!BER::decode_tlv(val, off2, t2, v2) || t2 != 0x02) return Result<SnmpValue>::failure("Missing version");
    int64_t ver;
    if (!BER::decode_integer(v2, ver) || ver != 1) return Result<SnmpValue>::failure("Not SNMPv2c");
    // community
    if (!BER::decode_tlv(val, off2, t2, v2) || t2 != 0x04) return Result<SnmpValue>::failure("Missing community");
    // PDU
    if (!BER::decode_tlv(val, off2, t2, v2) || t2 != 0xA2) return Result<SnmpValue>::failure("Not a GetResponse-PDU");
    size_t off3 = 0;
    uint8_t t3; std::vector<uint8_t> v3;
    // request-id
    if (!BER::decode_tlv(v2, off3, t3, v3) || t3 != 0x02) return Result<SnmpValue>::failure("Missing request-id");
    int64_t rid;
    if (!BER::decode_integer(v3, rid) || static_cast<int32_t>(rid) != requestId) return Result<SnmpValue>::failure("request-id mismatch");
    // error-status
    if (!BER::decode_tlv(v2, off3, t3, v3) || t3 != 0x02) return Result<SnmpValue>::failure("Missing error-status");
    int64_t errStatus = 0;
    BER::decode_integer(v3, errStatus);
    // error-index
    if (!BER::decode_tlv(v2, off3, t3, v3) || t3 != 0x02) return Result<SnmpValue>::failure("Missing error-index");
    // varbind list
    if (!BER::decode_tlv(v2, off3, t3, v3) || t3 != 0x30) return Result<SnmpValue>::failure("Missing varbind list");
    size_t off4 = 0;
    // first varbind only
    uint8_t t4; std::vector<uint8_t> v4;
    if (!BER::decode_tlv(v3, off4, t4, v4) || t4 != 0x30) return Result<SnmpValue>::failure("Missing varbind");
    size_t off5 = 0; uint8_t t5; std::vector<uint8_t> v5;
    if (!BER::decode_tlv(v4, off5, t5, v5) || t5 != 0x06) return Result<SnmpValue>::failure("Missing OID");
    std::string oid;
    if (!BER::decode_oid(v5, oid)) return Result<SnmpValue>::failure("Invalid OID");
    if (!BER::decode_tlv(v4, off5, t5, v5)) return Result<SnmpValue>::failure("Missing value");

    SnmpValue sv;
    sv.oid = oid;
    switch (t5) {
        case 0x02: { // INTEGER
            int64_t ival = 0;
            if (!BER::decode_integer(v5, ival)) return Result<SnmpValue>::failure("Bad INTEGER");
            sv.type = SnmpValue::Type::Integer;
            sv.intValue = ival;
            sv.uintValue = static_cast<uint64_t>(ival);
            break;
        }
        case 0x04: { // OCTET STRING
            sv.type = SnmpValue::Type::OctetString;
            sv.stringValue = v5;
            break;
        }
        case 0x06: { // OID
            sv.type = SnmpValue::Type::Oid;
            // store OID bytes in stringValue for completeness
            sv.stringValue = v5;
            break;
        }
        case 0x05: { // NULL
            sv.type = SnmpValue::Type::Null;
            break;
        }
        case 0x41: { // Counter32 (application 1)
            uint64_t u = 0; if (!BER::decode_unsigned(v5, u)) return Result<SnmpValue>::failure("Bad Counter32");
            sv.type = SnmpValue::Type::Counter32; sv.uintValue = u; break;
        }
        case 0x42: { // Gauge32 (application 2)
            uint64_t u = 0; if (!BER::decode_unsigned(v5, u)) return Result<SnmpValue>::failure("Bad Gauge32");
            sv.type = SnmpValue::Type::Gauge32; sv.uintValue = u; break;
        }
        case 0x43: { // TimeTicks (application 3)
            uint64_t u = 0; if (!BER::decode_unsigned(v5, u)) return Result<SnmpValue>::failure("Bad TimeTicks");
            sv.type = SnmpValue::Type::TimeTicks; sv.uintValue = u; break;
        }
        case 0x46: { // Counter64 (application 6)
            uint64_t u = 0; if (!BER::decode_unsigned(v5, u)) return Result<SnmpValue>::failure("Bad Counter64");
            sv.type = SnmpValue::Type::Counter64; sv.uintValue = u; break;
        }
        default:
            sv.type = SnmpValue::Type::Unknown;
            break;
    }

    if (errStatus != 0) {
        return Result<SnmpValue>::failure("SNMP error-status: " + std::to_string(errStatus));
    }
    return Result<SnmpValue>::success(sv);
}

Result<SnmpValue> SnmpClient::get(const std::string& oid) {
    static std::mt19937 rng(static_cast<unsigned>(steady_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<int32_t> dist(1, 0x7FFFFFFF);
    int32_t reqId = dist(rng);
    std::vector<uint8_t> req = build_get_request(reqId, oid);

    for (int attempt = 0; attempt <= retries_; ++attempt) {
        std::vector<uint8_t> resp;
        if (send_and_recv(req, resp)) {
            auto parsed = parse_get_response(resp, reqId);
            if (parsed.ok) return parsed;
            return Result<SnmpValue>::failure(parsed.error);
        }
    }
    return Result<SnmpValue>::failure("Timeout");
}

