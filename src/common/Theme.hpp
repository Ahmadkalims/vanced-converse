#pragma once

#include <string>
#include <vector>
#include <cstdint>

enum class BlurType {
    Acrylic = 0,
    Mica = 1,
    LiquidGlass = 2,
    FlatTranslucent = 3
};

enum class ThemePreset {
    MidnightAcrylic = 0,
    CyberpunkNeon = 1,
    FrostGlass = 2,
    SunsetGlow = 3,
    MonochromeMinimal = 4,
    Custom = 5
};

struct ThemeColor {
    uint8_t a = 255;
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;

    uint32_t to_argb() const {
        return (static_cast<uint32_t>(a) << 24) |
               (static_cast<uint32_t>(r) << 16) |
               (static_cast<uint32_t>(g) << 8)  |
               static_cast<uint32_t>(b);
    }

    static ThemeColor from_hex(const std::string& hex, uint8_t alpha = 255);
    std::string to_hex() const;
};

struct AuthorGlow {
    ThemeColor color;
    ThemeColor shadow;
};

struct Theme {
    ThemePreset preset = ThemePreset::MidnightAcrylic;
    std::string name = "Midnight Acrylic";
    
    // Blur and Material
    BlurType blur_type = BlurType::Acrylic;
    int blur_radius = 16;           // 0 to 40 px
    int bg_opacity = 40;            // 0% to 100%
    
    // Colors
    ThemeColor bg_overlay = {100, 20, 20, 30};      // main overlay surface
    ThemeColor card_bg = {20, 255, 255, 255};       // message bubble background
    ThemeColor text_primary = {255, 243, 244, 246}; // message text
    ThemeColor text_secondary = {255, 156, 163, 175};
    ThemeColor accent_highlight = {255, 244, 114, 182};     // #question border
    ThemeColor accent_highlight_bg = {40, 244, 114, 182};  // #question card background
    ThemeColor glass_border = {60, 255, 255, 255};         // liquid glass border rim
    
    // Typography
    std::string font_family = "Montserrat Alternates";
    int font_size = 15;             // base message size
    int author_font_size = 14;      // username size
    bool font_bold_author = true;
    
    // Customization
    std::string highlight_prefix = "#question";
    std::vector<AuthorGlow> glow_palette;

    static Theme get_preset(ThemePreset preset);
    static std::string blur_type_to_string(BlurType type);
    static BlurType string_to_blur_type(const std::string& s);
    static std::string preset_to_string(ThemePreset p);
    static ThemePreset string_to_preset(const std::string& s);
};
