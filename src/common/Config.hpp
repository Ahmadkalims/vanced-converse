#pragma once

#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include "ThemeModel.hpp"
#include "SecureStorage.hpp"

/**
 * @brief Application configuration model containing overlay preferences,
 * geometry, YouTube parameters, and the active StreamTheme.
 */
struct AppConfig {
    std::string api_key;           // Stored securely via Windows DPAPI; never plaintext on disk
    std::string video_id;          // Active YouTube video ID or stream URL
    int poll_interval_sec = 10;    // API polling frequency in seconds (default 10s: 360 req/hr = 1800 units/hr)
    
    // Active Theme
    std::string active_theme_name = "Midnight Pink";
    StreamTheme theme;

    // Overlay Window State
    bool click_through = false;    // Transparent to mouse clicks
    bool always_on_top = true;     // Keep topmost over fullscreen/games
    bool show_overlay = true;      // Visibility flag
    int max_messages = 200;        // Maximum chat messages to keep in history buffer

    // Geometry
    int window_x = -1;
    int window_y = -1;
    int window_width = 380;
    int window_height = 650;

    std::string to_json_string(bool pretty = true) const;
    bool from_json_string(const std::string& json_str);

    /**
     * @brief Normalizes raw YouTube URLs or stream IDs to a pure 11-character video ID.
     */
    static std::string extract_video_id(const std::string& input);
};

/**
 * @brief Central thread-safe configuration manager for Vanced Converse.
 */
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

    /**
     * @brief Loads configuration from AppData JSON and decrypts the API key via DPAPI.
     */
    bool load();

    /**
     * @brief Saves configuration to AppData JSON and encrypts the API key via DPAPI.
     */
    bool save();

    /**
     * @brief Retrieves a copy of the current configuration.
     */
    AppConfig get();

    /**
     * @brief Updates configuration and optionally fires listeners.
     */
    void update(const AppConfig& new_config, bool notify = true);

    /**
     * @brief Registers a callback invoked on config changes.
     */
    void add_change_listener(ChangeCallback callback);

    /**
     * @brief Dispatches current config state to all registered listeners.
     */
    void notify_listeners();

    std::string get_config_path() const { return config_path_; }
};

extern ConfigManager g_config;
