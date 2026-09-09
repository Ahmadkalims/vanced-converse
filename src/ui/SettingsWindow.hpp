#pragma once

#include <windows.h>
#include <string>
#include "../common/Config.hpp"
#include "../common/ThemeManager.hpp"

// Forward declarations for COM callback classes
class EnvCreatedHandler;
class ControllerCreatedHandler;
class WebMessageHandler;
struct ICoreWebView2Controller;
struct ICoreWebView2;

class SettingsWindow {
private:
    HWND hwnd_ = NULL;
    HINSTANCE hinstance_ = NULL;
    HWND hwnd_overlay_ = NULL;
    
    // WebView2 COM interfaces
    ICoreWebView2Controller* webview_controller_ = nullptr;
    ICoreWebView2* webview_ = nullptr;
    bool webview_ready_ = false;

    AppConfig live_config_;

    void init_webview2();
    void on_webview_ready();
    void send_config_to_webview();
    void handle_webview_message(const std::wstring& json_msg);
    
    void on_update_video(const std::string& url);

    // Allow COM callback classes to access private members
    friend class EnvCreatedHandler;
    friend class ControllerCreatedHandler;
    friend class WebMessageHandler;

public:
    SettingsWindow(HINSTANCE hinstance, HWND hwnd_overlay);
    ~SettingsWindow();

    bool create();
    void show(int initial_tab = 0);
    void update_status_text(const std::string& status);

    HWND get_hwnd() const { return hwnd_; }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};
