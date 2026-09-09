#pragma once

#include "ThemeModel.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <windows.h>

class ThemeManager {
public:
    using ThemeChangeCallback = std::function<void(const StreamTheme&)>;

private:
    std::string themes_dir_;
    std::vector<StreamTheme> available_themes_;
    StreamTheme active_theme_;
    std::string active_theme_name_ = "Midnight Pink";
    std::vector<ThemeChangeCallback> listeners_;
    std::mutex mutex_;

    std::thread watcher_thread_;
    std::atomic<bool> watcher_running_{ false };
    HANDLE h_stop_event_ = NULL;

    void ensure_default_themes();
    void start_file_watcher();
    void stop_file_watcher();

public:
    ThemeManager();
    ~ThemeManager();

    void init();
    void scan_themes_directory();

    const std::vector<StreamTheme>& get_available_themes() const { return available_themes_; }
    const StreamTheme& get_active_theme() const { return active_theme_; }
    std::string get_active_theme_name() const { return active_theme_name_; }

    bool set_active_theme_by_name(const std::string& name);
    void set_active_theme(const StreamTheme& theme);

    bool import_theme_from_file(const std::string& file_path, std::string* out_error = nullptr);
    bool export_theme_to_file(const std::string& theme_name, const std::string& dest_path, std::string* out_error = nullptr);
    bool save_custom_theme(const StreamTheme& theme, std::string* out_error = nullptr);

    void add_change_listener(ThemeChangeCallback cb);
    void notify_listeners();

    std::string get_themes_directory() const { return themes_dir_; }
};

extern ThemeManager g_theme_manager;
