#include "ColorEmojiRenderer.hpp"
#include <algorithm>

ColorEmojiRenderer::ColorEmojiRenderer() {
    init_d2d();
}

ColorEmojiRenderer::~ColorEmojiRenderer() {
    if (default_brush_) {
        default_brush_->Release();
        default_brush_ = nullptr;
    }
    if (dc_rt_) {
        dc_rt_->Release();
        dc_rt_ = nullptr;
    }
    if (dwrite_factory_) {
        dwrite_factory_->Release();
        dwrite_factory_ = nullptr;
    }
    if (d2d_factory_) {
        d2d_factory_->Release();
        d2d_factory_ = nullptr;
    }
}

void ColorEmojiRenderer::init_d2d() {
    if (is_initialized_) return;

    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory_);
    if (FAILED(hr) || !d2d_factory_) return;

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&dwrite_factory_)
    );
    if (FAILED(hr) || !dwrite_factory_) return;

    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
        0, 0,
        D2D1_RENDER_TARGET_USAGE_NONE,
        D2D1_FEATURE_LEVEL_DEFAULT
    );

    hr = d2d_factory_->CreateDCRenderTarget(&props, &dc_rt_);
    if (SUCCEEDED(hr) && dc_rt_) {
        dc_rt_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &default_brush_);
        is_initialized_ = true;
    }
}

float ColorEmojiRenderer::measure_emoji_width(const std::wstring& emoji_text, float font_size) {
    if (!dwrite_factory_ || emoji_text.empty()) {
        return font_size * 1.25f;
    }

    IDWriteTextFormat* text_format = nullptr;
    HRESULT hr = dwrite_factory_->CreateTextFormat(
        L"Segoe UI Emoji",
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        font_size,
        L"",
        &text_format
    );

    float width = font_size * 1.25f;
    if (SUCCEEDED(hr) && text_format) {
        IDWriteTextLayout* layout = nullptr;
        hr = dwrite_factory_->CreateTextLayout(
            emoji_text.c_str(),
            (UINT32)emoji_text.size(),
            text_format,
            10000.0f,
            10000.0f,
            &layout
        );

        if (SUCCEEDED(hr) && layout) {
            DWRITE_TEXT_METRICS metrics;
            if (SUCCEEDED(layout->GetMetrics(&metrics))) {
                width = metrics.widthIncludingTrailingWhitespace;
            }
            layout->Release();
        }
        text_format->Release();
    }

    return (std::max)(width, font_size * 1.0f);
}

void ColorEmojiRenderer::draw_color_emoji(
    HDC hdc,
    const RECT& clip_rect,
    const std::wstring& emoji_text,
    float font_size,
    float x,
    float y
) {
    if (!is_initialized_ || !dc_rt_ || !dwrite_factory_ || emoji_text.empty()) {
        return;
    }

    HRESULT hr = dc_rt_->BindDC(hdc, &clip_rect);
    if (FAILED(hr)) return;

    dc_rt_->BeginDraw();

    IDWriteTextFormat* text_format = nullptr;
    hr = dwrite_factory_->CreateTextFormat(
        L"Segoe UI Emoji",
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        font_size,
        L"",
        &text_format
    );

    if (SUCCEEDED(hr) && text_format) {
        // DirectWrite layout for precise color emoji rendering
        float render_box_w = (std::max)(font_size * 4.0f, 100.0f);
        float render_box_h = font_size * 2.0f;
        D2D1_RECT_F layout_rect = D2D1::RectF(x, y, x + render_box_w, y + render_box_h);

        dc_rt_->DrawText(
            emoji_text.c_str(),
            (UINT32)emoji_text.size(),
            text_format,
            &layout_rect,
            default_brush_,
            D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
        );

        text_format->Release();
    }

    dc_rt_->EndDraw();
}
