#include "EmojiCache.hpp"
#include "WinHttpClient.hpp"
#include "../common/UnicodeUtils.hpp"
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;
using namespace Gdiplus;

EmojiCache g_emoji_cache;

EmojiCache::EmojiCache() {
    char app_data[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, app_data))) {
        fs::path dir = fs::path(app_data) / "YT_Live_Chat_Overlay" / "cache" / "emojis";
        cache_dir_ = dir.string();
    } else {
        cache_dir_ = "cache/emojis";
    }

    ensure_cache_dir();
}

EmojiCache::~EmojiCache() {
    is_running_.store(false);
    cv_queue_.notify_all();
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

void EmojiCache::start_worker_if_needed() {
    if (!is_running_.load()) {
        is_running_.store(true);
        worker_thread_ = std::thread(&EmojiCache::download_worker, this);
    }
}

void EmojiCache::ensure_cache_dir() {
    try {
        fs::create_directories(cache_dir_);
    } catch (...) {}
}

std::string EmojiCache::url_to_filename(const std::string& url) {
    std::hash<std::string> hasher;
    size_t hash_val = hasher(url);

    std::ostringstream ss;
    ss << std::hex << hash_val;

    std::string ext = ".png";
    if (url.find(".gif") != std::string::npos) ext = ".gif";
    else if (url.find(".webp") != std::string::npos) ext = ".webp";
    else if (url.find(".jpg") != std::string::npos || url.find(".jpeg") != std::string::npos) ext = ".jpg";

    return ss.str() + ext;
}

void EmojiCache::decode_image_frames(CachedEmojiAsset& asset) {
    if (asset.disk_path.empty() || !fs::exists(asset.disk_path)) return;

    std::wstring w_path = unicode::utf8_to_utf16(asset.disk_path);
    std::unique_ptr<Gdiplus::Image> img = std::make_unique<Gdiplus::Image>(w_path.c_str());

    if (img->GetLastStatus() != Ok) return;

    UINT dim_count = img->GetFrameDimensionsCount();
    if (dim_count == 0) return;

    std::vector<GUID> dim_ids(dim_count);
    img->GetFrameDimensionsList(dim_ids.data(), dim_count);

    UINT frame_count = img->GetFrameCount(&dim_ids[0]);
    if (frame_count <= 1) {
        // Static image
        asset.is_animated = false;
        Gdiplus::Bitmap* bmp = new Gdiplus::Bitmap(img->GetWidth(), img->GetHeight(), PixelFormat32bppARGB);
        Gdiplus::Graphics g(bmp);
        g.DrawImage(img.get(), 0, 0, img->GetWidth(), img->GetHeight());
        asset.frames.push_back(std::shared_ptr<Gdiplus::Bitmap>(bmp));
        asset.frame_delays_ms.push_back(100);
        asset.total_duration_ms = 100;
        asset.is_loaded = true;
        return;
    }

    // Animated Image (e.g. GIF)
    asset.is_animated = true;
    asset.total_duration_ms = 0;

    // Read frame delays
    UINT prop_size = img->GetPropertyItemSize(PropertyTagFrameDelay);
    std::vector<uint8_t> prop_buf(prop_size);
    PropertyItem* prop_item = reinterpret_cast<PropertyItem*>(prop_buf.data());

    if (prop_size > 0 && img->GetPropertyItem(PropertyTagFrameDelay, prop_size, prop_item) == Ok) {
        long* delays = reinterpret_cast<long*>(prop_item->value);
        for (UINT f = 0; f < frame_count; ++f) {
            int d = static_cast<int>(delays[f] * 10);
            if (d < 20) d = 100;
            asset.frame_delays_ms.push_back(d);
            asset.total_duration_ms += d;
        }
    } else {
        for (UINT f = 0; f < frame_count; ++f) {
            asset.frame_delays_ms.push_back(100);
            asset.total_duration_ms += 100;
        }
    }

    for (UINT f = 0; f < frame_count; ++f) {
        img->SelectActiveFrame(&dim_ids[0], f);
        Gdiplus::Bitmap* frame_bmp = new Gdiplus::Bitmap(img->GetWidth(), img->GetHeight(), PixelFormat32bppARGB);
        Gdiplus::Graphics g(frame_bmp);
        g.DrawImage(img.get(), 0, 0, img->GetWidth(), img->GetHeight());
        asset.frames.push_back(std::shared_ptr<Gdiplus::Bitmap>(frame_bmp));
    }

    asset.is_loaded = true;
}

std::shared_ptr<CachedEmojiAsset> EmojiCache::load_from_disk_or_create(const std::string& url) {
    std::string filename = url_to_filename(url);
    std::string disk_path = (fs::path(cache_dir_) / filename).string();

    auto asset = std::make_shared<CachedEmojiAsset>();
    asset->url = url;
    asset->disk_path = disk_path;

    if (fs::exists(disk_path)) {
        decode_image_frames(*asset);
    }

    return asset;
}

std::shared_ptr<CachedEmojiAsset> EmojiCache::get_asset(const std::string& url, bool auto_download) {
    if (url.empty()) return nullptr;

    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = memory_cache_.find(url);
    if (it != memory_cache_.end()) {
        return it->second;
    }

    auto asset = load_from_disk_or_create(url);
    memory_cache_[url] = asset;

    if (auto_download && !asset->is_loaded) {
        request_download(url);
    }

    return asset;
}

void EmojiCache::request_download(const std::string& url) {
    if (url.empty()) return;

    std::lock_guard<std::mutex> lock(cache_mutex_);
    if (in_flight_downloads_.find(url) != in_flight_downloads_.end()) {
        return;
    }

    start_worker_if_needed();

    in_flight_downloads_.insert(url);
    download_queue_.push_back(url);
    cv_queue_.notify_one();
}

void EmojiCache::clear_memory_cache() {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    memory_cache_.clear();
}

void EmojiCache::download_worker() {
    WinHttpClient client(8000);

    while (is_running_.load()) {
        std::string current_url;

        {
            std::unique_lock<std::mutex> lock(cache_mutex_);
            cv_queue_.wait(lock, [this]() {
                return !download_queue_.empty() || !is_running_.load();
            });

            if (!is_running_.load()) break;

            current_url = download_queue_.front();
            download_queue_.pop_front();
        }

        if (current_url.empty()) continue;

        std::string host;
        std::string path_query;
        bool is_https = true;

        std::string temp = current_url;
        if (temp.find("https://") == 0) {
            temp = temp.substr(8);
            is_https = true;
        } else if (temp.find("http://") == 0) {
            temp = temp.substr(7);
            is_https = false;
        }

        size_t slash_pos = temp.find('/');
        if (slash_pos != std::string::npos) {
            host = temp.substr(0, slash_pos);
            path_query = temp.substr(slash_pos);
        } else {
            host = temp;
            path_query = "/";
        }

        HttpResponse res = client.get(host, path_query, is_https);

        if (res.success && !res.body.empty()) {
            std::string filename = url_to_filename(current_url);
            std::string disk_path = (fs::path(cache_dir_) / filename).string();

            std::ofstream out(disk_path, std::ios::binary);
            if (out.is_open()) {
                out.write(res.body.data(), res.body.size());
                out.close();

                std::lock_guard<std::mutex> lock(cache_mutex_);
                auto it = memory_cache_.find(current_url);
                if (it != memory_cache_.end()) {
                    decode_image_frames(*(it->second));
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(cache_mutex_);
            in_flight_downloads_.erase(current_url);
        }

        if (on_ready_) {
            on_ready_(current_url);
        }
    }
}
