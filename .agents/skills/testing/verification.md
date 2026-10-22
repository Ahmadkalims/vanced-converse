# Testing & Verification Protocols

## 1. Golden Verification Rule
**Never mark a task complete without proof of execution.**
Every code change must be verified against real runtime behavior, unit test assertions, or visual UI proof before concluding.

## 2. Test-First & Regression Defense
- **Unit Tests for New Features**:
  - Whenever introducing a new parser, formatter, quota calculator, or state manager, write or update accompanying unit tests in `tests/`.
  - Validate both happy paths and edge cases (e.g., malformed JSON, empty chat payloads, rate-limit 403 responses, zero values).
- **Automated Verification**:
  - Run the test suite (`run_tests.exe` or test runner) and verify 100% pass rate.
  - Fix any regressions immediately before proceeding.

## 3. UI & Runtime Browser Verification
- For Web/WebView2 components (`settings_ui.html`, overlay DOM, theme CSS):
  - Verify layout integrity across common window resolutions.
  - Test interactive elements (theme selectors, sliders, color pickers, input fields) to confirm event bindings actually fire and mutate state.
  - Check browser console logs for unhandled exceptions, CSS parse errors, or broken asset paths.

## 4. Visual Proof & Screenshot Artifacts
- When modifying UI layouts, themes, or visual effects:
  - Capture visual proof (screenshot / browser rendering) to verify alignment, font rendering, and contrast.
  - Verify that visual effects (shadows, glow intensities, edge fades) render accurately without visual glitching or clipping.

## 5. Verification Checklist (Before Completion)
1. **Compilation**: Clean build with zero compiler warnings or errors (`build.bat` / `make`).
2. **Tests**: All automated test suites executed and passing.
3. **Runtime**: Application launches without crash or hanging processes.
4. **Visual Inspection**: UI looks polished, aligned, and adheres to design tokens.
