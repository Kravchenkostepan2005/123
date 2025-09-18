#include "config.hpp"

#include <cctype>
#include <fstream>
#include <sstream>

static std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

bool Config::read_oid_list(const std::string& path, std::vector<std::string>& out) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    std::string line;
    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        out.push_back(line);
    }
    return true;
}

// Very small JSON parser for expected mapping format only.
bool Config::read_mapping(const std::string& path, MetricMapping& out) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    std::stringstream buffer; buffer << f.rdbuf();
    std::string s = buffer.str();
    // naive state machine
    size_t i = 0; auto skip_ws = [&]() { while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i; };
    auto parse_str = [&]() -> std::string {
        if (i >= s.size() || s[i] != '"') return std::string();
        ++i; std::string r; while (i < s.size()) { char c = s[i++]; if (c == '\\' && i < s.size()) { char n = s[i++]; r.push_back(n); } else if (c == '"') break; else r.push_back(c);} return r; };
    skip_ws(); if (i >= s.size() || s[i] != '{') return false; ++i; skip_ws();
    while (i < s.size() && s[i] != '}') {
        std::string key = parse_str(); if (key.empty()) return false; skip_ws(); if (i>=s.size()||s[i] != ':') return false; ++i; skip_ws();
        if (i>=s.size() || s[i] != '{') return false; ++i; skip_ws();
        MetricInfo mi; mi.type = "gauge";
        while (i < s.size() && s[i] != '}') {
            std::string k = parse_str(); skip_ws(); if (i>=s.size()||s[i] != ':') return false; ++i; skip_ws();
            if (k == "name" || k == "unit" || k == "type") {
                std::string v = parse_str();
                if (k == "name") mi.name = v; else if (k == "unit") mi.unit = v; else mi.type = v;
            } else {
                // skip value
                if (s[i] == '"') { (void)parse_str(); }
            }
            skip_ws(); if (i < s.size() && s[i] == ',') { ++i; skip_ws(); }
        }
        if (i>=s.size()||s[i] != '}') return false; ++i; skip_ws();
        out[key] = mi;
        if (i < s.size() && s[i] == ',') { ++i; skip_ws(); }
    }
    if (i>=s.size()||s[i] != '}') return false;
    return true;
}

