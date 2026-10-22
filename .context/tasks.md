# Project Tasks & Living Checklist: Native Rebuild

Living milestone checklist tracking progress of the native Windows rebuild of Vanced Converse.

## Native Rebuild Milestones
- [x] **Legacy archived**: `legacy/webview2-v1` branch & `legacy-webview2-v1.0` annotated tag pushed and verified on GitHub remote.
- [x] **Stack spike & decision documented**: Avalonia UI chosen over WinUI 3 based on empirical transparency and performance evidence (`.context/stack-decision.md`).
- [x] **Native solution scaffolded & building**: Clean layered architecture in `src-native/` with zero compiler warnings or errors.
- [x] **Transparent rounded LiveChat window verified**: Per-pixel alpha transparency verified with zero fringe over white, black, and checkered backgrounds (`docs/verification/`).
- [x] **JSON theme loader + schema + hot-reload**: Schema-validated JSON theming with debounced `FileSystemWatcher` hot-reload (`ThemeManager.cs`).
- [x] **XAML theme support limitation**: Documented trade-off; runtime XAML reflection risks trimming instability, whereas JSON schema provides 100% trim safety and two-way sync.
- [x] **In-app appearance settings with live preview & two-way sync**: Native Fluent settings panel with interactive sliders and real-time disk persistence.
- [x] **Text outline / glow / gradient / shadow renderer**: Built `GlowTextBlock` and `GlassPanel` custom controls with multi-pass glow and stroke rendering.
- [x] **Feature parity checklist complete**: All 27 legacy features ported and verified in `.context/feature-parity.md`.
- [x] **Perf targets measured**: Cold startup: ~214 ms; warm startup: ~53 ms; publish executable size: 44.2 MB self-contained portable.
- [x] **Portable publish verified**: Generated `publish\Vanced Converse.exe` with bundled theme presets and standalone execution.

## Automated Test Coverage (xUnit Native Test Suite)
- [x] DPAPI secure storage encryption & roundtrip decryption
- [x] YouTube video ID & URL normalization across all formats
- [x] Theme JSON serialization & deserialization
- [x] Two-way theme sync persistence
- [x] Corrupt/missing theme graceful fallback
- [x] Polling interval quota floor enforcement (minimum 10s)
- [x] Message collection capping and recycling
