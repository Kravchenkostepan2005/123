#include "http.h"
#include "util.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <algorithm>

static std::string toLower(const std::string &s){std::string o=s;std::transform(o.begin(),o.end(),o.begin(),::tolower);return o;}

bool parseUrl(const std::string &url, UrlParts &out) {
	out = {};
	std::string::size_type pos = url.find("://");
	if (pos == std::string::npos) return false;
	out.scheme = url.substr(0, pos);
	std::string rest = url.substr(pos+3);
	std::string::size_type slash = rest.find('/');
	std::string hostport = slash==std::string::npos ? rest : rest.substr(0, slash);
	out.path = slash==std::string::npos ? "/" : rest.substr(slash);
	std::string::size_type colon = hostport.find(':');
	if (colon == std::string::npos) {
		out.host = hostport;
		out.port = (out.scheme=="https") ? 443 : 4318; // default for OTLP, fallback 80 for http below
		if (out.scheme=="http" && out.port==4318) out.port = 80;
	} else {
		out.host = hostport.substr(0, colon);
		out.port = static_cast<uint16_t>(std::stoi(hostport.substr(colon+1)));
	}
	return (out.scheme=="http"); // HTTPS not implemented in base version
}

HttpResponse HttpClient::postJson(const std::string &url, const std::string &body, const std::map<std::string,std::string> &headers, int timeoutMs) {
	UrlParts u; HttpResponse resp; resp.status = 0;
	if (!parseUrl(url, u)) { resp.status = -1; return resp; }

	struct addrinfo hints{}; hints.ai_family=AF_INET; hints.ai_socktype=SOCK_STREAM; struct addrinfo *res=nullptr;
	int rc = getaddrinfo(u.host.c_str(), nullptr, &hints, &res);
	if (rc!=0||!res) { resp.status=-2; return resp; }
	int s = ::socket(AF_INET, SOCK_STREAM, 0);
	if (s<0) { freeaddrinfo(res); resp.status=-3; return resp; }
	// timeout
	struct timeval tv; tv.tv_sec = timeoutMs/1000; tv.tv_usec = (timeoutMs%1000)*1000; setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv)); setsockopt(s,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv));
	struct sockaddr_in addr{}; addr.sin_family=AF_INET; addr.sin_port=htons(u.port); addr.sin_addr=((struct sockaddr_in*)res->ai_addr)->sin_addr; freeaddrinfo(res);
	if (connect(s,(struct sockaddr*)&addr,sizeof(addr))<0) { close(s); resp.status=-4; return resp; }

	std::ostringstream req;
	req << "POST " << u.path << " HTTP/1.1\r\n";
	req << "Host: " << u.host << "\r\n";
	req << "Content-Type: application/json\r\n";
	for (auto &kv : headers) {
		req << kv.first << ": " << kv.second << "\r\n";
	}
	std::string bodyStr = body;
	req << "Content-Length: " << bodyStr.size() << "\r\n";
	req << "Connection: close\r\n\r\n";
	req << bodyStr;
	std::string reqStr = req.str();
	ssize_t sent = send(s, reqStr.data(), reqStr.size(), 0);
	if (sent < 0) { close(s); resp.status=-5; return resp; }

	std::string raw;
	char buf[4096];
	ssize_t n;
	while ((n = recv(s, buf, sizeof(buf), 0)) > 0) raw.append(buf, buf+n);
	close(s);

	// Parse response
	std::string::size_type headerEnd = raw.find("\r\n\r\n");
	if (headerEnd == std::string::npos) { resp.status=-6; return resp; }
	std::string statusLine;
	std::string::size_type lineEnd = raw.find("\r\n");
	if (lineEnd == std::string::npos) { resp.status=-6; return resp; }
	statusLine = raw.substr(0, lineEnd);
	std::istringstream sl(statusLine);
	std::string httpver; sl >> httpver; sl >> resp.status; // ignore reason phrase
	std::string headersStr = raw.substr(lineEnd+2, headerEnd - (lineEnd+2));
	std::istringstream hs(headersStr);
	std::string hline;
	while (std::getline(hs, hline)) {
		if (!hline.empty() && hline.back()=='\r') hline.pop_back();
		auto colon = hline.find(':');
		if (colon!=std::string::npos) {
			std::string name = toLower(hline.substr(0, colon));
			std::string val = hline.substr(colon+1);
			// trim spaces
			val.erase(val.begin(), std::find_if(val.begin(), val.end(), [](int ch){return ch!=' '&&ch!='\t';}));
			resp.headers[name]=val;
		}
	}
	resp.body = raw.substr(headerEnd+4);
	return resp;
}

