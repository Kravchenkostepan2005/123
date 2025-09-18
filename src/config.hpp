#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct MetricInfo {
    std::string name;
    std::string unit;
    std::string type; // only "gauge" supported
};

using MetricMapping = std::unordered_map<std::string, MetricInfo>;

namespace Config {

bool read_oid_list(const std::string& path, std::vector<std::string>& out);
bool read_mapping(const std::string& path, MetricMapping& out);

}

