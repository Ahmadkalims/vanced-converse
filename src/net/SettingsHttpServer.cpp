#include <winsock2.h>
#include <ws2tcpip.h>
#include "SettingsHttpServer.hpp"
#include "../common/Config.hpp"
#include "../common/ThemeManager.hpp"
#include "../common/UnicodeUtils.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

namespace fs = std::filesystem;

SettingsHttpServer g_settings_server;

static std::string get_mime_type(const std::string& path) {
    if (path.ends_with(".html") || path.ends_with(".htm")) return "text/html; charset=utf-8";
    if (path.ends_with(".css")) return "text/css; charset=utf-8";
    if (path.ends_with(".js")) return "application/javascript; charset=utf-8";
    if (path.ends_with(".json")) return "application/json; charset=utf-8";
    if (path.ends_with(".svg")) return "image/svg+xml";
    if (path.ends_with(".png")) return "image/png";
    if (path.ends_with(".jpg") || path.ends_with(".jpeg")) return "image/jpeg";
    if (path.ends_with(".ico")) return "image/x-icon";
    if (path.ends_with(".woff2")) return "font/woff2";
    if (path.ends_with(".woff")) return "font/woff";
    if (path.ends_with(".ttf")) return "font/ttf";
    return "text/plain; charset=utf-8";
}

SettingsHttpServer::SettingsHttpServer() = default;

SettingsHttpServer::~SettingsHttpServer() {
    stop();
}

bool SettingsHttpServer::start(const std::string& root_dir, const std::string& themes_dir, int port) {
    if (running_) return true;

    root_dir_ = root_dir;
    themes_dir_ = themes_dir;
    port_ = port;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        return false;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }

    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons((u_short)port_);

    if (bind(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        // Try fallback port 8089
        port_ = port + 1;
        addr.sin_port = htons((u_short)port_);
        if (bind(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            closesocket(s);
            WSACleanup();
            return false;
        }
    }

    if (listen(s, 10) == SOCKET_ERROR) {
        closesocket(s);
        WSACleanup();
        return false;
    }

    listen_socket_ = (uintptr_t)s;
    running_ = true;

    server_thread_ = std::thread(&SettingsHttpServer::run_server, this);
    return true;
}

void SettingsHttpServer::stop() {
    if (!running_) return;
    running_ = false;

    if (listen_socket_) {
        closesocket((SOCKET)listen_socket_);
        listen_socket_ = 0;
    }

    if (server_thread_.joinable()) {
        server_thread_.join();
    }

    WSACleanup();
}

void SettingsHttpServer::run_server() {
    while (running_) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET((SOCKET)listen_socket_, &read_fds);

        timeval tv = { 0, 500000 }; // 500ms timeout
        int sel = select(0, &read_fds, NULL, NULL, &tv);
        if (sel <= 0) continue;

        sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        SOCKET client_sock = accept((SOCKET)listen_socket_, (sockaddr*)&client_addr, &client_len);
        if (client_sock == INVALID_SOCKET) continue;

        // Read request
        char req_buf[8192] = { 0 };
        int bytes = recv(client_sock, req_buf, sizeof(req_buf) - 1, 0);
        if (bytes <= 0) {
            closesocket(client_sock);
            continue;
        }

        std::string req(req_buf, bytes);
        std::istringstream req_stream(req);
        std::string method, url, proto;
        req_stream >> method >> url >> proto;

        // Handle OPTIONS for CORS preflight
        if (method == "OPTIONS") {
            std::string resp = "HTTP/1.1 204 No Content\r\n"
                               "Access-Control-Allow-Origin: *\r\n"
                               "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                               "Access-Control-Allow-Headers: Content-Type\r\n"
                               "Content-Length: 0\r\n\r\n";
            send(client_sock, resp.c_str(), (int)resp.length(), 0);
            closesocket(client_sock);
            continue;
        }

        // 1. API Endpoints
        if (url == "/api/config" && method == "GET") {
            AppConfig cfg = g_config.get();
            std::string body = cfg.to_json_string();
            std::string resp = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: application/json; charset=utf-8\r\n"
                               "Access-Control-Allow-Origin: *\r\n"
                               "Content-Length: " + std::to_string(body.length()) + "\r\n\r\n" + body;
            send(client_sock, resp.c_str(), (int)resp.length(), 0);
            closesocket(client_sock);
            continue;
        }

        if (url == "/api/config" && method == "POST") {
            size_t body_pos = req.find("\r\n\r\n");
            if (body_pos != std::string::npos) {
                std::string body = req.substr(body_pos + 4);
                AppConfig cfg = g_config.get();
                if (cfg.from_json_string(body)) {
                    g_config.update(cfg, true);
                    g_config.save();
                }
            }
            std::string body = "{\"ok\":true}";
            std::string resp = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: application/json; charset=utf-8\r\n"
                               "Access-Control-Allow-Origin: *\r\n"
                               "Content-Length: " + std::to_string(body.length()) + "\r\n\r\n" + body;
            send(client_sock, resp.c_str(), (int)resp.length(), 0);
            closesocket(client_sock);
            continue;
        }

        if (url == "/api/themes" && method == "GET") {
            const auto& themes = g_theme_manager.get_available_themes();
            json::Value arr = json::Value::array();
            for (const auto& t : themes) {
                json::Value item = json::Value::object();
                item["id"] = t.metadata.name;
                item["name"] = t.metadata.name;
                item["author"] = t.metadata.author;
                item["version"] = (double)t.metadata.version;
                item["description"] = t.metadata.description;
                item["active"] = (t.metadata.name == g_theme_manager.get_active_theme_name());
                arr.push_back(item);
            }
            std::string body = arr.serialize(0);
            std::string resp = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: application/json; charset=utf-8\r\n"
                               "Access-Control-Allow-Origin: *\r\n"
                               "Content-Length: " + std::to_string(body.length()) + "\r\n\r\n" + body;
            send(client_sock, resp.c_str(), (int)resp.length(), 0);
            closesocket(client_sock);
            continue;
        }

        // 2. Static File Serving
        if (url == "/" || url.empty()) {
            url = "/index.html";
        }

        // Strip query params if any
        size_t q_pos = url.find('?');
        if (q_pos != std::string::npos) {
            url = url.substr(0, q_pos);
        }

        // Check if serving from themes_dir or root_dir
        fs::path file_path;
        if (url.starts_with("/themes/")) {
            file_path = fs::path(themes_dir_) / url.substr(8);
        } else {
            file_path = fs::path(root_dir_) / (url.starts_with("/") ? url.substr(1) : url);
        }

        if (fs::exists(file_path) && fs::is_regular_file(file_path)) {
            std::ifstream f(file_path.string(), std::ios::binary);
            if (f.is_open()) {
                std::stringstream file_data;
                file_data << f.rdbuf();
                std::string content = file_data.str();
                std::string mime = get_mime_type(file_path.string());

                std::string resp = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: " + mime + "\r\n"
                                   "Access-Control-Allow-Origin: *\r\n"
                                   "Content-Length: " + std::to_string(content.length()) + "\r\n"
                                   "Cache-Control: no-cache\r\n\r\n" + content;
                send(client_sock, resp.c_str(), (int)resp.length(), 0);
                closesocket(client_sock);
                continue;
            }
        }

        // 404 Not Found
        std::string not_found_body = "<h1>404 Not Found</h1><p>" + url + "</p>";
        std::string resp_404 = "HTTP/1.1 404 Not Found\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: " + std::to_string(not_found_body.length()) + "\r\n\r\n" + not_found_body;
        send(client_sock, resp_404.c_str(), (int)resp_404.length(), 0);
        closesocket(client_sock);
    }
}
