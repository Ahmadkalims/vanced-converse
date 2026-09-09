# Product UX & Overlay Design

## 1. Product Experience Vision
YT Live Chat Overlay is designed as a sleek, non-intrusive heads-up display (HUD) for live streamers and viewers. It floats seamlessly over desktop games, creative applications, or broadcasting tools, offering instant visibility of chat interactions without requiring a second monitor.

## 2. Window & Overlay Behavior
- **Window Posture:** Frameless, transparent floating strip docked to the right edge of the primary monitor by default.
- **Always on Top:** Stays pinned above active windows during streaming or gaming sessions.
- **Click-Through Mode:** In click-through mode (`setIgnoreMouseEvents(true, { forward: true })`), mouse clicks pass directly through to games or underlying desktop apps while preserving readability.
- **Taskbar Footprint:** Hidden from the Windows taskbar (`skipTaskbar: true`), managed unobtrusively via the Windows System Tray.

## 3. Visual Philosophy
- **Glassmorphism & Depth:** Modern backdrop filtering using **Acrylic**, **Mica**, and **Liquid Glass** aesthetic presets to blend naturally into dark or light desktop backgrounds.
- **Hierarchy & Contrast:** High-contrast text with subtle neon/pastel glows on usernames to maintain legibility across varying wallpaper and game graphics.
- **Streamlined Layout:** Vertical chat stream with smooth auto-scrolling, compact message bubbles, and distinct visual badges for high-priority messages (e.g. `#question`).

## 4. Customization Experience
The application provides two layers of customization:
1. **Quick Themes:** Curated 1-click presets (Midnight Acrylic, Neon Cyberpunk, Frost Glass, Dark Mica, Sunset Glow, Clean Minimal).
2. **Fine-Grained Controls:**
   - **Blur Mode:** Select between Acrylic, Mica, Liquid Glass, or Flat Translucent.
   - **Blur Intensity & Opacity:** Continuous sliders for background opacity (0% - 100%) and blur radius (0px - 40px).
   - **Typography:** Custom font selection (e.g. Montserrat, Inter, Roboto, Outfit, Consolas) and font size scaling (12px to 28px).
   - **Highlight Triggers:** User-defined prefix or regex rules (e.g. `#question`, `#idea`, `!vote`).
