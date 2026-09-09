#pragma once

#include <windows.h>
#include <shellapi.h>
#include <functional>

#define WM_TRAY_NOTIFY (WM_USER + 201)

#define IDM_TRAY_SHOW_HIDE       3001
#define IDM_TRAY_CLICKTHROUGH    3002
#define IDM_TRAY_TOPMOST         3003
#define IDM_TRAY_APPEARANCE      3004
#define IDM_TRAY_SETTINGS        3005
#define IDM_TRAY_QUIT            3006

class TrayManager {
private:
    HWND hwnd_parent_ = NULL;
    HINSTANCE hinstance_ = NULL;
    NOTIFYICONDATAW nid_ = { 0 };
    bool is_installed_ = false;

    HICON create_default_icon();

public:
    TrayManager(HINSTANCE hinstance, HWND hwnd_parent);
    ~TrayManager();

    bool install();
    void remove();
    void show_context_menu();
};
