# Architecture Documentation (Native C++)

## 1. Process & Thread Model Overview
The application runs as a single, ultra-lightweight native Windows process containing two dedicated threads:
1. **Main UI Thread:** Manages the Win32 message pump, the GDI+ double-buffered layered `OverlayWindow`, the tabbed `SettingsWindow`, and the `TrayManager`.
2. **Background Polling Thread (`YouTubeEngine`):** Executes asynchronous HTTPS network requests via Windows `WinHttp` and marshals parsed messages to the UI thread using Windows message posting (`PostMessageW`).

```
+-----------------------------------------------------------------------------------------+
|                                    Main UI Thread                                       |
|  - WinMain & Single-Instance Mutex                                                      |
|  - ConfigManager Observer & Disk JSON Persistence (%APPDATA%\...\config.json)           |
|  - TrayManager (Shell_NotifyIcon with Show/Hide, Click-through checkbox, Appearance)    |
|  - OverlayWindow (32-bit ARGB DIB + GDI+ Layered Window + DWM Acrylic/Mica Backdrop)     |
|  - SettingsWindow (Tabbed Win32 UI with Live Preview, API generator link, sliders)      |
+--------------------------------------------^--------------------------------------------+
                                             | PostMessageW (WM_USER + 101/102/103)
                                             |
+--------------------------------------------v--------------------------------------------+
|                          YouTube Background Engine (Worker Thread)                      |
|  - WinHttpClient (Windows WinHttp API with TLS 1.3 HTTPS)                               |
|  - Video ID & Stream Discovery (videos.list endpoint)                                   |
|  - Paginated Live Chat Polling (liveChatMessages.list)                                  |
|  - LRU Ring Buffer Message Deduplication (Max 2,000 IDs)                                |
|  - Status Monitoring (Live, Connecting, QuotaExceeded, StreamOffline)                   |
+-----------------------------------------------------------------------------------------+
```

## 2. Component Breakdown

### Config & Theming Subsystem (`src/common/`)
- `ConfigManager`: Thread-safe singleton managing user preferences, live change listeners, deterministic username color hashing, and JSON serialization.
- `Theme`: Encapsulates premade presets (*Midnight Acrylic*, *Cyberpunk Neon*, *Frost Glass*, *Sunset Glow*, *Monochrome Minimal*, *Custom*), blur types, typography, and author glow palettes.
- `SimpleJson`: Fast, robust JSON parser and serializer with no third-party dependencies.

### Networking Subsystem (`src/net/`)
- `WinHttpClient`: Wrapper over Windows `WinHttp` API providing zero-overhead HTTPS requests.
- `YouTubeEngine`: Background polling loop with configurable poll frequency, automatic rate pacing, and error backoff.

### User Interface Subsystem (`src/ui/`)
- `OverlayWindow`: Alpha-blended topmost window using `UpdateLayeredWindow` with 32-bit DIB section, GDI+ anti-aliased text rendering, author glow shadows, `#question` badge highlights, owner/mod badges, and mouse dragging/docking.
- `SettingsWindow`: Three-tabbed settings GUI (Stream & API, Appearance & Themes, Overlay Controls) with real-time live preview update hooks and direct Google Cloud API link.
- `TrayManager`: System tray icon with quick toggle context menu.
- `DwmHelper`: DWM composition hooks for native Windows Acrylic and Mica backdrop materials.
