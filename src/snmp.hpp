#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "ber.hpp"

struct SnmpValue {
    std::string oid;
    enum class Type {
        Integer,
        OctetString,
        Oid,
        Gauge32,
        TimeTicks,
        Counter32,
        Counter64,
        Null,
        Unknown
    } type = Type::Unknown;
    int64_t intValue = 0;
    uint64_t uintValue = 0;
    std::vector<uint8_t> stringValue; // raw bytes for OctetString
};

template <typename T>
struct Result {
    bool ok;
    T value;
    std::string error;
    static Result<T> success(const T& v) { return {true, v, ""}; }
    static Result<T> failure(const std::string& e) { return {false, T{}, e}; }
};

class SnmpClient {
public:
    SnmpClient(const std::string& targetHost,
               int targetPort,
               const std::string& community,
               int timeoutMs,
               int retries);

    Result<SnmpValue> get(const std::string& oid);

private:
    std::string host_;
    int port_;
    std::string community_;
    int timeoutMs_;
    int retries_;

    bool send_and_recv(const std::vector<uint8_t>& req,
                       std::vector<uint8_t>& resp);
    std::vector<uint8_t> build_get_request(int32_t requestId, const std::string& oid);
    Result<SnmpValue> parse_get_response(const std::vector<uint8_t>& buf, int32_t requestId);
};

