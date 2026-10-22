# Workspace Instructions & Skills Index

This workspace uses a modular, progressive-disclosure skill architecture located in `.agents/skills/`.
All skills are sourced and synthesized from industry-standard open-source repositories and guidelines.
Subagents and models must consult and strictly apply these skills based on the files and domains being modified:

## Progressive Disclosure Skill Index

| Domain | Skill Reference | Proven Source / Standard | Trigger / When to Activate | Key Directives |
|---|---|---|---|---|
| **Core** | [.agents/skills/core/ponytail.md](file:///.agents/skills/core/ponytail.md) | [DietrichGebert/ponytail](https://github.com/dietrichgebert/ponytail) | Writing new code, adding features, selecting dependencies | Solution Ladder: Drop -> Existing Code -> Native/Platform APIs -> Light Deps -> Custom code last. Avoid over-engineering. |
| **Core** | [.agents/skills/core/architecture.md](file:///.agents/skills/core/architecture.md) | Clean Architecture & C++ Core Guidelines | Structuring files, classes, methods, or IPC boundaries | Max 35 lines/function, max 350 lines/file, max 3 levels nesting, strict typing, single responsibility. |
| **Debugging** | [.agents/skills/debugging/systematic-debugging.md](file:///.agents/skills/debugging/systematic-debugging.md) | [obra/superpowers](https://github.com/obra/superpowers) | Diagnosing crashes, unexpected bugs, test failures | Iron Law: No fixes without root-cause investigation first. 4-phase diagnosis, 3-strike rule, defense-in-depth. |
| **Performance** | [.agents/skills/performance/resource-efficiency.md](file:///.agents/skills/performance/resource-efficiency.md) | [addyosmani/agent-skills](https://github.com/addyosmani/agent-skills) & Chromium | Overlay rendering, animation loops, memory profiling | 60 FPS animation budget, DOM message cap recycling, avoid layout thrashing, $<1\%$ idle CPU target. |
| **Network** | [.agents/skills/network/api-resilience.md](file:///.agents/skills/network/api-resilience.md) | Google Cloud & Microsoft Cloud Resiliency | YouTube API polling, quota handling, network errors | Defensive quota throttling (default 10s), exponential backoff with full jitter, circuit breaker on 403 quota exhaustion. |
| **Design** | [.agents/skills/design/ui-ux-design.md](file:///.agents/skills/design/ui-ux-design.md) | Microsoft Fluent & Modern Web Design Tokens | Styling, HTML, CSS tokens, themes, overlay layout | Anti-AI slop: 8pt spacing tokens, semantic contrast tiers, responsive margins, micro-interactions. |
| **Testing** | [.agents/skills/testing/verification.md](file:///.agents/skills/testing/verification.md) | TDD Standards & Verification Protocols | Verifying tasks, adding features, bug fixing | Never mark complete without proof: run tests (`run_tests.exe`), check runtime logs, capture visual proof. |
| **Security** | [.agents/skills/security/guardrails.md](file:///.agents/skills/security/guardrails.md) | OWASP & Least-Privilege Security Standards | API keys, input handling, network, WebView2 IPC | Zero hardcoded secrets, safe DOM text insertion (XSS defense), WebView2 bridge sanitization, error boundaries. |

---

## Agent Execution Rules
1. **Load Selectively (Progressive Disclosure)**: Read only the skill files relevant to your current edit to preserve token budget.
2. **Adhere Strictly**: Do not bypass line limits, reuse hierarchy, or root-cause investigation without explicit user direction.
3. **Verify Before Reporting**: Complete the verification checklist prior to reporting task completion.
