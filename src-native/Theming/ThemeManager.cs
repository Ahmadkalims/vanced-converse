using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using System.Timers;
using VancedConverse.Core.Models;

namespace VancedConverse.Theming;

public class ThemeManager
{
    private readonly string _themesDirectory;
    private FileSystemWatcher? _watcher;
    private readonly System.Timers.Timer _debounceTimer;
    private string _activeThemePath = string.Empty;

    public ThemeModel CurrentTheme { get; private set; } = new();
    public List<string> AvailableThemes { get; } = new();

    public event Action<ThemeModel>? ThemeChanged;
    public event Action<string>? ThemeErrorOccurred;

    public ThemeManager()
    {
        var appDir = AppDomain.CurrentDomain.BaseDirectory;
        _themesDirectory = Path.Combine(appDir, "Themes");
        if (!Directory.Exists(_themesDirectory))
        {
            // Fallback to project or AppData directory
            var appData = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
            _themesDirectory = Path.Combine(appData, "VancedConverse", "Themes");
            Directory.CreateDirectory(_themesDirectory);
        }

        _debounceTimer = new System.Timers.Timer(300) { AutoReset = false };
        _debounceTimer.Elapsed += (s, e) => ReloadActiveTheme();

        DiscoverThemes();
        SetupWatcher();
    }

    public void DiscoverThemes()
    {
        AvailableThemes.Clear();
        if (Directory.Exists(_themesDirectory))
        {
            var files = Directory.GetFiles(_themesDirectory, "*.json");
            foreach (var f in files)
            {
                var name = Path.GetFileNameWithoutExtension(f);
                if (!name.EndsWith(".schema"))
                {
                    AvailableThemes.Add(name);
                }
            }
        }
    }

    public void LoadTheme(string themeName)
    {
        var path = Path.Combine(_themesDirectory, $"{themeName}.json");
        if (!File.Exists(path))
        {
            path = Path.Combine(_themesDirectory, $"{themeName.ToLower().Replace(" ", "-")}.json");
        }

        if (File.Exists(path))
        {
            _activeThemePath = path;
            ApplyThemeFromFile(path);
        }
        else
        {
            // Safe fallback: built-in Midnight Pink
            CurrentTheme = new ThemeModel { Name = themeName };
            ThemeChanged?.Invoke(CurrentTheme);
        }
    }

    public void SaveCurrentTheme(string themeName)
    {
        try
        {
            var fileName = $"{themeName.ToLower().Replace(" ", "-")}.json";
            var path = Path.Combine(_themesDirectory, fileName);
            var json = JsonSerializer.Serialize(CurrentTheme, new JsonSerializerOptions { WriteIndented = true });
            
            // Temporarily disable watcher during self-save to prevent reload bounce
            if (_watcher != null) _watcher.EnableRaisingEvents = false;
            File.WriteAllText(path, json);
            if (_watcher != null) _watcher.EnableRaisingEvents = true;

            _activeThemePath = path;
            ThemeChanged?.Invoke(CurrentTheme);
        }
        catch (Exception ex)
        {
            ThemeErrorOccurred?.Invoke($"Failed to save theme: {ex.Message}");
        }
    }

    private void SetupWatcher()
    {
        try
        {
            if (Directory.Exists(_themesDirectory))
            {
                _watcher = new FileSystemWatcher(_themesDirectory, "*.json")
                {
                    NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.FileName | NotifyFilters.Size,
                    EnableRaisingEvents = true
                };

                _watcher.Changed += (s, e) => _debounceTimer.Start();
                _watcher.Created += (s, e) => { DiscoverThemes(); _debounceTimer.Start(); };
            }
        }
        catch
        {
            // Non-critical if watcher fails
        }
    }

    private void ReloadActiveTheme()
    {
        if (!string.IsNullOrEmpty(_activeThemePath) && File.Exists(_activeThemePath))
        {
            ApplyThemeFromFile(_activeThemePath);
        }
    }

    private void ApplyThemeFromFile(string path)
    {
        try
        {
            var json = File.ReadAllText(path);
            var parsed = JsonSerializer.Deserialize<ThemeModel>(json);
            if (parsed != null)
            {
                CurrentTheme = parsed;
                ThemeChanged?.Invoke(CurrentTheme);
            }
        }
        catch (Exception ex)
        {
            // CRITICAL: Never crash on bad theme! Keep current theme active.
            ThemeErrorOccurred?.Invoke($"Theme parsing error in {Path.GetFileName(path)}: {ex.Message}. Keeping last good theme active.");
        }
    }
}
