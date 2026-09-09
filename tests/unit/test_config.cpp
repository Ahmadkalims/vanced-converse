#include "../../src/common/Config.hpp"
#include "../../src/common/ThemeModel.hpp"
#include <cassert>
#include <iostream>

void test_config() {
    AppConfig cfg;
    cfg.api_key = "TEST_API_KEY";
    cfg.video_id = "vtL-JrlhT2g";
    cfg.poll_interval_sec = 20;
    cfg.click_through = true;
    cfg.theme.overlay.opacity = 0.5f;

    // Test listener
    bool listener_called = false;
    ConfigManager cm;
    cm.add_change_listener([&](const AppConfig& updated) {
        listener_called = true;
        assert(updated.api_key == "TEST_API_KEY");
        assert(updated.poll_interval_sec == 20);
    });

    cm.update(cfg, true);
    assert(listener_called);

    // Test persistent color deterministic hashing
    StreamTheme t = StreamTheme::create_midnight_pink();
    std::string color1 = t.get_user_color("UC12345", "Alice");
    std::string color2 = t.get_user_color("UC12345", "Alice");
    assert(color1 == color2 && "Deterministic user color must be identical for same user");

    std::cout << "[PASS] Config Manager & Live Observer Unit Tests" << std::endl;
}
