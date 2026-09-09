#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include "../../src/common/UnicodeUtils.hpp"

void test_unicode_multilingual() {
    std::cout << "[RUN] Testing Full Unicode & Multilingual Support..." << std::endl;

    std::vector<std::string> test_strings = {
        "Hello world",
        "नमस्ते दुनिया",                     // Hindi / Devanagari
        "مرحبا بالعالم",                      // Arabic (RTL)
        "你好世界",                          // Chinese (Simplified)
        "こんにちは世界",                    // Japanese
        "안녕하세요",                         // Korean
        "Привет мир",                        // Russian / Cyrillic
        "Hello नमस्ते مرحبا 你好 👋"          // Mixed Scripts & RTL/LTR
    };

    for (const auto& utf8_str : test_strings) {
        std::wstring utf16 = unicode::utf8_to_utf16(utf8_str);
        assert(!utf16.empty() && "UTF-16 conversion must not be empty");

        std::string roundtrip = unicode::utf16_to_utf8(utf16);
        assert(roundtrip == utf8_str && "Roundtrip UTF-8 <-> UTF-16 must be lossless and bit-exact");
    }

    // Test RTL detection
    std::wstring arabic_text = unicode::utf8_to_utf16("مرحبا");
    assert(unicode::is_rtl_char(arabic_text[0]) && "Arabic character must be detected as RTL");

    // Test Devanagari detection
    std::wstring hindi_text = unicode::utf8_to_utf16("नमस्ते");
    assert(unicode::is_devanagari(hindi_text[0]) && "Hindi character must be detected as Devanagari");

    // Test CJK detection
    std::wstring cjk_text = unicode::utf8_to_utf16("你好");
    assert(unicode::is_cjk(cjk_text[0]) && "Chinese character must be detected as CJK");

    std::cout << "[PASS] Unicode & Multilingual Text Verification Passed!" << std::endl;
}
