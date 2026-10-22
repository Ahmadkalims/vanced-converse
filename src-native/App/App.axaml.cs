using System;
using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using VancedConverse.Core.Services;
using VancedConverse.Core.ViewModels;
using VancedConverse.Theming;
using VancedConverse.Views;

namespace VancedConverse;

public partial class App : Application
{
    public override void Initialize()
    {
        AvaloniaXamlLoader.Load(this);
    }

    public override void OnFrameworkInitializationCompleted()
    {
        if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
        {
            // Composition Root
            var secureStorage = new SecureStorageService();
            var configService = new ConfigService(secureStorage);
            var themeManager = new ThemeManager();
            themeManager.LoadTheme(configService.Current.ActiveThemeName);
            var chatService = new YouTubeChatService();

            var overlayVm = new OverlayViewModel(themeManager, chatService, configService);
            var settingsVm = new SettingsViewModel(themeManager, configService, chatService);

            // Populate sample messages for immediate visual verification on launch
            overlayVm.AddSampleMessages();

            var overlayWin = new OverlayWindow { DataContext = overlayVm };
            var settingsWin = new SettingsWindow { DataContext = settingsVm };

            desktop.MainWindow = overlayWin;
            overlayWin.Show();
            settingsWin.Show();
        }

        base.OnFrameworkInitializationCompleted();
    }
}
