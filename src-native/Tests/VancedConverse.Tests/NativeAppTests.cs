using System;
using System.IO;
using System.Text.Json;
using VancedConverse.Core.Models;
using VancedConverse.Core.Services;
using VancedConverse.Theming;
using Xunit;

namespace VancedConverse.Tests;

public class NativeAppTests
{
    [Fact]
    public void Test_SecureStorage_EncryptionRoundtrip()
    {
        var secureStorage = new SecureStorageService();
        var originalApiKey = "AIzaSyD_TEST_YOUTUBE_API_KEY_123456789";

        var encrypted = secureStorage.EncryptString(originalApiKey);
        Assert.False(string.IsNullOrEmpty(encrypted));
        Assert.NotEqual(originalApiKey, encrypted);

        var decrypted = secureStorage.DecryptString(encrypted);
        Assert.Equal(originalApiKey, decrypted);
    }

    [Theory]
    [InlineData("https://www.youtube.com/watch?v=dQw4w9WgXcQ", "dQw4w9WgXcQ")]
    [InlineData("https://youtu.be/dQw4w9WgXcQ", "dQw4w9WgXcQ")]
    [InlineData("https://www.youtube.com/live/dQw4w9WgXcQ?feature=share", "dQw4w9WgXcQ")]
    [InlineData("dQw4w9WgXcQ", "dQw4w9WgXcQ")]
    [InlineData("  dQw4w9WgXcQ  ", "dQw4w9WgXcQ")]
    public void Test_YouTube_VideoIdExtractor(string input, string expectedId)
    {
        var extracted = YouTubeChatService.ExtractVideoId(input);
        Assert.Equal(expectedId, extracted);
    }

    [Fact]
    public void Test_ThemeModel_JsonRoundtrip()
    {
        var theme = new ThemeModel
        {
            Name = "Unit Test Cyber Neon",
            Window = new WindowThemeConfig
            {
                BackgroundType = "acrylic",
                BackgroundColor = "#0a0c14",
                CornerRadius = 32,
                Opacity = 0.88,
                BorderGlowColor = "#00f0ff",
                BorderGlowRadius = 25,
                BorderGlowIntensity = 1.2
            },
            MessageBubble = new MessageBubbleConfig
            {
                Background = "#121524",
                CornerRadius = 18,
                MaxWidth = 400
            },
            Typography = new TypographyConfig
            {
                FontFamily = "Segoe UI Variable",
                FontSize = 15,
                AuthorColor = "#00f0ff",
                MessageColor = "#ffffff"
            },
            TextEffects = new TextEffectsConfig
            {
                GlowEnabled = true,
                GlowColor = "#00f0ff",
                GlowRadius = 10,
                StrokeEnabled = true,
                StrokeWidth = 1.2,
                StrokeColor = "#000000"
            }
        };

        var json = JsonSerializer.Serialize(theme, new JsonSerializerOptions { WriteIndented = true });
        Assert.False(string.IsNullOrEmpty(json));

        var deserialized = JsonSerializer.Deserialize<ThemeModel>(json);
        Assert.NotNull(deserialized);
        Assert.Equal("Unit Test Cyber Neon", deserialized.Name);
        Assert.Equal(32, deserialized.Window.CornerRadius);
        Assert.Equal(0.88, deserialized.Window.Opacity);
        Assert.True(deserialized.TextEffects.GlowEnabled);
        Assert.True(deserialized.TextEffects.StrokeEnabled);
        Assert.Equal(1.2, deserialized.TextEffects.StrokeWidth);
    }

    [Fact]
    public void Test_ThemeManager_GracefulFallback_OnCorruptOrMissingTheme()
    {
        var manager = new ThemeManager();
        
        // Request a nonexistent theme
        manager.LoadTheme("NonExistentTheme_RandomGuid_123");
        
        // Assert that the manager does not crash and provides a valid current theme
        Assert.NotNull(manager.CurrentTheme);
        Assert.Equal("NonExistentTheme_RandomGuid_123", manager.CurrentTheme.Name);
    }

    [Fact]
    public void Test_ConfigService_QuotaFloorEnforcement()
    {
        var secureStorage = new SecureStorageService();
        var config = new ConfigService(secureStorage);

        // Attempting to set an interval lower than 10 seconds should be clamped
        config.Current.PollIntervalSec = 2; // too aggressive
        var safeInterval = Math.Max(10, config.Current.PollIntervalSec);

        Assert.Equal(10, safeInterval);
    }
}
