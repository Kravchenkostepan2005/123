#include "otel.hpp"

#include <sstream>

static std::string bytes_to_string(const std::vector<uint8_t>& bytes) {
    return std::string(bytes.begin(), bytes.end());
}

namespace Otel {

std::string escape_json(const std::string& s) {
    std::string out; out.reserve(s.size()+8);
    for (char c : s) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
        }
    }
    return out;
}

static std::string name_for_oid(const std::string& oid, const MetricMapping& mapping) {
    auto it = mapping.find(oid);
    if (it != mapping.end() && !it->second.name.empty()) return it->second.name;
    std::string name = "snmp." + oid;
    for (char& c : name) if (c == '.') c = '_';
    return name;
}

static std::string unit_for_oid(const std::string& oid, const MetricMapping& mapping) {
    auto it = mapping.find(oid);
    if (it != mapping.end()) return it->second.unit;
    return std::string();
}

std::string build_gauge_metrics_json(const std::vector<SnmpValue>& values,
                                     const MetricMapping& mapping,
                                     long long timeUnixNano) {
    std::ostringstream os;
    os << "{\"resourceMetrics\":[{\"resource\":{\"attributes\":[]},\"scopeMetrics\":[{\"scope\":{\"name\":\"snmp2otel\"},\"metrics\":[";
    bool first = true;
    for (const auto& v : values) {
        // Determine numeric value suitable for Gauge
        double num = 0.0;
        bool have = true;
        if (v.type == SnmpValue::Type::Integer) {
            num = static_cast<double>(v.intValue);
        } else if (v.type == SnmpValue::Type::Gauge32 || v.type == SnmpValue::Type::Counter32 || v.type == SnmpValue::Type::Counter64) {
            num = static_cast<double>(v.uintValue);
        } else if (v.type == SnmpValue::Type::TimeTicks) {
            // SNMP TimeTicks are in 1/100 seconds; export as milliseconds
            num = static_cast<double>(v.uintValue) * 10.0;
        } else if (v.type == SnmpValue::Type::OctetString) {
            // attempt parse as number if possible
            std::string s = bytes_to_string(v.stringValue);
            try { num = std::stod(s); } catch (...) { have = false; }
        } else {
            have = false;
        }
        if (!have) continue;
        if (!first) { os << ","; }
        first = false;
        std::string name = name_for_oid(v.oid, mapping);
        std::string unit = unit_for_oid(v.oid, mapping);
        os << "{\"name\":\"" << escape_json(name) << "\",\"unit\":\"" << escape_json(unit) << "\",\"gauge\":{\"dataPoints\":[{\"timeUnixNano\":" << timeUnixNano << ",\"asDouble\":" << num << "}]}}";
    }
    os << "]}]}]}";
    return os.str();
}

}

