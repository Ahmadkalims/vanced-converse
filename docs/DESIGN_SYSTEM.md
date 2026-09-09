# Design System & Tokens

## 1. Design Tokens (CSS Variables)

### Color Tokens & Palettes
```css
:root {
  /* Surface & Background */
  --bg-overlay-color: rgba(20, 20, 30, 0.4);
  --bg-card-color: rgba(255, 255, 255, 0.03);
  --bg-card-hover: rgba(255, 255, 255, 0.06);

  /* Typography Colors */
  --text-primary: #f3f4f6;
  --text-secondary: #9ca3af;
  --text-muted: #6b7280;

  /* Accent & Highlight Colors */
  --accent-highlight: #f472b6;
  --accent-highlight-bg: rgba(244, 114, 182, 0.15);
  --border-highlight: #f472b6;

  /* Backdrop Filter Defaults */
  --blur-intensity: 16px;
  --saturation-intensity: 130%;

  /* Spacing & Radii */
  --radius-window: 20px;
  --radius-message: 14px;
  --spacing-padding: 14px;
  --spacing-gap: 10px;

  /* Typography */
  --font-family: "Montserrat Alternates", system-ui, -apple-system, sans-serif;
  --font-size-base: 15px;
  --font-size-author: 14px;
  --font-size-timestamp: 11px;
}
```

## 2. Backdrop Blur & Material Types

| Blur Material | CSS Formula | Visual Description |
|---|---|---|
| **Acrylic** | `backdrop-filter: blur(var(--blur-intensity)) saturate(140%); background: rgba(18, 18, 28, var(--bg-opacity));` | Standard Windows 10/11 acrylic effect with rich saturation boost. |
| **Mica** | `backdrop-filter: blur(28px) saturate(110%); background: rgba(30, 30, 42, var(--bg-opacity));` | Windows 11 style subtle, clean frosted sheen with balanced neutral tint. |
| **Liquid Glass** | `backdrop-filter: blur(20px) saturate(180%) contrast(110%); border: 1px solid rgba(255,255,255,0.25); box-shadow: inset 0 0 12px rgba(255,255,255,0.1);` | Glossy, refractive glass surface with bright rim specular lighting. |
| **Flat Translucent** | `backdrop-filter: none; background: rgba(15, 15, 22, var(--bg-opacity));` | Pure alpha-blended flat background for maximum performance on low-end GPUs. |

## 3. Persistent Author Glow Palette
To distinguish different participants in active live chats, usernames are assigned deterministic or rotating glow badges:
- **Neon Blue:** Color `#38bdf8`, Glow `0 0 6px rgba(56, 189, 248, 0.7)`
- **Emerald Green:** Color `#34d399`, Glow `0 0 6px rgba(52, 211, 153, 0.7)`
- **Rose Pink:** Color `#f472b6`, Glow `0 0 6px rgba(244, 114, 182, 0.7)`
- **Vibrant Orange:** Color `#fb923c`, Glow `0 0 6px rgba(251, 146, 60, 0.7)`
- **Electric Violet:** Color `#c084fc`, Glow `0 0 6px rgba(192, 132, 252, 0.7)`

## 4. Premade Themes

1. **Midnight Glass (Default):** Deep slate-blue acrylic, rose accent, Montserrat font.
2. **Cyberpunk Neon:** High-contrast dark violet backdrop, neon cyan & magenta highlights, Orbitron/Inter font.
3. **Frost White:** Semi-opaque icy white backdrop with dark typography and crisp sky-blue accents.
4. **Sunset Glow:** Warm smoked-amber glassmorphism with golden-orange and coral accents.
5. **Monochrome Minimal:** Pure grayscale frosted surface with crisp white bold typography.
