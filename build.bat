@echo off
setlocal enabledelayedexpansion

echo ========================================================
echo  Building YouTube Live Chat Overlay (Native C++ Windows)
echo ========================================================

set GXX="C:\msys64\ucrt64\bin\g++.exe"
set WINDRES="C:\msys64\ucrt64\bin\windres.exe"

if not exist %GXX% (
    where g++.exe >nul 2>nul
    if %errorlevel% equ 0 (
        set GXX=g++
    ) else (
        echo [ERROR] g++.exe not found in PATH or C:\msys64\ucrt64\bin\
        exit /b 1
    )
)

if not exist %WINDRES% (
    set WINDRES=windres
)

echo [1/3] Compiling Windows Manifest and Resource...
%WINDRES% -i src/resource/app.rc -o src/resource/app.res.o -O coff
if %errorlevel% neq 0 (
    echo [ERROR] Resource compilation failed.
    exit /b 1
)

echo [2/3] Building Native C++ Test Suite and Verification Panel (run_tests.exe)...
%GXX% -std=c++20 -O2 tests/test_runner.cpp tests/unit/test_json.cpp tests/unit/test_url_parser.cpp tests/unit/test_theme.cpp tests/unit/test_config.cpp tests/unit/test_unicode_multilingual.cpp tests/unit/test_emoji_segmentation.cpp tests/unit/test_theme_system.cpp tests/unit/test_custom_emoji_cache.cpp src/common/Theme.cpp src/common/ThemeModel.cpp src/common/ThemeManager.cpp src/common/Config.cpp src/net/WinHttpClient.cpp src/net/YouTubeEngine.cpp src/net/EmojiCache.cpp src/ui/ColorEmojiRenderer.cpp -o run_tests.exe -lwinhttp -lgdiplus -ld2d1 -ldwrite -lcomctl32 -lcomdlg32 -lshlwapi -lole32 -lgdi32 -luser32 -ldwmapi -static -static-libgcc -static-libstdc++
if %errorlevel% neq 0 (
    echo [ERROR] Test suite compilation failed.
    exit /b 1
)

echo [RUN] Running Automated Unit Tests in CLI Mode...
.\run_tests.exe --cli
if %errorlevel% neq 0 (
    echo [ERROR] Automated unit tests failed!
    exit /b 1
)

echo [3/3] Compiling Optimized Standalone Executable (YTLiveChatOverlay.exe)...
%GXX% -std=c++20 -O3 -flto -s -mwindows -municode -Iinclude src/main.cpp src/common/Theme.cpp src/common/ThemeModel.cpp src/common/ThemeManager.cpp src/common/Config.cpp src/net/WinHttpClient.cpp src/net/YouTubeEngine.cpp src/net/EmojiCache.cpp src/ui/ColorEmojiRenderer.cpp src/ui/TextRenderer.cpp src/ui/OverlayWindow.cpp src/ui/SettingsWindow.cpp src/ui/TrayManager.cpp src/resource/app.res.o -o YTLiveChatOverlay.exe -lwinhttp -lgdiplus -ld2d1 -ldwrite -lgdi32 -luser32 -ldwmapi -lcomctl32 -lcomdlg32 -lole32 -luuid -luxtheme -lshlwapi -lshell32 -static -static-libgcc -static-libstdc++


if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    exit /b 1
)

echo ========================================================
echo  BUILD SUCCESSFUL:
echo   1. YTLiveChatOverlay.exe  (Main Portable Stream Overlay)
echo   2. run_tests.exe           (Interactive GUI Test Panel)
echo ========================================================
