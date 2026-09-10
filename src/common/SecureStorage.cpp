#include "SecureStorage.hpp"
#include <windows.h>
#include <wincrypt.h>
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

namespace SecureStorage {

    // Resolves path to %LOCALAPPDATA%\VancedConverse\api\key.enc
    static fs::path get_storage_path() {
        wchar_t app_data[MAX_PATH] = { 0 };
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, app_data))) {
            return fs::path(app_data) / L"VancedConverse" / L"api" / L"key.enc";
        }
        return fs::path("api") / "key.enc";
    }

    bool save_api_key(const std::string& api_key) {
        if (api_key.empty()) {
            clear_api_key();
            return true;
        }

        DATA_BLOB data_in;
        data_in.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(api_key.data()));
        data_in.cbData = static_cast<DWORD>(api_key.size());

        DATA_BLOB data_out;
        // Encrypt with DPAPI using current Windows user context, no UI prompts
        if (!CryptProtectData(&data_in, L"Vanced Converse YouTube API Key", nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &data_out)) {
            return false;
        }

        try {
            fs::path p = get_storage_path();
            fs::create_directories(p.parent_path());

            std::ofstream out(p, std::ios::binary);
            if (!out.is_open()) {
                LocalFree(data_out.pbData);
                return false;
            }

            out.write(reinterpret_cast<const char*>(data_out.pbData), data_out.cbData);
            out.close();
            LocalFree(data_out.pbData);
            return true;
        } catch (...) {
            LocalFree(data_out.pbData);
            return false;
        }
    }

    std::string load_api_key() {
        try {
            fs::path p = get_storage_path();
            if (!fs::exists(p)) return "";

            std::ifstream in(p, std::ios::binary);
            if (!in.is_open()) return "";

            std::vector<BYTE> cipher((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            in.close();

            if (cipher.empty()) return "";

            DATA_BLOB data_in;
            data_in.pbData = cipher.data();
            data_in.cbData = static_cast<DWORD>(cipher.size());

            DATA_BLOB data_out;
            if (!CryptUnprotectData(&data_in, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &data_out)) {
                return "";
            }

            std::string result(reinterpret_cast<char*>(data_out.pbData), data_out.cbData);
            LocalFree(data_out.pbData);
            return result;
        } catch (...) {
            return "";
        }
    }

    bool has_api_key() {
        try {
            return fs::exists(get_storage_path());
        } catch (...) {
            return false;
        }
    }

    void clear_api_key() {
        try {
            fs::path p = get_storage_path();
            if (fs::exists(p)) {
                fs::remove(p);
            }
        } catch (...) {}
    }

} // namespace SecureStorage
