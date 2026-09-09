#include <windows.h>
#include <ole2.h>
#include <gdiplus.h>
#include "common/Config.hpp"
#include "common/ThemeManager.hpp"
#include "net/YouTubeEngine.hpp"
#include "net/EmojiCache.hpp"
#include "ui/OverlayWindow.hpp"
#include "ui/SettingsWindow.hpp"
#include "ui/TrayManager.hpp"
#include <memory>
#include <filesystem>


static const wchar_t* MAIN_MSG_CLASS = L"YT_Live_Chat_Main_Msg_Class";

static HWND g_hwnd_msg = NULL;
static std::unique_ptr<OverlayWindow> g_overlay;
static std::unique_ptr<SettingsWindow> g_settings;
static std::unique_ptr<TrayManager> g_tray;
static std::unique_ptr<YouTubeEngine> g_engine;

LRESULT CALLBACK MainMsgWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_TRAY_NOTIFY: {
            if (lparam == WM_RBUTTONUP || lparam == WM_CONTEXTMENU) {
                if (g_tray) g_tray->show_context_menu();
            } else if (lparam == WM_LBUTTONDBLCLK || lparam == WM_LBUTTONUP) {
                if (g_settings) g_settings->show(0);
            }
            return 0;
        }

        case WM_COMMAND: {
            int id = LOWORD(wparam);
            AppConfig cfg = g_config.get();

            switch (id) {
                case IDM_TRAY_SHOW_HIDE: {
                    cfg.show_overlay = !cfg.show_overlay;
                    g_config.update(cfg, true);
                    if (g_overlay) g_overlay->show(cfg.show_overlay);
                    if (g_engine) {
                        if (cfg.show_overlay) g_engine->resume();
                        else g_engine->pause();
                    }
                    break;
                }
                case IDM_TRAY_CLICKTHROUGH: {
                    cfg.click_through = !cfg.click_through;
                    g_config.update(cfg, true);
                    if (g_overlay) g_overlay->set_click_through(cfg.click_through);
                    break;
                }
                case IDM_TRAY_TOPMOST: {
                    cfg.always_on_top = !cfg.always_on_top;
                    g_config.update(cfg, true);
                    if (g_overlay) g_overlay->set_always_on_top(cfg.always_on_top);
                    break;
                }
                case IDM_TRAY_APPEARANCE: {
                    if (g_settings) g_settings->show(1); // tab 1: Appearance
                    break;
                }
                case IDM_TRAY_SETTINGS: {
                    if (g_settings) g_settings->show(0); // tab 0: Stream
                    break;
                }
                case IDM_TRAY_QUIT: {
                    if (g_engine) g_engine->stop();
                    PostQuitMessage(0);
                    break;
                }
            }
            return 0;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    OleInitialize(NULL);

    ULONG_PTR gdi_token = 0;
    Gdiplus::GdiplusStartupInput gdi_in;
    Gdiplus::GdiplusStartup(&gdi_token, &gdi_in, NULL);

    // 1. Single instance check & focus existing
    HANDLE h_mutex = CreateMutexW(NULL, TRUE, L"Local\\YT_Live_Chat_Overlay_SingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND h_existing = FindWindowW(MAIN_MSG_CLASS, L"YT_Live_Chat_Msg_Sink");
        if (h_existing) {
            PostMessageW(h_existing, WM_COMMAND, IDM_TRAY_SETTINGS, 0);
        } else {
            HWND h_settings = FindWindowW(L"YT_Live_Chat_Settings_Class", NULL);
            if (h_settings) {
                ShowWindow(h_settings, SW_RESTORE);
                SetForegroundWindow(h_settings);
            }
        }
        if (gdi_token) Gdiplus::GdiplusShutdown(gdi_token);
        OleUninitialize();
        return 0;
    }

    // 2. Initialize Themes and Configuration
    g_theme_manager.init();
    g_config.load();
    AppConfig initial_cfg = g_config.get();

    // 3. Register Hidden Message Window Class for Tray Routing

    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = MainMsgWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = MAIN_MSG_CLASS;
    RegisterClassExW(&wc);

    g_hwnd_msg = CreateWindowExW(0, MAIN_MSG_CLASS, L"YT_Live_Chat_Msg_Sink", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

    // 4. Initialize Overlay and Settings Windows
    g_overlay = std::make_unique<OverlayWindow>(hInstance);
    if (!g_overlay->create()) {
        MessageBoxW(NULL, L"Failed to initialize overlay window.", L"Error", MB_OK | MB_ICONERROR);
        if (gdi_token) Gdiplus::GdiplusShutdown(gdi_token);
        OleUninitialize();
        return 1;
    }

    g_settings = std::make_unique<SettingsWindow>(hInstance, g_overlay->get_hwnd());
    g_settings->create();

    // 5. Initialize System Tray
    g_tray = std::make_unique<TrayManager>(hInstance, g_hwnd_msg);
    g_tray->install();

    // 6. Connect Emoji Cache Callback to Re-Render Overlay
    g_emoji_cache.set_on_asset_ready([](const std::string&) {
        if (g_overlay && g_overlay->get_hwnd()) {
            InvalidateRect(g_overlay->get_hwnd(), NULL, FALSE);
        }
    });

    // 7. Initialize YouTube Polling Engine
    g_engine = std::make_unique<YouTubeEngine>();
    
    g_engine->set_callbacks(
        [](const std::vector<ChatMessage>& msgs) {
            if (g_overlay && g_overlay->get_hwnd()) {
                auto* msgs_copy = new std::vector<ChatMessage>(msgs);
                PostMessageW(g_overlay->get_hwnd(), WM_OVERLAY_ADD_MESSAGES, 0, (LPARAM)msgs_copy);
            }
        },
        [](StreamStatus status, const std::string& detail) {
            if (g_overlay && g_overlay->get_hwnd()) {
                auto* status_copy = new std::string(detail);
                PostMessageW(g_overlay->get_hwnd(), WM_OVERLAY_UPDATE_STATUS, 0, (LPARAM)status_copy);
            }
            if (g_settings) {
                g_settings->update_status_text(detail);
            }
        }
    );

    // 8. Subscribe to Live Config Changes
    g_config.add_change_listener([](const AppConfig& cfg) {
        if (g_overlay && g_overlay->get_hwnd()) {
            auto* cfg_copy = new AppConfig(cfg);
            PostMessageW(g_overlay->get_hwnd(), WM_OVERLAY_UPDATE_CONFIG, 0, (LPARAM)cfg_copy);
        }
        if (g_engine) {
            g_engine->update_config(cfg.api_key, cfg.video_id, cfg.poll_interval_sec);
        }
    });

    // 9. Subscribe to Live Theme Manager Changes
    g_theme_manager.add_change_listener([](const StreamTheme& theme) {
        if (g_overlay && g_overlay->get_hwnd()) {
            auto* theme_copy = new StreamTheme(theme);
            PostMessageW(g_overlay->get_hwnd(), WM_OVERLAY_UPDATE_THEME, 0, (LPARAM)theme_copy);
        }
    });

    // 10. Start Polling Engine if credentials configured; always open settings window on direct launch
    if (!initial_cfg.api_key.empty() && !initial_cfg.video_id.empty()) {
        g_engine->start(initial_cfg.api_key, initial_cfg.video_id, initial_cfg.poll_interval_sec);
    }
    g_settings->show(0);

    // 11. Main Message Loop
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // 12. Clean Shutdown
    if (g_engine) g_engine->stop();
    if (g_tray) g_tray->remove();

    if (h_mutex) {
        ReleaseMutex(h_mutex);
        CloseHandle(h_mutex);
    }


    if (gdi_token) Gdiplus::GdiplusShutdown(gdi_token);
    OleUninitialize();
    return (int)msg.wParam;
}

