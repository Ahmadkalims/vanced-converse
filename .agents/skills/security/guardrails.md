# Security & Operational Guardrails

## 1. Secrets & Credentials Protection
- **Zero Hardcoded Secrets**:
  - Never commit API keys (e.g., YouTube Data API keys), OAuth tokens, client secrets, or private credentials into source code, comments, or test fixtures.
  - Load keys dynamically via secure environment variables (`.env`, Windows credential store, or runtime user input config).
  - Verify `.gitignore` explicitly blocks credentials, local configuration files, and build artifacts.
- **Pre-Commit Scan**:
  - Before writing or committing code, inspect diffs for accidental credential leakage or hardcoded token strings.

## 2. Safe API Handling & Quota Protection
- **YouTube Data API Quotas**:
  - Implement defensive rate limiting (e.g., respect configured minimum poll intervals such as 10s).
  - Exponential backoff: on HTTP 403 (Quota Exceeded) or HTTP 429 (Rate Limit), back off immediately rather than continuously spamming retries.
  - Cache responses where permissible and request only required resource fields (`part=snippet,authorDetails`).

## 3. Input Sanitization & IPC Defense
- **DOM Injection & XSS Defense**:
  - Treat all incoming live chat messages, author usernames, badges, and user inputs as untrusted.
  - Never inject raw chat strings into `innerHTML`. Use `textContent`, safe element creation, or strict DOM sanitization before rendering into the overlay.
- **IPC & WebView2 Security**:
  - Validate and sanitize all messages exchanged across the native C++ <-> WebView2 web bridge.
  - Restrict navigation to local assets (`file:///` or trusted origins); block arbitrary external URL navigation.

## 4. Error Boundaries & Crash Resilience
- **Fail Gracefully**:
  - Wrap network operations, file I/O, and JSON deserialization in robust error handling (`try/catch` or error codes).
  - Never allow a failed network call or corrupted config file to terminate or freeze the desktop application.
  - Fall back to sensible default configurations when persistent data is missing or invalid.
