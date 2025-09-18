#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

struct HttpResponse {
	int status{0};
	std::string body;
	std::map<std::string, std::string> headers;
};

struct UrlParts {
	std::string scheme;
	std::string host;
	uint16_t port{0};
	std::string path;
};

bool parseUrl(const std::string &url, UrlParts &out);

class HttpClient {
public:
	HttpClient(bool verbose=false) : verbose(verbose) {}
	HttpResponse postJson(const std::string &url, const std::string &body, const std::map<std::string,std::string> &headers, int timeoutMs);
private:
	bool verbose{false};
};

