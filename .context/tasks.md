# Project Tasks & Living Checklist

Living checklist of features, fixes, and automated test coverage across Vanced Converse.

## 1. Core Desktop & Native Architecture
- [x] Native C++20 Win32 application skeleton (`main.cpp`, `OverlayWindow.cpp`)
- [x] High-DPI awareness (Per-Monitor V2)
- [x] Windows DPAPI encrypted storage for sensitive API keys (`SecureStorage.hpp`)
- [x] Dual OS support: Windows 11 Mica/Acrylic + Windows 10 DWM composition fallback (`DwmHelper.hpp`)
- [x] Click-through and Always-on-Top overlay modes
- [x] System Tray integration with context menu (`TrayManager.cpp`)
- [x] Executable output standard: `Vanced Converse.exe`

## 2. YouTube Engine & Networking
- [x] YouTube URL & Video ID parsing (standard, short, livestream formats)
- [x] YouTube Data API v3 integration with defensive 10s poll default
- [x] Error handling with exponential backoff on transient errors
- [x] Quota exhaustion circuit-breaker (HTTP 403 handling)
- [x] Live chat message deserialization with author badges and superchats

## 3. UI, Themes & Customization
- [x] Modern Fluent dark settings panel in embedded WebView2 (`settings_ui.html`)
- [x] Two-way IPC synchronization between C++ and WebView2 UI
- [x] Dynamic CSS variable generation from active theme
- [x] Custom CSS loader & template CSS generator (`themes/template.css`)
- [x] Glow intensity and radius adjustment controls
- [x] Edge margin cushioning & vertical gradient fade masks
- [x] Full Unicode and multilingual text rendering (`UnicodeUtils.hpp`)
- [x] Custom emoji caching & rendering (`EmojiCache.hpp`, `ColorEmojiRenderer.cpp`)

## 4. Skills & Agent Development Standards
- [x] Core reuse-first skill (`.agents/skills/core/ponytail.md`)
- [x] Modular architecture rules (`.agents/skills/core/architecture.md`)
- [x] Systematic debugging protocol (`.agents/skills/debugging/systematic-debugging.md`)
- [x] Resource efficiency & overlay performance (`.agents/skills/performance/resource-efficiency.md`)
- [x] API resilience & quota safety (`.agents/skills/network/api-resilience.md`)
- [x] UI/UX design tokens & anti-slop guidelines (`.agents/skills/design/ui-ux-design.md`)
- [x] Testing & verification protocols (`.agents/skills/testing/verification.md`)
- [x] Security & operational guardrails (`.agents/skills/security/guardrails.md`)
- [x] Master Progressive Disclosure index in `AGENTS.md` and `.antigravity/instructions.md`

## 5. Automated Test Coverage
- [x] JSON parser & comments support unit tests
- [x] YouTube URL & video ID extractor tests
- [x] Config manager & observer pattern tests
- [x] Unicode & multilingual text verification tests
- [x] Unicode emoji & run segmentation tests
- [x] Theme architecture, schema validation & import/export tests
- [x] Custom emoji cache & asset tests
- [x] DPAPI secure storage roundtrip tests
- [x] CLI test runner integration (`run_tests.exe --cli`)
