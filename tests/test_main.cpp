#include <iostream>
#include <cassert>
#include <vector>

#include "asn1.h"
#include "oid.h"
#include "otel.h"

static void test_asn1_integer(){
	int64_t v = -1234567;
	auto enc = asn1::encodeInteger(v);
	asn1::Reader r(enc.data(), enc.size());
	asn1::Tag t; size_t l; std::vector<uint8_t> val;
	assert(asn1::readTag(r,t)); assert(t.number==2 && !t.constructed);
	assert(asn1::readLength(r,l)); assert(asn1::readValue(r,l,val));
	int64_t out; assert(asn1::decodeInteger(val, out));
	assert(out==v);
}

static void test_asn1_oid(){
	std::vector<uint32_t> oid{1,3,6,1,2,1,1,3,0};
	auto enc = asn1::encodeObjectIdentifier(oid);
	asn1::Reader r(enc.data(), enc.size());
	asn1::Tag t; size_t l; std::vector<uint8_t> v;
	assert(asn1::readTag(r,t)); assert(t.number==6 && !t.constructed);
	assert(asn1::readLength(r,l)); assert(asn1::readValue(r,l,v));
	std::vector<uint32_t> out; assert(asn1::decodeObjectIdentifier(v, out));
	assert(out==oid);
}

static void test_oid_parser(){
	std::vector<uint32_t> out;
	assert(oidutil::parseOidString("1.3.6.1.2.1.1.3.0", out));
	assert(out.size()>=2);
	assert(oidutil::isScalarOid(out));
}

static void test_otel_json(){
	std::vector<GaugeSample> s{{"snmp.test", "ms", 100.5}};
	std::string j = buildOtelMetricsJson(s);
	assert(j.find("snmp.test")!=std::string::npos);
	assert(j.find("100.5")!=std::string::npos);
}

int main(){
	std::cout << "Running tests..." << std::endl;
	test_asn1_integer();
	test_asn1_oid();
	test_oid_parser();
	test_otel_json();
	std::cout << "All tests passed" << std::endl;
	return 0;
}

