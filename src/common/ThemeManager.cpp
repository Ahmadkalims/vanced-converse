#include "ThemeManager.hpp"
#include "UnicodeUtils.hpp"
#include "SimpleJson.hpp"
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>

namespace fs = std::filesystem;

ThemeManager g_theme_manager;

static const char* TEMPLATE_CSS_CONTENT = R"RAW(/* ==============================================================================
   VANCED CONVERSE — THEME CUSTOMIZATION TEMPLATE & SPECIFICATION
   ==============================================================================
   This template contains all available CSS variables driving the Vanced Converse
   native overlay engine. Edit these values to customize colors, geometry,
   glass blur effects, typography, animations, and event triggers.

   Save as "Your_Theme_Name.css" inside the themes/ directory for instant hot-reload.
   ============================================================================== */

/* Theme Metadata
Theme Name: Custom Theme Template
Author: Streamer
Version: 1
Description: Clean starter template demonstrating all overlay styling options.
*/

:root {
    /* --------------------------------------------------------------------------
       OVERLAY CONTAINER (Backdrop Material, Glass Blur, Opacity, Padding & Fading)
       -------------------------------------------------------------------------- */
    /* Container tint color: Hex (#RRGGBB, #RRGGBBAA) or rgba(r,g,b,a) */
    --vc-overlay-bg-color: #14141E;

    /* Master container opacity: 0.0 (transparent) to 1.0 (opaque) */
    --vc-overlay-bg-opacity: 0.40;

    /* Windows Acrylic / DWM glass blur radius in pixels (0px to 60px) */
    --vc-overlay-bg-blur: 20px;

    /* Window corner radius in pixels (clipping region is strictly matched) */
    --vc-overlay-radius: 20px;

    /* Inner viewport padding around chat stream (gap between edges and messages) */
    --vc-overlay-padding-top: 16px;
    --vc-overlay-padding-bottom: 16px;
    --vc-overlay-padding-left: 16px;
    --vc-overlay-padding-right: 16px;

    /* Edge fading: 1 = smooth vertical alpha dissolve at top and bottom edges */
    --vc-overlay-fade-edges: 1;

    /* Height of the vertical gradient edge fade mask in pixels */
    --vc-overlay-fade-length: 32px;

    /* Outer border and glow */
    --vc-overlay-border-color: rgba(255, 255, 255, 0.12);
    --vc-overlay-border-width: 1px;
    --vc-overlay-glow-color: transparent;
    --vc-overlay-glow-radius: 0px;

    /* --------------------------------------------------------------------------
       USERNAME STYLING (Typography, Colors, Glow/Text-Shadow)
       -------------------------------------------------------------------------- */
    /* Primary font family */
    --vc-username-font: "Montserrat Alternates", sans-serif;

    /* Font size in pixels (10px to 48px) */
    --vc-username-size: 16px;

    /* Font weight (400 = Regular, 600 = SemiBold, 700 = Bold) */
    --vc-username-weight: 600;

    /* Default or static username color */
    --vc-username-color: #F9A8D4;

    /* Glow effect (mapped to CSS text-shadow: 0 0 <radius> <color>) */
    --vc-username-glow-color: #F9A8D4;
    --vc-username-glow-radius: 6px;
    --vc-username-glow-intensity: 0.75;

    /* --------------------------------------------------------------------------
       MESSAGE TEXT STYLING
       -------------------------------------------------------------------------- */
    --vc-message-font: "Montserrat Alternates", sans-serif;
    --vc-message-size: 16px;
    --vc-message-weight: 400;
    --vc-message-color: #F4F4F5;
    --vc-message-line-spacing: 0px;
    --vc-message-glow-color: transparent;
    --vc-message-glow-radius: 0px;
    --vc-message-glow-intensity: 0.0;

    /* --------------------------------------------------------------------------
       MESSAGE BUBBLE / CARD
       -------------------------------------------------------------------------- */
    /* Set to transparent for floating clean text without pill background */
    --vc-message-bg-color: rgba(255, 255, 255, 0.03);
    --vc-message-radius: 15px;
    --vc-message-border-color: transparent;
    --vc-message-border-width: 0px;
    --vc-message-padding-top: 10px;
    --vc-message-padding-bottom: 10px;
    --vc-message-padding-left: 14px;
    --vc-message-padding-right: 14px;

    /* --------------------------------------------------------------------------
       SCROLLBAR
       -------------------------------------------------------------------------- */
    --vc-scrollbar-width: 6px;
    --vc-scrollbar-thumb-color: #F9A8D4;
    --vc-scrollbar-radius: 3px;

    /* --------------------------------------------------------------------------
       MOTION & ANIMATIONS
       -------------------------------------------------------------------------- */
    --vc-motion-duration: 300ms;
    --vc-motion-ease: cubic-bezier(0, 0, 0.2, 1);

    /* --------------------------------------------------------------------------
       SPECIAL EVENTS
       -------------------------------------------------------------------------- */
    /* Question highlight trigger */
    --vc-event-question-bg: rgba(249, 168, 212, 0.12);
    --vc-event-question-border: #F472B6;
    --vc-event-question-border-width: 1.5px;
}
)RAW";

static fs::path resolve_primary_themes_dir() {
    char exe_path[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    fs::path exe_dir = fs::path(exe_path).parent_path();
    fs::path p_exe_themes = exe_dir / "themes";

    if (fs::exists(p_exe_themes)) return p_exe_themes;
    if (fs::exists("themes")) return fs::absolute("themes");

    return p_exe_themes;
}

ThemeManager::ThemeManager() {
    themes_dir_ = resolve_primary_themes_dir().string();
    active_theme_ = StreamTheme::create_midnight_pink();
    active_theme_name_ = active_theme_.metadata.name;
}

ThemeManager::~ThemeManager() {
    stop_file_watcher();
}

/**
 * @brief Initializes the theme directory, writes default presets, scans files,
 * and starts the live file watcher.
 */
void ThemeManager::init() {
    try {
        fs::create_directories(themes_dir_);
    } catch (...) {}

    ensure_default_themes();
    scan_themes_directory();
    start_file_watcher();
}

/**
 * @brief Ensures all built-in CSS themes and the template CSS file exist on disk.
 */
void ThemeManager::ensure_default_themes() {
    auto write_css_if_missing = [this](const StreamTheme& theme, const std::string& filename) {
        fs::path p = fs::path(themes_dir_) / filename;
        if (!fs::exists(p)) {
            std::ofstream out(p.string());
            if (out.is_open()) {
                out << theme.to_css_string();
                out.close();
            }
        }
    };

    write_css_if_missing(StreamTheme::create_midnight_pink(), "Midnight_Pink.css");
    write_css_if_missing(StreamTheme::create_cyberpunk_neon(), "Cyberpunk_Neon.css");
    write_css_if_missing(StreamTheme::create_frost_glass(), "Frost_Glass.css");
    write_css_if_missing(StreamTheme::create_sunset_glow(), "Sunset_Glow.css");
    write_css_if_missing(StreamTheme::create_monochrome(), "Monochrome_Minimal.css");

    // Template CSS with full documentation
    fs::path template_css_path = fs::path(themes_dir_) / "template.css";
    if (!fs::exists(template_css_path)) {
        std::ofstream out(template_css_path.string());
        if (out.is_open()) {
            out << TEMPLATE_CSS_CONTENT;
            out.close();
        }
    }
}

/**
 * @brief Scans the themes directory for all .css and .streamtheme files and updates available_themes_.
 */
void ThemeManager::scan_themes_directory() {
    std::lock_guard<std::mutex> lock(mutex_);
    available_themes_.clear();

    try {
        // 1. Scan modern .css files (first class format)
        for (const auto& entry : fs::directory_iterator(themes_dir_)) {
            if (entry.is_regular_file() && entry.path().extension() == ".css") {
                if (entry.path().filename() == "template.css") continue; // Exclude template from selectable themes

                std::ifstream in(entry.path().string());
                if (in.is_open()) {
                    std::stringstream ss;
                    ss << in.rdbuf();
                    in.close();

                    StreamTheme t;
                    std::string err;
                    if (t.from_css_string(ss.str(), &err)) {
                        available_themes_.push_back(t);
                    }
                }
            }
        }

        // 2. Scan legacy .streamtheme files if any
        for (const auto& entry : fs::directory_iterator(themes_dir_)) {
            if (entry.is_regular_file() && entry.path().extension() == ".streamtheme") {
                if (entry.path().filename() == "template.streamtheme") continue;

                std::ifstream in(entry.path().string());
                if (in.is_open()) {
                    std::stringstream ss;
                    ss << in.rdbuf();
                    in.close();

                    StreamTheme t;
                    std::string err;
                    if (t.from_json_string(ss.str(), &err)) {
                        bool exists = false;
                        for (const auto& existing : available_themes_) {
                            if (existing.metadata.name == t.metadata.name) {
                                exists = true;
                                break;
                            }
                        }
                        if (!exists) {
                            available_themes_.push_back(t);
                        }
                    }
                }
            }
        }
    } catch (...) {}

    if (available_themes_.empty()) {
        available_themes_.push_back(StreamTheme::create_midnight_pink());
    }

    // Preserve active theme selection if found
    for (const auto& t : available_themes_) {
        if (t.metadata.name == active_theme_name_) {
            active_theme_ = t;
            return;
        }
    }
}

/**
 * @brief Sets the active theme by its display name.
 */
bool ThemeManager::set_active_theme_by_name(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& t : available_themes_) {
        if (t.metadata.name == name) {
            active_theme_ = t;
            active_theme_name_ = name;
            notify_listeners();
            return true;
        }
    }
    return false;
}

/**
 * @brief Sets the active theme directly and notifies listeners.
 */
void ThemeManager::set_active_theme(const StreamTheme& theme) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        active_theme_ = theme;
        active_theme_name_ = theme.metadata.name;
    }
    notify_listeners();
}

/**
 * @brief Imports a .css or .streamtheme file into the themes library.
 */
bool ThemeManager::import_theme_from_file(const std::string& file_path, std::string* out_error) {
    fs::path p(file_path);
    if (!fs::exists(p)) {
        if (out_error) *out_error = "File does not exist: " + file_path;
        return false;
    }

    std::ifstream in(file_path);
    if (!in.is_open()) {
        if (out_error) *out_error = "Failed to open file: " + file_path;
        return false;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    in.close();

    std::string content = buffer.str();
    StreamTheme imported_theme;
    bool success = false;

    if (p.extension() == ".css") {
        success = imported_theme.from_css_string(content, out_error);
    } else {
        success = imported_theme.from_json_string(content, out_error);
    }

    if (!success) return false;

    // Save as clean CSS in themes directory
    std::string safe_name = imported_theme.metadata.name;
    for (char& c : safe_name) {
        if (c == ' ' || c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '\"' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }

    fs::path dest = fs::path(themes_dir_) / (safe_name + ".css");
    std::ofstream out(dest.string());
    if (!out.is_open()) {
        if (out_error) *out_error = "Failed to write theme to: " + dest.string();
        return false;
    }

    out << imported_theme.to_css_string();
    out.close();

    scan_themes_directory();
    return true;
}

/**
 * @brief Exports a theme from the library to a chosen file destination (.css).
 */
bool ThemeManager::export_theme_to_file(const std::string& theme_name, const std::string& dest_path, std::string* out_error) {
    StreamTheme theme_to_export;
    bool found = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& t : available_themes_) {
            if (t.metadata.name == theme_name) {
                theme_to_export = t;
                found = true;
                break;
            }
        }
    }

    if (!found) {
        if (out_error) *out_error = "Theme not found: " + theme_name;
        return false;
    }

    std::ofstream out(dest_path);
    if (!out.is_open()) {
        if (out_error) *out_error = "Could not open destination file for writing";
        return false;
    }

    out << theme_to_export.to_css_string();
    out.close();
    return true;
}

/**
 * @brief Saves an edited or newly created theme directly as a CSS file.
 */
bool ThemeManager::save_custom_theme(const StreamTheme& theme, std::string* out_error) {
    std::string safe_name = theme.metadata.name;
    for (char& c : safe_name) {
        if (c == ' ' || c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '\"' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }

    fs::path dest = fs::path(themes_dir_) / (safe_name + ".css");
    std::ofstream out(dest.string());
    if (!out.is_open()) {
        if (out_error) *out_error = "Failed to write theme file";
        return false;
    }

    out << theme.to_css_string();
    out.close();

    scan_themes_directory();
    return true;
}

/**
 * @brief Duplicates an existing theme with a new name.
 */
bool ThemeManager::duplicate_theme(const std::string& original_name, const std::string& new_name, std::string* out_error) {
    StreamTheme copy;
    bool found = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& t : available_themes_) {
            if (t.metadata.name == original_name) {
                copy = t;
                found = true;
                break;
            }
        }
    }

    if (!found) {
        if (out_error) *out_error = "Original theme not found: " + original_name;
        return false;
    }

    copy.metadata.name = new_name;
    return save_custom_theme(copy, out_error);
}

/**
 * @brief Renames a theme in the library and updates its disk file.
 */
bool ThemeManager::rename_theme(const std::string& old_name, const std::string& new_name, std::string* out_error) {
    if (duplicate_theme(old_name, new_name, out_error)) {
        delete_theme(old_name, nullptr);
        if (active_theme_name_ == old_name) {
            set_active_theme_by_name(new_name);
        }
        return true;
    }
    return false;
}

/**
 * @brief Deletes a theme file from the themes directory.
 */
bool ThemeManager::delete_theme(const std::string& theme_name, std::string* out_error) {
    std::string safe_name = theme_name;
    for (char& c : safe_name) {
        if (c == ' ' || c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '\"' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }

    fs::path css_path = fs::path(themes_dir_) / (safe_name + ".css");
    fs::path streamtheme_path = fs::path(themes_dir_) / (safe_name + ".streamtheme");

    try {
        if (fs::exists(css_path)) fs::remove(css_path);
        if (fs::exists(streamtheme_path)) fs::remove(streamtheme_path);
    } catch (const std::exception& e) {
        if (out_error) *out_error = e.what();
        return false;
    }

    scan_themes_directory();
    return true;
}

void ThemeManager::add_change_listener(ThemeChangeCallback cb) {
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_.push_back(cb);
}

void ThemeManager::notify_listeners() {
    StreamTheme copy;
    std::vector<ThemeChangeCallback> list_copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        copy = active_theme_;
        list_copy = listeners_;
    }
    for (const auto& cb : list_copy) {
        if (cb) cb(copy);
    }
}

/**
 * @brief Background thread monitoring the themes directory with 150ms debouncing.
 */
void ThemeManager::start_file_watcher() {
    if (watcher_running_.load()) return;

    h_stop_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
    watcher_running_.store(true);

    watcher_thread_ = std::thread([this]() {
        HANDLE h_dir = CreateFileA(
            themes_dir_.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            NULL
        );

        if (h_dir == INVALID_HANDLE_VALUE) {
            watcher_running_.store(false);
            return;
        }

        BYTE buffer[1024];
        DWORD bytes_returned = 0;

        while (watcher_running_.load()) {
            if (ReadDirectoryChangesW(
                h_dir, buffer, sizeof(buffer), FALSE,
                FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
                &bytes_returned, NULL, NULL
            )) {
                // 150ms debounce
                std::this_thread::sleep_for(std::chrono::milliseconds(150));

                if (!watcher_running_.load()) break;

                // Non-destructive reload of active theme
                scan_themes_directory();
                notify_listeners();
            } else {
                break;
            }
        }

        CloseHandle(h_dir);
        watcher_running_.store(false);
    });
}

void ThemeManager::stop_file_watcher() {
    if (watcher_running_.load()) {
        watcher_running_.store(false);
        if (h_stop_event_) {
            SetEvent(h_stop_event_);
        }
        if (watcher_thread_.joinable()) {
            watcher_thread_.join();
        }
        if (h_stop_event_) {
            CloseHandle(h_stop_event_);
            h_stop_event_ = NULL;
        }
    }
}
