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
- **2026-10-22 22:37**: Investigated Candidate B (WinUI 3 / Windows App SDK 1.6). Identified that WinUI 3 has a critical architectural failure on Windows 10 (19045): transparent window backdrops render as solid black rectangles due to `WS_EX_NOREDIRECTIONBITMAP` DirectComposition limitations, failing Criterion #1 immediately. Additionally, WindowsAppSDK pulls in WebView2 as a dependency and cannot produce a true single-file trimmed portable exe.
- **2026-10-22 22:39**: Executed 3-run benchmark on Avalonia spike:
  - Cold startup: 214 ms (trimmed) / 234 ms (JIT) — well under 300 ms target.
  - Warm startup: 53–66 ms.
  - Idle RAM: 110 MB.
  - Trimmed publish bundle size: ~32.5 MB (single executable: 17 MB).
- **2026-10-22 22:43**: Generated visual proof rendering over pure white, pure black, and checkered wallpaper patterns. Saved to `docs/verification/`. Confirmed 100% per-pixel alpha transparency and zero corner fringe artifacts.
- **2026-10-22 22:45**: Created `.context/stack-decision.md` with complete evidence matrix.
