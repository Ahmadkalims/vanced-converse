# System Architecture & Technical Patterns

## 1. High-Level Technology Stack
- **Language / Standards**: C++20 (MSVC / MinGW-w64), HTML5 / CSS3 / ES6.
- **Window Management**: Win32 API (`CreateWindowExW`, `DWMWA_SYSTEMBACKDROP_TYPE`, layered windows).
- **Web Runtime**: Microsoft Edge WebView2 (Chromium embedded).
- **Security / Storage**: Windows DPAPI (`CryptProtectData`, `CryptUnprotectData`) for credentials; JSON for non-sensitive configuration.
- **Rendering Engine**: Dual pipeline: Direct GDI+ with DirectWrite fallback for overlay text/emojis, WebView2 for Fluent settings management.

## 2. Key Architecture Boundaries
```
[YouTube Live API]
        │
        ▼ (HTTPS REST / WinINet)
[YouTubeEngine] ──> [JSON Parser] ──> [ChatMessage Queue]
                                                │
                                                ▼
                                      [OverlayWindow (Win32)]
                                      ├── Unicode / Emoji Renderer
                                      ├── DWM Composition / Acrylic
                                      └── Custom CSS Engine
                                                ▲
                                                │ (IPC: JSON WebMessage)
                                      [SettingsWindow (WebView2)]
                                      └── settings_ui.html
```

## 3. Core Design Principles
- **Progressive Disclosure**: Consult `.agents/skills/` selectively by domain before editing.
- **Reuse First (Ponytail)**: Native APIs $\rightarrow$ Existing Code $\rightarrow$ Dependencies $\rightarrow$ Minimal Inline Code.
- **Defensive API Quota Management**: Default 10s poll cycle (360 requests/hour), full jitter backoff, hard circuit-breaker on 403 quota exhaustion.
- **Zero-Waste Overlay Footprint**: $<1\%$ idle CPU, capped DOM / message history buffers, no continuous GPU rendering when idle.
- **Type Safety & Bounds Checking**: `enum class`, `const` correctness, smart pointers (`std::unique_ptr`), structured guard clauses with maximum 3 nesting levels.
