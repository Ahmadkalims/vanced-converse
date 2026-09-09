#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include "../../src/common/UnicodeUtils.hpp"

void test_emoji_segmentation() {
    std::cout << "[RUN] Testing Unicode Emoji & Run Segmentation..." << std::endl;

    // Test simple and complex emojis
    std::wstring mixed_msg = unicode::utf8_to_utf16("This is amazing 🔥😂 and great 👨‍💻🇮🇳👍🏽");
    std::vector<TextSegment> segs = unicode::segment_message_text(mixed_msg);

    assert(segs.size() >= 4 && "Should segment into alternating text and emoji runs");

    // Verify first run is text "This is amazing "
    assert(segs[0].type == SegmentType::Text);
    assert(segs[0].text == L"This is amazing ");

    // Verify second run is emoji run "🔥😂"
    assert(segs[1].type == SegmentType::UnicodeEmoji);

    // Verify third run is text " and great "
    assert(segs[2].type == SegmentType::Text);

    // Verify subsequent emoji run contains complex sequence
    assert(segs[3].type == SegmentType::UnicodeEmoji);

    // Verify that individual emojis are identified by is_emoji_codepoint
    size_t idx = 0;
    std::wstring fire = unicode::utf8_to_utf16("🔥");
    uint32_t cp = unicode::get_codepoint(fire, idx);
    assert(unicode::is_emoji_codepoint(cp) && "Fire emoji codepoint must be recognized");

    std::cout << "[PASS] Unicode Emoji & Glow Separation Segmentation Passed!" << std::endl;
}
