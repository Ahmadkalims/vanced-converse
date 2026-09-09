#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <deque>
#include <string>
#include <mutex>
#include "../common/Config.hpp"
#include "../common/ThemeModel.hpp"
#include "../net/YouTubeEngine.hpp"
#include "TextRenderer.hpp"

#define WM_OVERLAY_UPDATE_CONFIG (WM_USER + 101)
#define WM_OVERLAY_ADD_MESSAGES  (WM_USER + 102)
#define WM_OVERLAY_UPDATE_STATUS (WM_USER + 103)
#define WM_OVERLAY_UPDATE_THEME  (WM_USER + 104)

class OverlayWindow {
private:
    HWND hwnd_ = NULL;
    HINSTANCE hinstance_ = NULL;
    ULONG_PTR gdiplus_token_ = 0;

    std::deque<ChatMessage> messages_;
    std::mutex msg_mutex_;
    std::string status_text_ = "Connecting to YouTube live chat...";

    AppConfig config_;
    TextRenderer text_renderer_;
    
    // Smooth scrolling & view
    int scroll_offset_y_ = 0;
    int max_scroll_y_ = 0;
    bool auto_scroll_ = true;
    bool is_hovered_ = false;

    // Animation frame timing
    UINT_PTR anim_timer_id_ = 0;

    void render();
    void draw_rounded_rect(Gdiplus::GraphicsPath& path, Gdiplus::RectF rect, float radius);
    void apply_window_styles();

public:
    OverlayWindow(HINSTANCE hinstance);
    ~OverlayWindow();

    bool create();
    void show(bool visible);
    void set_click_through(bool enable);
    void set_always_on_top(bool enable);
    void reposition_right_dock();

    void add_messages(const std::vector<ChatMessage>& msgs);
    void set_status_text(const std::string& status);
    void on_config_changed(const AppConfig& new_cfg);
    void on_theme_changed(const StreamTheme& new_theme);

    HWND get_hwnd() const { return hwnd_; }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};
