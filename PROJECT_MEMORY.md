# PROJECT MEMORY

## 1. Project Purpose & High-Level Scope
YouTube Live Chat Overlay (`YTLiveChatOverlay.exe`) is an ultra-lightweight, high-performance native Windows desktop application built with modern C++ (C++20 / Win32). It retrieves live YouTube stream comments using the YouTube Data API v3 and displays them in a sleek, customizable, hardware-accelerated desktop overlay HUD.

## 2. Core Architecture & Highlights
1. **Ultra-Lightweight Native Binary:** Single standalone executable (~1.48 MB) with zero runtime dependencies and zero Electron/Chromium overhead. Runs in the background via Windows System Tray.
2. **Instant Live Updating Settings Panel:** A modern tabbed settings panel accessible from the tray context menu or double-click. Changes to themes, blur types, opacity, font size, or polling frequency immediately update the desktop overlay HUD in real time.
3. **API Key Assistance:** The settings panel contains a direct one-click action to open the Google Cloud Console credential generation portal.
4. **Rich Material Backdrop Support:** Supports Acrylic, Mica, Liquid Glass, and Flat Translucent blur styles with adjustable blur radius and background opacity.
5. **System Tray Integration:** Full context menu with show/hide toggle, click-through shortcut checkbox (`WS_EX_TRANSPARENT`), appearance tab shortcut, and clean quit.
6. **Smart Polling & Quota Guard:** Background `WinHttp` polling thread with configurable intervals (3s - 60s), live quota consumption calculation, message deduplication, and circular buffer memory management.

## 3. Technology Stack
- **Language:** C++20 (GCC 14.2 / MinGW-w64 / MSVC compatible)
- **UI Engine:** Win32 API, GDI+ Double-Buffered Alpha Layering (`UpdateLayeredWindow`), Desktop Window Manager (`DwmApi`, `SetWindowCompositionAttribute`), Common Controls v6 (Per-Monitor DPI v2 Aware)
- **Networking:** Native Windows HTTP Services (`WinHttp` via HTTPS)
- **JSON Engine:** Zero-dependency fast single-header JSON parser & serializer (`SimpleJson.hpp`)
- **Persistence:** Local JSON storage at `%APPDATA%\YT_Live_Chat_Overlay\config.json`

## 4. Key Metrics
- **Binary Footprint:** ~1.48 MB (Portable single `.exe`)
- **Idle Memory:** ~5 MB – 12 MB RAM (vs 150 MB+ in Electron)
- **Idle CPU:** < 0.1%
- **Startup Time:** < 30ms instant cold launch
