#pragma once

#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include "ThemeModel.hpp"

struct AppConfig {
    std::string api_key;
    std::string video_id;
    int poll_interval_sec = 15;
    
    // Theme Integration
    std::string active_theme_name = "Midnight Pink";
    StreamTheme theme;

    // Overlay Window State
    bool click_through = false;
    bool always_on_top = true;
    bool show_overlay = true;
    int max_messages = 200;

    int window_x = -1;
    int window_y = -1;
    int window_width = 380;
    int window_height = 650;

    std::string to_json_string(bool pretty = true) const;
    bool from_json_string(const std::string& json_str);

    static std::string extract_video_id(const std::string& input);
};


class ConfigManager {
public:
    using ChangeCallback = std::function<void(const AppConfig&)>;

private:
    std::string config_path_;
    AppConfig current_config_;
    std::vector<ChangeCallback> listeners_;
    std::mutex config_mutex_;

    void ensure_config_dir();

public:
    ConfigManager();
    ~ConfigManager();

    bool load();
    bool save();

    AppConfig get();
    void update(const AppConfig& new_config, bool notify = true);

    void add_change_listener(ChangeCallback callback);
    void notify_listeners();

    std::string get_config_path() const { return config_path_; }
};

extern ConfigManager g_config;
