using System;
using System.Collections.Generic;
using System.Text.Json.Serialization;

namespace VancedConverse.Core.Models;

public class ThemeModel
{
    [JsonPropertyName("schemaVersion")]
    public string SchemaVersion { get; set; } = "1.0.0";

    [JsonPropertyName("name")]
    public string Name { get; set; } = "Midnight Pink";

    [JsonPropertyName("author")]
    public string Author { get; set; } = "Vanced Converse";

    [JsonPropertyName("description")]
    public string Description { get; set; } = string.Empty;

    [JsonPropertyName("extends")]
    public string? Extends { get; set; }

    [JsonPropertyName("window")]
    public WindowThemeConfig Window { get; set; } = new();

    [JsonPropertyName("messageBubble")]
    public MessageBubbleConfig MessageBubble { get; set; } = new();

    [JsonPropertyName("typography")]
    public TypographyConfig Typography { get; set; } = new();

    [JsonPropertyName("textEffects")]
    public TextEffectsConfig TextEffects { get; set; } = new();
}

public class WindowThemeConfig
{
    [JsonPropertyName("backgroundType")]
    public string BackgroundType { get; set; } = "acrylic"; // solid, gradient, acrylic, blur, transparent

    [JsonPropertyName("backgroundColor")]
    public string BackgroundColor { get; set; } = "#121622";

    [JsonPropertyName("gradient")]
    public GradientConfig? Gradient { get; set; }

    [JsonPropertyName("opacity")]
    public double Opacity { get; set; } = 0.85;

    [JsonPropertyName("blurRadius")]
    public double BlurRadius { get; set; } = 25;

    [JsonPropertyName("acrylicTint")]
    public string AcrylicTint { get; set; } = "#0e111a";

    [JsonPropertyName("acrylicLuminosity")]
    public double AcrylicLuminosity { get; set; } = 0.65;

    [JsonPropertyName("cornerRadius")]
    public double CornerRadius { get; set; } = 24;

    [JsonPropertyName("borderWidth")]
    public double BorderWidth { get; set; } = 1.5;

    [JsonPropertyName("borderColor")]
    public string BorderColor { get; set; } = "#ff4b8c";

    [JsonPropertyName("borderGlowColor")]
    public string BorderGlowColor { get; set; } = "#ff4b8c";

    [JsonPropertyName("borderGlowRadius")]
    public double BorderGlowRadius { get; set; } = 20;

    [JsonPropertyName("borderGlowIntensity")]
    public double BorderGlowIntensity { get; set; } = 1.0;

    [JsonPropertyName("padding")]
    public double Padding { get; set; } = 16;

    [JsonPropertyName("edgeFadeMargin")]
    public double EdgeFadeMargin { get; set; } = 20;
}

public class GradientConfig
{
    [JsonPropertyName("type")]
    public string Type { get; set; } = "linear"; // linear or radial

    [JsonPropertyName("angle")]
    public double Angle { get; set; } = 135;

    [JsonPropertyName("stops")]
    public List<GradientStopConfig> Stops { get; set; } = new();
}

public class GradientStopConfig
{
    [JsonPropertyName("color")]
    public string Color { get; set; } = "#ff4b8c";

    [JsonPropertyName("offset")]
    public double Offset { get; set; } = 0.0;
}

public class MessageBubbleConfig
{
    [JsonPropertyName("background")]
    public string Background { get; set; } = "#181d2b";

    [JsonPropertyName("opacity")]
    public double Opacity { get; set; } = 0.92;

    [JsonPropertyName("cornerRadius")]
    public double CornerRadius { get; set; } = 16;

    [JsonPropertyName("padding")]
    public double Padding { get; set; } = 12;

    [JsonPropertyName("margin")]
    public double Margin { get; set; } = 6;

    [JsonPropertyName("spacing")]
    public double Spacing { get; set; } = 8;

    [JsonPropertyName("maxWidth")]
    public double MaxWidth { get; set; } = 420;

    [JsonPropertyName("borderWidth")]
    public double BorderWidth { get; set; } = 1.0;

    [JsonPropertyName("borderColor")]
    public string BorderColor { get; set; } = "#ff4b8c44";
}

public class TypographyConfig
{
    [JsonPropertyName("fontFamily")]
    public string FontFamily { get; set; } = "Segoe UI Variable, Segoe UI, sans-serif";

    [JsonPropertyName("fontSize")]
    public double FontSize { get; set; } = 14;

    [JsonPropertyName("fontWeight")]
    public string FontWeight { get; set; } = "Normal";

    [JsonPropertyName("lineHeight")]
    public double LineHeight { get; set; } = 1.4;

    [JsonPropertyName("authorColor")]
    public string AuthorColor { get; set; } = "#ff79b0";

    [JsonPropertyName("messageColor")]
    public string MessageColor { get; set; } = "#ffffff";

    [JsonPropertyName("modBadgeColor")]
    public string ModBadgeColor { get; set; } = "#00e5ff";

    [JsonPropertyName("superchatColor")]
    public string SuperchatColor { get; set; } = "#ffd700";
}

public class TextEffectsConfig
{
    [JsonPropertyName("strokeEnabled")]
    public bool StrokeEnabled { get; set; } = false;

    [JsonPropertyName("strokeWidth")]
    public double StrokeWidth { get; set; } = 1.0;

    [JsonPropertyName("strokeColor")]
    public string StrokeColor { get; set; } = "#000000";

    [JsonPropertyName("glowEnabled")]
    public bool GlowEnabled { get; set; } = true;

    [JsonPropertyName("glowColor")]
    public string GlowColor { get; set; } = "#ff4b8c";

    [JsonPropertyName("glowRadius")]
    public double GlowRadius { get; set; } = 8;

    [JsonPropertyName("glowIntensity")]
    public double GlowIntensity { get; set; } = 0.75;

    [JsonPropertyName("shadowEnabled")]
    public bool ShadowEnabled { get; set; } = true;

    [JsonPropertyName("shadowColor")]
    public string ShadowColor { get; set; } = "#000000aa";

    [JsonPropertyName("shadowBlur")]
    public double ShadowBlur { get; set; } = 4;

    [JsonPropertyName("shadowOffsetX")]
    public double ShadowOffsetX { get; set; } = 0;

    [JsonPropertyName("shadowOffsetY")]
    public double ShadowOffsetY { get; set; } = 2;

    [JsonPropertyName("gradientFillEnabled")]
    public bool GradientFillEnabled { get; set; } = false;

    [JsonPropertyName("gradientFill")]
    public GradientConfig? GradientFill { get; set; }
}
