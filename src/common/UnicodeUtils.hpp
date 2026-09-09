#pragma once

#include <string>
#include <vector>
#include <windows.h>
#include <cstdint>

enum class SegmentType {
    Text,
    UnicodeEmoji,
    CustomEmoji,
    AnimatedEmoji
};

struct TextSegment {
    SegmentType type = SegmentType::Text;
    std::wstring text;
    std::string asset_url;
    std::string alt_text;
    bool is_rtl = false;
};

namespace unicode {

inline std::wstring utf8_to_utf16(const std::string& utf8_str) {
    if (utf8_str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.size(), NULL, 0);
    if (size_needed <= 0) return L"";
    std::wstring result(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.size(), &result[0], size_needed);
    return result;
}

inline std::string utf16_to_utf8(const std::wstring& utf16_str) {
    if (utf16_str.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(), (int)utf16_str.size(), NULL, 0, NULL, NULL);
    if (size_needed <= 0) return "";
    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(), (int)utf16_str.size(), &result[0], size_needed, NULL, NULL);
    return result;
}

inline uint32_t get_codepoint(const std::wstring& str, size_t& index) {
    if (index >= str.size()) return 0;
    wchar_t lead = str[index];
    if (lead >= 0xD800 && lead <= 0xDBFF) {
        if (index + 1 < str.size()) {
            wchar_t trail = str[index + 1];
            if (trail >= 0xDC00 && trail <= 0xDFFF) {
                index += 2;
                return 0x10000 + ((lead - 0xD800) << 10) + (trail - 0xDC00);
            }
        }
    }
    index += 1;
    return static_cast<uint32_t>(lead);
}

inline bool is_emoji_codepoint(uint32_t cp) {
    // Standard Emoji ranges
    if (cp >= 0x1F600 && cp <= 0x1F64F) return true; // Emoticons
    if (cp >= 0x1F300 && cp <= 0x1F5FF) return true; // Misc Symbols & Pictographs
    if (cp >= 0x1F680 && cp <= 0x1F6FF) return true; // Transport & Map
    if (cp >= 0x1F1E6 && cp <= 0x1F1FF) return true; // Regional indicator flags
    if (cp >= 0x2600 && cp <= 0x26FF) return true;   // Misc symbols (e.g. ☀️, ❤️)
    if (cp >= 0x2700 && cp <= 0x27BF) return true;   // Dingbats (e.g. ✈️, ✌️)
    if (cp >= 0x1F900 && cp <= 0x1F9FF) return true; // Supplemental Symbols (e.g. 🤩, 🥺, 🦸)
    if (cp >= 0x1FA70 && cp <= 0x1FAFF) return true; // Symbols and Pictographs Extended-A
    if (cp >= 0x1F000 && cp <= 0x1F02F) return true; // Mahjong tiles
    if (cp >= 0x1F0A0 && cp <= 0x1F0FF) return true; // Playing cards
    if (cp == 0x200D) return true;                   // Zero Width Joiner (ZWJ)
    if (cp >= 0xFE00 && cp <= 0xFE0F) return true;   // Variation Selectors
    if (cp >= 0x1F3FB && cp <= 0x1F3FF) return true; // Skin tone modifiers
    if (cp == 0x203C || cp == 0x2049 || cp == 0x2122 || cp == 0x2139 ||
        (cp >= 0x2194 && cp <= 0x2199) || (cp >= 0x21A9 && cp <= 0x21AA) ||
        (cp >= 0x231A && cp <= 0x231B) || cp == 0x2328 || cp == 0x23CF ||
        (cp >= 0x23E9 && cp <= 0x23F3) || (cp >= 0x23F8 && cp <= 0x23FA) ||
        cp == 0x24C2 || (cp >= 0x25AA && cp <= 0x25AB) || cp == 0x25B6 ||
        cp == 0x25C0 || (cp >= 0x25FB && cp <= 0x25FE) || cp == 0x2934 ||
        cp == 0x2935 || (cp >= 0x2B05 && cp <= 0x2B07) || cp == 0x2B1B ||
        cp == 0x2B1C || cp == 0x2B50 || cp == 0x2B55 || cp == 0x3030 ||
        cp == 0x303D || cp == 0x3297 || cp == 0x3299) {
        return true;
    }
    return false;
}

inline bool is_rtl_char(wchar_t c) {
    // Arabic (0x0600 - 0x06FF), Hebrew (0x0590 - 0x05FF), Arabic Supplement (0x0750 - 0x077F)
    return (c >= 0x0590 && c <= 0x08FF) || (c >= 0xFB50 && c <= 0xFDFF) || (c >= 0xFE70 && c <= 0xFEFF);
}

inline bool is_devanagari(wchar_t c) {
    // Devanagari (Hindi) 0x0900 - 0x097F
    return (c >= 0x0900 && c <= 0x097F);
}

inline bool is_cjk(wchar_t c) {
    // CJK Unified Ideographs, Hiragana, Katakana, Hangul
    return (c >= 0x4E00 && c <= 0x9FFF) || (c >= 0x3040 && c <= 0x309F) ||
           (c >= 0x30A0 && c <= 0x30FF) || (c >= 0xAC00 && c <= 0xD7AF);
}

inline bool is_cyrillic(wchar_t c) {
    return (c >= 0x0400 && c <= 0x04FF);
}

// Fallback font selector based on codepoint characteristics
inline std::wstring get_fallback_font_for_char(wchar_t c) {
    if (is_devanagari(c)) return L"Nirmala UI";
    if (is_rtl_char(c)) return L"Segoe UI";
    if (is_cjk(c)) return L"Microsoft YaHei";
    if (is_cyrillic(c)) return L"Segoe UI";
    return L"Segoe UI";
}

// Splits a string into text segments and Unicode emoji segments
inline std::vector<TextSegment> segment_message_text(const std::wstring& input) {
    std::vector<TextSegment> segments;
    if (input.empty()) return segments;

    size_t i = 0;
    std::wstring current_text;
    std::wstring current_emoji;

    while (i < input.size()) {
        size_t prev_index = i;
        uint32_t cp = get_codepoint(input, i);
        size_t len = i - prev_index;
        std::wstring raw_char = input.substr(prev_index, len);

        if (is_emoji_codepoint(cp)) {
            if (!current_text.empty()) {
                TextSegment seg;
                seg.type = SegmentType::Text;
                seg.text = current_text;
                // Check if segment is predominantly RTL
                for (wchar_t wc : current_text) {
                    if (is_rtl_char(wc)) { seg.is_rtl = true; break; }
                }
                segments.push_back(seg);
                current_text.clear();
            }

            current_emoji += raw_char;

            // Peek ahead to absorb ZWJ sequences, skin tones, variation selectors
            while (i < input.size()) {
                size_t next_prev = i;
                uint32_t next_cp = get_codepoint(input, i);
                if (is_emoji_codepoint(next_cp)) {
                    current_emoji += input.substr(next_prev, i - next_prev);
                } else {
                    i = next_prev; // Rewind
                    break;
                }
            }

            TextSegment e_seg;
            e_seg.type = SegmentType::UnicodeEmoji;
            e_seg.text = current_emoji;
            segments.push_back(e_seg);
            current_emoji.clear();
        } else {
            current_text += raw_char;
        }
    }

    if (!current_text.empty()) {
        TextSegment seg;
        seg.type = SegmentType::Text;
        seg.text = current_text;
        for (wchar_t wc : current_text) {
            if (is_rtl_char(wc)) { seg.is_rtl = true; break; }
        }
        segments.push_back(seg);
    }

    return segments;
}

} // namespace unicode
