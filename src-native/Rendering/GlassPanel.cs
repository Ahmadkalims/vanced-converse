using System;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;

namespace VancedConverse.Rendering;

public class GlassPanel : Border
{
    public static readonly StyledProperty<Color> GlowColorProperty =
        AvaloniaProperty.Register<GlassPanel, Color>(nameof(GlowColor), Color.FromRgb(255, 75, 140));

    public static readonly StyledProperty<double> GlowRadiusProperty =
        AvaloniaProperty.Register<GlassPanel, double>(nameof(GlowRadius), 20.0);

    public static readonly StyledProperty<double> GlowIntensityProperty =
        AvaloniaProperty.Register<GlassPanel, double>(nameof(GlowIntensity), 1.0);

    public Color GlowColor { get => GetValue(GlowColorProperty); set => SetValue(GlowColorProperty, value); }
    public double GlowRadius { get => GetValue(GlowRadiusProperty); set => SetValue(GlowRadiusProperty, value); }
    public double GlowIntensity { get => GetValue(GlowIntensityProperty); set => SetValue(GlowIntensityProperty, value); }

    static GlassPanel()
    {
        GlowColorProperty.Changed.AddClassHandler<GlassPanel>((p, e) => p.UpdateGlow());
        GlowRadiusProperty.Changed.AddClassHandler<GlassPanel>((p, e) => p.UpdateGlow());
        GlowIntensityProperty.Changed.AddClassHandler<GlassPanel>((p, e) => p.UpdateGlow());
    }

    public GlassPanel()
    {
        UpdateGlow();
    }

    private void UpdateGlow()
    {
        if (GlowColor.A > 0 && GlowRadius > 0 && GlowIntensity > 0)
        {
            var alpha = (byte)Math.Clamp(GlowColor.A * 0.45 * GlowIntensity, 0, 255);
            var shadowColor = Color.FromArgb(alpha, GlowColor.R, GlowColor.G, GlowColor.B);
            BoxShadow = new BoxShadows(
                new BoxShadow
                {
                    Color = shadowColor,
                    Blur = GlowRadius,
                    Spread = 1.0,
                    OffsetX = 0,
                    OffsetY = 3
                }
            );
        }
        else
        {
            BoxShadow = default;
        }
    }
}
