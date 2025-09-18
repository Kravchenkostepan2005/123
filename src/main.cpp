#include "snmp.hpp"
#include "config.hpp"
#include "otel.hpp"
#include "http.hpp"
#include "log.hpp"

#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono;

struct CliOptions {
    std::string target;
    std::string community = "public";
    std::string oidsFile;
    std::string endpoint;
    int intervalSec = 10;
    int retries = 2;
    int timeoutMs = 1000;
    int port = 161;
    bool verbose = false;
    std::string mappingFile;
};

static void print_usage(const char* prog) {
    std::cerr << "Usage: " << prog << " -t target [-C community] -o oids_file -e endpoint [-i interval] [-r retries] [-T timeout] [-p port] [-m mapping_json] [-v]\n";
}

static bool parse_args(int argc, char** argv, CliOptions& opts) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-t" && i + 1 < argc) {
            opts.target = argv[++i];
        } else if (arg == "-C" && i + 1 < argc) {
            opts.community = argv[++i];
        } else if (arg == "-o" && i + 1 < argc) {
            opts.oidsFile = argv[++i];
        } else if (arg == "-e" && i + 1 < argc) {
            opts.endpoint = argv[++i];
        } else if (arg == "-i" && i + 1 < argc) {
            opts.intervalSec = std::stoi(argv[++i]);
        } else if (arg == "-r" && i + 1 < argc) {
            opts.retries = std::stoi(argv[++i]);
        } else if (arg == "-T" && i + 1 < argc) {
            opts.timeoutMs = std::stoi(argv[++i]);
        } else if (arg == "-p" && i + 1 < argc) {
            opts.port = std::stoi(argv[++i]);
        } else if (arg == "-m" && i + 1 < argc) {
            opts.mappingFile = argv[++i];
        } else if (arg == "-v") {
            opts.verbose = true;
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return false;
        } else {
            std::cerr << "Unknown or incomplete argument: " << arg << "\n";
            print_usage(argv[0]);
            return false;
        }
    }
    if (opts.target.empty() || opts.oidsFile.empty() || opts.endpoint.empty()) {
        print_usage(argv[0]);
        return false;
    }
    if (opts.intervalSec <= 0) {
        std::cerr << "Interval must be > 0\n";
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    CliOptions opts;
    if (!parse_args(argc, argv, opts)) {
        return 1;
    }
    Logger::instance().set_verbose(opts.verbose);

    std::vector<std::string> oids;
    if (!Config::read_oid_list(opts.oidsFile, oids)) {
        std::cerr << "Failed to read OIDs from file: " << opts.oidsFile << "\n";
        return 1;
    }
    for (const auto& oid : oids) {
        if (oid.size() < 2 || oid.rfind(".0") != oid.size() - 2) {
            std::cerr << "OID must be scalar and end with .0: " << oid << "\n";
            return 1;
        }
    }

    MetricMapping mapping;
    if (!opts.mappingFile.empty()) {
        if (!Config::read_mapping(opts.mappingFile, mapping)) {
            std::cerr << "Failed to read mapping file: " << opts.mappingFile << "\n";
            return 1;
        }
    }

    SnmpClient snmp(opts.target, opts.port, opts.community, opts.timeoutMs, opts.retries);
    HttpClient http;

    while (true) {
        auto nowNs = duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
        std::vector<SnmpValue> values;
        values.reserve(oids.size());

        for (const auto& oid : oids) {
            auto res = snmp.get(oid);
            if (!res.ok) {
                std::cerr << "SNMP get failed for OID " << oid << ": " << res.error << "\n";
                continue;
            }
            values.push_back(res.value);
        }

        if (!values.empty()) {
            std::string body = Otel::build_gauge_metrics_json(values, mapping, nowNs);
            if (opts.verbose) {
                Logger::instance().log("OTLP/HTTP JSON body: " + body);
            }
            auto httpRes = http.post_json(opts.endpoint, body, opts.timeoutMs);
            if (!httpRes.ok) {
                std::cerr << "Export failed: " << httpRes.error << "\n";
            } else if (opts.verbose) {
                Logger::instance().log("Export HTTP status: " + std::to_string(httpRes.status));
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(opts.intervalSec));
    }

    return 0;
}

