using System.Text.Json.Serialization;

namespace VancedConverse.Core.Models;

public class AppConfig
{
    [JsonPropertyName("apiKeyEncrypted")]
    public string ApiKeyEncrypted { get; set; } = string.Empty;

    [JsonPropertyName("videoId")]
    public string VideoId { get; set; } = string.Empty;

    [JsonPropertyName("pollIntervalSec")]
    public int PollIntervalSec { get; set; } = 10;

    [JsonPropertyName("activeThemeName")]
    public string ActiveThemeName { get; set; } = "Midnight Pink";

    [JsonPropertyName("clickThrough")]
    public bool ClickThrough { get; set; } = false;

    [JsonPropertyName("alwaysOnTop")]
    public bool AlwaysOnTop { get; set; } = true;

    [JsonPropertyName("maxMessages")]
    public int MaxMessages { get; set; } = 100;

    [JsonPropertyName("windowX")]
    public int WindowX { get; set; } = -1;

    [JsonPropertyName("windowY")]
    public int WindowY { get; set; } = -1;

    [JsonPropertyName("windowWidth")]
    public int WindowWidth { get; set; } = 380;

    [JsonPropertyName("windowHeight")]
    public int WindowHeight { get; set; } = 650;
}
