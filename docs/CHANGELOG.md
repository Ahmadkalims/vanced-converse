# Changelog

All notable changes to this project will be documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [2.1.0-cpp-enhancements] - 2026-08-24
### Added
- **API Cutoff & Zero Quota Leakage on Quit/Hide:** Instantly pauses background polling when the overlay is hidden and immediately cancels all in-flight network sleeps/requests on quit (`YouTubeEngine::pause()`, `YouTubeEngine::resume()`, and `cv_wake_` interruptible wait).
- **Interactive Verification & Test Panel GUI (`run_tests.exe`):**
  - Graphical test runner displaying all automated unit tests passing with live status badges.
  - Persistent YouTube API Key and live stream video link input fields with one-click Save button.
  - Live updating real-time chat stream monitor with adjustable polling frequency slider (3s–60s) to verify stream connectivity and chat parsing before launching the overlay.
  - Dual-mode support: GUI interactive panel by default, `--cli` for automated testing.
- **Overlay 8-Direction Edge & Corner Resizing:** Users can freely resize the overlay HUD by dragging any border or corner (`HTLEFT`, `HTRIGHT`, `HTTOP`, `HTBOTTOM`, `HTTOPLEFT`, `HTTOPRIGHT`, etc.) and move it smoothly via the top drag handle.
- **Smooth Mouse Wheel Scrolling without Scrollbars:** Full `WM_MOUSEWHEEL` support to scroll up and view older messages or scroll down to re-engage bottom auto-scrolling with zero distracting scrollbars.
- **Settings Panel UI Overhaul:**
  - Dedicated **"Update Video"** button next to the YouTube video URL/ID input to immediately validate stream IDs and query live chat status.
  - High-visibility, spacious layout (540x640) ensuring no text or sections are cut off.
  - Dedicated "Save Key" and "Save & Close" buttons with instant feedback.
- **Repository Cleanup:** Deleted all legacy JavaScript/Electron clutter files (`main.js`, `renderer.js`, `index.html`, `config.html`, `style.css`, `package.json`).

## [2.0.0-cpp] - 2026-08-24
### Added
- Initial native C++20 / Win32 port (`YTLiveChatOverlay.exe`, ~1.48 MB standalone binary).
- System tray background execution and context menu.
- Acrylic, Mica, Liquid Glass, and Flat Translucent backdrop filters.
- Real-time live updating settings panel.

## [1.0.0-audit] - 2026-08-24
### Added
- Architecture audit and project memory documentation.
