# Legacy WebView2 Architecture Archive

This document explains the status and maintenance policy for the legacy WebView2 / C++ desktop implementation of **Vanced Converse**.

---

## 1. Archive Status & Locations
- **Branch**: [`legacy/webview2-v1`](https://github.com/Ahmadkalims/vanced-converse/tree/legacy/webview2-v1)
- **Tag**: `legacy-webview2-v1.0` (commit `24a53b1f28e707bcdc621c2637bee41eee0420c9`)
- **Status**: **FROZEN**. No active development occurs on this branch.

---

## 2. Rationale for Native Rebuild
The legacy application utilized Microsoft Edge WebView2 and Win32 layered windows. Key reasons for transitioning to a 100% native Windows UI include:
1. **Window Transparency & Edge Artifacts**: Chromium composition under WebView2 introduced slight edge/corner clipping and halo artifacts at transparent rounded window boundaries on custom wallpapers.
2. **Resource Footprint**: WebView2 subprocesses incurred continuous memory overhead (~150MB+ idle) and noticeable startup latency compared to native rendering.
3. **Deep Visual Customization**: Maximum control over stacked text effects (multi-layer glow, outlines, gradient fills, and sub-pixel glyph rendering) requires a dedicated graphics pipeline (e.g., Skia / Windows Composition).

---

## 3. Maintenance & Bugfix Policy
- The legacy codebase is preserved in its entirety and remains recoverable at any time.
- If emergency hotfixes to the legacy version are ever required:
  1. Cut a dedicated branch from the tag: `git checkout -b hotfix/legacy-<issue> legacy-webview2-v1.0`
  2. Implement and test the fix strictly within that branch.
  3. Tag the hotfix release: `git tag -a legacy-webview2-v1.x -m "Hotfix description"`
  4. Merge into `legacy/webview2-v1` without touching active native feature branches.

---

## 4. Running Side-by-Side (Git Worktree)
To inspect or run the legacy version concurrently with the native application:
```bash
git worktree add ../vanced-converse-legacy legacy/webview2-v1
```
This mounts the legacy tree in an isolated adjacent directory without switching active branches.
