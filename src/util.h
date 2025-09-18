#pragma once

#include <string>
#include <chrono>
#include <thread>
#include <iostream>

namespace util {

inline void log(const std::string &msg) {
	std::cerr << msg << std::endl;
}

inline void sleepSeconds(int seconds) {
	std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

inline int toInt(const std::string &s, int def) {
	try { return std::stoi(s); } catch (...) { return def; }
}

}

