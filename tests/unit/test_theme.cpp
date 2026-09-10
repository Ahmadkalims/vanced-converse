#include "../../src/common/SecureStorage.hpp"
#include <cassert>
#include <iostream>

void test_secure_storage() {
    std::string test_key = "AIzaSyTestApiKey_1234567890abcdef";
    bool saved = SecureStorage::save_api_key(test_key);
    assert(saved && "DPAPI CryptProtectData must succeed on Windows");

    std::string loaded = SecureStorage::load_api_key();
    assert(loaded == test_key && "DPAPI CryptUnprotectData must recover the exact key");

    std::cout << "[PASS] DPAPI Secure Storage Roundtrip Tests" << std::endl;
}
