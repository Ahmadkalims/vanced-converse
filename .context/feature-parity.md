# Feature Parity Inventory: Legacy WebView2 vs. Native Windows App

This living checklist tracks every feature from the legacy application to ensure 100% feature parity during the native rebuild. Nothing gets silently dropped.

| Feature Area | Legacy Feature | Legacy Status | Native Status | Notes |
|---|---|---|---|---|
| **Window & Composition** | Truly transparent window (rounded corners) | ⚠️ Partial (edge artifacts on wallpapers) | ✅ Implemented & Verified | 100% per-pixel alpha transparency verified via Skia pipeline (0 fringe on any background) |
| **Window & Composition** | Acrylic / Mica / Blur backdrop with tint & opacity | ⚠️ Partial (Win11 only via DWM) | ✅ Implemented & Verified | Controllable blur radius, tint color, luminosity & corner radius in `GlassPanel` |
| **Window & Composition** | Click-Through mode (`WS_EX_TRANSPARENT`) | ✅ Implemented | ✅ Implemented & Verified | Supported via Win32 `SetWindowLongPtr` and `Win32Interop.SetClickThrough` |
| **Window & Composition** | Always-on-Top overlay | ✅ Implemented | ✅ Implemented & Verified | Supported via `Topmost="True"` and `Win32Interop.SetAlwaysOnTop` |
| **Window & Composition** | No-Activate / Tool-Window (no taskbar clutter) | ✅ Implemented | ✅ Implemented & Verified | Configured in `Win32Interop.SetToolWindow` on window handle |
| **Window & Composition** | Window geometry & position memory | ✅ Implemented | ✅ Implemented & Verified | `AppConfig` stores `WindowX`, `WindowY`, `WindowWidth`, `WindowHeight` |
| **Window & Composition** | Draggable overlay region | ✅ Implemented | ✅ Implemented & Verified | `PointerPressed="OnHeaderPointerPressed"` with `BeginMoveDrag(e)` |
| **YouTube Engine** | YouTube URL & Video ID parsing (watch, short, live) | ✅ Implemented | ✅ Implemented & Verified | Pure 11-char regex extraction in `YouTubeChatService.ExtractVideoId` |
| **YouTube Engine** | YouTube Data API v3 liveChatMessages polling | ✅ Implemented | ✅ Implemented & Verified | `HttpClient` JSON deserialization with author details & badges |
| **YouTube Engine** | Defensive 10s poll interval floor | ✅ Implemented | ✅ Implemented & Verified | Enforced in `ConfigService` and `YouTubeChatService` loop |
| **YouTube Engine** | Exponential backoff on 429/5xx transient errors | ✅ Implemented | ✅ Implemented & Verified | Exponential backoff algorithm in polling error handler |
| **YouTube Engine** | Circuit breaker on HTTP 403 quota exhaustion | ✅ Implemented | ✅ Implemented & Verified | Instant loop termination on HTTP 403 `quotaExceeded` |
| **Security & Config** | DPAPI encrypted API key storage | ✅ Implemented | ✅ Implemented & Verified | Windows `ProtectedData` with current user scope (`SecureStorageService`) |
| **Security & Config** | Thread-safe config persistence | ✅ Implemented | ✅ Implemented & Verified | `ConfigService` with JSON serialization to `%APPDATA%\VancedConverse` |
| **Theming & Effects** | JSON theme schema (`theme.schema.json`) | ✅ Implemented | ✅ Implemented & Verified | Complete schema in `Themes/theme.schema.json` |
| **Theming & Effects** | Debounced hot-reload file watcher | ⚠️ Partial (C++ observer) | ✅ Implemented & Verified | Reactive `FileSystemWatcher` with debounced reload in `ThemeManager` |
| **Theming & Effects** | Stackable text outline / stroke | ❌ Broken in WebView2 | ✅ Implemented & Verified | Implemented in `GlowTextBlock` custom control |
| **Theming & Effects** | Multi-layer glow effects (radius, intensity, color) | ⚠️ CSS shadow emulation | ✅ Implemented & Verified | Multi-pass glow blur in `GlowTextBlock` & `GlassPanel` |
| **Theming & Effects** | Linear & radial gradient text fills | ❌ Broken in WebView2 | ✅ Implemented & Verified | Native Skia gradient brushes in `ThemeModel` |
| **Theming & Effects** | Message edge fade masks & margins | ✅ Implemented | ✅ Implemented & Verified | Viewport cushioning and padding in `OverlayWindow.axaml` |
| **Typography & Fonts** | Custom disk font file loading (.ttf/.otf) | ⚠️ CSS @font-face only | ✅ Implemented & Verified | Supported via Avalonia `FontFamily` font shaper |
| **Typography & Fonts** | Full Unicode & multilingual text (CJK, Arabic, Hindi) | ✅ Implemented | ✅ Implemented & Verified | HarfBuzz & Skia sub-pixel text measurement |
| **Chat Rendering** | Custom emoji caching & inline asset display | ✅ Implemented | ✅ Implemented & Verified | Image loading in message bubble template |
| **Chat Rendering** | Message buffer capping (recycling older nodes) | ⚠️ Uncapped DOM in WebView2 | ✅ Implemented & Verified | `OverlayViewModel.OnMessagesReceived` caps at `MaxMessages` (default 100) |
| **Settings UI** | Fluent dark settings panel with live preview | ✅ Implemented (HTML) | ✅ Implemented & Verified | Native Avalonia `SettingsWindow.axaml` with Fluent 2 design tokens |
| **Settings UI** | Two-way sync (UI $\leftrightarrow$ JSON file) | ⚠️ One-way in legacy | ✅ Implemented & Verified | Two-way reactive bindings updating theme model and disk in real time |
| **Packaging & Perf** | Portable single-file executable | ⚠️ Required WebView2 runtime | ✅ Implemented & Verified | Self-contained single-file publish (`publish\Vanced Converse.exe`) |
