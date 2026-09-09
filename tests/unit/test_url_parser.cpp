#include "../../src/common/Config.hpp"
#include <cassert>
#include <iostream>

void test_url_parser() {
    // 1. Raw 11-char video ID
    assert(AppConfig::extract_video_id("vtL-JrlhT2g") == "vtL-JrlhT2g");

    // 2. Standard watch URL
    assert(AppConfig::extract_video_id("https://www.youtube.com/watch?v=vtL-JrlhT2g") == "vtL-JrlhT2g");
    assert(AppConfig::extract_video_id("https://youtube.com/watch?v=vtL-JrlhT2g&feature=share") == "vtL-JrlhT2g");

    // 3. Short youtu.be URL
    assert(AppConfig::extract_video_id("https://youtu.be/vtL-JrlhT2g") == "vtL-JrlhT2g");
    assert(AppConfig::extract_video_id("https://youtu.be/vtL-JrlhT2g?t=10") == "vtL-JrlhT2g");

    // 4. Live URL
    assert(AppConfig::extract_video_id("https://www.youtube.com/live/vtL-JrlhT2g") == "vtL-JrlhT2g");

    // 5. Embed URL
    assert(AppConfig::extract_video_id("https://www.youtube.com/embed/vtL-JrlhT2g") == "vtL-JrlhT2g");

    // 6. Whitespace trimming
    assert(AppConfig::extract_video_id("   vtL-JrlhT2g   ") == "vtL-JrlhT2g");

    // 7. Empty string
    assert(AppConfig::extract_video_id("") == "");

    std::cout << "[PASS] YouTube URL & Video ID Extractor Unit Tests" << std::endl;
}
