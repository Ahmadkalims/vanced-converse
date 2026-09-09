# Component Library

## 1. Component Architecture
UI elements are structured as lightweight, performant vanilla HTML/JS components rendered directly to the DOM without heavy framework overhead.

## 2. Existing Reusable Components

### 1. ChatContainer (`.chat-container`)
- **Location:** `index.html` / `style.css`
- **Purpose:** Primary scrollable viewport hosting the live chat stream and applying backdrop filter materials.
- **Attributes / Classes:**
  - `.chat-container`: Base glassmorphism layout, scrollbar suppression, auto-overflow.
  - Theme/Material modifier classes: `.material-acrylic`, `.material-mica`, `.material-liquid-glass`.
- **Interactions:** Pauses auto-scrolling on hover (`mouseenter`), resumes auto-scrolling on mouse leave (`mouseleave`).

### 2. ChatMessage (`.chat-message`)
- **Location:** `renderer.js` (`addMessage` function) / `style.css`
- **DOM Structure:**
  ```html
  <div class="chat-message [highlight]">
    <div class="message-content">
      <span class="user" style="color: #...; text-shadow: ...;">AuthorName:</span>
      <span class="text">Message body...</span>
    </div>
  </div>
  ```
- **Variants:**
  - Standard message: Semi-transparent pill card.
  - Highlighted message (`.highlight`): Colored border and glowing translucent background when matching prefix rules like `#question`.

### 3. SystemNotification / ErrorCard (`.system-message`)
- **Location:** `renderer.js`
- **Purpose:** Display operational messages (e.g. "Stream connecting...", "No active stream found", "Quota exhausted").

### 4. SettingsForm (`config.html`)
- **Location:** `config.html`
- **Purpose:** Dedicated modal input for YouTube stream URL/ID and API credentials.
- **Controls:** Video URL text field with auto-parser, API key input, Save & Close button.

## 3. Planned Components (Roadmap)
- **ThemeCustomizerPanel:** Tabbed UI inside Settings modal for visual customization (premade theme picker, sliders for opacity/blur, font dropdown, color pickers).
- **QuotaMeterWidget:** Visual indicator displaying estimated daily API quota usage.
- **StreamStatusBadge:** Real-time badge indicating Connection Status (Live, Connecting, Offline, Error).
