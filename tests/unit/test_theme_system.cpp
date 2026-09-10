#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include "../../src/common/ThemeModel.hpp"
#include "../../src/common/ThemeManager.hpp"
#include "../../src/common/SimpleJson.hpp"

namespace fs = std::filesystem;

void test_theme_system() {
    std::cout << "[RUN] Testing Theme Architecture, Schema Validation & Import/Export..." << std::endl;

    // 1. Test Default Midnight Pink Theme creation
    StreamTheme mp = StreamTheme::create_midnight_pink();
    assert(mp.metadata.name == "Midnight Pink");
    assert(mp.message_text.font.family == "Montserrat Alternates");
    assert(mp.message_text.font.size == 16.0f);
    assert(mp.overlay.opacity == 0.40f);
    assert(mp.overlay.padding.top == 16.0f);
    assert(mp.overlay.fade_edges == true);
    assert(mp.overlay.fade_length == 32.0f);
    assert(mp.username.effect.glow_radius == 6.0f);

    // 2. Test JSON Serialization and Roundtrip
    std::string json_str = mp.to_json_string(true);
    assert(!json_str.empty());
    assert(json_str.find("\"Midnight Pink\"") != std::string::npos);
    assert(json_str.find("\"Montserrat Alternates\"") != std::string::npos);

    StreamTheme parsed;
    std::string err;
    bool parse_ok = parsed.from_json_string(json_str, &err);
    assert(parse_ok && "Theme JSON must parse successfully");
    assert(parsed.metadata.name == mp.metadata.name);
    assert(parsed.message_text.color == mp.message_text.color);
    assert(parsed.overlay.padding.top == 16.0f);
    assert(parsed.overlay.fade_edges == true);

    // 3. Test Question Highlight Special Event
    std::wstring q_msg = L"How do I customize the font?";
    std::wstring n_msg = L"Looks awesome!";
    assert(parsed.is_special_event_triggered("question", q_msg) == true);
    assert(parsed.is_special_event_triggered("question", n_msg) == false);

    // 4. Test Schema Validation & Sanitization
    StreamTheme malformed;
    malformed.overlay.opacity = 2.5f; // Out of bounds
    malformed.overlay.blur = -10.0f;  // Out of bounds
    malformed.validate();
    assert(malformed.overlay.opacity == 1.0f && "Opacity must be clamped to 1.0");
    assert(malformed.overlay.blur == 0.0f && "Blur must be clamped to 0.0");

    // 5. Test Theme Directory & Template Initialization
    g_theme_manager.init();
    std::string themes_dir = g_theme_manager.get_themes_directory();
    assert(!themes_dir.empty());
    assert(fs::exists(themes_dir));
    assert(fs::exists(fs::path(themes_dir) / "template.css"));
    assert(fs::exists(fs::path(themes_dir) / "Midnight_Pink.css"));

    // Verify template.css parses with all comments and properties intact
    std::ifstream template_file(fs::path(themes_dir) / "template.css");
    assert(template_file.is_open());
    std::stringstream template_ss;
    template_ss << template_file.rdbuf();
    template_file.close();

    StreamTheme template_theme;
    bool template_ok = template_theme.from_css_string(template_ss.str(), &err);
    assert(template_ok && "template.css with comments must parse cleanly");
    assert(!template_theme.metadata.name.empty());

    // 6. Test CSS Theme Generation and CSS Parsing Roundtrip
    mp.custom_css_vars["--vc-user-custom-tag"] = "custom-glow-val";
    std::string css_output = mp.to_css_string();
    assert(!css_output.empty());
    assert(css_output.find("--vc-overlay-bg-color") != std::string::npos);
    assert(css_output.find("--vc-overlay-padding-top") != std::string::npos);
    assert(css_output.find("--vc-overlay-fade-edges") != std::string::npos);
    assert(css_output.find("--vc-username-glow-radius") != std::string::npos);
    assert(css_output.find("--vc-event-question-bg") != std::string::npos);
    assert(css_output.find("--vc-user-custom-tag: custom-glow-val;") != std::string::npos);

    StreamTheme css_parsed;
    bool css_parse_ok = css_parsed.from_css_string(css_output, &err);
    assert(css_parse_ok && "Theme CSS must parse successfully into C++ StreamTheme");
    assert(css_parsed.metadata.name == "Midnight Pink");
    assert(css_parsed.overlay.color == mp.overlay.color);
    assert(css_parsed.overlay.padding.top == 16.0f);
    assert(css_parsed.overlay.fade_edges == true);
    assert(css_parsed.message_text.color == mp.message_text.color);
    assert(css_parsed.username.font.family == "Montserrat Alternates");
    assert(css_parsed.custom_css_vars["--vc-user-custom-tag"] == "custom-glow-val");

    // 7. Test Theme Duplicate, Rename, Export, Delete CRUD
    bool dup_ok = g_theme_manager.duplicate_theme("Midnight Pink", "Test Theme Copy", &err);
    assert(dup_ok && "ThemeManager duplicate_theme must succeed");

    bool rename_ok = g_theme_manager.rename_theme("Test Theme Copy", "Test Theme Renamed", &err);
    assert(rename_ok && "ThemeManager rename_theme must succeed");

    std::string export_dest = "test_export.css";
    bool export_ok = g_theme_manager.export_theme_to_file("Test Theme Renamed", export_dest, &err);
    assert(export_ok && "ThemeManager export_theme_to_file must succeed");

    bool del_ok = g_theme_manager.delete_theme("Test Theme Renamed", &err);
    assert(del_ok && "ThemeManager delete_theme must succeed");

    try {
        fs::remove(export_dest);
    } catch (...) {}

    std::cout << "[PASS] Theme System, CSS Theme Engine & Import/Export Passed!" << std::endl;
}
