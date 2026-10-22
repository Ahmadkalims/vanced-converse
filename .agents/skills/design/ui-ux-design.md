# UI/UX Design System & Anti-Slop Guidelines

## 1. Anti-"AI Slop" Visual Principles
- **No Generic Templates**: Avoid overused AI defaults (e.g., standard saturated purple gradients on black, unstyled round pill buttons, or inconsistent padding).
- **Curated Palettes & Depth**:
  - Use subtle dark surfaces with semantic contrast tiers (e.g., `#0f1117` base, `#181b24` card, `#222736` hover).
  - Use elevation via delicate 1px borders with low-opacity rgba highlights (`rgba(255, 255, 255, 0.08)`) and diffused soft shadows.
  - Implement modern Windows Fluent / Mica / Acrylic feel with subtle backdrop blur where supported.

## 2. Design Tokens & Consistency
- **Spacing Scale**: Strictly adhere to an 8pt / 4pt grid system:
  - `4px` (micro), `8px` (compact), `12px` (gap), `16px` (standard padding), `24px` (section), `32px` (container).
- **Typography Tokens**:
  - Prefer modern system sans-serifs (`Segoe UI Variable`, `Segoe UI`, `Inter`, system-ui).
  - Explicit typographic hierarchy: `h1` (24px/600), `h2` (18px/600), body (13-14px/400), caption/meta (11-12px/400).
  - Minimum contrast ratio of 4.5:1 (WCAG AA) for all text against backgrounds.
- **Color Variables**:
  - Always consume centralized CSS variables (e.g., `var(--bg-primary)`, `var(--text-muted)`, `var(--accent-glow)`).
  - Never hardcode arbitrary hex colors directly inside component rules.

## 3. Micro-Interactions & Transitions
- Provide tactile feedback on all interactive controls (buttons, toggles, sliders, tabs):
  - Hover: subtle brightness elevation or outline shift (`150ms ease-out`).
  - Active: slight press-down scale (`transform: scale(0.98)`).
  - Focus: visible, high-contrast keyboard focus ring (`outline: 2px solid var(--accent)`).
- Animations must be purposeful, fast, and smooth (<250ms). Never cause layout thrashing or delayed input responses.

## 4. Responsive & Ergonomic Layouts
- **Flexbox & Grid First**: Build resilient layouts that flex without clipping text or controls on resize.
- **Edge Cushioning & Margins**:
  - Avoid cutoff at window or overlay edges. Apply generous edge paddings and soft gradient masks at scroll boundaries.
- **Scrollbar Styling**: Custom, slim, dark-mode scrollbars that stay unobtrusive until hovered.
