# Features Matrix (Native C++)

## 1. Feature Status Summary

| Feature Area | Feature | Status | Details |
|---|---|---|---|
| **Portability & Performance** | Native Portable Executable | Implemented | ~1.48 MB standalone `.exe` without Electron or node_modules dependencies. |
| | Low Resource Usage | Implemented | < 0.1% CPU idle, ~8-12 MB RAM footprint, instant <30ms startup. |
| **System Tray & Menus** | Background Tray Icon | Implemented | `Shell_NotifyIcon` with custom icon and tooltip. |
| | Tray Context Menu | Implemented | Show/Hide, Click-Through shortcut checkbox, Always On Top checkbox, Appearance shortcut, Settings, Quit. |
| **Settings Panel** | Tabbed Modern Settings Window | Implemented | Three tabs: Stream & API, Appearance & Themes, Overlay Controls. |
| | API Key Generator Link | Implemented | One-click button opens Google Cloud Console credentials page in default browser. |
| | Live Real-Time Updating | Implemented | Any slider, combo, or text change immediately re-renders the overlay HUD in real-time. |
| | Quota Consumption Calculator | Implemented | Real-time calculation of units/hr and streaming hours per 10k quota. |
| **Theming & Materials** | Blur Materials | Implemented | **Acrylic**, **Mica**, **Liquid Glass**, and **Flat Translucent**. |
| | Opacity & Blur Sliders | Implemented | 0% to 100% opacity slider and 0px to 40px blur radius slider. |
| | Typography Customizer | Implemented | Configurable Font Family (Montserrat, Segoe UI, Roboto, Consolas, Arial, Outfit) and Font Size (10px to 32px). |
| | Premade Theme Presets | Implemented | Midnight Acrylic, Cyberpunk Neon, Frost Glass, Sunset Glow, Monochrome Minimal. |
| | Persistent Username Glow | Implemented | Deterministic rotating neon glow colors for all chat participants. |
| | Highlight Trigger Badges | Implemented | Customizable command trigger (e.g. `#question`, `#ask`) with glowing border and background card. |
| **Window & Overlay** | Layered Glass HUD | Implemented | 32-bit ARGB GDI+ rendering with rounded corners and specular border. |
| | Click-Through Mode | Implemented | True Win32 `WS_EX_TRANSPARENT` click pass-through to underlying apps/games. |
| | Right-Side Display Docking | Implemented | Automatic right-dock calculation and manual re-dock button. |
