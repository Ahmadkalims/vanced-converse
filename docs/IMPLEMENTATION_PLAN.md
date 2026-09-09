# Implementation Plan & Roadmap

## 1. Roadmap Overview

```
Phase 1: Project Memory & Audit Setup (Done)
  │
  ▼
Phase 2: Security Hardening & Testing Infrastructure
  │  - Context Isolation & Preload contextBridge
  │  - XSS Sanitizer & DOM Security
  │  - Vitest Unit & Integration Test Suite + Mocks
  │
  ▼
Phase 3: YouTube API & Polling Engine Architecture
  │  - Decoupled API Service Layer
  │  - Configurable Polling Interval & Rate Limiter
  │  - Overlapping Request Guards & Error Backoff
  │  - Stream State & Quota Budget Manager
  │
  ▼
Phase 4: Design System, Theming & Customization
  │  - Backdrop Materials: Acrylic, Mica, Liquid Glass, Flat Translucent
  │  - Opacity & Blur Radius Sliders
  │  - Typography Customizer (Font Family, Size Scaling)
  │  - Premade Themes (Midnight, Cyberpunk, Frost, Sunset, Minimal)
  │  - Custom Highlight Trigger Rules
  │
  ▼
Phase 5: Overlay UX & Window Improvements
  │  - Right-Side Screen Positioning
  │  - Reliable Click-Through Toggle
  │  - Dedicated Tray & App Icon Assets
  │  - Tabbed Settings Window (Stream, Theming, Advanced)
  │
  ▼
Phase 6: Performance Optimization & Memory Management
  │  - DOM Message Cap (Rolling Buffer, e.g. 200 items)
  │  - Asynchronous & Debounced Config Persistence
  │  - Memory Profiling & Heap Validation
  │
  ▼
Phase 7: Native C++ Boundary Investigation
  │  - Profile Electron baseline under multi-hour heavy load
  │  - Evaluate C++ Node-API addon / sidecar for network/Win32 composition if needed
```

## 2. Detailed Phase Breakdown

### Phase 2: Security Hardening & Testing Infrastructure
- Create `preload.js` exposing safe IPC methods.
- Switch `nodeIntegration: false` and `contextIsolation: true` in `main.js`.
- Replace direct `innerHTML` string concatenations with safe text node population or sanitization.
- Configure Vitest testing framework with unit tests for config, URL extraction, and mocked API responses.

### Phase 3: YouTube API & Polling Engine Architecture
- Build `YouTubeService` class managing API requests, token pagination, and quota tracking.
- Build `PollingEngine` supporting dynamic interval configuration (slider in UI, 3s to 60s), auto-backoff on errors, and pause on window hide.

### Phase 4: Theming & Customization Engine
- Implement CSS custom property engine for dynamic live theme switching.
- Add Acrylic, Mica, and Liquid Glass backdrop blur modes.
- Build font picker and font-size scaler.
- Implement premade theme selector with instant live preview.

### Phase 5: Windowing & Settings Modal
- Add tabbed interface to `config.html` (Stream Settings, Themes & Blur, Performance & Quota).
- Fix tray icon loading with dedicated `.png` and `.ico` assets.
- Implement right-side monitor docking logic.
