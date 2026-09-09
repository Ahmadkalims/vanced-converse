#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include "../../src/net/EmojiCache.hpp"

void test_custom_emoji_cache() {
    std::cout << "[RUN] Testing Emoji Cache & Media Asset Architecture..." << std::endl;

    // Test asset creation & URL mapping
    std::string test_url = "https://example.com/emojis/cool_cat.png";
    auto asset = g_emoji_cache.get_asset(test_url, false);
    assert(asset != nullptr && "EmojiCache must return a valid asset container");
    assert(asset->url == test_url);

    // Verify duplicate URL fetches from memory cache
    auto asset2 = g_emoji_cache.get_asset(test_url, false);
    assert(asset.get() == asset2.get() && "Repeated URL lookups must return cached instance");

    std::cout << "[PASS] Emoji Cache & Media Asset Test Passed!" << std::endl;
}
