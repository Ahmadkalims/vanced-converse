# YouTube Data API v3 Integration

## 1. Overview
The application connects directly to Google's YouTube Data API v3 to discover live chat sessions and retrieve live chat messages.

## 2. API Endpoints Used

### Endpoint 1: Discover Active Live Chat ID
- **Method & Path:** `GET https://www.googleapis.com/youtube/v3/videos`
- **Query Parameters:**
  - `part`: `liveStreamingDetails`
  - `id`: `<videoId>` (extracted from user configuration)
  - `key`: `<apiKey>`
- **Quota Cost:** 1 unit per request.
- **Success Response Structure:**
  ```json
  {
    "items": [
      {
        "id": "vtL-JrlhT2g",
        "liveStreamingDetails": {
          "activeLiveChatId": "Cg0KC3Z0TC1KcmxoVDJn"
        }
      }
    ]
  }
  ```
- **Error Conditions:**
  - Invalid Video ID: `items: []`
  - Stream offline / not live: `liveStreamingDetails.activeLiveChatId` is undefined.
  - Invalid API Key: HTTP 400 `API key not valid` (error code `badRequest`).

### Endpoint 2: Fetch Live Chat Messages
- **Method & Path:** `GET https://www.googleapis.com/youtube/v3/liveChat/messages`
- **Query Parameters:**
  - `liveChatId`: `<activeLiveChatId>`
  - `part`: `snippet,authorDetails`
  - `key`: `<apiKey>`
  - `pageToken`: `<nextPageToken>` (optional; omitted on initial request)
- **Quota Cost:** 1 unit per request (or 5 units depending on parts requested).
- **Success Response Structure:**
  ```json
  {
    "pollingIntervalMillis": 15000,
    "nextPageToken": "GL2A-4a8z...==",
    "items": [
      {
        "id": "LCC.EhwKGkNOcmgtcEhmbTRzQ0ZYTUkxZ0FkVmdnSUVB",
        "snippet": {
          "type": "textMessageEvent",
          "hasDisplayContent": true,
          "displayMessage": "Hello stream!",
          "publishedAt": "2026-08-24T12:00:00.000Z"
        },
        "authorDetails": {
          "channelId": "UC...",
          "displayName": "Alice",
          "isChatOwner": false,
          "isChatModerator": false,
          "isVerified": false
        }
      }
    ]
  }
  ```

## 3. Polling Lifecycle & Request Flow
```
[Start Overlay] 
       |
       v
[Read Config: videoId + apiKey]
       |
       v
[GET videos.list?part=liveStreamingDetails]
       |
       +---> (If activeLiveChatId missing) ---> [Wait / Show Warning]
       |
       v (activeLiveChatId found)
[GET liveChat/messages.list (pageToken=null)]
       |
       +---> [Extract items, Deduplicate by ID, Render]
       +---> [Update nextPageToken]
       +---> [Read pollingIntervalMillis (e.g. 15000ms)]
       |
       v
[Sleep max(userConfiguredInterval, apiPollingIntervalMillis)]
       |
       v
[GET liveChat/messages.list (pageToken=nextPageToken)] ---> (Loop)
```

## 4. Quota Consumption & Constraints
- **Default Daily Quota:** 10,000 units per project per day.
- **Consumption Rate Analysis:**
  - At 1 request / 5s: 720 units/hour -> Quota exhausted in **~13.8 hours**.
  - At 1 request / 15s: 240 units/hour -> Quota exhausted in **~41.6 hours**.
  - At 1 request / 30s: 120 units/hour -> Quota exhausted in **~83.3 hours**.
- **Rules for Quota Optimization:**
  1. Never poll faster than `pollingIntervalMillis` returned by the API (hard limit enforced by Google).
  2. Enforce a minimum polling floor (e.g. 3,000ms minimum, 10,000ms recommended).
  3. Pause polling when overlay window is hidden or minimized.
  4. Implement exponential backoff on HTTP 403 (Quota Exceeded) and HTTP 5xx errors.

## 5. Error & Status Code Mapping
| Status Code | Reason | Handling Action |
|---|---|---|
| 200 OK | Success | Parse messages, store nextPageToken, schedule next poll at returned interval. |
| 400 Bad Request | Invalid parameter / key | Stop polling; notify user via UI to check API Key / Video ID. |
| 403 Forbidden | Quota Exceeded (`quotaExceeded`) | Stop polling immediately; display clear "Daily Quota Reached" badge. |
| 404 Not Found | Chat / Video not found | Stop polling; stream ended or URL invalid. |
| 500 / 503 | Backend server error | Retry with exponential backoff (5s, 10s, 20s, max 60s). |
| Network Error | Client offline / DNS fail | Retry after 5s without resetting `nextPageToken`. |
