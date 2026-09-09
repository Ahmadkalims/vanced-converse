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
    assert(mp.overlay.opacity == 0.4f);
    assert(mp.scrollbar.visible == false);

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

    // 5. Test Theme Import, Export, and Template Initialization
    g_theme_manager.init();
    std::string themes_dir = g_theme_manager.get_themes_directory();
    assert(!themes_dir.empty());
    assert(fs::exists(themes_dir));
    assert(fs::exists(fs::path(themes_dir) / "template.streamtheme"));
    assert(fs::exists(fs::path(themes_dir) / "Midnight Pink.streamtheme"));

    // Verify template.streamtheme parses with all comments intact
    std::ifstream template_file(fs::path(themes_dir) / "template.streamtheme");
    assert(template_file.is_open());
    std::stringstream template_ss;
    template_ss << template_file.rdbuf();
    template_file.close();

    StreamTheme template_theme;
    bool template_ok = template_theme.from_json_string(template_ss.str(), &err);
    assert(template_ok && "template.streamtheme with comments must parse cleanly");
    assert(template_theme.metadata.name == "Custom Stream Theme Template");

    std::string temp_theme_path = "test_custom_theme.streamtheme";
    std::ofstream out(temp_theme_path);
    out << json_str;
    out.close();

    bool import_ok = g_theme_manager.import_theme_from_file(temp_theme_path, &err);
    assert(import_ok && "ThemeManager must import .streamtheme files successfully");

    // 6. Test CSS Theme Generation and CSS Parsing Roundtrip
    std::string css_output = mp.to_css_string();
    assert(!css_output.empty());
    assert(css_output.find("--vc-overlay-bg-color") != std::string::npos);
    assert(css_output.find("--vc-username-font") != std::string::npos);
    assert(css_output.find("--vc-event-question-bg") != std::string::npos);

    StreamTheme css_parsed;
    bool css_parse_ok = css_parsed.from_css_string(css_output, &err);
    assert(css_parse_ok && "Theme CSS must parse successfully into C++ StreamTheme");
    assert(css_parsed.metadata.name == "Midnight Pink");
    assert(css_parsed.overlay.color == mp.overlay.color);
    assert(css_parsed.message_text.color == mp.message_text.color);
    assert(css_parsed.username.font.family == "Montserrat Alternates");

    // Test importing raw CSS theme file
    std::string temp_css_path = "test_custom_theme.css";
    std::ofstream css_out(temp_css_path);
    css_out << css_output;
    css_out.close();

    std::string export_dest = "test_export.css";
    bool export_ok = g_theme_manager.export_theme_to_file("Midnight Pink", export_dest, &err);
    assert(export_ok && "ThemeManager must export .css theme files successfully");

    // Cleanup temp files
    try {
        fs::remove(temp_theme_path);
        fs::remove(export_dest);
        fs::remove(temp_css_path);
    } catch (...) {}

    std::cout << "[PASS] Theme System, CSS Theme Engine & Import/Export Passed!" << std::endl;
}


