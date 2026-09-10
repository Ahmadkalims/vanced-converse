#include "Config.hpp"
#include "SimpleJson.hpp"
#include "ThemeManager.hpp"
#include "SecureStorage.hpp"
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

namespace fs = std::filesystem;

ConfigManager g_config;

/**
 * @brief Parses YouTube video URL, embed link, short link, or raw ID.
 */
std::string AppConfig::extract_video_id(const std::string& input) {
    if (input.empty()) return "";

    std::string trimmed = input;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
    trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

    // Direct 11-char ID
    if (trimmed.length() == 11 && trimmed.find('/') == std::string::npos && trimmed.find('?') == std::string::npos) {
        return trimmed;
    }

    // ?v=...
    std::regex v_param_regex(R"([?&]v=([a-zA-Z0-9_-]{11}))");
    std::smatch match;
    if (std::regex_search(trimmed, match, v_param_regex)) {
        return match[1].str();
    }

    // youtube.com/live/...
    std::regex live_regex(R"(youtube\.com/live/([a-zA-Z0-9_-]{11}))");
    if (std::regex_search(trimmed, match, live_regex)) {
        return match[1].str();
    }

    // youtu.be/...
    std::regex youtu_be_regex(R"(youtu\.be/([a-zA-Z0-9_-]{11}))");
    if (std::regex_search(trimmed, match, youtu_be_regex)) {
        return match[1].str();
    }

    // youtube.com/embed/...
    std::regex embeds_regex(R"(youtube\.com/embed/([a-zA-Z0-9_-]{11}))");
    if (std::regex_search(trimmed, match, embeds_regex)) {
        return match[1].str();
    }

    return trimmed;
}

ConfigManager::ConfigManager() {
    char app_data[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, app_data))) {
        fs::path modern_dir = fs::path(app_data) / "VancedConverse";
        config_path_ = (modern_dir / "config.json").string();

        // Check if legacy config exists and migrate if needed
        fs::path legacy_dir = fs::path(app_data) / "YT_Live_Chat_Overlay";
        if (!fs::exists(config_path_) && fs::exists(legacy_dir / "config.json")) {
            try {
                fs::create_directories(modern_dir);
                fs::copy_file(legacy_dir / "config.json", config_path_, fs::copy_options::overwrite_existing);
            } catch (...) {}
        }
    } else {
        config_path_ = "config.json";
    }

    current_config_.theme = StreamTheme::create_midnight_pink();
    current_config_.active_theme_name = "Midnight Pink";
}

ConfigManager::~ConfigManager() {}

void ConfigManager::ensure_config_dir() {
    try {
        fs::path p(config_path_);
        fs::create_directories(p.parent_path());
    } catch (...) {}
}

/**
 * @brief Reads configuration from disk and loads API key securely from DPAPI.
 */
bool ConfigManager::load() {
    ensure_config_dir();
    g_theme_manager.init();

    // 1. Load API key from DPAPI secure storage
    std::string secured_key = SecureStorage::load_api_key();

    std::ifstream file(config_path_);
    if (!file.is_open()) {
        current_config_.api_key = secured_key;
        current_config_.theme = g_theme_manager.get_active_theme();
        save();
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    json::Value root = json::parse(buffer.str());
    if (!root.is_object()) {
        current_config_.api_key = secured_key;
        return false;
    }

    std::lock_guard<std::mutex> lock(config_mutex_);

    // If DPAPI has key, use it; otherwise migrate plaintext key if present
    if (!secured_key.empty()) {
        current_config_.api_key = secured_key;
    } else {
        std::string plain_key = root["api_key"].as_string("");
        if (!plain_key.empty()) {
            current_config_.api_key = plain_key;
            SecureStorage::save_api_key(plain_key);
        }
    }

    current_config_.video_id = root["video_id"].as_string("");
    current_config_.poll_interval_sec = root["poll_interval_sec"].as_int(15);
    current_config_.click_through = root["click_through"].as_bool(false);
    current_config_.always_on_top = root["always_on_top"].as_bool(true);
    current_config_.show_overlay = root["show_overlay"].as_bool(true);
    current_config_.max_messages = root["max_messages"].as_int(200);

    current_config_.window_x = root["window_x"].as_int(-1);
    current_config_.window_y = root["window_y"].as_int(-1);
    current_config_.window_width = root["window_width"].as_int(380);
    current_config_.window_height = root["window_height"].as_int(650);

    current_config_.active_theme_name = root["active_theme_name"].as_string("Midnight Pink");

    if (root.contains("theme")) {
        current_config_.theme.from_json_string(root["theme"].serialize());
    } else {
        g_theme_manager.set_active_theme_by_name(current_config_.active_theme_name);
        current_config_.theme = g_theme_manager.get_active_theme();
    }

    return true;
}

/**
 * @brief Saves configuration to disk with encrypted DPAPI credential storage.
 */
bool ConfigManager::save() {
    ensure_config_dir();

    // 1. Securely encrypt API key using Windows DPAPI
    {
        std::lock_guard<std::mutex> lock(config_mutex_);
        SecureStorage::save_api_key(current_config_.api_key);
    }

    json::Value root = json::Value::object();
    {
        std::lock_guard<std::mutex> lock(config_mutex_);
        // NEVER write plaintext api_key to config file
        root["api_key"] = "";
        root["video_id"] = current_config_.video_id;
        root["poll_interval_sec"] = (double)current_config_.poll_interval_sec;
        root["click_through"] = current_config_.click_through;
        root["always_on_top"] = current_config_.always_on_top;
        root["show_overlay"] = current_config_.show_overlay;
        root["max_messages"] = (double)current_config_.max_messages;
        root["window_x"] = (double)current_config_.window_x;
        root["window_y"] = (double)current_config_.window_y;
        root["window_width"] = (double)current_config_.window_width;
        root["window_height"] = (double)current_config_.window_height;
        root["active_theme_name"] = current_config_.active_theme_name;

        json::Value theme_json = json::parse(current_config_.theme.to_json_string(false));
        root["theme"] = theme_json;
    }

    std::ofstream file(config_path_);
    if (!file.is_open()) return false;

    file << root.serialize(2);
    file.close();
    return true;
}

AppConfig ConfigManager::get() {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return current_config_;
}

void ConfigManager::update(const AppConfig& new_config, bool notify) {
    {
        std::lock_guard<std::mutex> lock(config_mutex_);
        current_config_ = new_config;
    }
    if (notify) {
        notify_listeners();
    }
}

void ConfigManager::add_change_listener(ChangeCallback callback) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    listeners_.push_back(callback);
}

void ConfigManager::notify_listeners() {
    AppConfig copy = get();
    std::vector<ChangeCallback> list_copy;
    {
        std::lock_guard<std::mutex> lock(config_mutex_);
        list_copy = listeners_;
    }
    for (const auto& cb : list_copy) {
        if (cb) cb(copy);
    }
}

std::string AppConfig::to_json_string(bool pretty) const {
    json::Value root = json::Value::object();
    root["api_key"] = ""; // Redacted in output
    root["video_id"] = video_id;
    root["poll_interval_sec"] = (double)poll_interval_sec;
    root["click_through"] = click_through;
    root["always_on_top"] = always_on_top;
    root["show_overlay"] = show_overlay;
    root["max_messages"] = (double)max_messages;
    root["active_theme_name"] = active_theme_name;
    return root.serialize(pretty ? 2 : 0);
}

bool AppConfig::from_json_string(const std::string& json_str) {
    json::Value root = json::parse(json_str);
    if (!root.is_object()) return false;

    video_id = root["video_id"].as_string(video_id);
    poll_interval_sec = root["poll_interval_sec"].as_int(poll_interval_sec);
    click_through = root["click_through"].as_bool(click_through);
    always_on_top = root["always_on_top"].as_bool(always_on_top);
    show_overlay = root["show_overlay"].as_bool(show_overlay);
    max_messages = root["max_messages"].as_int(max_messages);
    active_theme_name = root["active_theme_name"].as_string(active_theme_name);
    return true;
}
