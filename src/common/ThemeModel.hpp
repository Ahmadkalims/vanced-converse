#pragma once

#include "SimpleJson.hpp"
#include <string>
#include <vector>
#include <map>
#include <windows.h>
#include <gdiplus.h>
#include <cstdint>

/**
 * @brief Represents 4-side spacing (margin or padding) in pixels.
 */
struct BoxSpacing {
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
    float left = 0.0f;

    json::Value to_json() const {
        json::Value obj = json::Value::object();
        obj["top"] = (double)top;
        obj["right"] = (double)right;
        obj["bottom"] = (double)bottom;
        obj["left"] = (double)left;
        return obj;
    }

    void from_json(const json::Value& v) {
        if (!v.is_object()) return;
        top = (float)v["top"].as_double(top);
        right = (float)v["right"].as_double(right);
        bottom = (float)v["bottom"].as_double(bottom);
        left = (float)v["left"].as_double(left);
    }
};

/**
 * @brief Represents an RGBA 32-bit color with hex and css parsing/formatting helpers.
 */
struct ColorRGBA {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    ColorRGBA() : r(255), g(255), b(255), a(255) {}
    ColorRGBA(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255)
        : r(r_), g(g_), b(b_), a(a_) {}

    static ColorRGBA parse(const std::string& str, ColorRGBA fallback);
    static ColorRGBA parse(const std::string& str) { return parse(str, ColorRGBA(255, 255, 255, 255)); }
    std::string to_hex_string() const;
    std::string to_rgba_string() const;
    Gdiplus::Color to_gdiplus(float opacity_multiplier = 1.0f) const;
};

/**
 * @brief Author and version metadata for themes.
 */
struct ThemeMetadata {
    std::string name = "Midnight Pink";
    std::string author = "User";
    int version = 1;
    std::string description = "Dark acrylic overlay with pink highlights";

    json::Value to_json() const;
    void from_json(const json::Value& v);
};

/**
 * @brief Overlay container background material types.
 */
enum class OverlayBgType {
    Acrylic,
    Mica,
    LiquidGlass,
    Solid,
    Transparent,
    LinearGradient,
    RadialGradient,
    Image
};

/**
 * @brief Single gradient color stop.
 */
struct GradientStop {
    float offset = 0.0f;
    ColorRGBA color;
};

/**
 * @brief Configuration for the overlay window background, material, padding, and edge fading.
 */
struct OverlayBackgroundConfig {
    OverlayBgType type = OverlayBgType::Acrylic;
    std::string color = "#14141E";
    float opacity = 0.4f;

    bool gradient_enabled = false;
    std::string gradient_type = "linear";
    float gradient_angle = 90.0f;
    std::vector<GradientStop> gradient_stops;

    bool image_enabled = false;
    std::string image_path = "";
    std::string image_fit = "cover";
    float image_opacity = 1.0f;

    std::string effect_type = "acrylic";
    float blur = 15.0f;
    float saturation = 120.0f;

    bool outline_enabled = false;
    float outline_size = 0.0f;
    std::string outline_type = "solid";
    std::string outline_color = "#FFFFFF";
    std::string outline_glow_color = "#FFFFFF";
    float outline_glow_blur = 0.0f;

    float corner_radius = 20.0f;

    // Viewport Padding & Edge Fading
    BoxSpacing padding = { 16.0f, 16.0f, 16.0f, 16.0f };
    bool fade_edges = true;
    float fade_length = 32.0f;

    json::Value to_json() const;
    void from_json(const json::Value& v);
};

enum class ScrollbarInteractionMode {
    Always,
    ClickthroughOffHover,
    Disabled
};

/**
 * @brief Configuration for chat message scrollbar.
 */
struct ScrollbarConfig {
    bool visible = false;
    ScrollbarInteractionMode interaction_mode = ScrollbarInteractionMode::ClickthroughOffHover;
    float width = 8.0f;
    std::string color = "#F9A8D4";
    std::string type = "solid";
    std::string glow_color = "#F9A8D4";
    float corner_radius = 4.0f;

    json::Value to_json() const;
    void from_json(const json::Value& v);
};

enum class UsernamePlacement {
    BeforeMessage,
    AboveMessage
};

enum class UsernameColorMode {
    Single,
    RandomPalette,
    PersistentUserColor
};

/**
 * @brief Typography configuration (font family, size, weight).
 */
struct FontConfig {
    std::string family = "Montserrat Alternates";
    float size = 16.0f;
    int weight = 400;

    json::Value to_json() const;
    void from_json(const json::Value& v);
};

/**
 * @brief Text shadow & glow parameters (CSS text-shadow mapped to glow radius/intensity).
 */
struct TextEffectConfig {
    std::string type = "solid"; // "solid", "glow"
    std::string shadow = "";    // Raw CSS shadow e.g. "0 0 4px rgba(249, 168, 212, 0.7)"
    float glow_radius = 4.0f;
    float glow_intensity = 0.8f;
    std::string glow_color = "#F9A8D4";

    json::Value to_json() const;
    void from_json(const json::Value& v);
};

/**
 * @brief Username visual layout and typography.
 */
struct UsernameConfig {
    UsernamePlacement placement = UsernamePlacement::BeforeMessage;
    FontConfig font = { "Montserrat Alternates", 16.0f, 600 };
    
    UsernameColorMode color_mode = UsernameColorMode::PersistentUserColor;
    std::string color_value = "#F9A8D4";
    std::vector<std::string> palette = {
        "#F9A8D4", "#F472B6", "#38BDF8", "#34D399", "#A78BFA", "#FBBF24", "#FB7185", "#4ADE80"
    };

    TextEffectConfig effect = { "glow", "0 0 4px rgba(249, 168, 212, 0.7)", 4.0f, 0.7f, "#F9A8D4" };
    float line_spacing = 0.0f;
    BoxSpacing padding = { 0, 0, 0, 0 };
    BoxSpacing margin = { 0, 0, 0, 0 };

    json::Value to_json() const;
    void from_json(const json::Value& v);
};

/**
 * @brief Chat message text styling.
 */
struct MessageTextConfig {
    FontConfig font = { "Montserrat Alternates", 16.0f, 400 };
    std::string color = "#E5E7EB";
    TextEffectConfig effect = { "solid", "", 0.0f, 0.0f, "" };
    float line_spacing = 0.0f;
    BoxSpacing padding = { 0, 0, 0, 0 };
    BoxSpacing margin = { 0, 0, 0, 0 };

    json::Value to_json() const;
    void from_json(const json::Value& v);
};

/**
 * @brief Outline and border styling.
 */
struct OutlineConfig {
    bool enabled = false;
    float size = 0.0f;
    std::string type = "solid";
    std::string color = "#FFFFFF";
    std::string glow_color = "#FFFFFF";

    json::Value to_json() const;
    void from_json(const json::Value& v);
};

/**
 * @brief Message card bubble styling.
 */
struct MessageBackgroundConfig {
    bool enabled = true;
    std::string color = "rgba(255, 255, 255, 0.02)";
    OutlineConfig outline;
    float corner_radius = 15.0f;
    BoxSpacing padding = { 12.0f, 16.0f, 12.0f, 16.0f };
    BoxSpacing margin = { 10.0f, 0.0f, 10.0f, 0.0f };

    json::Value to_json() const;
    void from_json(const json::Value& v);
};

/**
 * @brief Event trigger condition (e.g. contains '?' or membership).
 */
struct SpecialEventTrigger {
    std::string type = "contains_character";
    std::string value = "?";

    json::Value to_json() const;
    void from_json(const json::Value& v);
};

/**
 * @brief Visual highlight styling when an event is triggered.
 */
struct SpecialEventHighlight {
    std::string background_color = "rgba(249, 168, 212, 0.12)";
    OutlineConfig outline = { true, 2.0f, "solid", "#F472B6", "#F472B6" };
    float corner_radius = 15.0f;

    json::Value to_json() const;
    void from_json(const json::Value& v);
};

/**
 * @brief Complete special event configuration.
 */
struct SpecialEventConfig {
    std::string id = "question";
    std::string name = "Question";
    bool enabled = true;
    SpecialEventTrigger trigger = { "contains_character", "?" };
    SpecialEventHighlight highlight;

    json::Value to_json() const;
    void from_json(const json::Value& v);
};

/**
 * @brief Complete theme definition for Vanced Converse.
 * Fully bidirectional with CSS (--vc-*) and JSON formats.
 */
class StreamTheme {
public:
    ThemeMetadata metadata;
    OverlayBackgroundConfig overlay;
    ScrollbarConfig scrollbar;
    UsernameConfig username;
    MessageTextConfig message_text;
    MessageBackgroundConfig message_background;
    std::vector<SpecialEventConfig> special_events;

    // Motion parameters
    std::string motion_duration = "300ms";
    std::string motion_ease = "cubic-bezier(0, 0, 0.2, 1)";

    // Preservation of custom CSS variables not recognized by standard schema
    std::map<std::string, std::string> custom_css_vars;

    StreamTheme() = default;

    /**
     * @brief Serializes theme to JSON string.
     */
    std::string to_json_string(bool pretty = true) const;

    /**
     * @brief Deserializes theme from JSON string.
     */
    bool from_json_string(const std::string& json_str, std::string* error_msg = nullptr);

    /**
     * @brief Generates complete clean CSS theme format.
     */
    std::string to_css_string() const;

    /**
     * @brief Parses theme directly from CSS variables (--vc-*).
     */
    bool from_css_string(const std::string& css_str, std::string* error_msg = nullptr);

    /**
     * @brief Validates and sanitizes values into safe ranges.
     */
    bool validate(std::vector<std::string>* warnings = nullptr);

    /**
     * @brief Resolves persistent or randomized username color.
     */
    std::string get_user_color(const std::string& author_id, const std::string& author_name) const;

    /**
     * @brief Checks if a chat message triggers an active special event highlight.
     */
    bool is_special_event_triggered(const std::string& event_id, const std::wstring& msg_text) const;

    // Built-in theme presets
    static StreamTheme create_midnight_pink();
    static StreamTheme create_cyberpunk_neon();
    static StreamTheme create_frost_glass();
    static StreamTheme create_sunset_glow();
    static StreamTheme create_monochrome();
};
