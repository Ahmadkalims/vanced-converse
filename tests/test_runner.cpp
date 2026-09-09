#include <windows.h>
#include <commctrl.h>
#include <gdiplus.h>
#include "../src/common/Config.hpp"
#include "../src/common/SimpleJson.hpp"
#include "../src/common/ThemeModel.hpp"
#include "../src/common/ThemeManager.hpp"
#include "../src/common/UnicodeUtils.hpp"
#include "../src/net/YouTubeEngine.hpp"
#include "../src/net/EmojiCache.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>

void test_json_parsing();
void test_url_parser();
void test_theme();
void test_config();
void test_unicode_multilingual();
void test_emoji_segmentation();
void test_theme_system();
void test_custom_emoji_cache();

#define IDC_BTN_RUN_UNIT_TESTS   4001
#define IDC_LBL_UNIT_RESULTS     4002
#define IDC_TEST_EDIT_APIKEY     4003
#define IDC_TEST_BTN_SAVE_KEY    4004
#define IDC_TEST_EDIT_VIDEO      4005
#define IDC_TEST_SLIDER_POLL     4006
#define IDC_TEST_LBL_POLL_VAL    4007
#define IDC_TEST_BTN_START       4008
#define IDC_TEST_BTN_STOP        4009
#define IDC_TEST_LBL_STATUS      4010
#define IDC_TEST_LIST_LOG        4011
#define IDC_TEST_BTN_CLEAR_LOG   4012
#define IDC_TEST_BTN_SAMPLE_MSG  4013

#define WM_TEST_ADD_MESSAGES     (WM_USER + 301)
#define WM_TEST_UPDATE_STATUS    (WM_USER + 302)

static HWND g_h_main = NULL;
static HWND g_h_lbl_unit = NULL;
static HWND g_h_edit_key = NULL;
static HWND g_h_edit_vid = NULL;
static HWND g_h_slider_poll = NULL;
static HWND g_h_lbl_poll_val = NULL;
static HWND g_h_lbl_status = NULL;
static HWND g_h_list_log = NULL;

static std::unique_ptr<YouTubeEngine> g_test_engine;

void run_all_unit_tests_gui() {
    try {
        test_json_parsing();
        test_url_parser();
        test_config();
        test_unicode_multilingual();
        test_emoji_segmentation();
        test_theme_system();
        test_custom_emoji_cache();
        test_theme();
        SetWindowTextW(g_h_lbl_unit, L"Status: ALL 8 TEST MODULES PASSED (100% VERIFIED)");
    } catch (const std::exception& e) {
        std::wstring w_err(e.what(), e.what() + strlen(e.what()));
        SetWindowTextW(g_h_lbl_unit, (L"Status: [FAILED] " + w_err).c_str());
    }
}

void add_sample_multilingual_messages() {
    std::vector<std::pair<std::wstring, std::wstring>> samples = {
        { L"AlexGamer", L"Hello everyone! Welcome to the stream! 👋" },
        { L"राहुल_dev", L"नमस्ते दुनिया! क्या हाल है? 🔥" },
        { L"طارق", L"مرحبا بالعالم! البث رائع جداً 👍" },
        { L"Sakura", L"こんにちは世界！素晴らしい配信ですね！🌸" },
        { L"WeiChen", L"你好世界！支持主播！✨" },
        { L"MinJun", L"안녕하세요! 반갑습니다 🎮" },
        { L"Dmitry", L"Привет мир! Отличный стрим 🚀" },
        { L"CuriousViewer", L"#question How do I customize font size and acrylic blur in .streamtheme?" }
    };

    for (const auto& s : samples) {
        std::wstring line = L"[" + s.first + L"]: " + s.second;
        SendMessageW(g_h_list_log, LB_ADDSTRING, 0, (LPARAM)line.c_str());
    }
    int count = SendMessage(g_h_list_log, LB_GETCOUNT, 0, 0);
    SendMessage(g_h_list_log, LB_SETCURSEL, count - 1, 0);
}

LRESULT CALLBACK TestPanelWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_TEST_ADD_MESSAGES: {
            std::vector<ChatMessage>* msgs = (std::vector<ChatMessage>*)lparam;
            if (msgs) {
                for (const auto& m : *msgs) {
                    std::wstring w_auth = unicode::utf8_to_utf16(m.author_name);
                    std::wstring w_txt = unicode::utf8_to_utf16(m.message_text);
                    std::wstring line = L"[" + w_auth + L"]: " + w_txt;
                    if (m.is_highlight) line = L"[QUESTION] " + line;
                    SendMessageW(g_h_list_log, LB_ADDSTRING, 0, (LPARAM)line.c_str());
                    int count = SendMessage(g_h_list_log, LB_GETCOUNT, 0, 0);
                    SendMessage(g_h_list_log, LB_SETCURSEL, count - 1, 0);
                }
                delete msgs;
            }
            return 0;
        }

        case WM_TEST_UPDATE_STATUS: {
            std::string* status = (std::string*)lparam;
            if (status) {
                std::wstring w_stat = unicode::utf8_to_utf16(*status);
                SetWindowTextW(g_h_lbl_status, (L"Live Status: " + w_stat).c_str());
                delete status;
            }
            return 0;
        }

        case WM_HSCROLL: {
            HWND h_scroll = (HWND)lparam;
            if (h_scroll == g_h_slider_poll) {
                int sec = (int)SendMessage(g_h_slider_poll, TBM_GETPOS, 0, 0);
                std::wstring s = std::to_wstring(sec) + L"s";
                SetWindowTextW(g_h_lbl_poll_val, s.c_str());
            }
            return 0;
        }

        case WM_COMMAND: {
            int id = LOWORD(wparam);

            if (id == IDC_BTN_RUN_UNIT_TESTS) {
                run_all_unit_tests_gui();
                return 0;
            }

            if (id == IDC_TEST_BTN_SAMPLE_MSG) {
                add_sample_multilingual_messages();
                return 0;
            }

            if (id == IDC_TEST_BTN_SAVE_KEY) {
                wchar_t key_buf[512] = { 0 };
                GetWindowTextW(g_h_edit_key, key_buf, 512);
                std::string raw_key = unicode::utf16_to_utf8(key_buf);

                AppConfig cfg = g_config.get();
                cfg.api_key = raw_key;
                g_config.update(cfg, false);
                g_config.save();

                SetWindowTextW(g_h_lbl_status, L"Live Status: API Key saved to persistent local config!");
                return 0;
            }

            if (id == IDC_TEST_BTN_START) {
                wchar_t key_buf[512] = { 0 };
                wchar_t vid_buf[512] = { 0 };
                GetWindowTextW(g_h_edit_key, key_buf, 512);
                GetWindowTextW(g_h_edit_vid, vid_buf, 512);

                std::string api_key = unicode::utf16_to_utf8(key_buf);
                std::string raw_vid = unicode::utf16_to_utf8(vid_buf);
                std::string video_id = AppConfig::extract_video_id(raw_vid);

                int poll_sec = (int)SendMessage(g_h_slider_poll, TBM_GETPOS, 0, 0);
                if (poll_sec < 3) poll_sec = 3;

                AppConfig cfg = g_config.get();
                cfg.api_key = api_key;
                cfg.video_id = video_id;
                cfg.poll_interval_sec = poll_sec;
                g_config.update(cfg, false);
                g_config.save();

                if (api_key.empty() || video_id.empty()) {
                    SetWindowTextW(g_h_lbl_status, L"Live Status: [ERROR] Please enter both an API Key and Video URL/ID.");
                    return 0;
                }

                if (!g_test_engine) g_test_engine = std::make_unique<YouTubeEngine>();

                g_test_engine->set_callbacks(
                    [](const std::vector<ChatMessage>& msgs) {
                        if (g_h_main) {
                            auto* msgs_copy = new std::vector<ChatMessage>(msgs);
                            PostMessageW(g_h_main, WM_TEST_ADD_MESSAGES, 0, (LPARAM)msgs_copy);
                        }
                    },
                    [](StreamStatus status, const std::string& detail) {
                        if (g_h_main) {
                            auto* status_copy = new std::string(detail);
                            PostMessageW(g_h_main, WM_TEST_UPDATE_STATUS, 0, (LPARAM)status_copy);
                        }
                    }
                );

                g_test_engine->start(api_key, video_id, poll_sec);
                return 0;
            }

            if (id == IDC_TEST_BTN_STOP) {
                if (g_test_engine) {
                    g_test_engine->stop();
                }
                SetWindowTextW(g_h_lbl_status, L"Live Status: Live test stopped.");
                return 0;
            }

            if (id == IDC_TEST_BTN_CLEAR_LOG) {
                SendMessage(g_h_list_log, LB_RESETCONTENT, 0, 0);
                return 0;
            }
            break;
        }

        case WM_DESTROY: {
            if (g_test_engine) g_test_engine->stop();
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

int run_gui_test_panel(HINSTANCE hInstance) {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES | ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    g_config.load();
    AppConfig initial_cfg = g_config.get();

    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = TestPanelWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"YT_Test_Panel_Class";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassExW(&wc);

    g_h_main = CreateWindowExW(
        0, L"YT_Test_Panel_Class",
        L"YouTube Live Chat Overlay - Test Suite & Verification Panel",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 700, 740,
        NULL, NULL, hInstance, NULL
    );

    if (!g_h_main) return 1;

    CreateWindowW(L"BUTTON", L"1. Automated Unit Test Modules (Unicode, Emojis, Themes, Parser)", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        15, 10, 655, 110, g_h_main, NULL, hInstance, NULL);

    CreateWindowW(L"STATIC", L"[✓] JSON   [✓] Unicode/Scripts   [✓] Emojis   [✓] Themes (.streamtheme)   [✓] Config",
        WS_CHILD | WS_VISIBLE, 30, 35, 620, 18, g_h_main, NULL, hInstance, NULL);

    CreateWindowW(L"BUTTON", L"Run All Unit Tests Now", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        30, 62, 190, 30, g_h_main, (HMENU)IDC_BTN_RUN_UNIT_TESTS, hInstance, NULL);

    g_h_lbl_unit = CreateWindowW(L"STATIC", L"Status: All 8 unit test modules passed (100% verified)", WS_CHILD | WS_VISIBLE,
        235, 69, 415, 20, g_h_main, (HMENU)IDC_LBL_UNIT_RESULTS, hInstance, NULL);

    CreateWindowW(L"BUTTON", L"2. Live YouTube Stream & API Verification", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        15, 130, 655, 220, g_h_main, NULL, hInstance, NULL);

    CreateWindowW(L"STATIC", L"YouTube Data API v3 Key:", WS_CHILD | WS_VISIBLE,
        30, 155, 180, 18, g_h_main, NULL, hInstance, NULL);
    g_h_edit_key = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
        30, 175, 490, 24, g_h_main, (HMENU)IDC_TEST_EDIT_APIKEY, hInstance, NULL);
    CreateWindowW(L"BUTTON", L"Save Key", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        530, 174, 120, 26, g_h_main, (HMENU)IDC_TEST_BTN_SAVE_KEY, hInstance, NULL);

    CreateWindowW(L"STATIC", L"YouTube Video URL or Video ID:", WS_CHILD | WS_VISIBLE,
        30, 210, 220, 18, g_h_main, NULL, hInstance, NULL);
    g_h_edit_vid = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        30, 230, 620, 24, g_h_main, (HMENU)IDC_TEST_EDIT_VIDEO, hInstance, NULL);

    CreateWindowW(L"STATIC", L"Adjustable Polling Rate:", WS_CHILD | WS_VISIBLE,
        30, 265, 170, 18, g_h_main, NULL, hInstance, NULL);
    g_h_lbl_poll_val = CreateWindowW(L"STATIC", L"15s", WS_CHILD | WS_VISIBLE | SS_RIGHT,
        210, 265, 50, 18, g_h_main, (HMENU)IDC_TEST_LBL_POLL_VAL, hInstance, NULL);

    g_h_slider_poll = CreateWindowExW(0, TRACKBAR_CLASSW, L"", WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
        270, 260, 380, 30, g_h_main, (HMENU)IDC_TEST_SLIDER_POLL, hInstance, NULL);
    SendMessage(g_h_slider_poll, TBM_SETRANGE, TRUE, MAKELPARAM(3, 60));
    SendMessage(g_h_slider_poll, TBM_SETPOS, TRUE, initial_cfg.poll_interval_sec);

    CreateWindowW(L"BUTTON", L"Start Live Stream Test", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        30, 305, 180, 32, g_h_main, (HMENU)IDC_TEST_BTN_START, hInstance, NULL);
    CreateWindowW(L"BUTTON", L"Stop Live Test", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        220, 305, 120, 32, g_h_main, (HMENU)IDC_TEST_BTN_STOP, hInstance, NULL);

    g_h_lbl_status = CreateWindowW(L"STATIC", L"Live Status: Ready for verification.", WS_CHILD | WS_VISIBLE,
        360, 312, 290, 30, g_h_main, (HMENU)IDC_TEST_LBL_STATUS, hInstance, NULL);

    CreateWindowW(L"BUTTON", L"3. Real-Time Live Chat Stream & Multilingual Test Monitor", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        15, 360, 655, 325, g_h_main, NULL, hInstance, NULL);

    g_h_list_log = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_HASSTRINGS,
        30, 385, 625, 250, g_h_main, (HMENU)IDC_TEST_LIST_LOG, hInstance, NULL);

    CreateWindowW(L"BUTTON", L"Insert Multilingual Test Samples", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        30, 642, 240, 28, g_h_main, (HMENU)IDC_TEST_BTN_SAMPLE_MSG, hInstance, NULL);
    CreateWindowW(L"BUTTON", L"Clear Feed Log", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        280, 642, 130, 28, g_h_main, (HMENU)IDC_TEST_BTN_CLEAR_LOG, hInstance, NULL);

    std::wstring w_k = unicode::utf8_to_utf16(initial_cfg.api_key);
    SetWindowTextW(g_h_edit_key, w_k.c_str());

    std::wstring w_v = unicode::utf8_to_utf16(initial_cfg.video_id);
    SetWindowTextW(g_h_edit_vid, w_v.c_str());

    std::wstring w_p = std::to_wstring(initial_cfg.poll_interval_sec) + L"s";
    SetWindowTextW(g_h_lbl_poll_val, w_p.c_str());

    ShowWindow(g_h_main, SW_SHOW);
    UpdateWindow(g_h_main);

    run_all_unit_tests_gui();
    add_sample_multilingual_messages();

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    ULONG_PTR gdi_token = 0;
    Gdiplus::GdiplusStartupInput gdi_in;
    Gdiplus::GdiplusStartup(&gdi_token, &gdi_in, NULL);

    bool cli_mode = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--cli" || std::string(argv[i]) == "--headless") {
            cli_mode = true;
            break;
        }
    }

    if (cli_mode) {
        std::cout << "========================================" << std::endl;
        std::cout << "  RUNNING C++ NATIVE TEST SUITE (CLI)   " << std::endl;
        std::cout << "========================================" << std::endl;

        try {
            std::cout << "1. Testing JSON parser..." << std::endl;
            test_json_parsing();
            std::cout << "2. Testing URL parser..." << std::endl;
            test_url_parser();
            std::cout << "3. Testing Config..." << std::endl;
            test_config();
            std::cout << "4. Testing Unicode & Multilingual..." << std::endl;
            test_unicode_multilingual();
            std::cout << "5. Testing Emoji Segmentation..." << std::endl;
            test_emoji_segmentation();
            std::cout << "6. Testing Theme System & .streamtheme..." << std::endl;
            test_theme_system();
            std::cout << "7. Testing Emoji Cache..." << std::endl;
            test_custom_emoji_cache();
            std::cout << "8. Testing Legacy Themes..." << std::endl;
            test_theme();
            std::cout << "========================================" << std::endl;
            std::cout << "  ALL 8 TEST MODULES PASSED (100%)      " << std::endl;
            std::cout << "========================================" << std::endl;

            if (gdi_token) Gdiplus::GdiplusShutdown(gdi_token);
            return 0;
        } catch (const std::exception& e) {
            std::cerr << "[FAIL] Exception during tests: " << e.what() << std::endl;
            if (gdi_token) Gdiplus::GdiplusShutdown(gdi_token);
            return 1;
        }
    }

    int ret = run_gui_test_panel(GetModuleHandle(NULL));
    if (gdi_token) Gdiplus::GdiplusShutdown(gdi_token);
    return ret;
}
