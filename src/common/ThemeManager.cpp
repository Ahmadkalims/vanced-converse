#include "ThemeManager.hpp"
#include "UnicodeUtils.hpp"
#include "SimpleJson.hpp"
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

ThemeManager g_theme_manager;

static const char* TEMPLATE_STREAMTHEME_CONTENT = R"RAW(// ==============================================================================
//  STREAMTHEME - COMPLETE CUSTOMIZATION TEMPLATE & CONFIGURATION GUIDE
// ==============================================================================
// This template demonstrates all available settings in the StreamTheme engine.
// You can edit this file directly or copy and rename it to "YourTheme.streamtheme".
//
// INSTANT HOT-RELOAD: Whenever you edit and save this file, the overlay updates
// live immediately on screen without restarting the app or using background CPU.
// ==============================================================================

{
  "schema_version": "1.0",

  // ----------------------------------------------------------------------------
  // THEME METADATA
  // ----------------------------------------------------------------------------
  "metadata": {
    "name": "Custom Stream Theme Template",
    "author": "Streamer / Designer",
    "description": "Fully commented template explaining all theme options and values.",
    "version": "1.0.0"
  },

  // ----------------------------------------------------------------------------
  // OVERLAY CONTAINER (Backdrop Material, Glass, Acrylic, Border, Radii)
  // ----------------------------------------------------------------------------
  "overlay": {
    // Backdrop Type:
    //  - "acrylic"     : Windows 11 Fluent Acrylic blur (saturate + noise + blur)
    //  - "mica"        : Windows 11 Mica dynamic desktop-tinted material
    //  - "liquid_glass": Deep glass morphism with specular glow highlight
    //  - "transparent" : 100% transparent container (only message cards visible)
    //  - "solid"       : Flat solid background color
    "type": "acrylic",

    // Container Background Tint:
    // Supports Hex (#RRGGBB, #RRGGBBAA), rgb(r,g,b), or rgba(r,g,b,a)
    "color": "rgba(20, 20, 30, 0.45)",

    // Master Opacity: 0.0 (fully transparent) to 1.0 (fully opaque)
    "opacity": 0.45,

    // Container Corner Radius in pixels (e.g. 0 for sharp, 20 for rounded)
    "corner_radius": 20.0,

    // Outer Container Border / Specular Rim
    "outline_enabled": true,
    "outline_color": "rgba(255, 255, 255, 0.15)",
    "outline_size": 1.5,

    // Inner Padding around chat messages (in pixels)
    "padding": {
      "top": 16.0,
      "bottom": 16.0,
      "left": 16.0,
      "right": 16.0
    }
  },

  // ----------------------------------------------------------------------------
  // MESSAGE CARD (Background pill/card styling for each chat message)
  // ----------------------------------------------------------------------------
  "message_background": {
    // Enable/disable individual message card bubbles (false = clean floating text)
    "enabled": true,

    // Card Background Color & Opacity
    "color": "rgba(255, 255, 255, 0.03)",

    // Card Corner Curvature (in pixels)
    "corner_radius": 15.0,

    // Message Card Border Outline
    "outline": {
      "enabled": false,
      "color": "rgba(255, 255, 255, 0.10)",
      "size": 1.0
    },

    // Card Inner Padding
    "padding": {
      "top": 12.0,
      "bottom": 12.0,
      "left": 16.0,
      "right": 16.0
    },

    // Card Outer Margins (Spacing between consecutive messages)
    "margin": {
      "top": 6.0,
      "bottom": 6.0,
      "left": 0.0,
      "right": 0.0
    }
  },

  // ----------------------------------------------------------------------------
  // USERNAME STYLING
  // ----------------------------------------------------------------------------
  "username": {
    // Typography
    "font": {
      "family": "Montserrat Alternates",  // Primary font (falls back to Segoe UI)
      "size": 16.0,                       // Font size in pixels (10 to 48)
      "weight": 600                       // 400 = Regular, 600 = SemiBold, 700 = Bold
    },

    // Placement: "inline" (Name: Message) or "above_message" (Name on top line)
    "placement": "inline",

    // Username Visual Effect:
    //  - "none" : Clean flat text
    //  - "glow" : Soft neon aura matching the user's color
    "effect": {
      "type": "glow",
      "radius": 4.0,
      "color": "rgba(249, 168, 212, 0.70)"
    },

    // User Color Assignment Mode:
    //  - "custom_palette" : Deterministic hash assigns distinct colors from list
    //  - "author_color"   : Uses YouTube's official user role color
    //  - "static"         : All usernames use the first color in palette
    "color_mode": "custom_palette",

    // Color Palette for randomized/assigned usernames
    "palette": [
      "#f9a8d4",  // Rose Pink
      "#38bdf8",  // Sky Blue
      "#a78bfa",  // Violet
      "#f472b6",  // Magenta
      "#4ade80",  // Neon Green
      "#fbbf24"   // Amber Gold
    ]
  },

  // ----------------------------------------------------------------------------
  // MESSAGE TEXT STYLING
  // ----------------------------------------------------------------------------
  "message_text": {
    "font": {
      "family": "Montserrat Alternates",  // Primary text font
      "size": 16.0,                       // Text size in pixels
      "weight": 400                       // 400 = Regular, 500 = Medium, 700 = Bold
    },

    // Primary Text Color
    "color": "#e5e7eb",

    // Extra line spacing (in pixels)
    "line_spacing": 4.0,

    // Text Glow Effect ("none" or "glow")
    "effect": {
      "type": "none",
      "radius": 0.0,
      "color": "rgba(0, 0, 0, 0)"
    }
  },

  // ----------------------------------------------------------------------------
  // SCROLLBAR STYLING
  // ----------------------------------------------------------------------------
  "scrollbar": {
    "visible": false,                   // Always visible (true) or auto-hide (false)
    "width": 6.0,                       // Scrollbar width in pixels
    "track_color": "rgba(255, 255, 255, 0.05)",
    "thumb_color": "rgba(244, 114, 182, 0.40)",
    "thumb_hover_color": "rgba(244, 114, 182, 0.80)",
    "corner_radius": 3.0,
    // Interaction Mode: "auto", "always_visible", or "disabled"
    "interaction_mode": "auto"
  },

  // ----------------------------------------------------------------------------
  // ANIMATION EFFECTS
  // ----------------------------------------------------------------------------
  "animation": {
    // Message Entrance Animation:
    //  - "slide_up"   : Slides up smoothly into view
    //  - "slide_down" : Slides down into view
    //  - "fade_in"    : Fades in smoothly
    //  - "none"       : Instant appearance
    "entrance": "slide_up",
    "duration_ms": 300,

    // Auto-Scroll Behavior: "smooth", "instant", or "off"
    "scroll_behavior": "smooth"
  },

  // ----------------------------------------------------------------------------
  // SPECIAL EVENT HIGHLIGHTS (Questions, Superchats, Member alerts)
  // ----------------------------------------------------------------------------
  "special_events": [
    {
      "trigger": "question",  // Highlights messages ending with '?'
      "highlight": {
        "background_color": "rgba(249, 168, 212, 0.12)",
        "outline": {
          "enabled": true,
          "color": "#f472b6",
          "size": 2.0
        }
      }
    }
  ]
}
)RAW";

static fs::path resolve_primary_themes_dir() {
    char exe_path[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    fs::path exe_dir = fs::path(exe_path).parent_path();
    fs::path p_exe_themes = exe_dir / "themes";

    // If themes exists beside exe, use it
    if (fs::exists(p_exe_themes)) return p_exe_themes;

    // Check current working directory ./themes
    if (fs::exists("themes")) return fs::absolute("themes");

    // Otherwise default to exe_dir / "themes"
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

void ThemeManager::init() {
    try {
        fs::create_directories(themes_dir_);
    } catch (...) {}

    ensure_default_themes();
    scan_themes_directory();
    start_file_watcher();
}

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

    // Also write template.css
    fs::path template_css_path = fs::path(themes_dir_) / "template.css";
    if (!fs::exists(template_css_path)) {
        std::ofstream out(template_css_path.string());
        if (out.is_open()) {
            out << StreamTheme::create_midnight_pink().to_css_string();
            out.close();
        }
    }
}

void ThemeManager::scan_themes_directory() {
    std::lock_guard<std::mutex> lock(mutex_);
    available_themes_.clear();

    try {
        // 1. Scan .css files (first-class)
        for (const auto& entry : fs::directory_iterator(themes_dir_)) {
            if (entry.is_regular_file() && entry.path().extension() == ".css") {
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
                std::ifstream in(entry.path().string());
                if (in.is_open()) {
                    std::stringstream ss;
                    ss << in.rdbuf();
                    in.close();

                    StreamTheme t;
                    std::string err;
                    if (t.from_json_string(ss.str(), &err)) {
                        // Check if theme with same name already added
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

    // Set active if found by name, else fallback
    bool found = false;
    for (const auto& t : available_themes_) {
        if (t.metadata.name == active_theme_name_) {
            active_theme_ = t;
            found = true;
            break;
        }
    }

    if (!found && !available_themes_.empty()) {
        active_theme_ = available_themes_[0];
        active_theme_name_ = active_theme_.metadata.name;
    }
}


bool ThemeManager::set_active_theme_by_name(const std::string& name) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        bool found = false;

        auto normalize = [](std::string s) -> std::string {
            std::string out;
            for (char c : s) {
                if (c == '_' || c == '-' || c == ' ') continue;
                out.push_back((char)::tolower((unsigned char)c));
            }
            if (out.size() >= 4 && out.substr(out.size() - 4) == ".css") {
                out = out.substr(0, out.size() - 4);
            }
            return out;
        };

        std::string target = normalize(name);

        for (const auto& t : available_themes_) {
            if (t.metadata.name == name || normalize(t.metadata.name) == target) {
                active_theme_ = t;
                active_theme_name_ = t.metadata.name;
                found = true;
                break;
            }
        }
        if (!found) return false;
    }

    notify_listeners();
    return true;
}

void ThemeManager::set_active_theme(const StreamTheme& theme) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        active_theme_ = theme;
        active_theme_name_ = theme.metadata.name;
    }
    notify_listeners();
}

bool ThemeManager::import_theme_from_file(const std::string& file_path, std::string* out_error) {
    std::ifstream in(file_path);
    if (!in.is_open()) {
        if (out_error) *out_error = "Could not open source file: " + file_path;
        return false;
    }

    std::stringstream ss;
    ss << in.rdbuf();
    in.close();

    std::string content = ss.str();
    StreamTheme imported_theme;
    bool parsed = false;

    if (file_path.ends_with(".css") || content.find("--vc-") != std::string::npos || content.find(":root") != std::string::npos) {
        parsed = imported_theme.from_css_string(content, out_error);
    } else {
        parsed = imported_theme.from_json_string(content, out_error);
    }

    if (!parsed) {
        return false;
    }

    // Save/copy into themes_dir_ as .css
    std::string safe_name = imported_theme.metadata.name.empty() ? "Imported Theme" : imported_theme.metadata.name;
    for (char& c : safe_name) {
        if (c == '\\' || c == '/' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }

    fs::path dest = fs::path(themes_dir_) / (safe_name + ".css");
    try {
        std::ofstream out(dest.string());
        if (out.is_open()) {
            out << imported_theme.to_css_string();
            out.close();
        }
    } catch (const std::exception& e) {
        if (out_error) *out_error = std::string("Failed to save imported theme: ") + e.what();
        return false;
    }

    scan_themes_directory();
    set_active_theme_by_name(imported_theme.metadata.name);
    return true;
}

bool ThemeManager::export_theme_to_file(const std::string& theme_name, const std::string& dest_path, std::string* out_error) {
    StreamTheme target;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        bool found = false;
        for (const auto& t : available_themes_) {
            if (t.metadata.name == theme_name) {
                target = t;
                found = true;
                break;
            }
        }
        if (!found) {
            target = active_theme_;
        }
    }

    try {
        std::ofstream out(dest_path);
        if (!out.is_open()) {
            if (out_error) *out_error = "Could not create destination file: " + dest_path;
            return false;
        }
        if (dest_path.ends_with(".css")) {
            out << target.to_css_string();
        } else {
            out << target.to_json_string(true);
        }
        out.close();
        return true;
    } catch (const std::exception& e) {
        if (out_error) *out_error = std::string("Export failed: ") + e.what();
        return false;
    }
}

bool ThemeManager::save_custom_theme(const StreamTheme& theme, std::string* out_error) {
    std::string safe_name = theme.metadata.name.empty() ? "Custom Theme" : theme.metadata.name;
    for (char& c : safe_name) {
        if (c == '\\' || c == '/' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }

    fs::path dest = fs::path(themes_dir_) / (safe_name + ".css");
    try {
        std::ofstream out(dest.string());
        if (!out.is_open()) {
            if (out_error) *out_error = "Could not open destination file: " + dest.string();
            return false;
        }
        out << theme.to_css_string();
        out.close();
    } catch (const std::exception& e) {
        if (out_error) *out_error = std::string("Save failed: ") + e.what();
        return false;
    }

    scan_themes_directory();
    set_active_theme_by_name(theme.metadata.name);
    return true;
}


void ThemeManager::add_change_listener(ThemeChangeCallback cb) {
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_.push_back(cb);
}

void ThemeManager::notify_listeners() {
    StreamTheme active_copy;
    std::vector<ThemeChangeCallback> listeners_copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        active_copy = active_theme_;
        listeners_copy = listeners_;
    }

    for (const auto& cb : listeners_copy) {
        if (cb) cb(active_copy);
    }
}

void ThemeManager::start_file_watcher() {
    if (watcher_running_) return;

    h_stop_event_ = CreateEventW(NULL, TRUE, FALSE, NULL);
    watcher_running_ = true;

    watcher_thread_ = std::thread([this]() {
        std::wstring w_dir = unicode::utf8_to_utf16(themes_dir_);
        HANDLE h_change = FindFirstChangeNotificationW(
            w_dir.c_str(),
            FALSE,
            FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME
        );

        if (h_change == INVALID_HANDLE_VALUE) {
            watcher_running_ = false;
            return;
        }

        HANDLE handles[2] = { h_stop_event_, h_change };

        while (watcher_running_) {
            // Kernel wait state: 0% CPU consumption
            DWORD wait_res = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
            if (wait_res == WAIT_OBJECT_0) {
                break; // Stop event
            }
            if (wait_res == WAIT_OBJECT_0 + 1) {
                // Debounce 100ms for file write to finish
                Sleep(100);
                scan_themes_directory();
                notify_listeners();
                FindNextChangeNotification(h_change);
            }
        }

        FindCloseChangeNotification(h_change);
    });
}

void ThemeManager::stop_file_watcher() {
    if (!watcher_running_) return;
    watcher_running_ = false;

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
