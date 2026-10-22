# Vanced Converse StreamTheme Reference & Customization Guide

Vanced Converse features an open, hot-reloadable JSON theme engine. Themes live in the `Themes/` directory alongside `Vanced Converse.exe`.

---

## 1. Theme File Structure & JSON Schema
Every theme conforms to `Themes/theme.schema.json`.

```json
{
  "schemaVersion": "1.0.0",
  "name": "Midnight Pink",
  "author": "Vanced Converse Team",
  "description": "Signature sleek dark acrylic theme with luminous magenta accent glows.",
  "window": {
    "backgroundType": "acrylic",
    "backgroundColor": "#121622",
    "opacity": 0.85,
    "blurRadius": 25,
    "acrylicTint": "#0e111a",
    "acrylicLuminosity": 0.65,
    "cornerRadius": 24,
    "borderWidth": 1.5,
    "borderColor": "#ff4b8c",
    "borderGlowColor": "#ff4b8c",
    "borderGlowRadius": 22,
    "borderGlowIntensity": 1.0,
    "padding": 16,
    "edgeFadeMargin": 20
  },
  "messageBubble": {
    "background": "#181d2b",
    "opacity": 0.92,
    "cornerRadius": 16,
    "padding": 12,
    "margin": 6,
    "spacing": 8,
    "maxWidth": 420,
    "borderWidth": 1.0,
    "borderColor": "#ff4b8c44"
  },
  "typography": {
    "fontFamily": "Segoe UI Variable, Segoe UI, sans-serif",
    "fontSize": 14,
    "fontWeight": "Normal",
    "lineHeight": 1.4,
    "authorColor": "#ff79b0",
    "messageColor": "#ffffff",
    "modBadgeColor": "#00e5ff",
    "superchatColor": "#ffd700"
  },
  "textEffects": {
    "strokeEnabled": false,
    "strokeWidth": 1.0,
    "strokeColor": "#000000",
    "glowEnabled": true,
    "glowColor": "#ff4b8c",
    "glowRadius": 8,
    "glowIntensity": 0.75,
    "shadowEnabled": true,
    "shadowColor": "#000000aa",
    "shadowBlur": 4,
    "shadowOffsetX": 0,
    "shadowOffsetY": 2,
    "gradientFillEnabled": false
  }
}
```

---

## 2. Properties Reference

### Window Section
- `backgroundType`: `solid`, `gradient`, `acrylic`, `blur`, or `transparent`.
- `backgroundColor`: Base 6- or 8-digit hex color code (`#RRGGBB` or `#RRGGBBAA`).
- `opacity`: Viewport transparency (from `0.0` invisible to `1.0` opaque).
- `cornerRadius`: Curvature of outer overlay borders (from `0` square to `60` pill).
- `borderGlowColor`: Hex color for outer diffused neon glow.
- `borderGlowRadius`: Spread radius of outer border glow in pixels.
- `borderGlowIntensity`: Multiplier for glow luminosity (`0.0` to `2.0`).

### Typography & Text Effects
- `fontFamily`: Standard system font or loaded font file.
- `fontSize`: Text point size (`10` to `24`).
- `strokeEnabled`: Enables high-contrast outer stroke around message text.
- `strokeWidth`: Outline stroke thickness (`0.5` to `3.0`).
- `glowEnabled`: Enables soft neon text glow.
- `glowColor`: Color of text glow.
- `glowRadius`: Spread radius of text glow blur.
- `shadowEnabled`: Enables directional drop shadow behind text.

---

## 3. Two-Way Sync & Hot Reloading
- **In-App to File**: Modifying sliders in the Settings window immediately writes to the active JSON theme file on disk.
- **File to In-App**: Editing any `.json` file in `Themes/` with a text editor (e.g., VS Code) immediately updates the live stream overlay via debounced file-watching without restarting the app.
- **Fail-Safe Fallback**: If a theme contains malformed syntax, the application displays a non-blocking status notification and keeps the last good theme active without crashing.
