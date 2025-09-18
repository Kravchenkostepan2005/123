#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace oidutil {

bool parseOidString(const std::string &s, std::vector<uint32_t> &out);
std::string oidToString(const std::vector<uint32_t> &oid);
bool isScalarOid(const std::vector<uint32_t> &oid);
bool loadOidsFromFile(const std::string &path, std::vector<std::vector<uint32_t>> &out);

}

