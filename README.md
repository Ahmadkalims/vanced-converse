# Vanced Converse

> **A high-performance, GPU-accelerated desktop YouTube Live Chat overlay and settings suite built with native C++20, Direct2D, and an embedded WebView2 interface.**

![Vanced Converse](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![Direct2D](https://img.shields.io/badge/Direct2D-Hardware%20Accelerated-green.svg)
![WebView2](https://img.shields.io/badge/UI-Microsoft%20Edge%20WebView2-informational.svg)
![Platform](https://img.shields.io/badge/Platform-Windows%2010%20%2F%2011-lightgrey.svg)
![License](https://img.shields.io/badge/License-MIT-purple.svg)

---

## Overview

**Vanced Converse** combines the speed, ultra-low resource footprint, and smoothness of a native C++ Direct2D transparent layered desktop overlay with the infinite customizability of CSS-driven theming and a sleek Fluent Design settings panel.

- **GPU-Accelerated Direct2D Overlay**: Ultra-smooth rendering, custom alpha-blended transparent backdrops (Acrylic, Mica, Liquid Glass), crisp DirectWrite typography, and full color emoji support.
- **Embedded WebView2 Settings Panel**: Sleek Fluent Design (inspired by Start11 / Fluent Search) operating directly inside the desktop application with zero external browser tabs.
- **Hybrid CSS-Driven Theming**: Themes are standard `.css` files using `:root { --vc-* }` custom properties as the single source of truth. Edit directly via the Settings UI or hand-craft in your favorite code editor with instant hot-reloading.
- **YouTube Live Chat Polling Engine**: Highly optimized background polling engine with quota optimization calculator, auto-reconnect, and multi-lingual UTF-8 parsing.

---

## Features

- 🎨 **Sleek Minimalistic Settings Panel**: Embedded Win32 window with Microsoft Edge WebView2 hosting a modern dark-mode Fluent UI with Mica and Acrylic effects.
- 🔄 **Two-Way Live Sync**: Settings adjusted in the panel stream live to the Direct2D overlay with sub-50ms latency.
- ⚡ **Instant CSS Hot-Reload**: Built-in file watcher detects changes to any `.css` theme file and updates the overlay immediately without restarts.
- 💬 **Color Emoji & Multi-run Typography**: Native DirectWrite rendering supporting color emojis, author roles (moderator, member), and message body segmentation.
- ❓ **Interactive Event Highlighting**: Configurable visual highlights and card borders for question messages (`?`, `#question`) and superchats.
- 🛡️ **Streamer-Friendly Controls**: Transparent clickthrough toggle, always-on-top, auto-scrolling with inactivity timers, and message retention bounds.
- 💾 **Single-Binary Portability**: Lightweight standalone build with no bulky Electron or Chromium wrappers.

---

## Included Themes

Themes reside in the `themes/` directory:

| Theme | Description |
|---|---|
| **Midnight Pink** | Dark acrylic material with soft pink accents and ambient glows |
| **Cyberpunk Neon** | High-contrast dark cyber-palette with cyan, neon yellow, and sharp borders |
| **Frost Glass** | Translucent frosted glass effect with cool sky-blue highlights |
| **Sunset Glow** | Warm purple-to-orange gradient glass card styling |
| **Monochrome Minimal** | Clean, distraction-free monochrome high-legibility dark theme |
| **template.css** | Fully documented reference template explaining all `--vc-*` properties |

---

## Building from Source

### Prerequisites

- **Windows 10 / 11** (x64)
- **MinGW-w64 (UCRT64 or MINGW64)** with GCC 13+ supporting C++20:
  ```bash
  pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-binutils
  ```
- **Microsoft Edge WebView2 Runtime** (pre-installed on Windows 10/11)

### Build with Batch Script

```cmd
build.bat
```

This compiles:
1. `YTLiveChatOverlay.exe` — The optimized standalone desktop overlay and settings app.
2. `run_tests.exe` — Automated C++ native test suite verifying all 8 core subsystems.

### Build with Make

```cmd
make
```

To run tests:
```cmd
make test
```

---

## Project Architecture

```
Vanced_Converse/
├── src/
│   ├── main.cpp                  # Application entry point, tray icon & message pump
│   ├── common/                   # Core models, configuration & theming
│   │   ├── Config.hpp / .cpp     # Persistent JSON configuration manager
│   │   ├── ThemeModel.hpp / .cpp # CSS custom properties parser & theme serializer
│   │   └── ThemeManager.hpp/.cpp # Theme scanning, directory watcher & hot-reload
│   ├── net/                      # Networking & YouTube Live Streaming
│   │   ├── YouTubeEngine.hpp/.cpp# YouTube Data API polling engine
│   │   ├── WinHttpClient.hpp/.cpp# Asynchronous WinHTTP client
│   │   └── EmojiCache.hpp / .cpp # In-memory emoji caching
│   ├── ui/                       # Rendering and Windowing
│   │   ├── OverlayWindow.hpp/.cpp# Transparent Direct2D hardware-accelerated overlay
│   │   ├── SettingsWindow.hpp/.cpp# Native Win32 window hosting Microsoft Edge WebView2
│   │   ├── TextRenderer.hpp/.cpp # DirectWrite typographic engine
│   │   ├── ColorEmojiRenderer.hpp# Color emoji rasterization
│   │   └── TrayManager.hpp / .cpp# System tray context menu
│   └── resource/                 # Windows application icon & manifest
├── include/                      # Official Microsoft WebView2 headers
├── themes/                       # CSS themes and template.css
├── tests/                        # Comprehensive native unit test suite
├── settings_ui.html              # Fluent Design WebView2 UI
├── build.bat                     # Windows automated build script
└── Makefile                      # GNU Make build configuration
```

---

## License

This project is open-source under the MIT License.
