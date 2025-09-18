#include "../src/config.hpp"
#include <fstream>
#include <iostream>

int main() {
    // Prepare temp files
    const char* oids = "/workspace/tests/tmp_oids.txt";
    const char* mapf = "/workspace/tests/tmp_map.json";
    {
        std::ofstream f(oids);
        f << "# comment\n\n1.3.6.1.2.1.1.3.0\n";
    }
    {
        std::ofstream f(mapf);
        f << "{\n  \"1.3.6.1.2.1.1.3.0\": { \"name\": \"snmp.sysUpTime\", \"unit\": \"ms\", \"type\": \"gauge\" }\n}\n";
    }
    std::vector<std::string> o;
    if (!Config::read_oid_list(oids, o)) { std::cerr << "read_oid_list failed" << std::endl; return 1; }
    if (o.size() != 1 || o[0] != "1.3.6.1.2.1.1.3.0") { std::cerr << "oids parse mismatch" << std::endl; return 1; }

    MetricMapping m;
    if (!Config::read_mapping(mapf, m)) { std::cerr << "read_mapping failed" << std::endl; return 1; }
    if (m.find("1.3.6.1.2.1.1.3.0") == m.end()) { std::cerr << "mapping missing" << std::endl; return 1; }
    if (m["1.3.6.1.2.1.1.3.0"].name != "snmp.sysUpTime") { std::cerr << "mapping name mismatch" << std::endl; return 1; }
    return 0;
}

