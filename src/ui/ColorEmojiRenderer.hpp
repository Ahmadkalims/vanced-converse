#pragma once

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <string>

class ColorEmojiRenderer {
private:
    ID2D1Factory* d2d_factory_ = nullptr;
    IDWriteFactory* dwrite_factory_ = nullptr;
    ID2D1DCRenderTarget* dc_rt_ = nullptr;
    ID2D1SolidColorBrush* default_brush_ = nullptr;
    bool is_initialized_ = false;

    void init_d2d();

public:
    ColorEmojiRenderer();
    ~ColorEmojiRenderer();

    bool is_ready() const { return is_initialized_; }

    void draw_color_emoji(
        HDC hdc,
        const RECT& clip_rect,
        const std::wstring& emoji_text,
        float font_size,
        float x,
        float y
    );

    float measure_emoji_width(const std::wstring& emoji_text, float font_size);
};
