#pragma once
#include <iostream>
#include <mutex>
#include <string>

class Logger {
public:
    static Logger& instance() {
        static Logger inst;
        return inst;
    }
    void set_verbose(bool v) { verbose_ = v; }
    void log(const std::string& msg) {
        if (!verbose_) return;
        std::lock_guard<std::mutex> lock(mu_);
        std::cerr << msg << "\n";
    }
private:
    std::mutex mu_;
    bool verbose_ = false;
};

