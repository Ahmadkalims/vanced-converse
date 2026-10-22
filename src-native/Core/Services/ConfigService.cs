using System;
using System.IO;
using System.Text.Json;
using VancedConverse.Core.Models;

namespace VancedConverse.Core.Services;

public class ConfigService
{
    private readonly string _configFilePath;
    private readonly SecureStorageService _secureStorage;
    private AppConfig _currentConfig = new();

    public AppConfig Current => _currentConfig;
    public string PlainApiKey { get; set; } = string.Empty;

    public event Action<AppConfig>? ConfigChanged;

    public ConfigService(SecureStorageService secureStorage)
    {
        _secureStorage = secureStorage;
        var appData = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
        var dir = Path.Combine(appData, "VancedConverse");
        Directory.CreateDirectory(dir);
        _configFilePath = Path.Combine(dir, "config.json");
        Load();
    }

    public void Load()
    {
        try
        {
            if (File.Exists(_configFilePath))
            {
                var json = File.ReadAllText(_configFilePath);
                _currentConfig = JsonSerializer.Deserialize<AppConfig>(json) ?? new AppConfig();
                PlainApiKey = _secureStorage.DecryptString(_currentConfig.ApiKeyEncrypted);
            }
            else
            {
                _currentConfig = new AppConfig();
                Save();
            }
        }
        catch
        {
            _currentConfig = new AppConfig();
        }
    }

    public void Save()
    {
        try
        {
            _currentConfig.ApiKeyEncrypted = _secureStorage.EncryptString(PlainApiKey);
            var json = JsonSerializer.Serialize(_currentConfig, new JsonSerializerOptions { WriteIndented = true });
            File.WriteAllText(_configFilePath, json);
            ConfigChanged?.Invoke(_currentConfig);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[ConfigService] Save failed: {ex.Message}");
        }
    }
}
