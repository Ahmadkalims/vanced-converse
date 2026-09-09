#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <functional>

class SettingsHttpServer {
private:
    std::string root_dir_;
    std::string themes_dir_;
    int port_ = 8088;
    std::thread server_thread_;
    std::atomic<bool> running_{ false };
    uintptr_t listen_socket_ = 0;

    void run_server();

public:
    SettingsHttpServer();
    ~SettingsHttpServer();

    bool start(const std::string& root_dir, const std::string& themes_dir, int port = 8088);
    void stop();

    bool is_running() const { return running_; }
    int get_port() const { return port_; }
    std::string get_url() const { return "http://127.0.0.1:" + std::to_string(port_) + "/"; }
};

extern SettingsHttpServer g_settings_server;
