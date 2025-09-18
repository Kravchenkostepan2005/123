#include "http.hpp"
#include "log.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <string>

static bool parse_http_url(const std::string& url, std::string& host, std::string& port, std::string& path) {
    // Expect http://host[:port]/path
    const std::string prefix = "http://";
    if (url.rfind(prefix, 0) != 0) return false;
    std::string rest = url.substr(prefix.size());
    size_t slash = rest.find('/');
    std::string hostport = (slash == std::string::npos) ? rest : rest.substr(0, slash);
    path = (slash == std::string::npos) ? "/" : rest.substr(slash);
    size_t colon = hostport.rfind(':');
    if (colon != std::string::npos) {
        host = hostport.substr(0, colon);
        port = hostport.substr(colon + 1);
    } else {
        host = hostport;
        port = "80";
    }
    if (host.empty()) return false;
    return true;
}

HttpResponse HttpClient::post_json(const std::string& url, const std::string& body, int timeoutMs) {
    HttpResponse r;
    std::string host, port, path;
    if (!parse_http_url(url, host, port, path)) {
        r.error = "Only http:// URLs are supported";
        return r;
    }
    addrinfo hints{}; hints.ai_socktype = SOCK_STREAM; hints.ai_family = AF_UNSPEC;
    addrinfo* res = nullptr;
    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &res) != 0) {
        r.error = "DNS resolution failed";
        return r;
    }
    int sock = -1;
    for (addrinfo* p = res; p; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock < 0) continue;
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
        int rc = connect(sock, p->ai_addr, p->ai_addrlen);
        if (rc < 0) {
            if (errno != EINPROGRESS) { close(sock); sock = -1; continue; }
            pollfd fds{sock, POLLOUT, 0};
            int pr = poll(&fds, 1, timeoutMs);
            if (pr <= 0) { close(sock); sock = -1; continue; }
            int err = 0; socklen_t errlen = sizeof(err);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &errlen);
            if (err != 0) { close(sock); sock = -1; continue; }
        }
        // restore blocking
        fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
        // Set timeouts
        timeval tv{}; tv.tv_sec = timeoutMs / 1000; tv.tv_usec = (timeoutMs % 1000) * 1000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        std::string req;
        req += "POST " + path + " HTTP/1.1\r\n";
        req += "Host: " + host + "\r\n";
        req += "Content-Type: application/json\r\n";
        req += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        req += "Connection: close\r\n\r\n";
        req += body;

        ssize_t sent = send(sock, req.data(), req.size(), 0);
        if (sent < 0) { close(sock); sock = -1; continue; }

        std::string resp;
        char buf[4096];
        while (true) {
            ssize_t n = recv(sock, buf, sizeof(buf), 0);
            if (n <= 0) break;
            resp.append(buf, buf + n);
        }
        close(sock); sock = -1;

        // parse status
        size_t pos = resp.find("\r\n");
        if (pos == std::string::npos) { r.error = "Bad HTTP response"; freeaddrinfo(res); return r; }
        std::string statusLine = resp.substr(0, pos);
        int code = 0;
        if (statusLine.size() >= 12 && statusLine.rfind("HTTP/1.", 0) == 0) {
            size_t sp = statusLine.find(' ');
            if (sp != std::string::npos) code = std::stoi(statusLine.substr(sp + 1));
        }
        r.status = code;
        r.ok = (code >= 200 && code < 300);
        r.body = resp;
        freeaddrinfo(res);
        return r;
    }
    if (res) freeaddrinfo(res);
    r.error = "Connection failed";
    return r;
}

