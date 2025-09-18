#include "otel.h"

#include <sstream>
#include <fstream>
#include <algorithm>

// Very small JSON builder (no external deps)
static std::string jsonEscape(const std::string &s){std::string o; for(char c: s){switch(c){case '"':o+="\\\"";break;case '\\':o+="\\\\";break;case '\n':o+="\\n";break;case '\r':o+="\\r";break;case '\t':o+="\\t";break;default: o+=c;}} return o;}

std::string buildOtelMetricsJson(const std::vector<GaugeSample> &samples) {
	// Minimal compliant OTLP JSON structure (ResourceMetrics -> ScopeMetrics -> Metrics -> Gauge -> DataPoints)
	std::ostringstream os;
	os << "{\n\"resourceMetrics\":[{\n\"resource\":{\"attributes\":[]},\n\"scopeMetrics\":[{\n\"scope\":{\"name\":\"snmp2otel\"},\n\"metrics\":[";
	for (size_t i=0;i<samples.size();++i){
		const auto &s = samples[i];
		if (i) os << ",";
		os << "{\"name\":\"" << jsonEscape(s.name) << "\",\"unit\":\"" << jsonEscape(s.unit) << "\",\"gauge\":{\"dataPoints\":[{\"asDouble\":" << s.value << "}]}}";
	}
	os << "]}]}]}";
	return os.str();
}

// Very small JSON mapping loader: expect a flat object mapping OID-> { name, unit, type }
bool loadMappingFile(const std::string &path, std::map<std::string, MetricDesc> &out) {
	out.clear();
	std::ifstream f(path);
	if (!f.is_open()) return false;
	std::string json((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	// naive parse: find occurrences of "oid": {"name":"...","unit":"...","type":"gauge"}
	size_t pos = 0;
	while (true) {
		size_t keyStart = json.find('"', pos);
		if (keyStart==std::string::npos) break;
		size_t keyEnd = json.find('"', keyStart+1);
		if (keyEnd==std::string::npos) break;
		std::string key = json.substr(keyStart+1, keyEnd-keyStart-1);
		size_t objStart = json.find('{', keyEnd);
		if (objStart==std::string::npos) break;
		size_t objEnd = json.find('}', objStart);
		if (objEnd==std::string::npos) break;
		std::string obj = json.substr(objStart, objEnd-objStart+1);
		// extract name
		size_t ns = obj.find("\"name\"");
		size_t ncol = ns==std::string::npos?std::string::npos: obj.find(':', ns);
		size_t nq1 = ncol==std::string::npos?std::string::npos: obj.find('"', ncol);
		size_t nq2 = nq1==std::string::npos?std::string::npos: obj.find('"', nq1+1);
		std::string name = (nq1!=std::string::npos && nq2!=std::string::npos) ? obj.substr(nq1+1, nq2-nq1-1) : key;
		// extract unit
		size_t us = obj.find("\"unit\"");
		size_t ucol = us==std::string::npos?std::string::npos: obj.find(':', us);
		size_t uq1 = ucol==std::string::npos?std::string::npos: obj.find('"', ucol);
		size_t uq2 = uq1==std::string::npos?std::string::npos: obj.find('"', uq1+1);
		std::string unit = (uq1!=std::string::npos && uq2!=std::string::npos) ? obj.substr(uq1+1, uq2-uq1-1) : "";
		// type must be gauge if present; ignore otherwise
		out[key] = MetricDesc{ name, unit };
		pos = objEnd+1;
	}
	return !out.empty();
}

