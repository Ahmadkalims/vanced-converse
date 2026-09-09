# Project Task Backlog

## Phase 1: Repository Audit & Documentation System
- [x] Complete 40-point architectural repository audit across all legacy versions.
- [x] Create project memory and documentation system (`PROJECT_MEMORY.md`, `docs/`, `tests/`).
- [x] Consolidate baseline source files.

## Phase 2: Native C++ Windows Port & Standalone Architecture
- [x] Implement fast zero-dependency JSON parser & serializer (`SimpleJson.hpp`).
- [x] Implement theme engine with Acrylic, Mica, Liquid Glass, and Flat Translucent blur types (`Theme.hpp`, `Theme.cpp`).
- [x] Implement config manager with JSON persistence in `%APPDATA%\YT_Live_Chat_Overlay\config.json` (`Config.hpp`, `Config.cpp`).
- [x] Implement native WinHttp HTTPS client (`WinHttpClient.hpp`, `WinHttpClient.cpp`).
- [x] Implement background YouTube live stream discovery and message polling engine with rate pacing and deduplication (`YouTubeEngine.hpp`, `YouTubeEngine.cpp`).
- [x] Implement layered alpha-blended GDI+ overlay window with question badges and right-dock positioning (`OverlayWindow.hpp`, `OverlayWindow.cpp`).
- [x] Implement modern 3-tabbed settings panel with live updates, API key link, sliders, and quota calculator (`SettingsWindow.hpp`, `SettingsWindow.cpp`).
- [x] Implement system tray icon with context menu shortcuts (Show/Hide, Click-through toggle checkbox, Appearance tab shortcut, Quit) (`TrayManager.hpp`, `TrayManager.cpp`).
- [x] Implement application entry point with single-instance mutex and lifecycle management (`main.cpp`).

## Phase 3: Quota Guard, UI Overhaul & Interactive Test Panel
- [x] Cut off all API calls immediately upon quit or overlay hide to prevent token waste (`YouTubeEngine::pause/resume/stop` with condition variables).
- [x] Remove legacy JavaScript / Electron clutter files (`main.js`, `renderer.js`, `index.html`, `config.html`, `style.css`, `package.json`).
- [x] Implement interactive Test Panel GUI (`run_tests.exe`) with unit test verification, persistent credentials, adjustable polling rate, and live updating chat feed log.
- [x] Add dedicated "Update Video" button with live status feedback in Settings Panel.
- [x] Overhaul Settings Panel dimensions and layout (540x640) ensuring no cutoffs.
- [x] Implement 8-way edge/corner overlay resizing and smooth mouse wheel scrolling without visible scrollbars.
