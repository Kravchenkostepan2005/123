#pragma once

#include <string>
#include <vector>
#include <map>

struct MetricDesc {
	std::string name;
	std::string unit;
};

struct GaugeSample {
	std::string name;
	std::string unit;
	double value;
};

// Build minimal OTLP/HTTP JSON request for gauge metrics
std::string buildOtelMetricsJson(const std::vector<GaugeSample> &samples);

// Optional mapping file support
bool loadMappingFile(const std::string &path, std::map<std::string, MetricDesc> &out);

