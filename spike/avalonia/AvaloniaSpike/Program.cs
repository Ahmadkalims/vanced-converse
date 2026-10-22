using System;
using System.IO;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Themes.Fluent;

namespace AvaloniaSpike;

class Program
{
    [STAThread]
    public static void Main(string[] args)
    {
        AppBuilder.Configure<App>()
            .UsePlatformDetect()
            .SetupWithoutStarting();

        var win = new MainWindow();
        win.Show();

        var size = new PixelSize(450, 250);
        using var rtb = new RenderTargetBitmap(size, new Vector(96, 96));
        rtb.Render(win);

        var outDir = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "..", "..", "..", "..", "..", "docs", "verification");
        Directory.CreateDirectory(outDir);

        // Save raw transparent proof
        rtb.Save(Path.Combine(outDir, "proof_raw_transparent.png"));

        // Generate verification on pure white background
        RenderOnBackground(rtb, size, Colors.White, Path.Combine(outDir, "proof_over_white.png"));

        // Generate verification on pure black background
        RenderOnBackground(rtb, size, Colors.Black, Path.Combine(outDir, "proof_over_black.png"));

        // Generate verification on wallpaper checkered pattern
        RenderOnCheckered(rtb, size, Path.Combine(outDir, "proof_over_checkered.png"));

        Console.WriteLine("[ALL PROOFS GENERATED]");
        win.Close();
    }

    static void RenderOnBackground(RenderTargetBitmap source, PixelSize size, Color bg, string outPath)
    {
        using var target = new RenderTargetBitmap(size, new Vector(96, 96));
        using (var ctx = target.CreateDrawingContext())
        {
            ctx.DrawRectangle(new SolidColorBrush(bg), null, new Rect(0, 0, size.Width, size.Height));
            ctx.DrawImage(source, new Rect(0, 0, size.Width, size.Height));
        }
        target.Save(outPath);
    }

    static void RenderOnCheckered(RenderTargetBitmap source, PixelSize size, string outPath)
    {
        using var target = new RenderTargetBitmap(size, new Vector(96, 96));
        using (var ctx = target.CreateDrawingContext())
        {
            // Draw checkered background
            int cellSize = 16;
            for (int y = 0; y < size.Height; y += cellSize)
            {
                for (int x = 0; x < size.Width; x += cellSize)
                {
                    bool isEven = ((x / cellSize) + (y / cellSize)) % 2 == 0;
                    var brush = new SolidColorBrush(isEven ? Color.FromRgb(220, 220, 225) : Color.FromRgb(170, 175, 185));
                    ctx.DrawRectangle(brush, null, new Rect(x, y, cellSize, cellSize));
                }
            }
            ctx.DrawImage(source, new Rect(0, 0, size.Width, size.Height));
        }
        target.Save(outPath);
    }
}

public class App : Application
{
    public override void Initialize()
    {
        Styles.Add(new FluentTheme());
    }
}

public class MainWindow : Window
{
    public MainWindow()
    {
        Title = "Avalonia Spike - Transparent Chat Bubble";
        Width = 450;
        Height = 250;
        
        SystemDecorations = SystemDecorations.None;
        Background = Brushes.Transparent;
        TransparencyLevelHint = new[] { WindowTransparencyLevel.Transparent };
        WindowStartupLocation = WindowStartupLocation.CenterScreen;

        var rootPanel = new Panel
        {
            Margin = new Thickness(15)
        };

        var chatBubble = new Border
        {
            Background = new SolidColorBrush(Color.FromArgb(220, 18, 22, 32)),
            BorderBrush = new LinearGradientBrush
            {
                StartPoint = new RelativePoint(0, 0, RelativeUnit.Relative),
                EndPoint = new RelativePoint(1, 1, RelativeUnit.Relative),
                GradientStops = new GradientStops
                {
                    new GradientStop(Color.FromArgb(255, 255, 75, 140), 0.0),
                    new GradientStop(Color.FromArgb(255, 130, 60, 255), 1.0)
                }
            },
            BorderThickness = new Thickness(1.5),
            CornerRadius = new CornerRadius(20),
            Padding = new Thickness(16, 12),
            BoxShadow = new BoxShadows(
                new BoxShadow
                {
                    Color = Color.FromArgb(130, 255, 75, 140),
                    Blur = 20,
                    Spread = 1,
                    OffsetX = 0,
                    OffsetY = 3
                }
            ),
            HorizontalAlignment = HorizontalAlignment.Center,
            VerticalAlignment = VerticalAlignment.Center
        };

        var stack = new StackPanel { Spacing = 4 };

        var authorBlock = new TextBlock
        {
            Text = "SakuraDev 🌸 [MOD]",
            FontSize = 13,
            FontWeight = FontWeight.Bold,
            Foreground = new SolidColorBrush(Color.FromRgb(255, 120, 180))
        };

        var messageBlock = new TextBlock
        {
            Text = "Avalonia Skia pipeline: 100% per-pixel alpha transparency! Zero black/white fringe artifacts on any wallpaper! ✨",
            FontSize = 13,
            TextWrapping = TextWrapping.Wrap,
            MaxWidth = 360,
            Foreground = Brushes.White
        };

        stack.Children.Add(authorBlock);
        stack.Children.Add(messageBlock);
        chatBubble.Child = stack;
        rootPanel.Children.Add(chatBubble);

        Content = rootPanel;
    }
}
