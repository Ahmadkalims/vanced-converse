# System Architecture & Technical Patterns: Native Windows App

## 1. High-Level Technology Stack
- **Framework / Runtime**: .NET 8 LTS (`net8.0-windows`), Avalonia UI 11.2.1 (Skia 2.88.8 rendering engine).
- **Pattern**: MVVM with `CommunityToolkit.Mvvm` 8.4.2.
- **Window Management**: Win32 Native Interop (`SetWindowLongPtr`, `SetWindowPos`, `WS_EX_TRANSPARENT`, `WS_EX_TOOLWINDOW`, `WS_EX_NOACTIVATE`).
- **Rendering Pipeline**: Skia direct rasterization (`SkiaSharp`, `DrawingContext`, `FormattedText.BuildGeometry()`, `BoxShadows`).
- **Security / Storage**: Windows DPAPI (`System.Security.Cryptography.ProtectedData`) with CurrentUser scope.
- **Theming**: Trim-safe JSON Schema (`theme.schema.json`) with debounced `FileSystemWatcher` hot-reload.

## 2. Layered Architecture (`src-native/`)
```
src-native/
├── App/                → Entry point (Program.cs), App.axaml, DI composition root
├── Core/
│   ├── Models/         → ChatMessage, ThemeModel, AppConfig
│   ├── Services/       → YouTubeChatService, ConfigService, SecureStorageService
│   └── ViewModels/     → OverlayViewModel, SettingsViewModel
├── Theming/            → ThemeManager, hot-reloader, fallback controller
├── Rendering/          → GlowTextBlock (stacked outlines/glows), GlassPanel (acrylic/mica blur)
├── Platform.Windows/   → Win32Interop (click-through, toolwindow, always-on-top)
├── Themes/             → theme.schema.json, midnight-pink.json, minimal-glass.json, neon-glow.json
├── Views/              → OverlayWindow.axaml, SettingsWindow.axaml
└── Tests/              → xUnit unit tests (VancedConverse.Tests)
```

## 3. Why Avalonia + Skia Was Chosen
Detailed in `.context/stack-decision.md`:
1. **Per-Pixel Alpha Transparency**: Skia provides true alpha blending with zero black/white fringe on any background (unlike WinUI 3 which produces solid black frames on Windows 10 Build 19045).
2. **Deep Text & Effect Customization**: Skia canvas enables stacked text outlines, multi-layer glow diffusions, and linear/radial gradient fills.
3. **True Portability**: Single-file standalone publish (`Vanced Converse.exe`) without MSIX, Windows App SDK runtime installations, or administrator privileges.
