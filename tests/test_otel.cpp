#include "../src/otel.hpp"
#include <iostream>

int main() {
    SnmpValue v; v.oid = "1.3.6.1.2.1.1.3.0"; v.type = SnmpValue::Type::Gauge32; v.uintValue = 12345;
    MetricMapping m; m[v.oid] = MetricInfo{"snmp.sysUpTime","ms","gauge"};
    auto json = Otel::build_gauge_metrics_json({v}, m, 1000);
    if (json.find("\"name\":\"snmp.sysUpTime\"") == std::string::npos) { std::cerr << "name missing" << std::endl; return 1; }
    if (json.find("\"asDouble\":12345") == std::string::npos) { std::cerr << "value missing" << std::endl; return 1; }
    return 0;
}

