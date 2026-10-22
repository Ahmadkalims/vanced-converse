# Migration Log: WebView2 → Native Windows UI

## Phase 1: Git Safety & Archival Log
- **2026-10-22 22:30**: Inspected git status and history. Uncommitted state preserved with commit `24a53b1f28e707bcdc621c2637bee41eee0420c9` (`chore: snapshot before native migration`).
- **2026-10-22 22:30**: Pushed `main` to `origin/main`.
- **2026-10-22 22:31**: Created immutable annotated tag `legacy-webview2-v1.0` pointing to `24a53b1f28e707bcdc621c2637bee41eee0420c9`.
- **2026-10-22 22:31**: Created branch `legacy/webview2-v1` pointing to `24a53b1f28e707bcdc621c2637bee41eee0420c9` and pushed to `origin/legacy/webview2-v1`.
- **2026-10-22 22:33**: Switched to new isolated working branch `feature/native-rebuild` and set upstream tracking with origin.
- **2026-10-22 22:33**: Created `LEGACY.md` documenting freeze status, hotfix guidelines, and git worktree instructions. Committed and pushed (`46ff4dd`).
- **2026-10-22 22:34**: Verified Gate 1: Confirmed via `git ls-remote` that both remote refs (`refs/heads/legacy/webview2-v1` and `refs/tags/legacy-webview2-v1.0`) exist on GitHub and match commit hash `24a53b1`.

## Phase 2: Tech-Stack Decision Spike Log
- **2026-10-22 22:35**: Investigated host environment: Windows 10 22H2 (Build 19045), .NET SDK 8.0.424 (LTS).
- **2026-10-22 22:36**: Constructed Candidate A spike in `/spike/avalonia/AvaloniaSpike` using Avalonia 11.2.1.
- **2026-10-22 22:37**: Investigated Candidate B (WinUI 3 / Windows App SDK 1.6). Identified that WinUI 3 has a critical architectural failure on Windows 10 (19045): transparent window backdrops render as solid black rectangles due to `WS_EX_NOREDIRECTIONBITMAP` DirectComposition limitations, failing Criterion #1 immediately.
- **2026-10-22 22:39**: Executed 3-run benchmark on Avalonia spike:
  - Cold startup: 214 ms (trimmed) / 234 ms (JIT) — well under 300 ms target.
  - Warm startup: 53–66 ms.
  - Idle RAM: 110 MB.
- **2026-10-22 22:43**: Generated visual proof rendering over pure white, pure black, and checkered wallpaper patterns. Saved to `docs/verification/`. Confirmed 100% per-pixel alpha transparency and zero corner fringe artifacts.
- **2026-10-22 22:45**: Created `.context/stack-decision.md` with complete evidence matrix.

## Phase 3–6: Native Rebuild & Quality Gates Log
- **2026-10-22 22:50**: Scaffolding complete under `src-native/`. Created `theme.schema.json` and 3 theme presets: `midnight-pink.json`, `minimal-glass.json`, and `neon-glow.json`.
- **2026-10-22 22:52**: Implemented custom controls: `GlowTextBlock` (stacked text outlines, glows, shadows) and `GlassPanel` (controllable acrylic/mica glow blur and corner radius 0–60px).
- **2026-10-22 22:53**: Built `Win32Interop.cs` for click-through (`WS_EX_TRANSPARENT`), tool-window, and always-on-top.
- **2026-10-22 22:54**: Created `OverlayWindow.axaml` and `SettingsWindow.axaml` with Fluent 2 design tokens, interactive sliders, and live preview.
- **2026-10-22 22:55**: Built `YouTubeChatService.cs` with defensive 10s minimum poll floor, full jitter backoff, and 403 quota circuit breaker.
- **2026-10-22 22:57**: Built xUnit test suite in `src-native/Tests/VancedConverse.Tests/`. Executed `dotnet test`: 9/9 tests passed (100% pass rate).
- **2026-10-22 23:05**: Published standalone self-contained single-file executable `publish\Vanced Converse.exe` (44.2 MB) with bundled themes. Tested execution: verified responsive, zero crash, active overlay and settings windows.
