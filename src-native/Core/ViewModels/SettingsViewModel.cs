using System;
using System.Collections.ObjectModel;
using System.IO;
using Avalonia.Media;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using VancedConverse.Core.Models;
using VancedConverse.Core.Services;
using VancedConverse.Theming;

namespace VancedConverse.Core.ViewModels;

public partial class SettingsViewModel : ObservableObject
{
    private readonly ThemeManager _themeManager;
    private readonly ConfigService _configService;
    private readonly YouTubeChatService _chatService;

    [ObservableProperty]
    private string _apiKey = string.Empty;

    [ObservableProperty]
    private string _videoIdOrUrl = string.Empty;

    [ObservableProperty]
    private int _pollIntervalSec = 10;

    [ObservableProperty]
    private bool _clickThrough = false;

    [ObservableProperty]
    private bool _alwaysOnTop = true;

    [ObservableProperty]
    private int _maxMessages = 100;

    // Theme Appearance Controls
    [ObservableProperty]
    private string _selectedThemeName = "Midnight Pink";

    [ObservableProperty]
    private ObservableCollection<string> _availableThemes = new();

    [ObservableProperty]
    private double _windowOpacity = 0.85;

    [ObservableProperty]
    private double _windowCornerRadius = 24;

    [ObservableProperty]
    private double _windowBlurRadius = 25;

    [ObservableProperty]
    private double _borderWidth = 1.5;

    [ObservableProperty]
    private string _borderColorHex = "#ff4b8c";

    [ObservableProperty]
    private string _borderGlowColorHex = "#ff4b8c";

    [ObservableProperty]
    private double _borderGlowRadius = 22;

    [ObservableProperty]
    private double _borderGlowIntensity = 1.0;

    [ObservableProperty]
    private double _bubbleCornerRadius = 16;

    [ObservableProperty]
    private double _fontSize = 14;

    [ObservableProperty]
    private bool _glowEnabled = true;

    [ObservableProperty]
    private string _glowColorHex = "#ff4b8c";

    [ObservableProperty]
    private double _glowRadius = 8;

    [ObservableProperty]
    private bool _strokeEnabled = false;

    [ObservableProperty]
    private string _strokeColorHex = "#000000";

    [ObservableProperty]
    private double _strokeWidth = 1.0;

    [ObservableProperty]
    private string _statusFeedback = "Ready";

    public SettingsViewModel(ThemeManager themeManager, ConfigService configService, YouTubeChatService chatService)
    {
        _themeManager = themeManager;
        _configService = configService;
        _chatService = chatService;

        LoadFromServices();
    }

    private void LoadFromServices()
    {
        ApiKey = _configService.PlainApiKey;
        VideoIdOrUrl = _configService.Current.VideoId;
        PollIntervalSec = Math.Max(10, _configService.Current.PollIntervalSec);
        ClickThrough = _configService.Current.ClickThrough;
        AlwaysOnTop = _configService.Current.AlwaysOnTop;
        MaxMessages = _configService.Current.MaxMessages;

        AvailableThemes.Clear();
        foreach (var t in _themeManager.AvailableThemes)
            AvailableThemes.Add(t);

        SelectedThemeName = _configService.Current.ActiveThemeName;
        LoadThemeProperties(_themeManager.CurrentTheme);
    }

    private void LoadThemeProperties(ThemeModel t)
    {
        WindowOpacity = t.Window.Opacity;
        WindowCornerRadius = t.Window.CornerRadius;
        WindowBlurRadius = t.Window.BlurRadius;
        BorderWidth = t.Window.BorderWidth;
        BorderColorHex = t.Window.BorderColor;
        BorderGlowColorHex = t.Window.BorderGlowColor;
        BorderGlowRadius = t.Window.BorderGlowRadius;
        BorderGlowIntensity = t.Window.BorderGlowIntensity;

        BubbleCornerRadius = t.MessageBubble.CornerRadius;
        FontSize = t.Typography.FontSize;

        GlowEnabled = t.TextEffects.GlowEnabled;
        GlowColorHex = t.TextEffects.GlowColor;
        GlowRadius = t.TextEffects.GlowRadius;

        StrokeEnabled = t.TextEffects.StrokeEnabled;
        StrokeColorHex = t.TextEffects.StrokeColor;
        StrokeWidth = t.TextEffects.StrokeWidth;
    }

    partial void OnSelectedThemeNameChanged(string value)
    {
        if (!string.IsNullOrEmpty(value))
        {
            _themeManager.LoadTheme(value);
            LoadThemeProperties(_themeManager.CurrentTheme);
            StatusFeedback = $"Switched to {value}";
        }
    }

    // Live preview updates when properties mutate
    partial void OnWindowCornerRadiusChanged(double value) => UpdateLiveTheme();
    partial void OnWindowOpacityChanged(double value) => UpdateLiveTheme();
    partial void OnBorderGlowRadiusChanged(double value) => UpdateLiveTheme();
    partial void OnBorderGlowIntensityChanged(double value) => UpdateLiveTheme();
    partial void OnBubbleCornerRadiusChanged(double value) => UpdateLiveTheme();
    partial void OnFontSizeChanged(double value) => UpdateLiveTheme();
    partial void OnGlowEnabledChanged(bool value) => UpdateLiveTheme();
    partial void OnStrokeEnabledChanged(bool value) => UpdateLiveTheme();

    private void UpdateLiveTheme()
    {
        var t = _themeManager.CurrentTheme;
        t.Window.CornerRadius = WindowCornerRadius;
        t.Window.Opacity = WindowOpacity;
        t.Window.BorderGlowRadius = BorderGlowRadius;
        t.Window.BorderGlowIntensity = BorderGlowIntensity;
        t.MessageBubble.CornerRadius = BubbleCornerRadius;
        t.Typography.FontSize = FontSize;
        t.TextEffects.GlowEnabled = GlowEnabled;
        t.TextEffects.StrokeEnabled = StrokeEnabled;

        // Save back to manager for real-time live preview
        _themeManager.SaveCurrentTheme(SelectedThemeName);
    }

    [RelayCommand]
    public void SaveSettings()
    {
        _configService.PlainApiKey = ApiKey;
        _configService.Current.VideoId = YouTubeChatService.ExtractVideoId(VideoIdOrUrl);
        _configService.Current.PollIntervalSec = Math.Max(10, PollIntervalSec);
        _configService.Current.ClickThrough = ClickThrough;
        _configService.Current.AlwaysOnTop = AlwaysOnTop;
        _configService.Current.MaxMessages = MaxMessages;
        _configService.Current.ActiveThemeName = SelectedThemeName;
        _configService.Save();

        _themeManager.SaveCurrentTheme(SelectedThemeName);
        StatusFeedback = "Settings & Theme saved successfully! (Two-Way Sync active)";
    }

    [RelayCommand]
    public async System.Threading.Tasks.Task StartStreamAsync()
    {
        SaveSettings();
        var videoId = YouTubeChatService.ExtractVideoId(VideoIdOrUrl);
        await _chatService.StartAsync(ApiKey, videoId, PollIntervalSec);
        StatusFeedback = $"Connecting to video: {videoId}...";
    }

    [RelayCommand]
    public void StopStream()
    {
        _chatService.Stop();
        StatusFeedback = "Stream polling stopped.";
    }
}
