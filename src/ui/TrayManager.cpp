#include "TrayManager.hpp"
#include "../common/Config.hpp"

TrayManager::TrayManager(HINSTANCE hinstance, HWND hwnd_parent)
    : hinstance_(hinstance), hwnd_parent_(hwnd_parent) {}

TrayManager::~TrayManager() {
    remove();
}

HICON TrayManager::create_default_icon() {
    int cx = GetSystemMetrics(SM_CXSMICON);
    int cy = GetSystemMetrics(SM_CYSMICON);
    if (cx <= 0) cx = 16;
    if (cy <= 0) cy = 16;

    HDC hdc = GetDC(NULL);
    HDC hdc_mem = CreateCompatibleDC(hdc);
    HBITMAP hbm_color = CreateCompatibleBitmap(hdc, cx, cy);
    HBITMAP hbm_mask = CreateBitmap(cx, cy, 1, 1, NULL);

    HBITMAP hbm_old = (HBITMAP)SelectObject(hdc_mem, hbm_color);

    // Draw dark magenta / cyan chat bubble icon
    HBRUSH hbr_bg = CreateSolidBrush(RGB(20, 20, 30));
    RECT rc = { 0, 0, cx, cy };
    FillRect(hdc_mem, &rc, hbr_bg);
    DeleteObject(hbr_bg);

    HBRUSH hbr_bubble = CreateSolidBrush(RGB(244, 114, 182));
    HPEN hpen_bubble = CreatePen(PS_SOLID, 1, RGB(56, 189, 248));
    HGDIOBJ hold_brush = SelectObject(hdc_mem, hbr_bubble);
    HGDIOBJ hold_pen = SelectObject(hdc_mem, hpen_bubble);

    RoundRect(hdc_mem, 1, 1, cx - 1, cy - 3, 4, 4);

    SelectObject(hdc_mem, hold_brush);
    SelectObject(hdc_mem, hold_pen);
    DeleteObject(hbr_bubble);
    DeleteObject(hpen_bubble);

    SelectObject(hdc_mem, hbm_old);
    DeleteDC(hdc_mem);
    ReleaseDC(NULL, hdc);

    ICONINFO ii = { 0 };
    ii.fIcon = TRUE;
    ii.hbmColor = hbm_color;
    ii.hbmMask = hbm_mask;

    HICON h_icon = CreateIconIndirect(&ii);

    DeleteObject(hbm_color);
    DeleteObject(hbm_mask);

    return h_icon;
}

bool TrayManager::install() {
    if (is_installed_) return true;

    nid_.cbSize = sizeof(NOTIFYICONDATAW);
    nid_.hWnd = hwnd_parent_;
    nid_.uID = 1001;
    nid_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid_.uCallbackMessage = WM_TRAY_NOTIFY;
    nid_.hIcon = create_default_icon();
    wcscpy_s(nid_.szTip, L"YouTube Live Chat Overlay (C++)");

    is_installed_ = Shell_NotifyIconW(NIM_ADD, &nid_);
    return is_installed_;
}

void TrayManager::remove() {
    if (is_installed_) {
        Shell_NotifyIconW(NIM_DELETE, &nid_);
        if (nid_.hIcon) {
            DestroyIcon(nid_.hIcon);
            nid_.hIcon = NULL;
        }
        is_installed_ = false;
    }
}

void TrayManager::show_context_menu() {
    POINT pt;
    GetCursorPos(&pt);

    HMENU h_menu = CreatePopupMenu();
    if (!h_menu) return;

    AppConfig cfg = g_config.get();

    AppendMenuW(h_menu, MF_STRING, IDM_TRAY_SHOW_HIDE, cfg.show_overlay ? L"Hide Overlay" : L"Show Overlay");
    
    UINT click_flags = MF_STRING | (cfg.click_through ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(h_menu, click_flags, IDM_TRAY_CLICKTHROUGH, L"Click-Through Mode");

    UINT top_flags = MF_STRING | (cfg.always_on_top ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(h_menu, top_flags, IDM_TRAY_TOPMOST, L"Always on Top");

    AppendMenuW(h_menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(h_menu, MF_STRING, IDM_TRAY_APPEARANCE, L"Appearance & Themes...");
    AppendMenuW(h_menu, MF_STRING, IDM_TRAY_SETTINGS, L"Open Settings Panel...");
    AppendMenuW(h_menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(h_menu, MF_STRING, IDM_TRAY_QUIT, L"Quit");

    SetForegroundWindow(hwnd_parent_);
    TrackPopupMenu(h_menu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd_parent_, NULL);
    DestroyMenu(h_menu);
}
