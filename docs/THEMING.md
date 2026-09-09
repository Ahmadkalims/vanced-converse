# Stream Theme Specification (`.streamtheme`)

This document defines the architecture, JSON schema, and properties for the **YouTube Live Chat Overlay** theme system.

---

## 1. Overview & File Location

Themes in YouTube Live Chat Overlay use the `.streamtheme` file format, which is standard JSON stored in:
```
%APPDATA%\YT_Live_Chat_Overlay\Themes\
```
Example: `C:\Users\<User>\AppData\Roaming\YT_Live_Chat_Overlay\Themes\Midnight Pink.streamtheme`

### Key Features
- **Human-Readable & LLM-Friendly**: Pure JSON schema with intuitive property naming.
- **Drag-and-Drop Installation**: Drag any `.streamtheme` file directly onto the Settings Window (`WM_DROPFILES`) for instant validation, persistence, and live overlay switching.
- **Live Updating**: Overlay instantly applies visual changes without restarting the app or disrupting the stream.
- **Strict Emoji Glow Isolation**: Glow or text shadow effects configured for usernames or text *never* contaminate Unicode or custom emojis. Emojis are always rendered cleanly.

---

## 2. Complete `.streamtheme` Reference Example (Midnight Pink)

```json
{
  "metadata": {
    "name": "Midnight Pink",
    "author": "Antigravity",
    "version": 1,
    "description": "Dark acrylic overlay with pink highlights matching reference theme"
  },
  "overlay": {
    "background": {
      "type": "acrylic",
      "color": "#14141E",
      "opacity": 0.4,
      "blur": 15.0,
      "saturation": 120.0,
      "outline": {
        "enabled": false,
        "size": 0.0,
        "type": "solid",
        "color": "#FFFFFF",
        "glowColor": "#FFFFFF",
        "glowBlur": 0.0
      },
      "cornerRadius": 20.0
    }
  },
  "scrollbar": {
    "visible": false,
    "interactionMode": "clickthrough_off_hover",
    "width": 8.0,
    "color": "#F9A8D4",
    "type": "solid",
    "glowColor": "#F9A8D4",
    "cornerRadius": 4.0
  },
  "username": {
    "placement": "inline",
    "font": {
      "family": "Montserrat Alternates",
      "size": 16.0,
      "weight": 600
    },
    "color": {
      "mode": "persistent_user_color",
      "singleColor": "#F9A8D4",
      "palette": [
        "#F9A8D4",
        "#F472B6",
        "#38BDF8",
        "#34D399",
        "#A78BFA",
        "#FBBF24",
        "#FB7185",
        "#4ADE80"
      ]
    },
    "effect": {
      "type": "glow",
      "shadow": "0 0 4px rgba(249, 168, 212, 0.7)"
    },
    "lineSpacing": 0.0,
    "padding": { "top": 0.0, "right": 0.0, "bottom": 0.0, "left": 0.0 },
    "margin": { "top": 0.0, "right": 0.0, "bottom": 0.0, "left": 0.0 }
  },
  "messageText": {
    "font": {
      "family": "Montserrat Alternates",
      "size": 16.0,
      "weight": 400
    },
    "color": "#E5E7EB",
    "effect": {
      "type": "solid",
      "shadow": ""
    },
    "lineSpacing": 0.0,
    "padding": { "top": 0.0, "right": 0.0, "bottom": 0.0, "left": 0.0 },
    "margin": { "top": 0.0, "right": 0.0, "bottom": 0.0, "left": 0.0 }
  },
  "messageBackground": {
    "enabled": true,
    "color": "rgba(255, 255, 255, 0.02)",
    "outline": {
      "enabled": false,
      "size": 0.0,
      "type": "solid",
      "color": "#FFFFFF",
      "glowColor": "#FFFFFF"
    },
    "cornerRadius": 15.0,
    "padding": { "top": 12.0, "right": 16.0, "bottom": 12.0, "left": 16.0 },
    "margin": { "top": 10.0, "right": 0.0, "bottom": 10.0, "left": 0.0 }
  },
  "specialEvents": [
    {
      "id": "question",
      "name": "Question",
      "enabled": true,
      "trigger": {
        "type": "contains_character",
        "value": "?"
      },
      "highlight": {
        "backgroundColor": "rgba(249, 168, 212, 0.12)",
        "outline": {
          "enabled": true,
          "size": 2.0,
          "type": "solid",
          "color": "#F472B6",
          "glowColor": "#F472B6"
        },
        "cornerRadius": 15.0
      }
    }
  ]
}
```

---

## 3. Schema Property Definitions & Allowed Values

### `metadata`
| Field | Type | Description |
|---|---|---|
| `name` | `string` | Display name of the theme (e.g. `"Midnight Pink"`). |
| `author` | `string` | Creator's name or tag. |
| `version` | `integer` | Schema version (currently `1`). |
| `description` | `string` | Human-readable description of visual style. |

### `overlay.background`
| Field | Type | Allowed Values | Description |
|---|---|---|---|
| `type` | `string` | `"acrylic"`, `"mica"`, `"liquid_glass"`, `"solid"`, `"transparent"`, `"linear_gradient"`, `"radial_gradient"`, `"image"` | Backdrop material shader. |
| `color` | `string` | Hex (`"#14141E"`) or RGBA (`"rgba(20, 20, 30, 0.4)"`) | Base tint color. |
| `opacity` | `number` | `0.0` to `1.0` | Global backdrop opacity. |
| `blur` | `number` | `0.0` to `50.0` | Blur radius (e.g. `15.0` for acrylic backdrop-filter). |
| `saturation` | `number` | `0.0` to `200.0` | Saturation boost percentage (e.g. `120.0`). |
| `cornerRadius` | `number` | `0.0` to `50.0` | Window corner rounding (e.g. `20.0`). |

### `scrollbar`
| Field | Type | Allowed Values | Description |
|---|---|---|---|
| `visible` | `boolean` | `true`, `false` | Whether the scrollbar is drawn. |
| `interactionMode` | `string` | `"always"`, `"clickthrough_off_hover"`, `"disabled"` | Display/interactive mode. |
| `width` | `number` | `2.0` to `30.0` | Width of scrollbar in pixels. |
| `color` | `string` | Hex or RGBA | Scrollbar thumb color. |

### `username` & `messageText`
| Field | Type | Allowed Values | Description |
|---|---|---|---|
| `font.family` | `string` | Any installed Windows font (e.g. `"Montserrat Alternates"`, `"Segoe UI"`) | Primary typeface. Font fallback automatically chains to `Nirmala UI` (Hindi), `Microsoft YaHei` (CJK), `Segoe UI` (Arabic RTL), and `Segoe UI Emoji`. |
| `font.size` | `number` | `8.0` to `72.0` | Font size in points (e.g. `16.0`). |
| `font.weight` | `integer` | `100` to `900` | Font weight (`400` = Regular, `600` = SemiBold, `700` = Bold). |
| `color.mode` | `string` | `"single"`, `"random_palette"`, `"persistent_user_color"` | How username colors are picked. `"persistent_user_color"` hashes the user ID deterministically. |
| `color.palette` | `array of strings` | Hex or RGBA colors | Palette used for username color assignment. |
| `effect.type` | `string` | `"solid"`, `"glow"`, `"shadow"` | Text effect shader. |
| `effect.shadow` | `string` | CSS-style shadow string (e.g. `"0 0 4px rgba(249, 168, 212, 0.7)"`) | Multi-pass glow or shadow parameters. |

### `specialEvents`
Special events allow conditional message card highlights:
| Field | Type | Description |
|---|---|---|
| `id` | `string` | Event identifier (e.g. `"question"`, `"superchat"`). |
| `trigger.type` | `string` | `"contains_character"`, `"contains_text"`, `"starts_with"`. |
| `trigger.value` | `string` | Character or text substring to match (e.g. `"?"`, `"#question"`). |
| `highlight.backgroundColor` | `string` | Highlight card background tint. |
| `highlight.outline` | `object` | Highlight outline border (`size`, `color`, `glowColor`). |

---

## 4. Built-in Theme Presets

The overlay includes 5 pre-configured `.streamtheme` presets ready out of the box:
1. **Midnight Pink** (Default reference theme matching dark acrylic backdrop with vibrant pink accents).
2. **Cyberpunk Neon** (Deep dark backdrop with high-contrast neon cyan & magenta glows).
3. **Frost Glass** (Frosted mica material with crisp cool-white and subtle sky blue accents).
4. **Sunset Glow** (Warm evening orange-to-purple gradient with luminous peach highlights).
5. **Monochrome Minimal** (Clean, high-legibility minimalist dark theme).

---

## 5. Drag-and-Drop Installation Workflow

1. Create or download any `.streamtheme` JSON file.
2. Open the Settings Window (`Right-click Tray Icon -> Settings` or `Appearance`).
3. Drag the `.streamtheme` file into the Settings Window.
4. The system validates the schema, imports the theme into `%APPDATA%\YT_Live_Chat_Overlay\Themes\`, activates it immediately, and updates the live overlay in real time!
