#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <memory>
#include "../common/ThemeModel.hpp"
#include "../common/UnicodeUtils.hpp"
#include "../net/YouTubeEngine.hpp"
#include "../net/EmojiCache.hpp"
#include "ColorEmojiRenderer.hpp"

struct InlineItem {
    SegmentType type;
    std::wstring text;
    std::string asset_url;
    std::string alt_text;
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    bool is_username = false;
    Gdiplus::Color color;
};

struct RenderLine {
    std::vector<InlineItem> items;
    float y = 0.0f;
    float height = 0.0f;
};

struct MessageCardLayout {
    const ChatMessage* msg = nullptr;
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    std::vector<RenderLine> lines;
    bool is_highlight = false;
};

class TextRenderer {
private:
    std::unique_ptr<Gdiplus::FontFamily> primary_font_family_;
    std::unique_ptr<Gdiplus::FontFamily> fallback_font_family_;
    std::unique_ptr<Gdiplus::FontFamily> emoji_font_family_;
    std::unique_ptr<Gdiplus::FontFamily> indic_font_family_;
    std::unique_ptr<Gdiplus::FontFamily> cjk_font_family_;

    ColorEmojiRenderer color_emoji_renderer_;
    std::string current_font_name_;

    void ensure_fonts(const std::string& family_name);

public:
    TextRenderer();
    ~TextRenderer();

    void update_font_family(const std::string& family_name);

    Gdiplus::Font* select_font_for_segment(const TextSegment& seg, float font_size, int weight);
    Gdiplus::Font* get_emoji_font(float font_size);
    Gdiplus::Font* get_primary_font(float font_size, int weight);

    MessageCardLayout layout_message(
        Gdiplus::Graphics& g,
        const ChatMessage& msg,
        const StreamTheme& theme,
        float card_width,
        float start_y
    );

    void draw_message_card(
        Gdiplus::Graphics& g,
        HDC hdc,
        const RECT& clip_rect,
        const MessageCardLayout& layout,
        const StreamTheme& theme,
        uint64_t current_time_ms
    );
};
