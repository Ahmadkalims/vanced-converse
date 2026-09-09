#include "../../src/common/Theme.hpp"
#include <cassert>
#include <iostream>

void test_theme() {
    // 1. Color from hex
    ThemeColor c = ThemeColor::from_hex("#38bdf8");
    assert(c.r == 0x38);
    assert(c.g == 0xbd);
    assert(c.b == 0xf8);
    assert(c.to_hex() == "#38bdf8");

    // 2. Blur type conversions
    assert(Theme::blur_type_to_string(BlurType::Acrylic) == "Acrylic");
    assert(Theme::blur_type_to_string(BlurType::Mica) == "Mica");
    assert(Theme::blur_type_to_string(BlurType::LiquidGlass) == "Liquid Glass");
    assert(Theme::string_to_blur_type("Liquid Glass") == BlurType::LiquidGlass);
    assert(Theme::string_to_blur_type("Mica") == BlurType::Mica);

    // 3. Preset verification
    Theme midnight = Theme::get_preset(ThemePreset::MidnightAcrylic);
    assert(midnight.blur_type == BlurType::Acrylic);
    assert(midnight.bg_opacity == 45);

    Theme cyberpunk = Theme::get_preset(ThemePreset::CyberpunkNeon);
    assert(cyberpunk.blur_type == BlurType::LiquidGlass);
    assert(cyberpunk.glow_palette.size() == 5);

    Theme frost = Theme::get_preset(ThemePreset::FrostGlass);
    assert(frost.blur_type == BlurType::Mica);

    std::cout << "[PASS] Theme & Material Presets Unit Tests" << std::endl;
}
