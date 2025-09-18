#include "asn1.h"
#include "oid.h"
#include "snmp.h"
#include "http.h"
#include "otel.h"
#include "util.h"

#include <vector>
#include <string>
#include <map>
#include <cstring>
#include <cstdlib>

struct Args {
	std::string target;
	std::string community = "public";
	std::string oidsFile;
	std::string endpoint;
	int interval = 10;
	int retries = 2;
	int timeoutMs = 1000;
	int port = 161;
	bool verbose = false;
	std::string mappingFile;
};

static void printHelp() {
	const char *h =
		"Usage: snmp2otel -t target [-C community] -o oids_file -e endpoint [-i interval] [-r retries] [-T timeout] [-p port] [-m mapping.json] [-v]\n";
	std::cout << h;
}

static bool parseArgs(int argc, char **argv, Args &a) {
	for (int i=1;i<argc;i++) {
		std::string arg = argv[i];
		if (arg == "-h" || arg == "--help") { printHelp(); std::exit(0);} 
		else if (arg == "-t" && i+1<argc) { a.target = argv[++i]; }
		else if (arg == "-C" && i+1<argc) { a.community = argv[++i]; }
		else if (arg == "-o" && i+1<argc) { a.oidsFile = argv[++i]; }
		else if (arg == "-e" && i+1<argc) { a.endpoint = argv[++i]; }
		else if (arg == "-i" && i+1<argc) { a.interval = util::toInt(argv[++i], 10); }
		else if (arg == "-r" && i+1<argc) { a.retries = util::toInt(argv[++i], 2); }
		else if (arg == "-T" && i+1<argc) { a.timeoutMs = util::toInt(argv[++i], 1000); }
		else if (arg == "-p" && i+1<argc) { a.port = util::toInt(argv[++i], 161); }
		else if (arg == "-m" && i+1<argc) { a.mappingFile = argv[++i]; }
		else if (arg == "-v") { a.verbose = true; }
		else { std::cerr << "Unknown or incomplete option: " << arg << "\n"; return false; }
	}
	if (a.target.empty() || a.oidsFile.empty() || a.endpoint.empty()) return false;
	if (a.interval <= 0) a.interval = 10;
	return true;
}

int main(int argc, char **argv) {
	Args args;
	if (!parseArgs(argc, argv, args)) { printHelp(); return 1; }

	std::vector<std::vector<uint32_t>> oids;
	if (!oidutil::loadOidsFromFile(args.oidsFile, oids)) {
		std::cerr << "Failed to load OIDs from file\n";
		return 1;
	}
	// enforce scalar OIDs
	std::vector<std::vector<uint32_t>> scalarOids;
	for (auto &o : oids) if (oidutil::isScalarOid(o)) scalarOids.push_back(o);
	if (scalarOids.empty()) {
		std::cerr << "No scalar OIDs (.0) found\n"; return 1;
	}

	std::map<std::string, MetricDesc> mapping;
	if (!args.mappingFile.empty()) {
		if (!loadMappingFile(args.mappingFile, mapping) && args.verbose) {
			util::log("Mapping file not loaded or empty; proceeding with default names");
		}
	}

	SnmpConfig sc; sc.targetHost=args.target; sc.targetPort=args.port; sc.community=args.community; sc.timeoutMs=args.timeoutMs; sc.retries=args.retries; sc.verbose=args.verbose;
	SnmpClient snmp(sc);
	HttpClient http(args.verbose);

	while (true) {
		SnmpPdu resp;
		bool ok = snmp.get(scalarOids, resp);
		std::vector<GaugeSample> samples;
		if (ok && resp.errorStatus==0) {
			for (const auto &vb : resp.varBinds) {
				std::string oidStr = oidutil::oidToString(vb.oid);
				MetricDesc md;
				auto it = mapping.find(oidStr);
				if (it != mapping.end()) {
					md = it->second;
				} else {
					md = MetricDesc{ "snmp." + oidStr, "" };
				}
				double value = 0.0;
				if (vb.value.type == SnmpValue::Type::Integer) {
					value = static_cast<double>(vb.value.intValue);
				} else if (vb.value.type == SnmpValue::Type::OctetString) {
					// try parse numeric from string
					std::string s(vb.value.bytesValue.begin(), vb.value.bytesValue.end());
					try { value = std::stod(s); } catch (...) { if (args.verbose) util::log("Non-numeric octet string for " + oidStr); continue; }
				} else if (vb.value.type == SnmpValue::Type::Oid) {
					// encode last arc as value
					if (!vb.value.oidValue.empty()) value = static_cast<double>(vb.value.oidValue.back()); else continue;
				} else {
					if (args.verbose) util::log("Unsupported type for OID " + oidStr);
					continue;
				}
				samples.push_back(GaugeSample{ md.name, md.unit, value });
			}
		} else {
			if (args.verbose) util::log("SNMP request failed or error status=" + std::to_string(resp.errorStatus));
		}

		if (!samples.empty()) {
			std::string json = buildOtelMetricsJson(samples);
			std::map<std::string,std::string> hdr{{"User-Agent","snmp2otel/1"}};
			auto r = http.postJson(args.endpoint, json, hdr, args.timeoutMs);
			if (args.verbose) util::log("Export HTTP status=" + std::to_string(r.status));
		}

		util::sleepSeconds(args.interval);
	}

	return 0;
}

