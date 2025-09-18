#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <optional>
#include <map>

struct SnmpValue {
	enum class Type { Integer, OctetString, Oid, Null, Unsupported };
	Type type{Type::Unsupported};
	int64_t intValue{0};
	std::vector<uint8_t> bytesValue;
	std::vector<uint32_t> oidValue;
};

struct SnmpVarBind {
	std::vector<uint32_t> oid;
	SnmpValue value;
};

struct SnmpPdu {
	int32_t requestId{0};
	int32_t errorStatus{0};
	int32_t errorIndex{0};
	std::vector<SnmpVarBind> varBinds;
};

struct SnmpConfig {
	std::string targetHost;
	uint16_t targetPort{161};
	std::string community{"public"};
	int timeoutMs{1000};
	int retries{2};
	bool verbose{false};
};

class SnmpClient {
public:
	explicit SnmpClient(const SnmpConfig &cfg);
	~SnmpClient();

	bool get(const std::vector<std::vector<uint32_t>> &oids, SnmpPdu &response);

private:
	SnmpConfig config;
	int sock{-1};
	bool ensureSocket();
	bool sendGetRequest(const std::vector<std::vector<uint32_t>> &oids, int32_t requestId);
	bool receiveResponse(int32_t requestId, SnmpPdu &pdu);
	std::vector<uint8_t> buildGetPdu(const std::vector<std::vector<uint32_t>> &oids, int32_t requestId);
	bool parseResponse(const std::vector<uint8_t> &buf, SnmpPdu &out);
};

