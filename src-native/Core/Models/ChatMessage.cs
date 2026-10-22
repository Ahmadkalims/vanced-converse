using System;

namespace VancedConverse.Core.Models;

public class ChatMessage
{
    public string Id { get; set; } = Guid.NewGuid().ToString();
    public string AuthorName { get; set; } = string.Empty;
    public string AuthorPhotoUrl { get; set; } = string.Empty;
    public string MessageText { get; set; } = string.Empty;
    public bool IsModerator { get; set; }
    public bool IsOwner { get; set; }
    public bool IsSuperchat { get; set; }
    public string? SuperchatAmount { get; set; }
    public DateTime PublishedAt { get; set; } = DateTime.UtcNow;
}
