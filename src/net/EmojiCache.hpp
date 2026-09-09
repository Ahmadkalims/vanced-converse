#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <mutex>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <deque>

struct CachedEmojiAsset {
    std::string url;
    std::string disk_path;
    bool is_loaded = false;
    bool is_animated = false;
    std::vector<std::shared_ptr<Gdiplus::Bitmap>> frames;
    std::vector<int> frame_delays_ms;
    int total_duration_ms = 0;

    Gdiplus::Bitmap* get_frame(uint64_t timestamp_ms) const {
        if (frames.empty()) return nullptr;
        if (!is_animated || frames.size() == 1 || total_duration_ms <= 0) {
            return frames[0].get();
        }

        int elapsed = static_cast<int>(timestamp_ms % total_duration_ms);
        int accumulated = 0;
        for (size_t i = 0; i < frames.size(); ++i) {
            accumulated += frame_delays_ms[i];
            if (elapsed < accumulated) {
                return frames[i].get();
            }
        }
        return frames[0].get();
    }
};

class EmojiCache {
public:
    using AssetReadyCallback = std::function<void(const std::string& url)>;

private:
    std::string cache_dir_;
    std::unordered_map<std::string, std::shared_ptr<CachedEmojiAsset>> memory_cache_;
    std::set<std::string> in_flight_downloads_;
    std::mutex cache_mutex_;

    std::deque<std::string> download_queue_;
    std::thread worker_thread_;
    std::atomic<bool> is_running_{false};
    std::condition_variable cv_queue_;

    AssetReadyCallback on_ready_;

    void ensure_cache_dir();
    std::string url_to_filename(const std::string& url);
    std::shared_ptr<CachedEmojiAsset> load_from_disk_or_create(const std::string& url);
    void decode_image_frames(CachedEmojiAsset& asset);
    void download_worker();
    void start_worker_if_needed();

public:
    EmojiCache();
    ~EmojiCache();

    void set_on_asset_ready(AssetReadyCallback callback) { on_ready_ = callback; }

    std::shared_ptr<CachedEmojiAsset> get_asset(const std::string& url, bool auto_download = true);
    void request_download(const std::string& url);
    void clear_memory_cache();
};

extern EmojiCache g_emoji_cache;
