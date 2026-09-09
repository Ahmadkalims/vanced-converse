#include "Theme.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

ThemeColor ThemeColor::from_hex(const std::string& hex, uint8_t alpha) {
    ThemeColor c;
    c.a = alpha;
    std::string clean = hex;
    if (!clean.empty() && clean[0] == '#') clean = clean.substr(1);

    if (clean.size() == 6) {
        try {
            unsigned long val = std::stoul(clean, nullptr, 16);
            c.r = static_cast<uint8_t>((val >> 16) & 0xFF);
            c.g = static_cast<uint8_t>((val >> 8) & 0xFF);
            c.b = static_cast<uint8_t>(val & 0xFF);
        } catch (...) {}
    } else if (clean.size() == 8) {
        try {
            unsigned long val = std::stoul(clean, nullptr, 16);
            c.a = static_cast<uint8_t>((val >> 24) & 0xFF);
            c.r = static_cast<uint8_t>((val >> 16) & 0xFF);
            c.g = static_cast<uint8_t>((val >> 8) & 0xFF);
            c.b = static_cast<uint8_t>(val & 0xFF);
        } catch (...) {}
    }
    return c;
}

std::string ThemeColor::to_hex() const {
    std::ostringstream ss;
    ss << "#" << std::hex << std::setfill('0')
       << std::setw(2) << (int)r
       << std::setw(2) << (int)g
       << std::setw(2) << (int)b;
    return ss.str();
}

std::string Theme::blur_type_to_string(BlurType type) {
    switch (type) {
        case BlurType::Acrylic: return "Acrylic";
        case BlurType::Mica: return "Mica";
        case BlurType::LiquidGlass: return "Liquid Glass";
        case BlurType::FlatTranslucent: return "Flat Translucent";
    }
    return "Acrylic";
}

BlurType Theme::string_to_blur_type(const std::string& s) {
    if (s == "Mica") return BlurType::Mica;
    if (s == "Liquid Glass" || s == "LiquidGlass") return BlurType::LiquidGlass;
    if (s == "Flat Translucent" || s == "Flat") return BlurType::FlatTranslucent;
    return BlurType::Acrylic;
}

std::string Theme::preset_to_string(ThemePreset p) {
    switch (p) {
        case ThemePreset::MidnightAcrylic: return "Midnight Acrylic";
        case ThemePreset::CyberpunkNeon: return "Cyberpunk Neon";
        case ThemePreset::FrostGlass: return "Frost Glass";
        case ThemePreset::SunsetGlow: return "Sunset Glow";
        case ThemePreset::MonochromeMinimal: return "Monochrome Minimal";
        case ThemePreset::Custom: return "Custom";
    }
    return "Midnight Acrylic";
}

ThemePreset Theme::string_to_preset(const std::string& s) {
    if (s == "Cyberpunk Neon") return ThemePreset::CyberpunkNeon;
    if (s == "Frost Glass") return ThemePreset::FrostGlass;
    if (s == "Sunset Glow") return ThemePreset::SunsetGlow;
    if (s == "Monochrome Minimal") return ThemePreset::MonochromeMinimal;
    if (s == "Custom") return ThemePreset::Custom;
    return ThemePreset::MidnightAcrylic;
}

Theme Theme::get_preset(ThemePreset preset) {
    Theme t;
    t.preset = preset;
    t.name = preset_to_string(preset);

    // Default glow palette
    t.glow_palette = {
        { ThemeColor::from_hex("#38bdf8"), ThemeColor::from_hex("#38bdf8", 180) }, // Sky Blue
        { ThemeColor::from_hex("#34d399"), ThemeColor::from_hex("#34d399", 180) }, // Emerald Green
        { ThemeColor::from_hex("#f472b6"), ThemeColor::from_hex("#f472b6", 180) }, // Rose Pink
        { ThemeColor::from_hex("#fb923c"), ThemeColor::from_hex("#fb923c", 180) }, // Orange
        { ThemeColor::from_hex("#c084fc"), ThemeColor::from_hex("#c084fc", 180) }  // Purple
    };

    switch (preset) {
        case ThemePreset::MidnightAcrylic:
            t.blur_type = BlurType::Acrylic;
            t.blur_radius = 18;
            t.bg_opacity = 45;
            t.bg_overlay = {115, 18, 18, 28};
            t.card_bg = {25, 255, 255, 255};
            t.text_primary = {255, 243, 244, 246};
            t.accent_highlight = ThemeColor::from_hex("#f472b6");
            t.accent_highlight_bg = {45, 244, 114, 182};
            t.glass_border = {50, 255, 255, 255};
            t.font_family = "Montserrat Alternates";
            t.font_size = 15;
            break;

        case ThemePreset::CyberpunkNeon:
            t.blur_type = BlurType::LiquidGlass;
            t.blur_radius = 24;
            t.bg_opacity = 60;
            t.bg_overlay = {150, 15, 8, 30};
            t.card_bg = {35, 255, 0, 128};
            t.text_primary = {255, 230, 255, 255};
            t.accent_highlight = ThemeColor::from_hex("#00f0ff");
            t.accent_highlight_bg = {60, 0, 240, 255};
            t.glass_border = {90, 0, 240, 255};
            t.font_family = "Segoe UI";
            t.font_size = 15;
            t.glow_palette = {
                { ThemeColor::from_hex("#00f0ff"), ThemeColor::from_hex("#00f0ff", 220) },
                { ThemeColor::from_hex("#ff007f"), ThemeColor::from_hex("#ff007f", 220) },
                { ThemeColor::from_hex("#ffe600"), ThemeColor::from_hex("#ffe600", 220) },
                { ThemeColor::from_hex("#00ff66"), ThemeColor::from_hex("#00ff66", 220) },
                { ThemeColor::from_hex("#b026ff"), ThemeColor::from_hex("#b026ff", 220) }
            };
            break;

        case ThemePreset::FrostGlass:
            t.blur_type = BlurType::Mica;
            t.blur_radius = 22;
            t.bg_opacity = 50;
            t.bg_overlay = {125, 220, 235, 245};
            t.card_bg = {40, 255, 255, 255};
            t.text_primary = {255, 20, 30, 45};
            t.text_secondary = {255, 80, 100, 120};
            t.accent_highlight = ThemeColor::from_hex("#0284c7");
            t.accent_highlight_bg = {50, 2, 132, 199};
            t.glass_border = {110, 255, 255, 255};
            t.font_family = "Segoe UI";
            t.font_size = 15;
            t.glow_palette = {
                { ThemeColor::from_hex("#0284c7"), ThemeColor::from_hex("#0284c7", 180) },
                { ThemeColor::from_hex("#059669"), ThemeColor::from_hex("#059669", 180) },
                { ThemeColor::from_hex("#d946ef"), ThemeColor::from_hex("#d946ef", 180) },
                { ThemeColor::from_hex("#ea580c"), ThemeColor::from_hex("#ea580c", 180) },
                { ThemeColor::from_hex("#7c3aed"), ThemeColor::from_hex("#7c3aed", 180) }
            };
            break;

        case ThemePreset::SunsetGlow:
            t.blur_type = BlurType::LiquidGlass;
            t.blur_radius = 20;
            t.bg_opacity = 55;
            t.bg_overlay = {140, 35, 18, 12};
            t.card_bg = {35, 255, 180, 120};
            t.text_primary = {255, 254, 243, 199};
            t.accent_highlight = ThemeColor::from_hex("#fb923c");
            t.accent_highlight_bg = {55, 251, 146, 60};
            t.glass_border = {70, 255, 200, 150};
            t.font_family = "Segoe UI";
            t.font_size = 15;
            break;

        case ThemePreset::MonochromeMinimal:
            t.blur_type = BlurType::FlatTranslucent;
            t.blur_radius = 0;
            t.bg_opacity = 65;
            t.bg_overlay = {165, 12, 12, 14};
            t.card_bg = {30, 255, 255, 255};
            t.text_primary = {255, 255, 255, 255};
            t.text_secondary = {255, 160, 160, 160};
            t.accent_highlight = ThemeColor::from_hex("#ffffff");
            t.accent_highlight_bg = {40, 255, 255, 255};
            t.glass_border = {45, 255, 255, 255};
            t.font_family = "Consolas";
            t.font_size = 14;
            t.glow_palette = {
                { ThemeColor::from_hex("#ffffff"), ThemeColor::from_hex("#ffffff", 180) },
                { ThemeColor::from_hex("#d4d4d8"), ThemeColor::from_hex("#d4d4d8", 180) },
                { ThemeColor::from_hex("#a1a1aa"), ThemeColor::from_hex("#a1a1aa", 180) },
                { ThemeColor::from_hex("#e4e4e7"), ThemeColor::from_hex("#e4e4e7", 180) },
                { ThemeColor::from_hex("#f4f4f5"), ThemeColor::from_hex("#f4f4f5", 180) }
            };
            break;

        case ThemePreset::Custom:
            // keeps default modified custom values
            break;
    }

    return t;
}
