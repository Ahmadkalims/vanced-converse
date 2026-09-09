# Quality Gates & Engineering Standards

## 1. Quality Gates
Before any pull request or development milestone is considered ready for merge/release, it must pass the following sequence:

```
INSPECT -> PLAN -> IMPLEMENT -> TEST -> BUILD -> LINT/TYPECHECK -> VERIFY -> DOCUMENT
```

1. **Unit & Integration Tests:** All tests in `tests/unit` and `tests/integration` pass 100% without live API calls.
2. **Build Verification:** `npm run build` or `npm run dist` produces a clean portable package without packaging errors.
3. **Security Audit:**
   - No hardcoded API keys, secrets, or tokens.
   - Context isolation enabled and verified.
   - No direct `innerHTML` usage with unsanitized external strings.
4. **Performance & Memory Gate:**
   - No unbounded memory leaks in message collections.
   - Polling obeys the minimum interval threshold.
5. **Documentation Gate:**
   - Relevant documentation in `docs/` updated.
   - `TASKS.md` updated with completed items.

## 2. Definition of Done (DoD)
A task or feature is "Done" only when:
- [ ] Requirements fully met and cross-checked against `PROJECT_MEMORY.md`.
- [ ] Code follows all rules in `docs/CODE_STANDARDS.md`.
- [ ] Edge cases and failure states (network drops, invalid API keys, offline streams) handled gracefully.
- [ ] Unit tests added/updated and passing.
- [ ] Verified manually in the Electron runtime.
- [ ] Relevant documentation in `docs/` and `CHANGELOG.md` updated.
- [ ] Zero leftover debug logs, temporary test scaffolding, or commented-out dead code.
