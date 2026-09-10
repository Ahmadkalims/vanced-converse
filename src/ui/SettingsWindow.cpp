#include "SettingsWindow.hpp"
#include "OverlayWindow.hpp"
#include "../common/UnicodeUtils.hpp"
#include "../common/SimpleJson.hpp"
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

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

static const wchar_t* SETTINGS_CLASS_NAME = L"Vanced_Converse_Settings_Class";

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

// ══════════════════════════════════════════════════════════════════════════════
// WebView2 COM Handlers
// ══════════════════════════════════════════════════════════════════════════════

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

    // Fill client area
    RECT bounds;
    GetClientRect(hwnd, &bounds);
    controller->put_Bounds(bounds);

    // Browser settings
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

    // Resolve settings_ui.html path
    std::string exe_dir = get_exe_dir();
    std::string html_path = exe_dir + "\\settings_ui.html";
    if (!fs::exists(html_path) && fs::exists("settings_ui.html")) {
        html_path = fs::absolute("settings_ui.html").string();
    }

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

// ══════════════════════════════════════════════════════════════════════════════
// SettingsWindow Lifecycle
// ══════════════════════════════════════════════════════════════════════════════

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
}

/**
 * @brief Creates the native top-level settings window with dark Fluent title bar.
 */
bool SettingsWindow::create() {
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = SettingsWindow::WndProc;
    wc.hInstance = hinstance_;
    wc.lpszClassName = SETTINGS_CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(20, 20, 24));

    RegisterClassExW(&wc);

    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);
    int win_w = 1060, win_h = 740;
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

    // Enable dark title bar (Windows 11 & Windows 10 build 17763+)
    BOOL darkMode = TRUE;
    if (FAILED(DwmSetWindowAttribute(hwnd_, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode)))) {
        DwmSetWindowAttribute(hwnd_, 19, &darkMode, sizeof(darkMode));
    }

    // Windows 11 Mica backdrop
    int mica = 2;
    DwmSetWindowAttribute(hwnd_, DWMWA_SYSTEMBACKDROP_TYPE, &mica, sizeof(mica));

    init_webview2();
    return true;
}

/**
 * @brief Initializes Edge WebView2 environment.
 */
void SettingsWindow::init_webview2() {
    wchar_t app_data[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, app_data);
    std::wstring user_data = std::wstring(app_data) + L"\\VancedConverse\\WebView2Data";
    try {
        fs::create_directories(user_data);
    } catch (...) {}

    // 1. Try local WebView2Loader.dll
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

    // 2. Fallback: Edge Runtime
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
        L"Vanced Converse",
        MB_OK | MB_ICONWARNING);
}

void SettingsWindow::on_webview_ready() {
    send_config_to_webview();
}

/**
 * @brief Dispatches the complete current configuration and theme library to WebView2.
 */
void SettingsWindow::send_config_to_webview() {
    if (!webview_ready_ || !webview_) return;

    live_config_ = g_config.get();
    g_theme_manager.scan_themes_directory();
    const auto& theme_list = g_theme_manager.get_available_themes();

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
       << "\"theme\":" << live_config_.theme.to_json_string(false) << ","
       << "\"available_themes\":[";

    for (size_t i = 0; i < theme_list.size(); ++i) {
        if (i > 0) js << ",";
        js << "{\"name\":\"" << theme_list[i].metadata.name << "\","
           << "\"description\":\"" << theme_list[i].metadata.description << "\","
           << "\"color\":\"" << theme_list[i].overlay.color << "\","
           << "\"accent\":\"" << theme_list[i].username.color_value << "\"}";
    }
    js << "]"
       << "}"
       << "}";

    std::wstring w_json = unicode::utf8_to_utf16(js.str());
    webview_->PostWebMessageAsJson(w_json.c_str());
}

/**
 * @brief Handles incoming IPC messages from the settings panel WebView.
 */
void SettingsWindow::handle_webview_message(const std::wstring& json_msg) {
    std::string msg = unicode::utf16_to_utf8(json_msg);
    json::Value root = json::parse(msg);
    if (!root.is_object()) return;

    std::string type = root["type"].as_string("");

    if (type == "save_all" || type == "preview_update") {
        if (root.contains("video_id")) live_config_.video_id = AppConfig::extract_video_id(root["video_id"].as_string(""));
        if (root.contains("api_key")) live_config_.api_key = root["api_key"].as_string("");
        if (root.contains("poll_interval_sec")) live_config_.poll_interval_sec = root["poll_interval_sec"].as_int(live_config_.poll_interval_sec);
        if (root.contains("max_messages")) live_config_.max_messages = root["max_messages"].as_int(live_config_.max_messages);
        if (root.contains("click_through")) live_config_.click_through = root["click_through"].as_bool(live_config_.click_through);
        if (root.contains("always_on_top")) live_config_.always_on_top = root["always_on_top"].as_bool(live_config_.always_on_top);
        if (root.contains("show_overlay")) live_config_.show_overlay = root["show_overlay"].as_bool(live_config_.show_overlay);

        // Parse theme object
        if (root.contains("theme") && root["theme"].is_object()) {
            live_config_.theme.from_json_string(root["theme"].serialize());
        }

        // Apply immediately to runtime and notify overlay
        g_config.update(live_config_, true);
        g_theme_manager.set_active_theme(live_config_.theme);

        if (type == "save_all") {
            g_config.save();
            std::string err;
            g_theme_manager.save_custom_theme(live_config_.theme, &err);
        }

    } else if (type == "load_theme") {
        std::string name = root["name"].as_string("");
        if (!name.empty() && g_theme_manager.set_active_theme_by_name(name)) {
            live_config_.theme = g_theme_manager.get_active_theme();
            live_config_.active_theme_name = live_config_.theme.metadata.name;
            g_config.update(live_config_, true);
            g_config.save();
            send_config_to_webview();
        }

    } else if (type == "theme_duplicate") {
        std::string old_name = root["old_name"].as_string("");
        std::string new_name = root["new_name"].as_string("");
        if (!old_name.empty() && !new_name.empty()) {
            std::string err;
            g_theme_manager.duplicate_theme(old_name, new_name, &err);
            send_config_to_webview();
        }

    } else if (type == "theme_rename") {
        std::string old_name = root["old_name"].as_string("");
        std::string new_name = root["new_name"].as_string("");
        if (!old_name.empty() && !new_name.empty()) {
            std::string err;
            g_theme_manager.rename_theme(old_name, new_name, &err);
            send_config_to_webview();
        }

    } else if (type == "theme_delete") {
        std::string name = root["name"].as_string("");
        if (!name.empty()) {
            std::string err;
            g_theme_manager.delete_theme(name, &err);
            send_config_to_webview();
        }

    } else if (type == "theme_import_dragdrop") {
        std::string css_content = root["content"].as_string("");
        if (!css_content.empty()) {
            StreamTheme t;
            std::string err;
            if (t.from_css_string(css_content, &err)) {
                g_theme_manager.save_custom_theme(t, &err);
                send_config_to_webview();
            }
        }

    } else if (type == "update_video") {
        std::string url = root["url"].as_string("");
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
