# Resource Efficiency & Overlay Performance

> Standards for low-footprint desktop applications, zero-stutter overlays, and clean memory management.
> Sources: [addyosmani/agent-skills](https://github.com/addyosmani/agent-skills), Google Chrome Performance Guidelines, Microsoft Windows App SDK.

## 1. Zero-Impact Desktop Overlay Standards
- Vanced Converse runs on top of active livestreams and high-refresh games:
  - **CPU Target**: $< 1\%$ idle CPU usage during normal chat polling.
  - **Memory Target**: Stable RAM footprint ($< 150\text{MB}$ total, including WebView2 subprocesses).
  - **GPU Target**: Zero continuous GPU load when chat is static; hardware acceleration only for composite layers.

## 2. DOM Recycling & Memory Leak Defense
- **Chat Node Pruning**:
  - Live chat streams can receive hundreds of messages per minute.
  - Enforce a strict DOM message cap (e.g., maximum 50–100 message elements in the overlay container).
  - Remove stale nodes from the top of the container as new messages arrive to prevent unbounded memory growth.
- **Listener & Timer Cleanup**:
  - Remove event listeners, observers, and polling timers when tearing down or rebuilding UI components.
  - Never retain dangling references in global JavaScript or C++ callbacks.

## 3. 60 FPS Render Pipeline & Smooth Layout
- **Avoid Layout Thrashing**:
  - Never interleave DOM style reads (`offsetHeight`, `getBoundingClientRect`) with DOM writes.
  - Batch DOM mutations using `requestAnimationFrame` or `DocumentFragment`.
- **CSS Compositing & GPU Layers**:
  - Animate only composite properties (`transform`, `opacity`).
  - Avoid animating properties that trigger full layout or paint recalculations (`top`, `left`, `margin`, `width`, `height`).
  - Use `will-change: transform, opacity` sparingly and only on active animated elements.

## 4. Debounce, Throttle & Background State
- Debounce high-frequency user inputs (e.g., color pickers, opacity sliders) before sending IPC messages across the C++ <-> WebView2 bridge.
- Throttle non-critical background polling when the overlay is hidden or minimized.
- Measure performance with actual metrics before and after optimization.
