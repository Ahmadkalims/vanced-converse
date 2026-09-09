# Architecture Decision Records (ADR)

## ADR-001: Native C++ Port for Ultra-Low Resource HUD
- **Context:** The application is a background streamer overlay that must run continuously alongside high-demand games and streaming software. Electron's 100MB+ bundle, multiple helper processes, and ~150MB RAM footprint are sub-optimal for this use case.
- **Decision:** Port the entire core to a modern C++ (C++20 / Win32) standalone executable using `WinHttp`, `GDI+`, and `DwmApi`.
- **Rationale:** Reduces binary size to **~1.48 MB**, RAM usage to **~8-12 MB**, CPU usage to **<0.1%**, and startup time to **<30ms**, while enabling true Win32 `WS_EX_TRANSPARENT` click-through and live reactive updates.

## ADR-002: Native Windows WinHttp over Third-Party HTTP Libraries
- **Context:** The application requires HTTPS communication with YouTube Data API v3.
- **Decision:** Use the native Windows HTTP Services (`WinHttp`) library built into the OS.
- **Rationale:** Zero external DLL dependencies (unlike libcurl or OpenSSL), native Windows TLS/certificate store integration, and rock-solid connection handling.

## ADR-003: GDI+ Double-Buffered Alpha Layering (`UpdateLayeredWindow`)
- **Context:** The chat overlay needs smooth alpha transparency, rounded corners, glowing text, and high readability over dark or light games and wallpapers.
- **Decision:** Use 32-bit ARGB DIB sections rendered via GDI+ with `UpdateLayeredWindow`.
- **Rationale:** Provides 60 FPS hardware-blended per-pixel transparency and antialiased text without the weight of a full web browser engine.

## ADR-004: Direct Observer Pattern for Live Settings Updates
- **Context:** Streamers need immediate visual feedback when tweaking themes, fonts, opacity, or blur in the settings panel.
- **Decision:** Wire `ConfigManager` with a thread-safe observer callback that dispatches `WM_OVERLAY_UPDATE_CONFIG` to the overlay window on every slider or dropdown event.
- **Rationale:** Instant real-time preview on screen without saving/reloading or restarting the app.
