#pragma once

#include <string>

/**
 * SecureStorage - Windows DPAPI (Data Protection API) wrapper.
 * 
 * Securely encrypts and decrypts sensitive credentials (such as the YouTube Data API key)
 * using the current Windows user's cryptographic master key.
 * 
 * Works seamlessly on both Windows 10 and Windows 11.
 * Prevents credentials from being committed to source control or exposed in plaintext configs.
 */
namespace SecureStorage {

    /**
     * Encrypts and saves the YouTube Data API key to local storage (%LOCALAPPDATA%\VancedConverse\api\key.enc).
     * @param api_key Plaintext API key to securely persist.
     * @return True if encryption and write succeeded.
     */
    bool save_api_key(const std::string& api_key);

    /**
     * Loads and decrypts the YouTube Data API key from local storage.
     * @return Plaintext API key, or empty string if not found or decryption failed.
     */
    std::string load_api_key();

    /**
     * Checks if an encrypted API key exists on disk.
     * @return True if the encrypted credential file exists.
     */
    bool has_api_key();

    /**
     * Securely deletes the persisted API key.
     */
    void clear_api_key();

} // namespace SecureStorage
