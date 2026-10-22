# Systematic Debugging & Root Cause Analysis

> "NO FIXES WITHOUT ROOT CAUSE INVESTIGATION FIRST."
> Source: [obra/superpowers](https://github.com/obra/superpowers) (Systematic Debugging methodology).

## 1. The Iron Law of Debugging
**Never apply speculative fixes, shotgun patches, or blind workarounds.**
Always establish a proven root cause before modifying source code. If you cannot explain *why* a bug occurs, you do not know how to fix it safely.

## 2. The 4-Phase Diagnostic Process

### Phase 1: Root Cause Investigation
- Collect complete, untruncated error logs, stack traces, and console errors.
- Construct a minimal, reproducible reproduction case.
- Trace backward through execution flow to identify the exact origin of invalid state or crash.

### Phase 2: Pattern & Discrepancy Analysis
- Compare the failing execution path against working paths or known good references.
- Identify what changed recently (git diff, config updates, environment flags).
- Verify environment assumptions (Win32 window handles, WebView2 initialization state, network connectivity).

### Phase 3: Single-Hypothesis Testing
- Formulate a single, falsifiable hypothesis for the failure.
- Test that specific hypothesis using targeted log probes or a minimal test harness.
- Confirm or refute the hypothesis before taking action.

### Phase 4: Implementation & Verification
- Apply the minimal change directly addressing the root cause.
- Run tests (`run_tests.exe`) to verify the bug is eliminated.
- Verify that adjacent functionality suffered zero regressions.

## 3. The 3-Strike Rule
- If **3 successive fix attempts fail**, immediately STOP modifying code.
- A 3-strike failure indicates an invalid architectural assumption or a misunderstood root cause, not a syntax error.
- Step back, re-read the original component design, re-examine assumptions, and re-trace from scratch.

## 4. Defense-in-Depth
- Whenever a bug is fixed, add validation or assertion guards at component boundaries so the invalid state becomes structurally impossible in the future.
