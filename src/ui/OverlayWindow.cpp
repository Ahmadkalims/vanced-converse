#include "OverlayWindow.hpp"
#include "DwmHelper.hpp"
#include <algorithm>
#include <windowsx.h>
#include <chrono>

using namespace Gdiplus;

static const wchar_t* OVERLAY_CLASS_NAME = L"Vanced_Converse_Overlay_Class";
static const int RESIZE_BORDER_WIDTH = 8;

OverlayWindow::OverlayWindow(HINSTANCE hinstance) : hinstance_(hinstance) {
    config_ = g_config.get();
}

OverlayWindow::~OverlayWindow() {
    if (hwnd_) {
        if (anim_timer_id_) KillTimer(hwnd_, anim_timer_id_);
        DestroyWindow(hwnd_);
        hwnd_ = NULL;
    }
}

/**
 * @brief Registers window class, creates layered popup HWND, and applies styles.
 */
bool OverlayWindow::create() {
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = OverlayWindow::WndProc;
    wc.hInstance = hinstance_;
    wc.lpszClassName = OVERLAY_CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

    RegisterClassExW(&wc);

    RECT work_area;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);

    int win_w = config_.window_width > 0 ? config_.window_width : 380;
    int win_h = config_.window_height > 0 ? config_.window_height : (work_area.bottom - work_area.top - 40);
    int win_x = config_.window_x >= 0 ? config_.window_x : (work_area.right - win_w - 20);
    int win_y = config_.window_y >= 0 ? config_.window_y : (work_area.top + 20);

    DWORD ex_style = WS_EX_LAYERED | WS_EX_TOOLWINDOW;
    if (config_.always_on_top) ex_style |= WS_EX_TOPMOST;
    if (config_.click_through) ex_style |= WS_EX_TRANSPARENT;

    hwnd_ = CreateWindowExW(
        ex_style,
        OVERLAY_CLASS_NAME,
        L"Vanced Converse Overlay",
        WS_POPUP | WS_VISIBLE,
        win_x, win_y, win_w, win_h,
        NULL, NULL, hinstance_, this
    );

    if (!hwnd_) return false;

    anim_timer_id_ = SetTimer(hwnd_, 1, 40, NULL); // ~25 FPS
    apply_window_styles();
    render();
    return true;
}

/**
 * @brief Applies DWM backdrop materials, click-through, and topmost state.
 */
void OverlayWindow::apply_window_styles() {
    if (!hwnd_) return;

    ColorRGBA bg_col = ColorRGBA::parse(config_.theme.overlay.color);
    DwmHelper::apply_backdrop(hwnd_, config_.theme.overlay.type, config_.theme.overlay.opacity, bg_col);
    DwmHelper::set_click_through(hwnd_, config_.click_through);

    if (config_.always_on_top) {
        SetWindowPos(hwnd_, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    } else {
        SetWindowPos(hwnd_, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

void OverlayWindow::show(bool visible) {
    if (!hwnd_) return;
    ShowWindow(hwnd_, visible ? SW_SHOWNOACTIVATE : SW_HIDE);
    if (visible) render();
}

void OverlayWindow::set_click_through(bool enable) {
    config_.click_through = enable;
    DwmHelper::set_click_through(hwnd_, enable);
}

void OverlayWindow::set_always_on_top(bool enable) {
    config_.always_on_top = enable;
    if (enable) {
        SetWindowPos(hwnd_, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    } else {
        SetWindowPos(hwnd_, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

void OverlayWindow::reposition_right_dock() {
    if (!hwnd_) return;
    RECT work_area;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);
    int win_w = config_.window_width > 0 ? config_.window_width : 380;
    int win_h = work_area.bottom - work_area.top - 40;
    int win_x = work_area.right - win_w - 20;
    int win_y = work_area.top + 20;

    config_.window_x = win_x;
    config_.window_y = win_y;
    config_.window_width = win_w;
    config_.window_height = win_h;

    SetWindowPos(hwnd_, NULL, win_x, win_y, win_w, win_h, SWP_NOZORDER | SWP_NOACTIVATE);
    g_config.update(config_, false);
    render();
}

void OverlayWindow::add_messages(const std::vector<ChatMessage>& msgs) {
    {
        std::lock_guard<std::mutex> lock(msg_mutex_);
        for (const auto& m : msgs) {
            messages_.push_back(m);
            while (messages_.size() > (size_t)config_.max_messages) {
                messages_.pop_front();
            }
        }
    }
    if (auto_scroll_) {
        scroll_offset_y_ = 0;
    }
    render();
}

void OverlayWindow::set_status_text(const std::string& status) {
    std::lock_guard<std::mutex> lock(msg_mutex_);
    status_text_ = status;
}

void OverlayWindow::on_config_changed(const AppConfig& new_cfg) {
    config_ = new_cfg;
    text_renderer_.update_font_family(config_.theme.message_text.font.family);
    apply_window_styles();
    render();
}

void OverlayWindow::on_theme_changed(const StreamTheme& new_theme) {
    config_.theme = new_theme;
    config_.active_theme_name = new_theme.metadata.name;
    text_renderer_.update_font_family(new_theme.message_text.font.family);
    apply_window_styles();
    render();
}

void OverlayWindow::draw_rounded_rect(GraphicsPath& path, RectF rect, float radius) {
    float diameter = radius * 2.0f;
    if (diameter > rect.Width) diameter = rect.Width;
    if (diameter > rect.Height) diameter = rect.Height;

    path.StartFigure();
    path.AddArc(rect.X + rect.Width - diameter, rect.Y, diameter, diameter, 270, 90);
    path.AddArc(rect.X + rect.Width - diameter, rect.Y + rect.Height - diameter, diameter, diameter, 0, 90);
    path.AddArc(rect.X, rect.Y + rect.Height - diameter, diameter, diameter, 90, 90);
    path.AddArc(rect.X, rect.Y, diameter, diameter, 180, 90);
    path.CloseFigure();
}

/**
 * @brief Performs 32-bit alpha compositing, corner region clipping, and edge gradient fading.
 */
void OverlayWindow::render() {
    if (!hwnd_ || !IsWindowVisible(hwnd_)) return;

    RECT rc;
    GetWindowRect(hwnd_, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    if (width <= 0 || height <= 0) return;

    // 1. Clip window region to match corner radius cleanly without DWM rectangular leakage
    DwmHelper::update_window_region(hwnd_, width, height, config_.theme.overlay.corner_radius);

    HDC hdc_screen = GetDC(NULL);
    HDC hdc_mem = CreateCompatibleDC(hdc_screen);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    uint32_t* pixel_bits = nullptr;
    HBITMAP hbmp = CreateDIBSection(hdc_mem, &bmi, DIB_RGB_COLORS, (void**)&pixel_bits, NULL, 0);
    HBITMAP hbmp_old = (HBITMAP)SelectObject(hdc_mem, hbmp);

    uint64_t current_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();

    {
        Graphics g(hdc_mem);
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        // 2. Draw Background Glass Container
        float corner_radius = config_.theme.overlay.corner_radius;
        RectF container_rect(1.0f, 1.0f, (float)width - 2.0f, (float)height - 2.0f);

        GraphicsPath bg_path;
        draw_rounded_rect(bg_path, container_rect, corner_radius);

        if (config_.theme.overlay.type != OverlayBgType::Transparent) {
            ColorRGBA bg_col = ColorRGBA::parse(config_.theme.overlay.color);
            SolidBrush bg_brush(bg_col.to_gdiplus(config_.theme.overlay.opacity));
            g.FillPath(&bg_brush, &bg_path);
        }

        // Outer Border / Outline Rim
        if (config_.theme.overlay.outline_enabled || config_.theme.overlay.type == OverlayBgType::LiquidGlass || config_.theme.overlay.type == OverlayBgType::Acrylic) {
            ColorRGBA border_col = ColorRGBA::parse(config_.theme.overlay.outline_color);
            Pen border_pen(border_col.to_gdiplus(), (std::max)(config_.theme.overlay.outline_size, 1.0f));
            g.DrawPath(&border_pen, &bg_path);
        }

        // Top drag handle indicator
        if (!config_.click_through) {
            SolidBrush drag_pill(Color(40, 255, 255, 255));
            RectF pill_rect((float)width / 2.0f - 20.0f, 6.0f, 40.0f, 4.0f);
            GraphicsPath pill_path;
            draw_rounded_rect(pill_path, pill_rect, 2.0f);
            g.FillPath(&drag_pill, &pill_path);
        }

        // 3. Viewport Geometry from Config Padding
        float pad_top = (std::max)(config_.theme.overlay.padding.top, 8.0f);
        float pad_bottom = (std::max)(config_.theme.overlay.padding.bottom, 8.0f);
        float pad_left = (std::max)(config_.theme.overlay.padding.left, 8.0f);
        float pad_right = (std::max)(config_.theme.overlay.padding.right, 8.0f);

        float card_width = (float)width - (pad_left + pad_right);
        float view_height = (float)height - (pad_top + pad_bottom);

        // 4. Layout & Measure All Messages Responsively
        std::vector<ChatMessage> msgs_copy;
        {
            std::lock_guard<std::mutex> lock(msg_mutex_);
            msgs_copy.assign(messages_.begin(), messages_.end());
        }

        std::vector<MessageCardLayout> layouts;
        float total_height = 0.0f;

        for (const auto& m : msgs_copy) {
            MessageCardLayout lay = text_renderer_.layout_message(g, m, config_.theme, card_width, 0.0f);
            layouts.push_back(lay);
            total_height += lay.height + config_.theme.message_background.margin.top + config_.theme.message_background.margin.bottom;
        }

        max_scroll_y_ = (total_height > view_height) ? static_cast<int>(total_height - view_height) : 0;

        float start_draw_y = pad_top;
        if (total_height > view_height) {
            start_draw_y = pad_top - (total_height - view_height) + static_cast<float>(scroll_offset_y_);
        }

        // Inner viewport clipping
        RectF clip_rect(pad_left - 2.0f, pad_top, card_width + 4.0f, view_height);
        g.SetClip(clip_rect);

        RECT full_target_rc = { 0, 0, width, height };

        // 5. Draw Message Cards
        for (auto& lay : layouts) {
            float card_top = start_draw_y + config_.theme.message_background.margin.top;
            lay.x = pad_left;
            lay.y = card_top;

            start_draw_y += lay.height + config_.theme.message_background.margin.top + config_.theme.message_background.margin.bottom;

            if (lay.y + lay.height < pad_top || lay.y > (float)height - pad_bottom) {
                continue; // culled
            }

            text_renderer_.draw_message_card(g, hdc_mem, full_target_rc, lay, config_.theme, current_time_ms);
        }

        g.ResetClip();

        // 6. Draw Scrollbar if enabled
        bool show_sb = config_.theme.scrollbar.visible || (is_hovered_ && config_.theme.scrollbar.interaction_mode != ScrollbarInteractionMode::Disabled && !config_.click_through);
        if (show_sb && max_scroll_y_ > 0) {
            float sb_w = config_.theme.scrollbar.width;
            float sb_x = (float)width - sb_w - 4.0f;
            float track_h = view_height;
            float thumb_h = (std::max)(track_h * (view_height / total_height), 20.0f);

            float scroll_ratio = (max_scroll_y_ > 0) ? (float)(max_scroll_y_ - scroll_offset_y_) / (float)max_scroll_y_ : 0.0f;
            float thumb_y = pad_top + scroll_ratio * (track_h - thumb_h);

            RectF thumb_rect(sb_x, thumb_y, sb_w, thumb_h);
            GraphicsPath thumb_path;
            draw_rounded_rect(thumb_path, thumb_rect, config_.theme.scrollbar.corner_radius);

            ColorRGBA sb_col = ColorRGBA::parse(config_.theme.scrollbar.color);
            SolidBrush sb_brush(sb_col.to_gdiplus(0.7f));
            g.FillPath(&sb_brush, &thumb_path);
        }
    }

    // 7. Smooth Vertical Edge Gradient Fading
    if (config_.theme.overlay.fade_edges && pixel_bits) {
        float fade_len = (std::max)(config_.theme.overlay.fade_length, 12.0f);
        float pad_top = (std::max)(config_.theme.overlay.padding.top, 8.0f);
        float pad_bottom = (std::max)(config_.theme.overlay.padding.bottom, 8.0f);

        int top_fade_start = static_cast<int>(pad_top);
        int top_fade_end = static_cast<int>(pad_top + fade_len);
        int bot_fade_start = static_cast<int>((float)height - pad_bottom - fade_len);
        int bot_fade_end = static_cast<int>((float)height - pad_bottom);

        int x_min = static_cast<int>(config_.theme.overlay.padding.left);
        int x_max = static_cast<int>((float)width - config_.theme.overlay.padding.right);
        x_min = (std::max)(0, x_min);
        x_max = (std::min)(width - 1, x_max);

        // Top edge fade
        for (int y = top_fade_start; y < top_fade_end && y < height; ++y) {
            float factor = (float)(y - top_fade_start) / fade_len;
            if (factor < 0.0f) factor = 0.0f;
            if (factor > 1.0f) factor = 1.0f;
            uint32_t* row = pixel_bits + (y * width);
            for (int x = x_min; x <= x_max; ++x) {
                uint32_t pixel = row[x];
                uint8_t a = (pixel >> 24) & 0xFF;
                uint8_t r = (pixel >> 16) & 0xFF;
                uint8_t g = (pixel >> 8) & 0xFF;
                uint8_t b = pixel & 0xFF;

                a = static_cast<uint8_t>(a * factor);
                r = static_cast<uint8_t>(r * factor);
                g = static_cast<uint8_t>(g * factor);
                b = static_cast<uint8_t>(b * factor);

                row[x] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }

        // Bottom edge fade
        for (int y = bot_fade_start; y <= bot_fade_end && y < height; ++y) {
            if (y < 0) continue;
            float factor = (float)(bot_fade_end - y) / fade_len;
            if (factor < 0.0f) factor = 0.0f;
            if (factor > 1.0f) factor = 1.0f;
            uint32_t* row = pixel_bits + (y * width);
            for (int x = x_min; x <= x_max; ++x) {
                uint32_t pixel = row[x];
                uint8_t a = (pixel >> 24) & 0xFF;
                uint8_t r = (pixel >> 16) & 0xFF;
                uint8_t g = (pixel >> 8) & 0xFF;
                uint8_t b = pixel & 0xFF;

                a = static_cast<uint8_t>(a * factor);
                r = static_cast<uint8_t>(r * factor);
                g = static_cast<uint8_t>(g * factor);
                b = static_cast<uint8_t>(b * factor);

                row[x] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }

    // 8. Update Layered Window with Per-Pixel 32-bit Alpha
    POINT pt_src = { 0, 0 };
    SIZE size_wnd = { width, height };
    POINT pt_dst = { rc.left, rc.top };

    BLENDFUNCTION blend = { 0 };
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    UpdateLayeredWindow(
        hwnd_, hdc_screen,
        &pt_dst, &size_wnd,
        hdc_mem, &pt_src,
        0, &blend, ULW_ALPHA
    );

    SelectObject(hdc_mem, hbmp_old);
    DeleteObject(hbmp);
    DeleteDC(hdc_mem);
    ReleaseDC(NULL, hdc_screen);
}

LRESULT CALLBACK OverlayWindow::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    OverlayWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = (CREATESTRUCT*)lparam;
        self = (OverlayWindow*)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)self);
    } else {
        self = (OverlayWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (!self) return DefWindowProc(hwnd, msg, wparam, lparam);

    switch (msg) {
    case WM_TIMER:
        if (wparam == 1) {
            self->render();
        }
        return 0;

    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wparam);
        self->scroll_offset_y_ += (delta / WHEEL_DELTA) * 45;
        if (self->scroll_offset_y_ < 0) {
            self->scroll_offset_y_ = 0;
            self->auto_scroll_ = true;
        } else if (self->scroll_offset_y_ > self->max_scroll_y_) {
            self->scroll_offset_y_ = self->max_scroll_y_;
            self->auto_scroll_ = false;
        } else {
            self->auto_scroll_ = false;
        }
        self->render();
        return 0;
    }

    case WM_MOUSEMOVE: {
        if (!self->is_hovered_) {
            self->is_hovered_ = true;
            TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, hwnd, 0 };
            TrackMouseEvent(&tme);
            self->render();
        }
        return 0;
    }

    case WM_MOUSELEAVE:
        self->is_hovered_ = false;
        self->render();
        return 0;

    case WM_NCHITTEST: {
        if (self->config_.click_through) {
            return HTTRANSPARENT;
        }

        POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
        RECT rc;
        GetWindowRect(hwnd, &rc);

        // Edge resizing
        bool left = (pt.x >= rc.left && pt.x < rc.left + RESIZE_BORDER_WIDTH);
        bool right = (pt.x <= rc.right && pt.x > rc.right - RESIZE_BORDER_WIDTH);
        bool top = (pt.y >= rc.top && pt.y < rc.top + RESIZE_BORDER_WIDTH);
        bool bottom = (pt.y <= rc.bottom && pt.y > rc.bottom - RESIZE_BORDER_WIDTH);

        if (left && top) return HTTOPLEFT;
        if (right && top) return HTTOPRIGHT;
        if (left && bottom) return HTBOTTOMLEFT;
        if (right && bottom) return HTBOTTOMRIGHT;
        if (left) return HTLEFT;
        if (right) return HTRIGHT;
        if (top) return HTTOP;
        if (bottom) return HTBOTTOM;

        // Top drag handle region
        if (pt.y < rc.top + 30) {
            return HTCAPTION;
        }

        return HTCLIENT;
    }

    case WM_EXITSIZEMOVE: {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        self->config_.window_x = rc.left;
        self->config_.window_y = rc.top;
        self->config_.window_width = rc.right - rc.left;
        self->config_.window_height = rc.bottom - rc.top;
        g_config.update(self->config_, false);
        self->render();
        return 0;
    }

    case WM_OVERLAY_UPDATE_CONFIG: {
        AppConfig* cfg = (AppConfig*)lparam;
        if (cfg) {
            self->on_config_changed(*cfg);
            delete cfg;
        }
        return 0;
    }

    case WM_OVERLAY_UPDATE_THEME: {
        StreamTheme* th = (StreamTheme*)lparam;
        if (th) {
            self->on_theme_changed(*th);
            delete th;
        }
        return 0;
    }

    case WM_DESTROY:
        return 0;

    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}
