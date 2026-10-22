using System;
using System.Collections.ObjectModel;
using System.Linq;
using Avalonia.Media;
using Avalonia.Threading;
using CommunityToolkit.Mvvm.ComponentModel;
using VancedConverse.Core.Models;
using VancedConverse.Core.Services;
using VancedConverse.Theming;

namespace VancedConverse.Core.ViewModels;

public partial class OverlayViewModel : ObservableObject
{
    private readonly ThemeManager _themeManager;
    private readonly YouTubeChatService _chatService;
    private readonly ConfigService _configService;

    [ObservableProperty]
    private ObservableCollection<ChatMessage> _messages = new();

    [ObservableProperty]
    private string _streamStatus = "Ready";

    [ObservableProperty]
    private IBrush _windowBackground = new SolidColorBrush(Color.FromArgb(215, 18, 22, 34));

    [ObservableProperty]
    private IBrush _windowBorderBrush = new SolidColorBrush(Color.FromRgb(255, 75, 140));

    [ObservableProperty]
    private double _windowBorderThickness = 1.5;

    [ObservableProperty]
    private Avalonia.CornerRadius _windowCornerRadius = new(24);

    [ObservableProperty]
    private Color _borderGlowColor = Color.FromRgb(255, 75, 140);

    [ObservableProperty]
    private double _borderGlowRadius = 22;

    [ObservableProperty]
    private double _borderGlowIntensity = 1.0;

    [ObservableProperty]
    private IBrush _bubbleBackground = new SolidColorBrush(Color.FromArgb(235, 24, 29, 43));

    [ObservableProperty]
    private Avalonia.CornerRadius _bubbleCornerRadius = new(16);

    [ObservableProperty]
    private string _fontFamily = "Segoe UI Variable, Segoe UI, sans-serif";

    [ObservableProperty]
    private double _fontSize = 14;

    [ObservableProperty]
    private IBrush _authorBrush = new SolidColorBrush(Color.FromRgb(255, 121, 176));

    [ObservableProperty]
    private IBrush _messageBrush = Brushes.White;

    [ObservableProperty]
    private bool _glowEnabled = true;

    [ObservableProperty]
    private Color _textGlowColor = Color.FromRgb(255, 75, 140);

    [ObservableProperty]
    private double _textGlowRadius = 8;

    [ObservableProperty]
    private bool _strokeEnabled = false;

    [ObservableProperty]
    private Color _strokeColor = Colors.Black;

    [ObservableProperty]
    private double _strokeWidth = 1.0;

    public OverlayViewModel(ThemeManager themeManager, YouTubeChatService chatService, ConfigService configService)
    {
        _themeManager = themeManager;
        _chatService = chatService;
        _configService = configService;

        _themeManager.ThemeChanged += OnThemeChanged;
        _chatService.MessagesReceived += OnMessagesReceived;
        _chatService.StatusChanged += status => Dispatcher.UIThread.Post(() => StreamStatus = status);

        ApplyTheme(_themeManager.CurrentTheme);
    }

    private void OnThemeChanged(ThemeModel theme)
    {
        Dispatcher.UIThread.Post(() => ApplyTheme(theme));
    }

    private void ApplyTheme(ThemeModel theme)
    {
        var win = theme.Window;
        var bubble = theme.MessageBubble;
        var typo = theme.Typography;
        var fx = theme.TextEffects;

        if (Color.TryParse(win.BackgroundColor, out var winBgColor))
        {
            var alpha = (byte)(winBgColor.A * win.Opacity);
            WindowBackground = new SolidColorBrush(Color.FromArgb(alpha, winBgColor.R, winBgColor.G, winBgColor.B));
        }

        if (Color.TryParse(win.BorderColor, out var borderColor))
            WindowBorderBrush = new SolidColorBrush(borderColor);

        WindowBorderThickness = win.BorderWidth;
        WindowCornerRadius = new Avalonia.CornerRadius(win.CornerRadius);

        if (Color.TryParse(win.BorderGlowColor, out var glowCol))
            BorderGlowColor = glowCol;

        BorderGlowRadius = win.BorderGlowRadius;
        BorderGlowIntensity = win.BorderGlowIntensity;

        if (Color.TryParse(bubble.Background, out var bBgColor))
        {
            var bAlpha = (byte)(bBgColor.A * bubble.Opacity);
            BubbleBackground = new SolidColorBrush(Color.FromArgb(bAlpha, bBgColor.R, bBgColor.G, bBgColor.B));
        }

        BubbleCornerRadius = new Avalonia.CornerRadius(bubble.CornerRadius);
        FontFamily = typo.FontFamily;
        FontSize = typo.FontSize;

        if (Color.TryParse(typo.AuthorColor, out var aColor))
            AuthorBrush = new SolidColorBrush(aColor);

        if (Color.TryParse(typo.MessageColor, out var mColor))
            MessageBrush = new SolidColorBrush(mColor);

        GlowEnabled = fx.GlowEnabled;
        if (Color.TryParse(fx.GlowColor, out var tGlowColor))
            TextGlowColor = tGlowColor;
        TextGlowRadius = fx.GlowRadius;

        StrokeEnabled = fx.StrokeEnabled;
        if (Color.TryParse(fx.StrokeColor, out var sColor))
            StrokeColor = sColor;
        StrokeWidth = fx.StrokeWidth;
    }

    private void OnMessagesReceived(System.Collections.Generic.List<ChatMessage> incoming)
    {
        Dispatcher.UIThread.Post(() =>
        {
            var max = Math.Max(20, _configService.Current.MaxMessages);
            foreach (var m in incoming)
            {
                Messages.Add(m);
            }

            // Prune older messages to guarantee low memory usage (DOM recycling)
            while (Messages.Count > max)
            {
                Messages.RemoveAt(0);
            }
        });
    }

    public void AddSampleMessages()
    {
        var samples = new[]
        {
            new ChatMessage { AuthorName = "SakuraDev 🌸", MessageText = "Welcome to the 100% native stream overlay! ✨", IsModerator = true },
            new ChatMessage { AuthorName = "AlexGamer", MessageText = "Look at those smooth rounded corners and zero black fringe! 🔥" },
            new ChatMessage { AuthorName = "طارق", MessageText = "مرحبا بالعالم! التصميم رائع جداً وسريع جداً 🚀" },
            new ChatMessage { AuthorName = "Rahul_Dev", MessageText = "Awesome! Full Unicode, multi-glow & instant startup! 🎮" },
            new ChatMessage { AuthorName = "GoldenViewer 💎", MessageText = "Superchat $50: Best desktop chat app ever!", IsSuperchat = true, SuperchatAmount = "$50.00" }
        };

        foreach (var s in samples)
        {
            Messages.Add(s);
        }
    }
}
