# Modular Architecture & Code Quality Standards

## 1. Single Responsibility & Modular Design
- **One Reason to Change**: Every module, class, file, and function must have a single, well-defined responsibility.
- **Separation of Concerns**:
  - Keep OS/Win32 APIs strictly decoupled from business logic and state.
  - Keep WebView2 bridge / IPC communication in dedicated adapter modules.
  - Keep UI view rendering separated from data fetching and state synchronization.
- **No God Objects or Monoliths**: Split large files exceeding limits into logical sub-modules with cohesive interfaces.

## 2. Hard Size & Complexity Limits
- **Max 35 Lines per Function**: Break complex logic into clear, composable helper functions with descriptive verb-noun names.
- **Max 350 Lines per Source File**: When a file crosses 350 lines, evaluate splitting into sub-components or utility modules.
- **Max 3 Levels of Nesting**: Avoid deep `if/else` or loop nesting. Use early returns (guard clauses) to reduce cognitive load.
- **Parameter Limit**: Maximum 4 arguments per function. Use structured configuration structs/objects for larger parameter sets.

## 3. Strict Type Safety & Clean Contracts
- **C++ (C++20)**:
  - Use strongly typed enums (`enum class`), explicit type conversions, and `const` correctness everywhere.
  - Prefer `std::string_view`, `std::span`, and smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw pointers.
  - Never use magic constants; extract them into named `constexpr` constants.
- **TypeScript / JavaScript**:
  - Enforce strict typing where TS is used; in JS/Web code, use explicit JSDoc typedefs or strict validation on IPC boundaries.
  - No untyped `any` or ambiguous data transfers across the WebView2 bridge.
  - Parse and validate all JSON payloads entering or leaving native boundaries.

## 4. Domain Naming & Organization
- Use domain-driven, self-documenting identifiers:
  - Files: `kebab-case.ext` for web assets, `snake_case.cpp` / `PascalCase.h` matching repository conventions.
  - Functions: Clear action verbs (e.g., `FetchLiveChatMessages`, `ApplyOverlayTheme`, `SyncSettingsToBridge`).
  - Variables: Descriptive nouns; avoid single-letter variables except short loop indices.
- Co-locate related styles, templates, tests, and headers next to their domain implementations.
