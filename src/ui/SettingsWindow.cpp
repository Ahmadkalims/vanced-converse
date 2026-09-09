#include "SettingsWindow.hpp"
#include "OverlayWindow.hpp"
#include "../common/UnicodeUtils.hpp"
#include <windows.h>
#include <shellapi.h>
#include <commdlg.h>
#include <dwmapi.h>
#include <shlobj.h>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <WebView2.h>

namespace fs = std::filesystem;

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#ifndef DWMWA_MICA_EFFECT
#define DWMWA_MICA_EFFECT 1029
#endif

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

static const wchar_t* SETTINGS_CLASS_NAME = L"YT_Live_Chat_Settings_Class";

typedef HRESULT (STDAPICALLTYPE *CreateWebView2EnvWithOptionsFn)(
    PCWSTR browserExecutableFolder,
    PCWSTR userDataFolder,
    ICoreWebView2EnvironmentOptions* environmentOptions,
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* environmentCreatedHandler);

static std::string get_exe_dir() {
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(NULL, buf, MAX_PATH);
    std::wstring wpath(buf);
    auto pos = wpath.find_last_of(L'\\');
    if (pos != std::wstring::npos) wpath = wpath.substr(0, pos);
    return unicode::utf16_to_utf8(wpath);
}

// ═══════════════════════════════════════════════
// WebView2 COM Handlers
// ═══════════════════════════════════════════════

class EnvCreatedHandler : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler {
public:
    SettingsWindow* owner;
    HWND hwnd;
    LONG ref_count = 1;

    EnvCreatedHandler(SettingsWindow* o, HWND h) : owner(o), hwnd(h) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler) {
            *ppv = static_cast<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override {
        return InterlockedIncrement(&ref_count);
    }

    ULONG STDMETHODCALLTYPE Release() override {
        LONG r = InterlockedDecrement(&ref_count);
        if (r == 0) delete this;
        return r;
    }

    HRESULT STDMETHODCALLTYPE Invoke(HRESULT result, ICoreWebView2Environment* env) override;
};

class ControllerCreatedHandler : public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler {
public:
    SettingsWindow* owner;
    HWND hwnd;
    LONG ref_count = 1;

    ControllerCreatedHandler(SettingsWindow* o, HWND h) : owner(o), hwnd(h) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler) {
            *ppv = static_cast<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override {
        return InterlockedIncrement(&ref_count);
    }

    ULONG STDMETHODCALLTYPE Release() override {
        LONG r = InterlockedDecrement(&ref_count);
        if (r == 0) delete this;
        return r;
    }

    HRESULT STDMETHODCALLTYPE Invoke(HRESULT result, ICoreWebView2Controller* controller) override;
};

class WebMessageHandler : public ICoreWebView2WebMessageReceivedEventHandler {
public:
    SettingsWindow* owner;
    LONG ref_count = 1;

    WebMessageHandler(SettingsWindow* o) : owner(o) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_ICoreWebView2WebMessageReceivedEventHandler) {
            *ppv = static_cast<ICoreWebView2WebMessageReceivedEventHandler*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override {
        return InterlockedIncrement(&ref_count);
    }

    ULONG STDMETHODCALLTYPE Release() override {
        LONG r = InterlockedDecrement(&ref_count);
        if (r == 0) delete this;
        return r;
    }

    HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) override;
};

HRESULT EnvCreatedHandler::Invoke(HRESULT result, ICoreWebView2Environment* env) {
    if (FAILED(result) || !env) return result;
    auto* handler = new ControllerCreatedHandler(owner, hwnd);
    HRESULT hr = env->CreateCoreWebView2Controller(hwnd, handler);
    handler->Release();
    return hr;
}

HRESULT ControllerCreatedHandler::Invoke(HRESULT result, ICoreWebView2Controller* controller) {
    if (FAILED(result) || !controller) return result;

    controller->AddRef();
    owner->webview_controller_ = controller;

    ICoreWebView2* webview = nullptr;
    controller->get_CoreWebView2(&webview);
    owner->webview_ = webview;

    // Resize WebView to fill client area
    RECT bounds;
    GetClientRect(hwnd, &bounds);
    controller->put_Bounds(bounds);

    // Configure settings
    ICoreWebView2Settings* settings = nullptr;
    if (SUCCEEDED(webview->get_Settings(&settings)) && settings) {
        settings->put_IsWebMessageEnabled(TRUE);
        settings->put_AreDefaultContextMenusEnabled(FALSE);
        settings->put_IsZoomControlEnabled(FALSE);
        settings->put_IsStatusBarEnabled(FALSE);
        settings->put_AreDevToolsEnabled(FALSE);
        settings->Release();
    }

    // Register message handler
    auto* msgHandler = new WebMessageHandler(owner);
    EventRegistrationToken token;
    webview->add_WebMessageReceived(msgHandler, &token);
    msgHandler->Release();

    // Navigate to local settings_ui.html
    std::string exe_dir = get_exe_dir();
    std::string html_path = exe_dir + "\\settings_ui.html";
    if (fs::exists(html_path)) {
        std::wstring w_path = L"file:///" + unicode::utf8_to_utf16(html_path);
        for (auto& c : w_path) if (c == L'\\') c = L'/';
        webview->Navigate(w_path.c_str());
    }

    owner->webview_ready_ = true;
    owner->on_webview_ready();

    return S_OK;
}

HRESULT WebMessageHandler::Invoke(ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) {
    LPWSTR msg = nullptr;
    if (SUCCEEDED(args->TryGetWebMessageAsString(&msg)) && msg) {
        owner->handle_webview_message(msg);
        CoTaskMemFree(msg);
    } else if (SUCCEEDED(args->get_WebMessageAsJson(&msg)) && msg) {
        owner->handle_webview_message(msg);
        CoTaskMemFree(msg);
    }
    return S_OK;
}

// ═══════════════════════════════════════════════
// SettingsWindow Lifecycle
// ═══════════════════════════════════════════════

SettingsWindow::SettingsWindow(HINSTANCE hinstance, HWND hwnd_overlay)
    : hinstance_(hinstance), hwnd_overlay_(hwnd_overlay) {
    live_config_ = g_config.get();
}

SettingsWindow::~SettingsWindow() {
    if (webview_) {
        webview_->Release();
        webview_ = nullptr;
    }
    if (webview_controller_) {
        webview_controller_->Close();
        webview_controller_->Release();
        webview_controller_ = nullptr;
    }
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = NULL;
    }
}

bool SettingsWindow::create() {
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = SettingsWindow::WndProc;
    wc.hInstance = hinstance_;
    wc.lpszClassName = SETTINGS_CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(17, 17, 20)); // Match dark Fluent background

    RegisterClassExW(&wc);

    // Calculate centered position
    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);
    int win_w = 1040, win_h = 720;
    int x = (screen_w - win_w) / 2;
    int y = (screen_h - win_h) / 2;

    hwnd_ = CreateWindowExW(
        WS_EX_APPWINDOW,
        SETTINGS_CLASS_NAME,
        L"Vanced Converse — Settings & Themes",
        WS_OVERLAPPEDWINDOW,
        x, y, win_w, win_h,
        NULL, NULL, hinstance_, this
    );

    if (!hwnd_) return false;

    // Enable dark titlebar
    BOOL darkMode = TRUE;
    DwmSetWindowAttribute(hwnd_, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));

    // Mica dynamic material on Windows 11
    int mica = 2;
    DwmSetWindowAttribute(hwnd_, DWMWA_SYSTEMBACKDROP_TYPE, &mica, sizeof(mica));

    init_webview2();
    return true;
}

void SettingsWindow::init_webview2() {
    wchar_t app_data[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, app_data);
    std::wstring user_data = std::wstring(app_data) + L"\\VancedConverse\\WebView2Data";
    try {
        fs::create_directories(user_data);
    } catch (...) {}

    // Load WebView2Loader.dll from exe directory
    std::wstring loader_path = unicode::utf8_to_utf16(get_exe_dir()) + L"\\WebView2Loader.dll";
    HMODULE loader = LoadLibraryW(loader_path.c_str());
    if (!loader) {
        loader = LoadLibraryW(L"WebView2Loader.dll");
    }

    if (loader) {
        auto createEnv = (CreateWebView2EnvWithOptionsFn)GetProcAddress(loader, "CreateCoreWebView2EnvironmentWithOptions");
        if (createEnv) {
            auto* handler = new EnvCreatedHandler(this, hwnd_);
            HRESULT hr = createEnv(nullptr, user_data.c_str(), nullptr, handler);
            handler->Release();
            if (SUCCEEDED(hr)) return;
        }
    }

    // Fallback: system Edge runtime
    HMODULE edge = LoadLibraryW(L"EmbeddedBrowserWebView.dll");
    if (edge) {
        auto createEnv = (CreateWebView2EnvWithOptionsFn)GetProcAddress(edge, "CreateCoreWebView2EnvironmentWithOptions");
        if (createEnv) {
            auto* handler = new EnvCreatedHandler(this, hwnd_);
            HRESULT hr = createEnv(nullptr, user_data.c_str(), nullptr, handler);
            handler->Release();
            if (SUCCEEDED(hr)) return;
        }
    }

    MessageBoxW(hwnd_,
        L"Microsoft Edge WebView2 Runtime is required to render the settings panel.\n\nPlease ensure WebView2 Runtime is installed.",
        L"Vanced Converse — Settings",
        MB_OK | MB_ICONWARNING);
}

void SettingsWindow::on_webview_ready() {
    send_config_to_webview();
}

void SettingsWindow::send_config_to_webview() {
    if (!webview_ready_ || !webview_) return;

    live_config_ = g_config.get();

    std::ostringstream js;
    js << "{"
       << "\"type\":\"apply_config\","
       << "\"config\":{"
       << "\"video_id\":\"" << live_config_.video_id << "\","
       << "\"api_key\":\"" << live_config_.api_key << "\","
       << "\"poll_interval_sec\":" << live_config_.poll_interval_sec << ","
       << "\"max_messages\":" << live_config_.max_messages << ","
       << "\"click_through\":" << (live_config_.click_through ? "true" : "false") << ","
       << "\"always_on_top\":" << (live_config_.always_on_top ? "true" : "false") << ","
       << "\"show_overlay\":" << (live_config_.show_overlay ? "true" : "false") << ","
       << "\"active_theme\":\"" << live_config_.active_theme_name << "\","
       << "\"theme\":{"
       << "\"overlay_opacity\":" << std::fixed << std::setprecision(2) << live_config_.theme.overlay.opacity << ","
       << "\"overlay_blur\":" << (int)live_config_.theme.overlay.blur << ","
       << "\"overlay_radius\":" << (int)live_config_.theme.overlay.corner_radius << ","
       << "\"border_width\":" << (int)live_config_.theme.overlay.outline_size << ","
       << "\"border_color\":\"" << live_config_.theme.overlay.outline_color << "\","
       << "\"font_size\":" << (int)live_config_.theme.message_text.font.size << ","
       << "\"username_color\":\"" << live_config_.theme.username.color_value << "\","
       << "\"message_color\":\"" << live_config_.theme.message_text.color << "\","
       << "\"bg_color\":\"" << live_config_.theme.overlay.color << "\""
       << "}"
       << "}"
       << "}";

    std::wstring w_json = unicode::utf8_to_utf16(js.str());
    webview_->PostWebMessageAsJson(w_json.c_str());
}

void SettingsWindow::handle_webview_message(const std::wstring& json_msg) {
    std::string msg = unicode::utf16_to_utf8(json_msg);

    auto find_str = [&](const std::string& key) -> std::string {
        std::string search = "\"" + key + "\":\"";
        auto pos = msg.find(search);
        if (pos == std::string::npos) {
            // Check without quotes for key
            search = "\"" + key + "\": \"";
            pos = msg.find(search);
            if (pos == std::string::npos) return "";
        }
        pos += search.size();
        auto end = msg.find('"', pos);
        if (end == std::string::npos) return "";
        return msg.substr(pos, end - pos);
    };

    auto find_int = [&](const std::string& key) -> int {
        std::string search = "\"" + key + "\":";
        auto pos = msg.find(search);
        if (pos == std::string::npos) return -1;
        pos += search.size();
        while (pos < msg.size() && (msg[pos] == ' ' || msg[pos] == '\t')) pos++;
        return std::atoi(msg.c_str() + pos);
    };

    auto find_bool = [&](const std::string& key) -> int {
        std::string search = "\"" + key + "\":";
        auto pos = msg.find(search);
        if (pos == std::string::npos) return -1;
        pos += search.size();
        while (pos < msg.size() && (msg[pos] == ' ' || msg[pos] == '\t')) pos++;
        if (msg[pos] == 't' || msg[pos] == 'T') return 1;
        if (msg[pos] == 'f' || msg[pos] == 'F') return 0;
        return -1;
    };

    auto find_float = [&](const std::string& key) -> float {
        std::string search = "\"" + key + "\":";
        auto pos = msg.find(search);
        if (pos == std::string::npos) return -1.0f;
        pos += search.size();
        while (pos < msg.size() && (msg[pos] == ' ' || msg[pos] == '\t')) pos++;
        return (float)std::atof(msg.c_str() + pos);
    };

    std::string type = find_str("type");

    if (type == "save_all" || type == "preview_update") {
        std::string vid = find_str("video_id");
        if (!vid.empty()) live_config_.video_id = AppConfig::extract_video_id(vid);
        
        std::string key = find_str("api_key");
        if (!key.empty()) live_config_.api_key = key;

        int poll = find_int("poll_interval_sec");
        if (poll > 0) live_config_.poll_interval_sec = poll;

        int maxm = find_int("max_messages");
        if (maxm > 0) live_config_.max_messages = maxm;

        int ct = find_bool("click_through");
        if (ct >= 0) live_config_.click_through = (ct == 1);

        int aot = find_bool("always_on_top");
        if (aot >= 0) live_config_.always_on_top = (aot == 1);

        int so = find_bool("show_overlay");
        if (so >= 0) live_config_.show_overlay = (so == 1);

        // Theme properties
        float op = find_float("overlay_opacity");
        if (op >= 0.0f) live_config_.theme.overlay.opacity = op;

        int blur = find_int("overlay_blur");
        if (blur >= 0) live_config_.theme.overlay.blur = (float)blur;

        int rad = find_int("overlay_radius");
        if (rad >= 0) live_config_.theme.overlay.corner_radius = (float)rad;

        int bw = find_int("border_width");
        if (bw >= 0) {
            live_config_.theme.overlay.outline_size = (float)bw;
            live_config_.theme.overlay.outline_enabled = (bw > 0);
        }

        std::string bc = find_str("border_color");
        if (!bc.empty()) live_config_.theme.overlay.outline_color = bc;

        int fs_val = find_int("font_size");
        if (fs_val > 0) {
            live_config_.theme.message_text.font.size = (float)fs_val;
            live_config_.theme.username.font.size = (float)fs_val;
        }

        std::string ucol = find_str("username_color");
        if (!ucol.empty()) live_config_.theme.username.color_value = ucol;

        std::string mcol = find_str("message_color");
        if (!mcol.empty()) live_config_.theme.message_text.color = mcol;

        std::string bgcol = find_str("bg_color");
        if (!bgcol.empty()) live_config_.theme.overlay.color = bgcol;

        // Apply to runtime config immediately
        g_config.update(live_config_, true);
        g_theme_manager.set_active_theme(live_config_.theme);

        if (type == "save_all") {
            g_config.save();
            std::string err;
            g_theme_manager.save_custom_theme(live_config_.theme, &err);
        }

    } else if (type == "load_theme") {
        std::string name = find_str("name");
        if (!name.empty()) {
            if (g_theme_manager.set_active_theme_by_name(name)) {
                live_config_.theme = g_theme_manager.get_active_theme();
                live_config_.active_theme_name = live_config_.theme.metadata.name;
                g_config.update(live_config_, true);
                g_config.save();
                send_config_to_webview();
            }
        }

    } else if (type == "update_video") {
        std::string url = find_str("url");
        on_update_video(url);

    } else if (type == "import_theme") {
        OPENFILENAMEW ofn = { sizeof(OPENFILENAMEW) };
        wchar_t sz_file[MAX_PATH] = { 0 };
        ofn.hwndOwner = hwnd_;
        ofn.lpstrFilter = L"CSS Theme Files (*.css)\0*.css\0All Files (*.*)\0*.*\0";
        ofn.lpstrFile = sz_file;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        if (GetOpenFileNameW(&ofn)) {
            std::string utf8_path = unicode::utf16_to_utf8(sz_file);
            std::string err;
            if (g_theme_manager.import_theme_from_file(utf8_path, &err)) {
                live_config_.theme = g_theme_manager.get_active_theme();
                live_config_.active_theme_name = live_config_.theme.metadata.name;
                g_config.update(live_config_, true);
                g_config.save();
                send_config_to_webview();
            } else if (!err.empty()) {
                std::wstring werr = unicode::utf8_to_utf16("Theme Import Error:\n" + err);
                MessageBoxW(hwnd_, werr.c_str(), L"Import Failed", MB_OK | MB_ICONWARNING);
            }
        }

    } else if (type == "export_theme") {
        OPENFILENAMEW ofn = { sizeof(OPENFILENAMEW) };
        wchar_t sz_file[MAX_PATH] = L"MyCustomTheme.css";
        ofn.hwndOwner = hwnd_;
        ofn.lpstrFilter = L"CSS Theme Files (*.css)\0*.css\0";
        ofn.lpstrFile = sz_file;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_OVERWRITEPROMPT;
        if (GetSaveFileNameW(&ofn)) {
            std::string err;
            g_theme_manager.export_theme_to_file(live_config_.active_theme_name, unicode::utf16_to_utf8(sz_file), &err);
        }
    }
}

void SettingsWindow::on_update_video(const std::string& url) {
    std::string extracted = AppConfig::extract_video_id(url);
    if (extracted.empty()) return;

    live_config_.video_id = extracted;
    g_config.update(live_config_, true);
    g_config.save();
}

void SettingsWindow::show(int initial_tab) {
    if (!hwnd_) create();

    live_config_ = g_config.get();
    send_config_to_webview();

    ShowWindow(hwnd_, SW_RESTORE);
    ShowWindow(hwnd_, SW_SHOW);
    SetForegroundWindow(hwnd_);

    if (webview_ready_ && webview_ && initial_tab > 0) {
        std::wstring script;
        if (initial_tab == 1)
            script = L"document.querySelector('[data-page=\"theme-loader\"]')?.click();";
        else if (initial_tab == 2)
            script = L"document.querySelector('[data-page=\"overlay\"]')?.click();";
        webview_->ExecuteScript(script.c_str(), nullptr);
    }
}

void SettingsWindow::update_status_text(const std::string& status) {
    if (!webview_ready_ || !webview_) return;
    std::wstring json = L"{\"type\":\"status\",\"text\":\"" + unicode::utf8_to_utf16(status) + L"\"}";
    webview_->PostWebMessageAsJson(json.c_str());
}

LRESULT CALLBACK SettingsWindow::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    SettingsWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = (CREATESTRUCT*)lparam;
        self = (SettingsWindow*)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)self);
    } else {
        self = (SettingsWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (!self) return DefWindowProc(hwnd, msg, wparam, lparam);

    switch (msg) {
        case WM_SIZE: {
            if (self->webview_controller_) {
                RECT bounds;
                GetClientRect(hwnd, &bounds);
                self->webview_controller_->put_Bounds(bounds);
            }
            return 0;
        }

        case WM_CLOSE: {
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}
