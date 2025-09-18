#pragma once
#include <string>

struct HttpResponse {
    bool ok = false;
    int status = 0;
    std::string body;
    std::string error;
};

class HttpClient {
public:
    HttpResponse post_json(const std::string& url, const std::string& body, int timeoutMs);
};

