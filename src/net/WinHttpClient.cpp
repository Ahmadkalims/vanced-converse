#include "WinHttpClient.hpp"
#include <sstream>
#include <iomanip>
#include <vector>

WinHttpClient::WinHttpClient(int timeout_ms) : timeout_ms_(timeout_ms) {
    h_session_ = WinHttpOpen(
        L"YTLiveChatOverlay/1.0 (Windows NT; x64)",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );

    if (h_session_) {
        WinHttpSetTimeouts(h_session_, timeout_ms_, timeout_ms_, timeout_ms_, timeout_ms_);
    }
}

WinHttpClient::~WinHttpClient() {
    if (h_session_) {
        WinHttpCloseHandle(h_session_);
        h_session_ = NULL;
    }
}

std::string WinHttpClient::url_encode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : value) {
        if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::setw(2) << int(static_cast<unsigned char>(c));
        }
    }

    return escaped.str();
}

HttpResponse WinHttpClient::get(const std::string& host, const std::string& path_and_query, bool is_https) {
    HttpResponse resp;
    if (!h_session_) {
        resp.error_message = "WinHttp session initialization failed";
        return resp;
    }

    std::wstring w_host(host.begin(), host.end());
    std::wstring w_path(path_and_query.begin(), path_and_query.end());

    INTERNET_PORT port = is_https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
    HINTERNET h_connect = WinHttpConnect(h_session_, w_host.c_str(), port, 0);
    if (!h_connect) {
        resp.error_message = "Failed to connect to host: " + host;
        return resp;
    }

    DWORD flags = is_https ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET h_request = WinHttpOpenRequest(
        h_connect,
        L"GET",
        w_path.c_str(),
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        flags
    );

    if (!h_request) {
        WinHttpCloseHandle(h_connect);
        resp.error_message = "Failed to open WinHttp request";
        return resp;
    }

    // Send request
    BOOL send_res = WinHttpSendRequest(
        h_request,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0
    );

    if (!send_res) {
        DWORD err = GetLastError();
        WinHttpCloseHandle(h_request);
        WinHttpCloseHandle(h_connect);
        resp.error_message = "WinHttpSendRequest failed with error: " + std::to_string(err);
        return resp;
    }

    if (!WinHttpReceiveResponse(h_request, NULL)) {
        DWORD err = GetLastError();
        WinHttpCloseHandle(h_request);
        WinHttpCloseHandle(h_connect);
        resp.error_message = "WinHttpReceiveResponse failed with error: " + std::to_string(err);
        return resp;
    }

    // Get HTTP status code
    DWORD status_code = 0;
    DWORD status_size = sizeof(status_code);
    WinHttpQueryHeaders(
        h_request,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &status_code,
        &status_size,
        WINHTTP_NO_HEADER_INDEX
    );
    resp.status_code = static_cast<int>(status_code);

    // Read response body
    std::string response_data;
    DWORD bytes_available = 0;
    while (WinHttpQueryDataAvailable(h_request, &bytes_available) && bytes_available > 0) {
        std::vector<char> buffer(bytes_available + 1, 0);
        DWORD bytes_read = 0;
        if (WinHttpReadData(h_request, buffer.data(), bytes_available, &bytes_read) && bytes_read > 0) {
            response_data.append(buffer.data(), bytes_read);
        } else {
            break;
        }
    }

    WinHttpCloseHandle(h_request);
    WinHttpCloseHandle(h_connect);

    resp.body = response_data;
    resp.success = (resp.status_code >= 200 && resp.status_code < 300);
    return resp;
}
