# Code Standards & Conventions

## 1. General Principles
- Write the minimal correct code necessary to solve the problem.
- Prioritize reliability, security, and low memory usage over speculative abstractions.
- Never duplicate logic or configuration across files.

## 2. JavaScript / Node.js Standards
- **Syntax:** Modern ES6+ (CommonJS for Node/Electron modules, ES Modules where supported).
- **Asynchronous Code:** Prefer `async/await` over raw promise chaining and nested callbacks.
- **Error Handling:** Always wrap I/O, IPC, and network calls in `try / catch` blocks with meaningful fallbacks and structured error logging. Never silently swallow errors.
- **Variable Declarations:** Use `const` by default; use `let` only when re-assignment is required. Never use `var`.
- **Naming Conventions:**
  - `camelCase` for functions, methods, and variables (`pollMessages`, `activeLiveChatId`).
  - `PascalCase` for classes and constructor functions (`YouTubeService`, `ThemeEngine`).
  - `UPPER_SNAKE_CASE` for global constants (`CONFIG_NAME`, `DEFAULT_POLL_INTERVAL_MS`).
  - `kebab-case` for file names and CSS classes (`chat-message`, `message-parser.js`).

## 3. DOM & UI Standards
- **DOM Safety (XSS Prevention):** Never assign unsanitized API strings directly to `innerHTML`. Use `textContent` for dynamic text or use a strict HTML escaping sanitizer utility.
- **DOM Efficiency:** When rendering batches of messages, use `DocumentFragment` to batch DOM insertions into a single reflow/repaint cycle.
- **Resource Management:** Enforce a maximum cap on active DOM elements (e.g. max 200 message nodes) by removing oldest elements.

## 4. Electron Standards
- **Security First:** Keep `contextIsolation: true` and `nodeIntegration: false` enabled. Expose minimal, strictly validated APIs via `preload.js` with `contextBridge`.
- **IPC Handlers:** Keep IPC handlers lightweight and non-blocking. Never perform synchronous file I/O inside IPC handlers.
- **State Management:** Keep single sources of truth. The main process manages persistent configuration, while renderer processes manage transient UI state.
