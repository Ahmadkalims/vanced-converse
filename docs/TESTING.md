# Testing Strategy & Architecture

## 1. Testing Framework & Tooling
- **Unit & Integration Runner:** Vitest / Node Native Test Runner (compatible with ES Modules / CommonJS).
- **DOM Testing:** JSDOM / Happy-DOM for renderer logic verification without launching full Electron windows.
- **E2E Testing:** Playwright / Spectron for full Electron lifecycle validation.

## 2. Testing Principles & Rules

> [!IMPORTANT]
> **API Testing Rule:** Normal automated test suites MUST NEVER call live YouTube API endpoints. All tests must use mocked responses, fixtures, or dependency injection. Real API smoke tests must be strictly isolated, run on-demand only, and use environment variables.

## 3. Test Directory Structure
```
tests/
  ├── unit/
  │    ├── config.test.js           # Config loading, defaults, validation, serialization
  │    ├── url-parser.test.js       # YouTube video URL/ID extractor
  │    ├── message-parser.test.js   # Chat parsing, #question detection, sanitization
  │    ├── color-palette.test.js    # Persistent username color assignment logic
  │    └── polling-engine.test.js   # Polling interval, backoff, and overlapping guards
  ├── integration/
  │    ├── ipc-contract.test.js     # Main <-> Renderer IPC channels
  │    └── api-service.test.js      # Mocked HTTP requests (videos.list, liveChat.messages)
  ├── e2e/
  │    └── overlay-window.test.js   # Window creation, tray interactions, settings modal
  ├── mocks/
  │    ├── electron.mock.js         # Mocked ipcRenderer, ipcMain, BrowserWindow, Tray
  │    └── fetch.mock.js            # Simulated fetch with HTTP response status injection
  └── fixtures/
       ├── videos-response.json     # Sample YouTube videos.list payload
       ├── messages-response.json   # Sample liveChatMessages.list payload
       ├── empty-chat.json          # Empty chat payload
       └── error-403-quota.json     # Quota exceeded error payload
```

## 4. Priority Test Matrix

| Area | Test Scenario | Criticality |
|---|---|---|
| **YouTube API** | Successful 200 message batch parsing | CRITICAL |
| | Missing `activeLiveChatId` (stream offline) | CRITICAL |
| | 403 Quota Exceeded error handling & backoff | CRITICAL |
| | Network failure / timeout resilience | HIGH |
| | Page token pagination & deduplication | CRITICAL |
| **Polling Engine** | Default vs custom polling interval | HIGH |
| | Minimum interval enforcement (quota floor) | HIGH |
| | Overlapping request prevention on delay | CRITICAL |
| | Stop / Resume polling on window hide/show | MEDIUM |
| **Security & DOM** | XSS sanitization in author names and text | CRITICAL |
| | Rolling DOM message buffer limits | HIGH |
| **Configuration** | Default config fallback on corrupt file | HIGH |
| | URL parsing (standard, short, timestamped) | HIGH |
| | Color assignment deterministic rotation | MEDIUM |

## 5. Test Commands
```bash
npm test                # Run all unit and integration tests with mocks
npm run test:unit       # Run unit tests only
npm run test:e2e        # Run end-to-end Electron tests
npm run test:coverage   # Generate code coverage report
```
