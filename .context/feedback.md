# User Feedback Triage (`fb1`, `fb2`...)

*Secondary Priority: Quality-of-life enhancements, styling refinements, and developer experience.*

---

## Addressed Feedback (`[x]`)

- [x] **fb1**: **Fluent Search & Start 11 Inspired Settings UI**
  - **Summary**: Design an ultra-clean, modern dark settings panel comparable to Fluent Search and Start 11.
  - **Resolution**: Designed `settings_ui.html` using Microsoft Fluent 2 tokens, mica-inspired acrylic surface tiers, smooth toggle switches, accent sliders, and instant live preview.

- [x] **fb2**: **Custom CSS & Template Theme Support**
  - **Summary**: Provide full custom CSS support with a documented template file containing all variable placeholders.
  - **Resolution**: Created `themes/template.css` and `.streamtheme` JSON specification, exporting CSS variables for background colors, font styles, glow parameters, and border radii.

- [x] **fb3**: **User-Friendly Glow Effect Parameters**
  - **Summary**: Rename underlying shadow CSS parameters to "Glow Intensity" and "Glow Radius" for intuitive user understanding.
  - **Resolution**: Updated the UI labels and CSS token mappings to present intuitive glow controls while generating appropriate `text-shadow` and `box-shadow` values internally.

- [x] **fb4**: **Windows 10 & Windows 11 Dual Compatibility**
  - **Summary**: Application must operate smoothly on both Windows 10 (GDI+/DWM fallback) and Windows 11 (Mica/Acrylic composition).
  - **Resolution**: Implemented OS version detection in `DwmHelper.hpp` with graceful fallback from Mica/Acrylic to layered glass composition on Windows 10.

- [x] **fb5**: **Modular Skills Architecture (.agents/skills/)**
  - **Summary**: Set up an industry-grade skills directory for clean vibe coding and maintainability.
  - **Resolution**: Created `.agents/skills/` with 8 modular skills (`ponytail.md`, `architecture.md`, `systematic-debugging.md`, `resource-efficiency.md`, `api-resilience.md`, `ui-ux-design.md`, `verification.md`, `guardrails.md`) and indexed them via Progressive Disclosure in `AGENTS.md`.

---

## Pending / Future Feedback (`[ ]`)

- [ ] **fb6**: **Twitch / Kick Multi-Platform Chat Ingestion**
  - **Summary**: Expand chat overlay capabilities to support multi-stream setups (Twitch IRC / Kick WebSocket) alongside YouTube Live.
  - **Action Plan**: Design an abstract `ChatEngine` interface, allowing pluggable streaming sources with unified message normalization.
