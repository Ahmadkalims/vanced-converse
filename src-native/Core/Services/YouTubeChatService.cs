using System;
using System.Collections.Generic;
using System.Net.Http;
using System.Text.Json;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using VancedConverse.Core.Models;

namespace VancedConverse.Core.Services;

public class YouTubeChatService
{
    private static readonly HttpClient Http = new();
    private CancellationTokenSource? _pollCts;
    private string? _nextPageToken;
    private string? _liveChatId;

    public bool IsPolling { get; private set; }
    public string StatusMessage { get; private set; } = "Idle";

    public event Action<List<ChatMessage>>? MessagesReceived;
    public event Action<string>? StatusChanged;
    public event Action<string>? QuotaExhausted;

    public static string ExtractVideoId(string input)
    {
        if (string.IsNullOrWhiteSpace(input)) return string.Empty;
        input = input.Trim();

        // 11-character pure alphanumeric ID
        if (Regex.IsMatch(input, @"^[a-zA-Z0-9_-]{11}$"))
            return input;

        // Standard watch?v= format
        var match = Regex.Match(input, @"(?:v=|\/embed\/|\/live\/|youtu\.be\/|\/v\/)([a-zA-Z0-9_-]{11})");
        return match.Success ? match.Groups[1].Value : input;
    }

    public async Task StartAsync(string apiKey, string videoId, int pollIntervalSec)
    {
        await Task.Yield();
        Stop();
        if (string.IsNullOrWhiteSpace(apiKey) || string.IsNullOrWhiteSpace(videoId))
        {
            UpdateStatus("Error: Missing API key or Video ID");
            return;
        }

        _pollCts = new CancellationTokenSource();
        var ct = _pollCts.Token;
        IsPolling = true;

        _ = Task.Run(async () =>
        {
            try
            {
                UpdateStatus("Connecting to YouTube Live...");
                _liveChatId = await FetchLiveChatIdAsync(apiKey, videoId, ct);

                if (string.IsNullOrEmpty(_liveChatId))
                {
                    UpdateStatus("Error: No active live chat found for stream");
                    IsPolling = false;
                    return;
                }

                UpdateStatus($"Connected! Polling every {Math.Max(10, pollIntervalSec)}s...");
                int backoffAttempt = 0;

                while (!ct.IsCancellationRequested)
                {
                    var interval = Math.Max(10, pollIntervalSec);
                    try
                    {
                        var (messages, nextToken, delaySec) = await FetchChatMessagesAsync(apiKey, _liveChatId, _nextPageToken, ct);
                        _nextPageToken = nextToken;
                        backoffAttempt = 0; // Reset backoff on success

                        if (messages.Count > 0)
                        {
                            MessagesReceived?.Invoke(messages);
                        }

                        if (delaySec > interval) interval = delaySec;
                    }
                    catch (HttpRequestException ex) when (ex.StatusCode == System.Net.HttpStatusCode.Forbidden)
                    {
                        // Circuit breaker on Quota Exceeded (HTTP 403)
                        UpdateStatus("Quota Exhausted (HTTP 403). Circuit breaker tripped.");
                        QuotaExhausted?.Invoke("Daily YouTube API quota has been exhausted. Polling stopped.");
                        IsPolling = false;
                        return;
                    }
                    catch (Exception ex)
                    {
                        backoffAttempt++;
                        var backoffSec = Math.Min(60, interval * (int)Math.Pow(1.5, backoffAttempt));
                        UpdateStatus($"Connection error: {ex.Message}. Retrying in {backoffSec}s...");
                        await Task.Delay(TimeSpan.FromSeconds(backoffSec), ct);
                        continue;
                    }

                    await Task.Delay(TimeSpan.FromSeconds(interval), ct);
                }
            }
            catch (OperationCanceledException)
            {
                UpdateStatus("Polling stopped");
            }
            finally
            {
                IsPolling = false;
            }
        }, ct);
    }

    public void Stop()
    {
        _pollCts?.Cancel();
        _pollCts = null;
        IsPolling = false;
        _nextPageToken = null;
        _liveChatId = null;
        UpdateStatus("Stopped");
    }

    private void UpdateStatus(string status)
    {
        StatusMessage = status;
        StatusChanged?.Invoke(status);
    }

    private async Task<string?> FetchLiveChatIdAsync(string apiKey, string videoId, CancellationToken ct)
    {
        var url = $"https://www.googleapis.com/youtube/v3/videos?part=liveStreamingDetails&id={videoId}&key={apiKey}";
        var resp = await Http.GetAsync(url, ct);
        resp.EnsureSuccessStatusCode();

        var json = await resp.Content.ReadAsStringAsync(ct);
        using var doc = JsonDocument.Parse(json);

        if (doc.RootElement.TryGetProperty("items", out var items) && items.GetArrayLength() > 0)
        {
            var item = items[0];
            if (item.TryGetProperty("liveStreamingDetails", out var details) &&
                details.TryGetProperty("activeLiveChatId", out var chatId))
            {
                return chatId.GetString();
            }
        }
        return null;
    }

    private async Task<(List<ChatMessage> Messages, string? NextToken, int DelaySec)> FetchChatMessagesAsync(
        string apiKey, string liveChatId, string? pageToken, CancellationToken ct)
    {
        var url = $"https://www.googleapis.com/youtube/v3/liveChat/messages?part=id,snippet,authorDetails&liveChatId={liveChatId}&key={apiKey}";
        if (!string.IsNullOrEmpty(pageToken)) url += $"&pageToken={pageToken}";

        var resp = await Http.GetAsync(url, ct);
        resp.EnsureSuccessStatusCode();

        var json = await resp.Content.ReadAsStringAsync(ct);
        using var doc = JsonDocument.Parse(json);

        var list = new List<ChatMessage>();
        string? nextToken = null;
        int delaySec = 10;

        if (doc.RootElement.TryGetProperty("nextPageToken", out var tokenProp))
            nextToken = tokenProp.GetString();

        if (doc.RootElement.TryGetProperty("pollingIntervalMillis", out var pollProp))
            delaySec = Math.Max(10, (int)(pollProp.GetInt32() / 1000));

        if (doc.RootElement.TryGetProperty("items", out var items))
        {
            foreach (var item in items.EnumerateArray())
            {
                var msg = new ChatMessage();
                if (item.TryGetProperty("id", out var idProp)) msg.Id = idProp.GetString() ?? msg.Id;

                if (item.TryGetProperty("snippet", out var snip))
                {
                    if (snip.TryGetProperty("displayMessage", out var disp))
                        msg.MessageText = disp.GetString() ?? string.Empty;
                    if (snip.TryGetProperty("publishedAt", out var pub) && DateTime.TryParse(pub.GetString(), out var dt))
                        msg.PublishedAt = dt;
                    if (snip.TryGetProperty("type", out var typeProp) && typeProp.GetString() == "superChatMessageEvent")
                        msg.IsSuperchat = true;
                }

                if (item.TryGetProperty("authorDetails", out var auth))
                {
                    if (auth.TryGetProperty("displayName", out var name)) msg.AuthorName = name.GetString() ?? "Unknown";
                    if (auth.TryGetProperty("isChatModerator", out var mod)) msg.IsModerator = mod.GetBoolean();
                    if (auth.TryGetProperty("isChatOwner", out var own)) msg.IsOwner = own.GetBoolean();
                }

                list.Add(msg);
            }
        }

        return (list, nextToken, delaySec);
    }
}
