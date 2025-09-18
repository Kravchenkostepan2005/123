#include "oid.h"

#include <sstream>
#include <fstream>
#include <algorithm>
#include <cstdint>

namespace oidutil {

bool parseOidString(const std::string &s, std::vector<uint32_t> &out) {
	out.clear();
	std::string token;
	std::stringstream ss(s);
	while (std::getline(ss, token, '.')) {
		if (token.empty()) continue;
		for (char c : token) {
			if (c < '0' || c > '9') return false;
		}
		try {
			unsigned long v = std::stoul(token);
			if (v > 0xFFFFFFFFUL) return false;
			out.push_back(static_cast<uint32_t>(v));
		} catch (...) {
			return false;
		}
	}
	return out.size() >= 2;
}

std::string oidToString(const std::vector<uint32_t> &oid) {
	std::ostringstream os;
	for (size_t i = 0; i < oid.size(); ++i) {
		if (i) os << '.';
		os << oid[i];
	}
	return os.str();
}

bool isScalarOid(const std::vector<uint32_t> &oid) {
	return !oid.empty() && oid.back() == 0;
}

bool loadOidsFromFile(const std::string &path, std::vector<std::vector<uint32_t>> &out) {
	out.clear();
	std::ifstream f(path);
	if (!f.is_open()) return false;
	std::string line;
	while (std::getline(f, line)) {
		// trim
		line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](int ch){return ch!=' ' && ch!='\t' && ch!='\r';}));
		line.erase(std::find_if(line.rbegin(), line.rend(), [](int ch){return ch!=' ' && ch!='\t' && ch!='\r';}).base(), line.end());
		if (line.empty() || line[0] == '#') continue;
		std::vector<uint32_t> v;
		if (parseOidString(line, v)) {
			out.push_back(std::move(v));
		}
	}
	return true;
}

}

