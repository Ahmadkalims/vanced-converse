#pragma once

#include <windows.h>
#include <dwmapi.h>
#include <algorithm>
#include "../common/ThemeModel.hpp"

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

#ifndef DWMSBT_NONE
#define DWMSBT_NONE 1
#define DWMSBT_MAINWINDOW 2      // Mica
#define DWMSBT_TRANSIENTWINDOW 3 // Acrylic
#define DWMSBT_TABBEDWINDOW 4    // Mica Alt
#endif

enum ACCENT_STATE {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_ENABLE_HOSTBACKDROP = 5,
    ACCENT_INVALID_STATE = 6
};

struct ACCENT_POLICY {
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor;
    DWORD AnimationId;
};

struct WINCOMPATTRDATA {
    DWORD attribute;
    PVOID pData;
    ULONG dataSize;
};

typedef BOOL(WINAPI* pfnSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

/**
 * @brief Helper utility for Windows Desktop Window Manager (DWM) effects.
 * Supports Windows 10 & Windows 11 dual backdrop materials (Acrylic, Mica, BlurBehind)
 * and seamless rounded window region clipping.
 */
class DwmHelper {
public:
    /**
     * @brief Applies system backdrop / blur effect to window (Windows 10 & 11 compatible).
     * @param hwnd Target window handle
     * @param type Overlay background material type
     * @param opacity Window opacity [0.0 - 1.0]
     * @param tint_color Base tint color
     */
    static void apply_backdrop(HWND hwnd, OverlayBgType type, float opacity, const ColorRGBA& tint_color) {
        if (!hwnd) return;

        // 1. Try Windows 11 DwmSetWindowAttribute (Build 22000+)
        int backdrop_type = DWMSBT_NONE;
        if (type == OverlayBgType::Acrylic) backdrop_type = DWMSBT_TRANSIENTWINDOW;
        else if (type == OverlayBgType::Mica) backdrop_type = DWMSBT_MAINWINDOW;
        else if (type == OverlayBgType::LiquidGlass) backdrop_type = DWMSBT_TABBEDWINDOW;

        DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop_type, sizeof(backdrop_type));

        // 2. Try Windows 10 & 11 SetWindowCompositionAttribute (user32 undocumented API)
        HMODULE h_user32 = GetModuleHandleW(L"user32.dll");
        if (h_user32) {
            pfnSetWindowCompositionAttribute pSetAttr =
                (pfnSetWindowCompositionAttribute)GetProcAddress(h_user32, "SetWindowCompositionAttribute");
            
            if (pSetAttr) {
                ACCENT_POLICY policy = { ACCENT_DISABLED, 0, 0, 0 };

                if (type == OverlayBgType::Acrylic || type == OverlayBgType::LiquidGlass) {
                    policy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
                    policy.AccentFlags = 2; // draw border
                    DWORD a = (DWORD)(std::clamp(opacity * 255.0f, 0.0f, 255.0f));
                    DWORD r = (DWORD)(tint_color.r);
                    DWORD g = (DWORD)(tint_color.g);
                    DWORD b = (DWORD)(tint_color.b);
                    policy.GradientColor = (a << 24) | (b << 16) | (g << 8) | r;
                } else if (type == OverlayBgType::Mica) {
                    policy.AccentState = ACCENT_ENABLE_BLURBEHIND;
                } else {
                    policy.AccentState = ACCENT_DISABLED;
                }

                WINCOMPATTRDATA data = { 19, &policy, sizeof(policy) }; // WCA_ACCENT_POLICY = 19
                pSetAttr(hwnd, &data);
            }
        }
    }

    /**
     * @brief Configures mouse click-through behavior using WS_EX_TRANSPARENT.
     */
    static void set_click_through(HWND hwnd, bool enable) {
        if (!hwnd) return;
        LONG_PTR ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        if (enable) {
            ex_style |= WS_EX_TRANSPARENT;
        } else {
            ex_style &= ~WS_EX_TRANSPARENT;
        }
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, ex_style);
    }

    /**
     * @brief Clips the window region to match the corner radius, preventing DWM backdrop blur
     * from leaking outside rounded corners.
     */
    static void update_window_region(HWND hwnd, int width, int height, float corner_radius) {
        if (!hwnd || width <= 0 || height <= 0) return;
        if (corner_radius <= 1.0f) {
            SetWindowRgn(hwnd, NULL, TRUE);
        } else {
            int r = static_cast<int>(corner_radius);
            HRGN hrgn = CreateRoundRectRgn(0, 0, width + 1, height + 1, r * 2, r * 2);
            SetWindowRgn(hwnd, hrgn, TRUE);
        }
    }
};
