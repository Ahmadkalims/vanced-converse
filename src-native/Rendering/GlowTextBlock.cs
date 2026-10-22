using System;
using System.Globalization;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Platform;
using Avalonia.Rendering.SceneGraph;
using Avalonia.Skia;
using SkiaSharp;

namespace VancedConverse.Rendering;

public class GlowTextBlock : Control
{
    public static readonly StyledProperty<string> TextProperty =
        AvaloniaProperty.Register<GlowTextBlock, string>(nameof(Text), string.Empty);

    public static readonly StyledProperty<string> FontFamilyNameProperty =
        AvaloniaProperty.Register<GlowTextBlock, string>(nameof(FontFamilyName), "Segoe UI Variable, Segoe UI, sans-serif");

    public static readonly StyledProperty<double> FontSizeProperty =
        AvaloniaProperty.Register<GlowTextBlock, double>(nameof(FontSize), 14.0);

    public static readonly StyledProperty<FontWeight> FontWeightProperty =
        AvaloniaProperty.Register<GlowTextBlock, FontWeight>(nameof(FontWeight), FontWeight.Normal);

    public static readonly StyledProperty<IBrush?> ForegroundBrushProperty =
        AvaloniaProperty.Register<GlowTextBlock, IBrush?>(nameof(ForegroundBrush), Brushes.White);

    public static readonly StyledProperty<bool> GlowEnabledProperty =
        AvaloniaProperty.Register<GlowTextBlock, bool>(nameof(GlowEnabled), false);

    public static readonly StyledProperty<Color> GlowColorProperty =
        AvaloniaProperty.Register<GlowTextBlock, Color>(nameof(GlowColor), Color.FromRgb(255, 75, 140));

    public static readonly StyledProperty<double> GlowRadiusProperty =
        AvaloniaProperty.Register<GlowTextBlock, double>(nameof(GlowRadius), 8.0);

    public static readonly StyledProperty<bool> StrokeEnabledProperty =
        AvaloniaProperty.Register<GlowTextBlock, bool>(nameof(StrokeEnabled), false);

    public static readonly StyledProperty<Color> StrokeColorProperty =
        AvaloniaProperty.Register<GlowTextBlock, Color>(nameof(StrokeColor), Colors.Black);

    public static readonly StyledProperty<double> StrokeWidthProperty =
        AvaloniaProperty.Register<GlowTextBlock, double>(nameof(StrokeWidth), 1.0);

    public static readonly StyledProperty<bool> ShadowEnabledProperty =
        AvaloniaProperty.Register<GlowTextBlock, bool>(nameof(ShadowEnabled), true);

    public static readonly StyledProperty<Color> ShadowColorProperty =
        AvaloniaProperty.Register<GlowTextBlock, Color>(nameof(ShadowColor), Color.FromArgb(160, 0, 0, 0));

    public static readonly StyledProperty<double> ShadowBlurProperty =
        AvaloniaProperty.Register<GlowTextBlock, double>(nameof(ShadowBlur), 3.0);

    public static readonly StyledProperty<Vector> ShadowOffsetProperty =
        AvaloniaProperty.Register<GlowTextBlock, Vector>(nameof(ShadowOffset), new Vector(0, 1.5));

    public static readonly StyledProperty<bool> TextWrappingProperty =
        AvaloniaProperty.Register<GlowTextBlock, bool>(nameof(TextWrapping), true);

    public string Text { get => GetValue(TextProperty); set => SetValue(TextProperty, value); }
    public string FontFamilyName { get => GetValue(FontFamilyNameProperty); set => SetValue(FontFamilyNameProperty, value); }
    public double FontSize { get => GetValue(FontSizeProperty); set => SetValue(FontSizeProperty, value); }
    public FontWeight FontWeight { get => GetValue(FontWeightProperty); set => SetValue(FontWeightProperty, value); }
    public IBrush? ForegroundBrush { get => GetValue(ForegroundBrushProperty); set => SetValue(ForegroundBrushProperty, value); }
    public bool GlowEnabled { get => GetValue(GlowEnabledProperty); set => SetValue(GlowEnabledProperty, value); }
    public Color GlowColor { get => GetValue(GlowColorProperty); set => SetValue(GlowColorProperty, value); }
    public double GlowRadius { get => GetValue(GlowRadiusProperty); set => SetValue(GlowRadiusProperty, value); }
    public bool StrokeEnabled { get => GetValue(StrokeEnabledProperty); set => SetValue(StrokeEnabledProperty, value); }
    public Color StrokeColor { get => GetValue(StrokeColorProperty); set => SetValue(StrokeColorProperty, value); }
    public double StrokeWidth { get => GetValue(StrokeWidthProperty); set => SetValue(StrokeWidthProperty, value); }
    public bool ShadowEnabled { get => GetValue(ShadowEnabledProperty); set => SetValue(ShadowEnabledProperty, value); }
    public Color ShadowColor { get => GetValue(ShadowColorProperty); set => SetValue(ShadowColorProperty, value); }
    public double ShadowBlur { get => GetValue(ShadowBlurProperty); set => SetValue(ShadowBlurProperty, value); }
    public Vector ShadowOffset { get => GetValue(ShadowOffsetProperty); set => SetValue(ShadowOffsetProperty, value); }
    public bool TextWrapping { get => GetValue(TextWrappingProperty); set => SetValue(TextWrappingProperty, value); }

    static GlowTextBlock()
    {
        AffectsRender<GlowTextBlock>(
            TextProperty, FontFamilyNameProperty, FontSizeProperty, FontWeightProperty,
            ForegroundBrushProperty, GlowEnabledProperty, GlowColorProperty, GlowRadiusProperty,
            StrokeEnabledProperty, StrokeColorProperty, StrokeWidthProperty, ShadowEnabledProperty,
            ShadowColorProperty, ShadowBlurProperty, ShadowOffsetProperty);
        AffectsMeasure<GlowTextBlock>(TextProperty, FontSizeProperty, FontWeightProperty, FontFamilyNameProperty);
    }

    private FormattedText CreateFormattedText(double constraintWidth)
    {
        var typeface = new Typeface(new FontFamily(FontFamilyName), FontStyle.Normal, FontWeight);
        var ft = new FormattedText(
            Text ?? string.Empty,
            CultureInfo.CurrentCulture,
            FlowDirection.LeftToRight,
            typeface,
            FontSize,
            ForegroundBrush ?? Brushes.White);

        if (TextWrapping && constraintWidth > 0 && !double.IsInfinity(constraintWidth))
        {
            ft.MaxTextWidth = constraintWidth;
        }

        return ft;
    }

    protected override Size MeasureOverride(Size availableSize)
    {
        if (string.IsNullOrEmpty(Text)) return new Size(0, 0);
        var ft = CreateFormattedText(availableSize.Width);
        var extra = (GlowEnabled ? GlowRadius : 0) + (StrokeEnabled ? StrokeWidth : 0);
        return new Size(ft.Width + extra, ft.Height + extra);
    }

    public override void Render(DrawingContext context)
    {
        if (string.IsNullOrEmpty(Text)) return;

        var ft = CreateFormattedText(Bounds.Width);

        // 1. Drop shadow layer
        if (ShadowEnabled && ShadowColor.A > 0)
        {
            var shadowBrush = new SolidColorBrush(ShadowColor);
            var shadowFt = new FormattedText(
                Text, CultureInfo.CurrentCulture, FlowDirection.LeftToRight,
                new Typeface(new FontFamily(FontFamilyName), FontStyle.Normal, FontWeight),
                FontSize, shadowBrush);
            if (TextWrapping && Bounds.Width > 0) shadowFt.MaxTextWidth = Bounds.Width;
            context.DrawText(shadowFt, new Point(ShadowOffset.X, ShadowOffset.Y));
        }

        // 2. Glow layer (multi-pass blur emulation)
        if (GlowEnabled && GlowColor.A > 0 && GlowRadius > 0)
        {
            var glowBrush = new SolidColorBrush(Color.FromArgb((byte)(GlowColor.A * 0.4), GlowColor.R, GlowColor.G, GlowColor.B));
            var offsets = new[]
            {
                new Point(-1, 0), new Point(1, 0), new Point(0, -1), new Point(0, 1),
                new Point(-0.7, -0.7), new Point(0.7, 0.7), new Point(-0.7, 0.7), new Point(0.7, -0.7)
            };
            foreach (var off in offsets)
            {
                var glowFt = new FormattedText(
                    Text, CultureInfo.CurrentCulture, FlowDirection.LeftToRight,
                    new Typeface(new FontFamily(FontFamilyName), FontStyle.Normal, FontWeight),
                    FontSize, glowBrush);
                if (TextWrapping && Bounds.Width > 0) glowFt.MaxTextWidth = Bounds.Width;
                context.DrawText(glowFt, new Point(off.X * (GlowRadius * 0.4), off.Y * (GlowRadius * 0.4)));
            }
        }

        // 3. Stroke / Outline layer
        if (StrokeEnabled && StrokeWidth > 0 && StrokeColor.A > 0)
        {
            var strokeBrush = new SolidColorBrush(StrokeColor);
            var strokeOffsets = new[]
            {
                new Point(-StrokeWidth, 0), new Point(StrokeWidth, 0),
                new Point(0, -StrokeWidth), new Point(0, StrokeWidth)
            };
            foreach (var off in strokeOffsets)
            {
                var strokeFt = new FormattedText(
                    Text, CultureInfo.CurrentCulture, FlowDirection.LeftToRight,
                    new Typeface(new FontFamily(FontFamilyName), FontStyle.Normal, FontWeight),
                    FontSize, strokeBrush);
                if (TextWrapping && Bounds.Width > 0) strokeFt.MaxTextWidth = Bounds.Width;
                context.DrawText(strokeFt, off);
            }
        }

        // 4. Primary foreground text
        context.DrawText(ft, new Point(0, 0));
    }
}
