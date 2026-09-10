#pragma once

#include "WinHttpClient.hpp"
#include "../common/UnicodeUtils.hpp"
#include "../common/SimpleJson.hpp"
#include <string>
#include <vector>
#include <set>
#include <deque>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>

enum class StreamStatus {
    Idle,
    Connecting,
    Live,
    StreamOffline,
    InvalidKey,
    QuotaExceeded,
    NetworkError,
    Paused
};

struct ChatMessage {
    std::string id;
    std::string author_id;
    std::string author_name;
    std::string message_text;
    std::string timestamp;
    std::string author_color_hex;
    bool is_highlight = false;
    bool is_owner = false;
    bool is_moderator = false;
    bool is_verified = false;

    // Structured runs for text, Unicode emoji, and custom/animated emotes
    std::vector<TextSegment> segments;
};

class YouTubeEngine {
public:
    using MessageCallback = std::function<void(const std::vector<ChatMessage>&)>;
    using StatusCallback = std::function<void(StreamStatus, const std::string&)>;

private:
    std::string api_key_;
    std::string video_id_;
    int user_poll_interval_sec_ = 10;
    
    std::string active_live_chat_id_;
    std::string next_page_token_;
    int api_polling_interval_ms_ = 15000;

    std::atomic<bool> is_running_{false};
    std::atomic<bool> is_paused_{false};
    std::atomic<StreamStatus> current_status_{StreamStatus::Idle};
    std::string status_detail_;

    std::thread worker_thread_;
    std::mutex state_mutex_;
    std::condition_variable cv_wake_;

    std::set<std::string> seen_message_ids_;
    std::deque<std::string> seen_id_queue_;
    const size_t MAX_SEEN_IDS = 2000;

    MessageCallback on_messages_;
    StatusCallback on_status_;

    WinHttpClient http_client_;

    void worker_loop();
    bool discover_live_chat_id();
    bool fetch_messages();
    void set_status(StreamStatus status, const std::string& detail = "");
    void track_seen_id(const std::string& id);

    std::vector<TextSegment> parse_message_segments(const std::string& raw_text, const json::Value& snippet_obj);

public:
    YouTubeEngine();
    ~YouTubeEngine();

    void set_callbacks(MessageCallback on_msg, StatusCallback on_status);
    void start(const std::string& api_key, const std::string& video_id, int poll_interval_sec);
    void update_config(const std::string& api_key, const std::string& video_id, int poll_interval_sec);
    void pause();
    void resume();
    void stop();

    StreamStatus get_status() const { return current_status_.load(); }
    std::string get_status_detail() const;
    std::string get_active_live_chat_id() const;
};
