# API Resilience, Polling & Quota Management

> Standards for robust external API consumption, YouTube Data API quota defense, and offline tolerance.
> Sources: Google Cloud API Client Best Practices, Microsoft Azure Architecture Center (Transient Fault Handling).

## 1. YouTube Data API Quota Guardrails
- **Strict Polling Limits**:
  - Live chat list queries consume 1 quota unit per request. Continuous rapid polling quickly exhausts the 10,000 units/day default YouTube quota.
  - Enforce a minimum polling interval (default 10s = $\approx 360$ units/hour). Never allow interval sliders or configs to drop below safe minimums without explicit warnings.
  - Honor the `pollingIntervalMillis` returned in the YouTube API response payload if it is higher than the user configuration.

## 2. Exponential Backoff with Full Jitter
- When receiving transient network failures (HTTP 500, 502, 503, 504) or rate limits (HTTP 429):
  $$\text{Sleep} = \text{random}(0, \min(\text{MaxBackoff}, \text{BaseInterval} \times 2^{\text{attempt}}))$$
- Adding full jitter avoids the "thundering herd" problem where multiple client instances hit the API at the identical second.

## 3. Circuit Breaker for Quota Exhaustion
- If the API returns HTTP 403 with `quotaExceeded`:
  - **Trip the circuit breaker immediately**: Stop all recurring polling loops instantly.
  - Do not retry automatically until the next quota reset window (midnight Pacific Time) or until the user supplies a new API key.
  - Notify the UI with a clean, actionable status message explaining quota exhaustion.

## 4. Network Disconnection & Offline Tolerance
- Detect offline states gracefully; transition overlay state to "Reconnecting..." without clearing already-rendered messages.
- Buffer failed telemetry or non-critical state updates instead of dropping or crashing.
- On connection restore, re-verify stream status before resuming standard polling.
