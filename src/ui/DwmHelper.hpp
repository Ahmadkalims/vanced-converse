#pragma once

#include <windows.h>
#include <dwmapi.h>
#include "../common/Theme.hpp"

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

#ifndef DWMSBT_NONE
#define DWMSBT_NONE 1
#define DWMSBT_MAINWINDOW 2   // Mica
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

class DwmHelper {
public:
    static void apply_backdrop(HWND hwnd, BlurType blur_type, uint8_t opacity, ThemeColor tint_color) {
        if (!hwnd) return;

        // Try Windows 11 DwmSetWindowAttribute first
        int backdrop_type = DWMSBT_NONE;
        if (blur_type == BlurType::Acrylic) backdrop_type = DWMSBT_TRANSIENTWINDOW;
        else if (blur_type == BlurType::Mica) backdrop_type = DWMSBT_MAINWINDOW;
        else if (blur_type == BlurType::LiquidGlass) backdrop_type = DWMSBT_TABBEDWINDOW;

        DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop_type, sizeof(backdrop_type));

        // Try Windows 10/11 SetWindowCompositionAttribute (Undocumented API in user32)
        HMODULE h_user32 = GetModuleHandleW(L"user32.dll");
        if (h_user32) {
            pfnSetWindowCompositionAttribute SetWindowCompositionAttribute =
                (pfnSetWindowCompositionAttribute)GetProcAddress(h_user32, "SetWindowCompositionAttribute");
            
            if (SetWindowCompositionAttribute) {
                ACCENT_POLICY policy = { ACCENT_DISABLED, 0, 0, 0 };

                if (blur_type == BlurType::Acrylic || blur_type == BlurType::LiquidGlass) {
                    policy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
                    policy.AccentFlags = 2; // draw border
                    DWORD a = (DWORD)(opacity);
                    DWORD r = (DWORD)(tint_color.r);
                    DWORD g = (DWORD)(tint_color.g);
                    DWORD b = (DWORD)(tint_color.b);
                    policy.GradientColor = (a << 24) | (b << 16) | (g << 8) | r;
                } else if (blur_type == BlurType::Mica) {
                    policy.AccentState = ACCENT_ENABLE_BLURBEHIND;
                } else {
                    policy.AccentState = ACCENT_DISABLED;
                }

                WINCOMPATTRDATA data = { 19, &policy, sizeof(policy) }; // WCA_ACCENT_POLICY = 19
                SetWindowCompositionAttribute(hwnd, &data);
            }
        }
    }

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
};
