#include "YouTubeEngine.hpp"
#include "EmojiCache.hpp"
#include "../common/SimpleJson.hpp"
#include "../common/Config.hpp"
#include <chrono>
#include <algorithm>

YouTubeEngine::YouTubeEngine() : http_client_(8000) {}

YouTubeEngine::~YouTubeEngine() {
    stop();
}

void YouTubeEngine::set_callbacks(MessageCallback on_msg, StatusCallback on_status) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    on_messages_ = on_msg;
    on_status_ = on_status;
}

std::string YouTubeEngine::get_status_detail() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(state_mutex_));
    return status_detail_;
}

std::string YouTubeEngine::get_active_live_chat_id() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(state_mutex_));
    return active_live_chat_id_;
}

void YouTubeEngine::set_status(StreamStatus status, const std::string& detail) {
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        current_status_.store(status);
        status_detail_ = detail;
    }
    if (on_status_) {
        on_status_(status, detail);
    }
}

void YouTubeEngine::track_seen_id(const std::string& id) {
    if (seen_message_ids_.find(id) != seen_message_ids_.end()) return;

    seen_message_ids_.insert(id);
    seen_id_queue_.push_back(id);

    while (seen_id_queue_.size() > MAX_SEEN_IDS) {
        std::string oldest = seen_id_queue_.front();
        seen_id_queue_.pop_front();
        seen_message_ids_.erase(oldest);
    }
}

void YouTubeEngine::start(const std::string& api_key, const std::string& video_id, int poll_interval_sec) {
    stop();

    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        api_key_ = api_key;
        video_id_ = AppConfig::extract_video_id(video_id);
        user_poll_interval_sec_ = poll_interval_sec;
        active_live_chat_id_.clear();
        next_page_token_.clear();
        seen_message_ids_.clear();
        seen_id_queue_.clear();
        is_running_.store(true);
        is_paused_.store(false);
    }

    if (api_key_.empty() || video_id_.empty()) {
        set_status(StreamStatus::Idle, "API Key or Video ID missing. Configure in Settings.");
        return;
    }

    set_status(StreamStatus::Connecting, "Connecting to YouTube stream...");
    worker_thread_ = std::thread(&YouTubeEngine::worker_loop, this);
}

void YouTubeEngine::update_config(const std::string& api_key, const std::string& video_id, int poll_interval_sec) {
    std::string clean_vid = AppConfig::extract_video_id(video_id);
    bool restart_needed = false;

    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        if (api_key_ != api_key || video_id_ != clean_vid) {
            restart_needed = true;
        } else {
            user_poll_interval_sec_ = poll_interval_sec;
            cv_wake_.notify_all();
        }
    }

    if (restart_needed) {
        start(api_key, clean_vid, poll_interval_sec);
    }
}

void YouTubeEngine::pause() {
    is_paused_.store(true);
    cv_wake_.notify_all();
    set_status(StreamStatus::Paused, "Polling paused (overlay hidden)");
}

void YouTubeEngine::resume() {
    if (is_paused_.load()) {
        is_paused_.store(false);
        cv_wake_.notify_all();
        set_status(StreamStatus::Live, "Resuming live chat polling...");
    }
}

void YouTubeEngine::stop() {
    is_running_.store(false);
    is_paused_.store(false);
    cv_wake_.notify_all();

    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    set_status(StreamStatus::Idle, "Stopped (0 API calls active)");
}

bool YouTubeEngine::discover_live_chat_id() {
    std::string host = "www.googleapis.com";
    std::string path;
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        path = "/youtube/v3/videos?part=liveStreamingDetails&id=" +
               WinHttpClient::url_encode(video_id_) +
               "&key=" + WinHttpClient::url_encode(api_key_);
    }

    HttpResponse res = http_client_.get(host, path, true);

    if (!res.success) {
        if (res.status_code == 400) {
            set_status(StreamStatus::InvalidKey, "Invalid YouTube API Key or Video ID (HTTP 400)");
        } else if (res.status_code == 403) {
            set_status(StreamStatus::QuotaExceeded, "YouTube Data API daily quota exceeded (HTTP 403)");
        } else {
            set_status(StreamStatus::NetworkError, "Network error (" + std::to_string(res.status_code) + "): " + res.error_message);
        }
        return false;
    }

    json::Value root = json::parse(res.body);
    if (!root.contains("items") || root["items"].size() == 0) {
        set_status(StreamStatus::StreamOffline, "Video ID valid, but no live stream found.");
        return false;
    }

    const auto& item = root["items"][0];
    if (item.contains("liveStreamingDetails") && item["liveStreamingDetails"].contains("activeLiveChatId")) {
        std::string found_id = item["liveStreamingDetails"]["activeLiveChatId"].as_string();
        if (!found_id.empty()) {
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                active_live_chat_id_ = found_id;
            }
            set_status(StreamStatus::Live, "Connected! Live Chat ID: " + found_id);
            return true;
        }
    }

    set_status(StreamStatus::StreamOffline, "Stream is not currently live or chat is disabled.");
    return false;
}

std::vector<TextSegment> YouTubeEngine::parse_message_segments(const std::string& raw_text, const json::Value& snippet_obj) {
    std::wstring w_text = unicode::utf8_to_utf16(raw_text);
    std::vector<TextSegment> segs = unicode::segment_message_text(w_text);

    // If snippet has custom emoji runs, map them
    if (snippet_obj.contains("displayMessageBody") && snippet_obj["displayMessageBody"].contains("runs")) {
        const auto& runs = snippet_obj["displayMessageBody"]["runs"];
        if (runs.is_array() && runs.size() > 0) {
            std::vector<TextSegment> custom_segs;
            for (size_t i = 0; i < runs.size(); ++i) {
                const auto& run = runs[i];
                if (run.contains("emoji")) {
                    const auto& em = run["emoji"];
                    TextSegment es;
                    es.type = em["isCustomEmoji"].as_bool(false) ? SegmentType::CustomEmoji : SegmentType::UnicodeEmoji;
                    if (em.contains("image") && em["image"].contains("thumbnails") && em["image"]["thumbnails"].size() > 0) {
                        es.asset_url = em["image"]["thumbnails"][0]["url"].as_string();
                        if (es.asset_url.find(".gif") != std::string::npos || es.asset_url.find("animated") != std::string::npos) {
                            es.type = SegmentType::AnimatedEmoji;
                        }
                        g_emoji_cache.get_asset(es.asset_url);
                    }
                    es.alt_text = em["shortcuts"].size() > 0 ? em["shortcuts"][0].as_string() : "";
                    es.text = unicode::utf8_to_utf16(em["emojiId"].as_string(es.alt_text));
                    custom_segs.push_back(es);
                } else if (run.contains("text")) {
                    std::wstring r_txt = unicode::utf8_to_utf16(run["text"].as_string());
                    auto sub_segs = unicode::segment_message_text(r_txt);
                    custom_segs.insert(custom_segs.end(), sub_segs.begin(), sub_segs.end());
                }
            }
            if (!custom_segs.empty()) return custom_segs;
        }
    }

    return segs;
}

bool YouTubeEngine::fetch_messages() {
    std::string path;
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        if (active_live_chat_id_.empty()) return false;

        path = "/youtube/v3/liveChat/messages?part=snippet,authorDetails&liveChatId=" +
               WinHttpClient::url_encode(active_live_chat_id_) +
               "&key=" + WinHttpClient::url_encode(api_key_);

        if (!next_page_token_.empty()) {
            path += "&pageToken=" + WinHttpClient::url_encode(next_page_token_);
        }
    }

    HttpResponse res = http_client_.get("www.googleapis.com", path, true);

    if (!res.success) {
        if (res.status_code == 403) {
            set_status(StreamStatus::QuotaExceeded, "YouTube API quota exceeded (HTTP 403)");
        } else if (res.status_code == 404) {
            set_status(StreamStatus::StreamOffline, "Live stream chat ended (HTTP 404)");
            std::lock_guard<std::mutex> lock(state_mutex_);
            active_live_chat_id_.clear();
        } else {
            set_status(StreamStatus::NetworkError, "Poll error (HTTP " + std::to_string(res.status_code) + ")");
        }
        return false;
    }

    json::Value root = json::parse(res.body);
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        next_page_token_ = root["nextPageToken"].as_string();
        api_polling_interval_ms_ = root["pollingIntervalMillis"].as_int(15000);
    }

    std::vector<ChatMessage> new_msgs;
    AppConfig current_cfg = g_config.get();

    if (root.contains("items") && root["items"].is_array()) {
        for (size_t i = 0; i < root["items"].size(); ++i) {
            const auto& it = root["items"][i];
            std::string msg_id = it["id"].as_string();
            if (msg_id.empty() || seen_message_ids_.find(msg_id) != seen_message_ids_.end()) {
                continue;
            }

            track_seen_id(msg_id);

            ChatMessage cm;
            cm.id = msg_id;
            
            if (it.contains("snippet")) {
                const auto& snip = it["snippet"];
                cm.message_text = snip["displayMessage"].as_string();
                cm.timestamp = snip["publishedAt"].as_string();
                cm.segments = parse_message_segments(cm.message_text, snip);
            }

            if (it.contains("authorDetails")) {
                const auto& auth = it["authorDetails"];
                cm.author_id = auth["channelId"].as_string();
                cm.author_name = auth["displayName"].as_string("Unknown");
                cm.is_owner = auth["isChatOwner"].as_bool(false);
                cm.is_moderator = auth["isChatModerator"].as_bool(false);
                cm.is_verified = auth["isVerified"].as_bool(false);
            }

            // Special Event / Question Highlight check
            std::wstring w_msg = unicode::utf8_to_utf16(cm.message_text);
            if (current_cfg.theme.is_special_event_triggered("question", w_msg)) {
                cm.is_highlight = true;
            }

            // Persistent deterministic user color assignment
            cm.author_color_hex = current_cfg.theme.get_user_color(cm.author_id, cm.author_name);

            new_msgs.push_back(cm);
        }
    }

    if (!new_msgs.empty() && on_messages_) {
        on_messages_(new_msgs);
    }

    set_status(StreamStatus::Live, "Live chat active (" + std::to_string(seen_message_ids_.size()) + " messages)");
    return true;
}

void YouTubeEngine::worker_loop() {
    while (is_running_.load()) {
        if (is_paused_.load()) {
            std::unique_lock<std::mutex> lock(state_mutex_);
            cv_wake_.wait(lock, [this]() { return !is_paused_.load() || !is_running_.load(); });
            if (!is_running_.load()) break;
        }

        bool has_chat_id = false;
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            has_chat_id = !active_live_chat_id_.empty();
        }

        if (!has_chat_id) {
            if (!discover_live_chat_id()) {
                std::unique_lock<std::mutex> lock(state_mutex_);
                cv_wake_.wait_for(lock, std::chrono::seconds(8), [this]() { return !is_running_.load() || is_paused_.load(); });
                continue;
            }
        }

        fetch_messages();

        int wait_sec = user_poll_interval_sec_;
        int api_sec = api_polling_interval_ms_ / 1000;
        int effective_wait_sec = (std::max)(wait_sec, api_sec);
        if (effective_wait_sec < 3) effective_wait_sec = 3;

        std::unique_lock<std::mutex> lock(state_mutex_);
        cv_wake_.wait_for(lock, std::chrono::seconds(effective_wait_sec), [this]() {
            return !is_running_.load() || is_paused_.load();
        });
    }
}
