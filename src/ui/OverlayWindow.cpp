#include "OverlayWindow.hpp"
#include "DwmHelper.hpp"
#include <algorithm>
#include <windowsx.h>
#include <chrono>

using namespace Gdiplus;

static const wchar_t* OVERLAY_CLASS_NAME = L"YT_Live_Chat_Overlay_Class";
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
        L"YouTube Live Chat Overlay",
        WS_POPUP | WS_VISIBLE,
        win_x, win_y, win_w, win_h,
        NULL, NULL, hinstance_, this
    );

    if (!hwnd_) return false;

    anim_timer_id_ = SetTimer(hwnd_, 1, 40, NULL); // ~25 FPS for smooth GIF/animated emojis
    apply_window_styles();
    render();
    return true;
}

void OverlayWindow::apply_window_styles() {
    if (!hwnd_) return;

    BlurType bt = BlurType::Acrylic;
    if (config_.theme.overlay.type == OverlayBgType::Mica) bt = BlurType::Mica;
    else if (config_.theme.overlay.type == OverlayBgType::LiquidGlass) bt = BlurType::LiquidGlass;
    else if (config_.theme.overlay.type == OverlayBgType::Solid || config_.theme.overlay.type == OverlayBgType::Transparent) bt = BlurType::FlatTranslucent;

    ColorRGBA bg_col = ColorRGBA::parse(config_.theme.overlay.color);
    ThemeColor tc = { bg_col.r, bg_col.g, bg_col.b, bg_col.a };

    uint8_t alpha = static_cast<uint8_t>(config_.theme.overlay.opacity * 255.0f);
    DwmHelper::apply_backdrop(hwnd_, bt, alpha, tc);
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
    {
        std::lock_guard<std::mutex> lock(msg_mutex_);
        status_text_ = status;
    }
    render();
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

void OverlayWindow::render() {
    if (!hwnd_ || !IsWindowVisible(hwnd_)) return;

    RECT rc;
    GetWindowRect(hwnd_, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    if (width <= 0 || height <= 0) return;

    HDC hdc_screen = GetDC(NULL);
    HDC hdc_mem = CreateCompatibleDC(hdc_screen);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP hbmp = CreateDIBSection(hdc_mem, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    HBITMAP hbmp_old = (HBITMAP)SelectObject(hdc_mem, hbmp);

    uint64_t current_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();

    {
        Graphics g(hdc_mem);
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        // 1. Draw Background Glass Container
        float corner_radius = config_.theme.overlay.corner_radius;
        RectF container_rect(2.0f, 2.0f, (float)width - 4.0f, (float)height - 4.0f);

        GraphicsPath bg_path;
        draw_rounded_rect(bg_path, container_rect, corner_radius);

        if (config_.theme.overlay.type != OverlayBgType::Transparent) {
            ColorRGBA bg_col = ColorRGBA::parse(config_.theme.overlay.color);
            SolidBrush bg_brush(bg_col.to_gdiplus(config_.theme.overlay.opacity));
            g.FillPath(&bg_brush, &bg_path);
        }

        // Outline / Specular Rim
        if (config_.theme.overlay.outline_enabled || config_.theme.overlay.type == OverlayBgType::LiquidGlass || config_.theme.overlay.type == OverlayBgType::Acrylic) {
            ColorRGBA border_col = ColorRGBA::parse(config_.theme.overlay.outline_color);
            Pen border_pen(border_col.to_gdiplus(), (std::max)(config_.theme.overlay.outline_size, 1.2f));
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

        // 2. Status Header
        float current_y = 16.0f;
        float padding_x = 16.0f;
        float card_width = (float)width - (padding_x * 2.0f);

        {
            std::lock_guard<std::mutex> lock(msg_mutex_);
            std::wstring w_status = unicode::utf8_to_utf16(status_text_);
            Gdiplus::Font font_meta(L"Segoe UI", 11.0f, FontStyleRegular, UnitPixel);
            SolidBrush meta_brush(Color(180, 200, 205, 215));
            RectF status_rect(padding_x, current_y, card_width, 18.0f);
            StringFormat sf;
            sf.SetAlignment(StringAlignmentNear);
            g.DrawString(w_status.c_str(), -1, &font_meta, status_rect, &sf, &meta_brush);
            current_y += 24.0f;
        }

        // 3. Layout & Measure All Messages Responsively
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

        float view_height = (float)height - current_y - 14.0f;
        max_scroll_y_ = (total_height > view_height) ? static_cast<int>(total_height - view_height) : 0;

        float start_draw_y = current_y;
        if (total_height > view_height) {
            start_draw_y = current_y - (total_height - view_height) + static_cast<float>(scroll_offset_y_);
        }

        RectF clip_rect(padding_x - 2.0f, current_y - 2.0f, card_width + 4.0f, view_height + 4.0f);
        RECT full_target_rc = { 0, 0, width, height };

        // 4. Draw Cards
        for (auto& lay : layouts) {
            float card_top = start_draw_y + config_.theme.message_background.margin.top;
            lay.x = padding_x;
            lay.y = card_top;

            start_draw_y += lay.height + config_.theme.message_background.margin.top + config_.theme.message_background.margin.bottom;

            if (lay.y + lay.height < current_y || lay.y > (float)height) {
                continue; // culled
            }

            text_renderer_.draw_message_card(g, hdc_mem, full_target_rc, lay, config_.theme, current_time_ms);
        }

        g.ResetClip();

        // 5. Draw Scrollbar if enabled or hover
        bool show_sb = config_.theme.scrollbar.visible || (is_hovered_ && config_.theme.scrollbar.interaction_mode != ScrollbarInteractionMode::Disabled && !config_.click_through);
        if (show_sb && max_scroll_y_ > 0) {
            float sb_w = config_.theme.scrollbar.width;
            float sb_x = (float)width - sb_w - 4.0f;
            float track_h = view_height;
            float thumb_h = (std::max)(track_h * (view_height / total_height), 20.0f);

            float scroll_ratio = (max_scroll_y_ > 0) ? (float)(max_scroll_y_ - scroll_offset_y_) / (float)max_scroll_y_ : 0.0f;
            float thumb_y = current_y + scroll_ratio * (track_h - thumb_h);

            RectF thumb_rect(sb_x, thumb_y, sb_w, thumb_h);
            GraphicsPath thumb_path;
            draw_rounded_rect(thumb_path, thumb_rect, config_.theme.scrollbar.corner_radius);

            ColorRGBA sb_col = ColorRGBA::parse(config_.theme.scrollbar.color);
            SolidBrush sb_brush(sb_col.to_gdiplus(0.7f));
            g.FillPath(&sb_brush, &thumb_path);
        }
    }

    // 6. Update Layered Window with Per-Pixel 32-bit Alpha
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
        case WM_TIMER: {
            if (wparam == 1) {
                self->render();
            }
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

        case WM_MOUSELEAVE: {
            self->is_hovered_ = false;
            self->render();
            return 0;
        }

        case WM_MOUSEWHEEL: {
            if (!self->config_.click_through && self->config_.theme.scrollbar.interaction_mode != ScrollbarInteractionMode::Disabled) {
                short delta = GET_WHEEL_DELTA_WPARAM(wparam);
                if (delta > 0) {
                    self->scroll_offset_y_ += 50;
                    if (self->scroll_offset_y_ > self->max_scroll_y_) {
                        self->scroll_offset_y_ = self->max_scroll_y_;
                    }
                    self->auto_scroll_ = false;
                } else {
                    self->scroll_offset_y_ -= 50;
                    if (self->scroll_offset_y_ <= 0) {
                        self->scroll_offset_y_ = 0;
                        self->auto_scroll_ = true;
                    }
                }
                self->render();
            }
            return 0;
        }

        case WM_SIZE: {
            self->config_.window_width = LOWORD(lparam);
            self->config_.window_height = HIWORD(lparam);
            self->render();
            return 0;
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

        case WM_OVERLAY_ADD_MESSAGES: {
            std::vector<ChatMessage>* msgs = (std::vector<ChatMessage>*)lparam;
            if (msgs) {
                self->add_messages(*msgs);
                delete msgs;
            }
            return 0;
        }

        case WM_OVERLAY_UPDATE_STATUS: {
            std::string* status = (std::string*)lparam;
            if (status) {
                self->set_status_text(*status);
                delete status;
            }
            return 0;
        }

        case WM_NCHITTEST: {
            if (self->config_.click_through) {
                return HTTRANSPARENT;
            }

            POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
            RECT rc;
            GetWindowRect(hwnd, &rc);

            int x = pt.x - rc.left;
            int y = pt.y - rc.top;
            int w = rc.right - rc.left;
            int h = rc.bottom - rc.top;

            // Corner Resizing
            if (x < RESIZE_BORDER_WIDTH && y < RESIZE_BORDER_WIDTH) return HTTOPLEFT;
            if (x > w - RESIZE_BORDER_WIDTH && y < RESIZE_BORDER_WIDTH) return HTTOPRIGHT;
            if (x < RESIZE_BORDER_WIDTH && y > h - RESIZE_BORDER_WIDTH) return HTBOTTOMLEFT;
            if (x > w - RESIZE_BORDER_WIDTH && y > h - RESIZE_BORDER_WIDTH) return HTBOTTOMRIGHT;

            // Edge Resizing
            if (x < RESIZE_BORDER_WIDTH) return HTLEFT;
            if (x > w - RESIZE_BORDER_WIDTH) return HTRIGHT;
            if (y < RESIZE_BORDER_WIDTH) return HTTOP;
            if (y > h - RESIZE_BORDER_WIDTH) return HTBOTTOM;

            // Top Drag Handle (Allows moving the window)
            if (y <= 36) return HTCAPTION;

            return HTCLIENT;
        }

        case WM_DESTROY: {
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}
