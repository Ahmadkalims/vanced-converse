#pragma once

#include <string>
#include <map>
#include <windows.h>
#include <winhttp.h>

struct HttpResponse {
    int status_code = 0;
    std::string body;
    std::string error_message;
    bool success = false;
};

class WinHttpClient {
private:
    HINTERNET h_session_ = NULL;
    int timeout_ms_ = 10000;

public:
    WinHttpClient(int timeout_ms = 10000);
    ~WinHttpClient();

    HttpResponse get(const std::string& host, const std::string& path_and_query, bool is_https = true);
    
    static std::string url_encode(const std::string& value);
};
