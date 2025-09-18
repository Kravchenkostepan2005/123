#pragma once
#include <string>
#include <vector>
#include "snmp.hpp"
#include "config.hpp"

namespace Otel {

std::string escape_json(const std::string& s);
std::string build_gauge_metrics_json(const std::vector<SnmpValue>& values,
                                     const MetricMapping& mapping,
                                     long long timeUnixNano);

}

