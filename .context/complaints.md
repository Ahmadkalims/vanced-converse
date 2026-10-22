# High-Priority User Complaints Tracking (`c1`, `c2`...)

*Priority Rule: High-priority complaints take strict engineering precedence over standard feature requests.*

---

## Resolved Complaints (`[x]`)

- [x] **c1**: **Settings Panel Blank & Loading External Browser**
  - **Summary**: The settings panel was opening as an external browser page or loading blank dark content instead of functioning within the desktop app itself.
  - **Root Cause**: `SettingsWindow.cpp` previously relied on launching a local HTTP server / external browser tab, and the WebView2 bridge environment initialization lacked proper local resource mapping and fallback handling.
  - **Fix**: Replaced with an embedded WebView2 controller loading local HTML/CSS assets (`settings_ui.html`) directly within an application window, using two-way WebMessage IPC bridge (`window.chrome.webview.postMessage`) and native DPAPI encryption for API keys.

- [x] **c2**: **Live Chat Active Message Cutoff / Abrupt Truncation**
  - **Summary**: Live chat messages were abruptly clipping against the top and bottom window boundaries.
  - **Root Cause**: The overlay message container lacked margin cushions and top/bottom gradient alpha masks.
  - **Fix**: Added configurable padding (`--overlay-padding`) and vertical fade masks (`-webkit-mask-image: linear-gradient(...)`) so messages smoothly fade in and out at viewport edges.

- [x] **c3**: **YouTube API Quota Exhausted Prematurely**
  - **Summary**: Daily YouTube Data API quota was exhausted within hours due to rapid polling and inaccurate calculations.
  - **Root Cause**: Polling was running on a sub-optimal interval without defensive throttling or backoff on 403 quota exhaustion.
  - **Fix**: Set default polling frequency to 10 seconds (360 req/hr), added exponential backoff on transient errors, and built an instant circuit-breaker on HTTP 403 `quotaExceeded` to halt runaway loops.

- [x] **c4**: **Noisy Chat Status Overlay Text**
  - **Summary**: Overlay displayed clutter text like "live chat active (74 messages)" on screen.
  - **Root Cause**: Redundant status header was rendered into the chat view hierarchy.
  - **Fix**: Removed the status counter element from the overlay viewport, leaving only pure, distraction-free chat messages.

- [x] **c5**: **Binary Naming Mismatch**
  - **Summary**: The built executable was named `YTLiveChatOverlay.exe` instead of `Vanced Converse.exe`.
  - **Root Cause**: `Makefile` and `build.bat` target output was configured with legacy executable name.
  - **Fix**: Updated build system to output `Vanced Converse.exe` matching project branding and product specifications.

---

## Pending / Unresolved Complaints (`[ ]`)
*(No open high-priority complaints at this time. All reported issues verified and resolved.)*
