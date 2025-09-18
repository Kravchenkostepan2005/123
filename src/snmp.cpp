#include "snmp.h"
#include "asn1.h"
#include "oid.h"
#include "util.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <random>

SnmpClient::SnmpClient(const SnmpConfig &cfg) : config(cfg) {}

SnmpClient::~SnmpClient() {
	if (sock >= 0) close(sock);
}

bool SnmpClient::ensureSocket() {
	if (sock >= 0) return true;
	sock = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) return false;
	struct timeval tv;
	tv.tv_sec = config.timeoutMs / 1000;
	tv.tv_usec = (config.timeoutMs % 1000) * 1000;
	::setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	return true;
}

bool SnmpClient::sendGetRequest(const std::vector<std::vector<uint32_t>> &oids, int32_t requestId) {
	std::vector<uint8_t> pdu = buildGetPdu(oids, requestId);

	// Resolve target
	struct addrinfo hints{};
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4 only to keep it simple per environment
	hints.ai_socktype = SOCK_DGRAM;
	struct addrinfo *res = nullptr;
	int rc = getaddrinfo(config.targetHost.c_str(), nullptr, &hints, &res);
	if (rc != 0 || !res) return false;
	struct sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(config.targetPort);
	addr.sin_addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
	freeaddrinfo(res);

	ssize_t sent = sendto(sock, pdu.data(), pdu.size(), 0, (struct sockaddr*)&addr, sizeof(addr));
	return sent == (ssize_t)pdu.size();
}

bool SnmpClient::receiveResponse(int32_t requestId, SnmpPdu &pdu) {
	uint8_t buf[1500];
	ssize_t n = recv(sock, buf, sizeof(buf), 0);
	if (n <= 0) return false;
	std::vector<uint8_t> v(buf, buf + n);
	return parseResponse(v, pdu) && pdu.requestId == requestId;
}

bool SnmpClient::get(const std::vector<std::vector<uint32_t>> &oids, SnmpPdu &response) {
	if (!ensureSocket()) return false;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int32_t> dist(1, 0x7FFFFFFF);
	int32_t reqId = dist(gen);

	for (int attempt = 0; attempt <= config.retries; ++attempt) {
		if (!sendGetRequest(oids, reqId)) return false;
		if (config.verbose) util::log("SNMP sent requestId=" + std::to_string(reqId));
		if (receiveResponse(reqId, response)) {
			if (config.verbose) util::log("SNMP got response varbinds=" + std::to_string(response.varBinds.size()));
			return true;
		}
		if (config.verbose) util::log("SNMP timeout, retry " + std::to_string(attempt+1));
	}
	return false;
}

std::vector<uint8_t> SnmpClient::buildGetPdu(const std::vector<std::vector<uint32_t>> &oids, int32_t requestId) {
	using namespace asn1;
	std::vector<uint8_t> varbindsContent;
	for (const auto &oid : oids) {
		std::vector<uint8_t> vbContent;
		// name
		auto oidEncoded = encodeObjectIdentifier(oid);
		vbContent.insert(vbContent.end(), oidEncoded.begin(), oidEncoded.end());
		// value: NULL
		auto nullEncoded = encodeNull();
		vbContent.insert(vbContent.end(), nullEncoded.begin(), nullEncoded.end());
		auto vbSeq = encodeSequence(vbContent);
		varbindsContent.insert(varbindsContent.end(), vbSeq.begin(), vbSeq.end());
	}
	auto varbindList = encodeSequence(varbindsContent);

	std::vector<uint8_t> pduContent;
	// request-id
	{
		auto rid = encodeInteger(requestId);
		pduContent.insert(pduContent.end(), rid.begin(), rid.end());
	}
	// error-status, error-index
	{
		auto zero = encodeInteger(0);
		pduContent.insert(pduContent.end(), zero.begin(), zero.end());
		pduContent.insert(pduContent.end(), zero.begin(), zero.end());
	}
	// varbind list
	pduContent.insert(pduContent.end(), varbindList.begin(), varbindList.end());

	// context-specific, constructed, tag number 0 for GetRequest-PDU
	asn1::Tag pduTag{ asn1::TagClass::ContextSpecific, true, 0 };
	auto tagBytes = encodeTag(pduTag);
	auto lenBytes = encodeLength(pduContent.size());
	std::vector<uint8_t> pdu;
	pdu.insert(pdu.end(), tagBytes.begin(), tagBytes.end());
	pdu.insert(pdu.end(), lenBytes.begin(), lenBytes.end());
	pdu.insert(pdu.end(), pduContent.begin(), pduContent.end());

	// SNMP message = SEQUENCE { version INTEGER(1), community OCTET STRING, data PDUs }
	std::vector<uint8_t> msgContent;
	// version = 1 for v2c
	{
		auto ver = encodeInteger(1);
		msgContent.insert(msgContent.end(), ver.begin(), ver.end());
	}
	// community
	{
		std::vector<uint8_t> comm(config.community.begin(), config.community.end());
		auto commEnc = encodeOctetString(comm);
		msgContent.insert(msgContent.end(), commEnc.begin(), commEnc.end());
	}
	// data
	msgContent.insert(msgContent.end(), pdu.begin(), pdu.end());

	return encodeSequence(msgContent);
}

bool SnmpClient::parseResponse(const std::vector<uint8_t> &buf, SnmpPdu &out) {
	using namespace asn1;
	Reader r(buf.data(), buf.size());
	Tag tag;
	if (!readTag(r, tag)) return false;
	if (!(tag.cls == TagClass::Universal && tag.constructed && tag.number == 16)) return false; // message seq
	size_t msgLen; if (!readLength(r, msgLen)) return false; size_t msgEnd = r.pos + msgLen; if (msgEnd > r.size) return false;
	// version
	{
		Tag t; if (!readTag(r, t)) return false; if (!(t.cls==TagClass::Universal && !t.constructed && t.number==2)) return false;
		size_t l; if (!readLength(r,l)) return false; std::vector<uint8_t> v; if (!readValue(r,l,v)) return false; int64_t ver; if (!decodeInteger(v, ver)) return false; if (ver != 1) return false;
	}
	// community
	{
		Tag t; if (!readTag(r, t)) return false; if (!(t.cls==TagClass::Universal && !t.constructed && t.number==4)) return false;
		size_t l; if (!readLength(r,l)) return false; std::vector<uint8_t> v; if (!readValue(r,l,v)) return false; // ignore community
	}
	// PDU tag: GetResponse is context-specific constructed tag 2
	{
		Tag t; if (!readTag(r,t)) return false; if (!(t.cls==TagClass::ContextSpecific && t.constructed && t.number==2)) return false;
		size_t l; if (!readLength(r,l)) return false; size_t end = r.pos + l; if (end > r.size) return false;
		// request-id
		{
			Tag it; if (!readTag(r,it)) return false; if (!(it.cls==TagClass::Universal && !it.constructed && it.number==2)) return false;
			size_t il; if (!readLength(r,il)) return false; std::vector<uint8_t> iv; if (!readValue(r,il,iv)) return false; int64_t rid; if (!decodeInteger(iv, rid)) return false; out.requestId = static_cast<int32_t>(rid);
		}
		// error-status
		{
			Tag it; if (!readTag(r,it)) return false; if (!(it.cls==TagClass::Universal && !it.constructed && it.number==2)) return false;
			size_t il; if (!readLength(r,il)) return false; std::vector<uint8_t> iv; if (!readValue(r,il,iv)) return false; int64_t v; if (!decodeInteger(iv, v)) return false; out.errorStatus = static_cast<int32_t>(v);
		}
		// error-index
		{
			Tag it; if (!readTag(r,it)) return false; if (!(it.cls==TagClass::Universal && !it.constructed && it.number==2)) return false;
			size_t il; if (!readLength(r,il)) return false; std::vector<uint8_t> iv; if (!readValue(r,il,iv)) return false; int64_t v; if (!decodeInteger(iv, v)) return false; out.errorIndex = static_cast<int32_t>(v);
		}
		// varbind list
		Tag vlt; if (!readTag(r, vlt)) return false; if (!(vlt.cls==TagClass::Universal && vlt.constructed && vlt.number==16)) return false; size_t vll; if (!readLength(r, vll)) return false; size_t vlEnd = r.pos + vll; if (vlEnd > r.size) return false;
		out.varBinds.clear();
		while (r.pos < vlEnd) {
			Tag vbt; if (!readTag(r, vbt)) return false; if (!(vbt.cls==TagClass::Universal && vbt.constructed && vbt.number==16)) return false; size_t vbl; if (!readLength(r, vbl)) return false; size_t vbEnd = r.pos + vbl; if (vbEnd > r.size) return false;
			SnmpVarBind vb;
			// name
			Tag nt; if (!readTag(r, nt)) return false; if (!(nt.cls==TagClass::Universal && !nt.constructed && nt.number==6)) return false; size_t nl; if (!readLength(r, nl)) return false; std::vector<uint8_t> nv; if (!readValue(r, nl, nv)) return false; if (!asn1::decodeObjectIdentifier(nv, vb.oid)) return false;
			// value: could be many types; handle INTEGER, OCTET STRING, OID, NULL
			Tag vt; if (!readTag(r, vt)) return false; size_t vl; if (!readLength(r, vl)) return false; std::vector<uint8_t> vv; if (!readValue(r, vl, vv)) return false;
			if (vt.cls==TagClass::Universal && !vt.constructed && vt.number==2) {
				vb.value.type = SnmpValue::Type::Integer; int64_t x; if (!asn1::decodeInteger(vv, x)) return false; vb.value.intValue = x;
			} else if (vt.cls==TagClass::Universal && !vt.constructed && vt.number==4) {
				vb.value.type = SnmpValue::Type::OctetString; vb.value.bytesValue = vv;
			} else if (vt.cls==TagClass::Universal && !vt.constructed && vt.number==6) {
				vb.value.type = SnmpValue::Type::Oid; if (!asn1::decodeObjectIdentifier(vv, vb.value.oidValue)) return false;
			} else if (vt.cls==TagClass::Universal && !vt.constructed && vt.number==5) {
				vb.value.type = SnmpValue::Type::Null;
			} else {
				vb.value.type = SnmpValue::Type::Unsupported;
			}
			out.varBinds.push_back(std::move(vb));
			if (r.pos != vbEnd) r.pos = vbEnd; // skip any leftover
		}
		if (r.pos != vlEnd) r.pos = vlEnd;
	}
	return true;
}

