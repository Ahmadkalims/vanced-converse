#include "TextRenderer.hpp"
#include <algorithm>
#include <sstream>

using namespace Gdiplus;

static void helper_draw_rounded_rect(GraphicsPath& path, RectF rect, float radius) {
    float diameter = radius * 2.0f;
    if (diameter > rect.Width) diameter = rect.Width;
    if (diameter > rect.Height) diameter = rect.Height;

    path.StartFigure();
    path.AddArc(rect.X + rect.Width - diameter, rect.Y, diameter, diameter, 270, 90);
    path.AddArc(rect.X + rect.Width - diameter, rect.Y + rect.Height - diameter, diameter, diameter, 0, 90);
    path.AddArc(rect.X, rect.Y + rect.Height - diameter, diameter, diameter, 90, 90);
    path.AddArc(rect.X, rect.Y, diameter, diameter, 180, 90);
    path.CloseFigure();
}

TextRenderer::TextRenderer() {
    ensure_fonts("Montserrat Alternates");
}

TextRenderer::~TextRenderer() {}

void TextRenderer::ensure_fonts(const std::string& family_name) {
    if (current_font_name_ == family_name && primary_font_family_) return;
    current_font_name_ = family_name;

    std::wstring w_name = unicode::utf8_to_utf16(family_name);
    primary_font_family_ = std::make_unique<FontFamily>(w_name.c_str());
    if (primary_font_family_->GetLastStatus() != Ok) {
        primary_font_family_ = std::make_unique<FontFamily>(L"Segoe UI");
    }

    fallback_font_family_ = std::make_unique<FontFamily>(L"Segoe UI");
    emoji_font_family_ = std::make_unique<FontFamily>(L"Segoe UI Emoji");
    if (emoji_font_family_->GetLastStatus() != Ok) {
        emoji_font_family_ = std::make_unique<FontFamily>(L"Segoe UI Symbol");
    }

    indic_font_family_ = std::make_unique<FontFamily>(L"Nirmala UI");
    if (indic_font_family_->GetLastStatus() != Ok) {
        indic_font_family_ = std::make_unique<FontFamily>(L"Mangal");
    }

    cjk_font_family_ = std::make_unique<FontFamily>(L"Microsoft YaHei");
    if (cjk_font_family_->GetLastStatus() != Ok) {
        cjk_font_family_ = std::make_unique<FontFamily>(L"SimHei");
    }
}

void TextRenderer::update_font_family(const std::string& family_name) {
    ensure_fonts(family_name);
}

FontFamily* get_best_font_family(const TextSegment& seg, FontFamily* primary, FontFamily* indic, FontFamily* cjk, FontFamily* fallback) {
    if (!seg.text.empty()) {
        wchar_t first = seg.text[0];
        if (unicode::is_devanagari(first)) return indic;
        if (unicode::is_cjk(first)) return cjk;
    }
    return primary ? primary : fallback;
}

MessageCardLayout TextRenderer::layout_message(
    Graphics& g,
    const ChatMessage& msg,
    const StreamTheme& theme,
    float card_width,
    float start_y
) {
    ensure_fonts(theme.message_text.font.family);

    MessageCardLayout card;
    card.msg = &msg;
    card.x = 0.0f;
    card.y = start_y;
    card.width = card_width;
    card.is_highlight = msg.is_highlight;

    float pad_l = theme.message_background.padding.left;
    float pad_r = theme.message_background.padding.right;
    float pad_t = theme.message_background.padding.top;
    float pad_b = theme.message_background.padding.bottom;

    float usable_width = card_width - (pad_l + pad_r);
    if (usable_width < 40.0f) usable_width = 40.0f;

    float u_size = std::clamp(theme.username.font.size, 10.0f, 48.0f);
    int u_style = (theme.username.font.weight >= 600) ? FontStyleBold : FontStyleRegular;
    Gdiplus::Font font_user(primary_font_family_.get(), (REAL)u_size, u_style, UnitPixel);

    float t_size = std::clamp(theme.message_text.font.size, 10.0f, 48.0f);
    int t_style = (theme.message_text.font.weight >= 600) ? FontStyleBold : FontStyleRegular;
    Gdiplus::Font font_text_primary(primary_font_family_.get(), (REAL)t_size, t_style, UnitPixel);
    Gdiplus::Font font_text_indic(indic_font_family_.get(), (REAL)t_size, t_style, UnitPixel);
    Gdiplus::Font font_text_cjk(cjk_font_family_.get(), (REAL)t_size, t_style, UnitPixel);
    Gdiplus::Font font_text_fallback(fallback_font_family_.get(), (REAL)t_size, t_style, UnitPixel);

    StringFormat sf(StringFormat::GenericTypographic());
    sf.SetFormatFlags(StringFormatFlagsMeasureTrailingSpaces | StringFormatFlagsNoClip);

    float cur_x = 0.0f;
    float cur_y = 0.0f;
    float line_h = (std::max)(u_size, t_size) * 1.35f + theme.message_text.line_spacing;
    if (line_h < 18.0f) line_h = 18.0f;

    RenderLine current_line;
    current_line.y = cur_y;
    current_line.height = line_h;

    // 1. Layout Username
    std::wstring w_author = unicode::utf8_to_utf16(msg.author_name);
    if (!w_author.empty()) {
        std::wstring u_display = w_author + L": ";
        RectF u_bound;
        g.MeasureString(u_display.c_str(), (INT)u_display.size(), &font_user, PointF(0, 0), &sf, &u_bound);

        std::string col_str = msg.author_color_hex.empty() 
            ? theme.get_user_color(msg.author_id, msg.author_name) 
            : msg.author_color_hex;
        ColorRGBA u_rgba = ColorRGBA::parse(col_str, ColorRGBA(249, 168, 212, 255));

        InlineItem u_item;
        u_item.type = SegmentType::Text;
        u_item.text = u_display;
        u_item.is_username = true;
        u_item.x = cur_x;
        u_item.y = cur_y;
        u_item.width = u_bound.Width;
        u_item.height = line_h;
        u_item.color = u_rgba.to_gdiplus();

        current_line.items.push_back(u_item);
        cur_x += u_bound.Width;

        if (theme.username.placement == UsernamePlacement::AboveMessage) {
            card.lines.push_back(current_line);
            current_line.items.clear();
            cur_x = 0.0f;
            cur_y += line_h;
            current_line.y = cur_y;
            current_line.height = line_h;
        }
    }

    // 2. Message Content Segments
    std::vector<TextSegment> segs = msg.segments;
    if (segs.empty()) {
        std::wstring w_txt = unicode::utf8_to_utf16(msg.message_text);
        segs = unicode::segment_message_text(w_txt);
    }

    float emoji_box_size = t_size * 1.25f;

    for (const auto& s : segs) {
        if (s.type == SegmentType::CustomEmoji || s.type == SegmentType::AnimatedEmoji) {
            float item_w = emoji_box_size + 4.0f;
            if (cur_x + item_w > usable_width && cur_x > 0.0f) {
                card.lines.push_back(current_line);
                current_line.items.clear();
                cur_x = 0.0f;
                cur_y += line_h;
                current_line.y = cur_y;
                current_line.height = line_h;
            }

            InlineItem item;
            item.type = s.type;
            item.asset_url = s.asset_url;
            item.alt_text = s.alt_text;
            item.x = cur_x;
            item.y = cur_y + (line_h - emoji_box_size) * 0.5f;
            item.width = emoji_box_size;
            item.height = emoji_box_size;
            current_line.items.push_back(item);
            cur_x += item_w;
        } else if (s.type == SegmentType::UnicodeEmoji) {
            float measured_w = color_emoji_renderer_.measure_emoji_width(s.text, t_size);
            float item_w = measured_w + 2.0f;

            if (cur_x + item_w > usable_width && cur_x > 0.0f) {
                card.lines.push_back(current_line);
                current_line.items.clear();
                cur_x = 0.0f;
                cur_y += line_h;
                current_line.y = cur_y;
                current_line.height = line_h;
            }

            InlineItem item;
            item.type = SegmentType::UnicodeEmoji;
            item.text = s.text;
            item.x = cur_x;
            item.y = cur_y;
            item.width = measured_w;
            item.height = line_h;
            current_line.items.push_back(item);
            cur_x += item_w;
        } else {
            // Text segment with multi-language font selection
            Gdiplus::Font* font_to_use = &font_text_primary;
            if (!s.text.empty()) {
                wchar_t c = s.text[0];
                if (unicode::is_devanagari(c)) font_to_use = &font_text_indic;
                else if (unicode::is_cjk(c)) font_to_use = &font_text_cjk;
            }

            // Word-wrap splitting
            std::wstringstream ss(s.text);
            std::wstring token;
            while (ss >> token) {
                std::wstring token_with_space = token + L" ";
                RectF t_bound;
                g.MeasureString(token_with_space.c_str(), (INT)token_with_space.size(), font_to_use, PointF(0, 0), &sf, &t_bound);

                if (cur_x + t_bound.Width > usable_width && cur_x > 0.0f) {
                    card.lines.push_back(current_line);
                    current_line.items.clear();
                    cur_x = 0.0f;
                    cur_y += line_h;
                    current_line.y = cur_y;
                    current_line.height = line_h;
                }

                // If single word exceeds entire line width, break by characters
                if (t_bound.Width > usable_width) {
                    for (wchar_t ch : token) {
                        std::wstring ch_str(1, ch);
                        RectF ch_bound;
                        g.MeasureString(ch_str.c_str(), 1, font_to_use, PointF(0, 0), &sf, &ch_bound);

                        if (cur_x + ch_bound.Width > usable_width && cur_x > 0.0f) {
                            card.lines.push_back(current_line);
                            current_line.items.clear();
                            cur_x = 0.0f;
                            cur_y += line_h;
                            current_line.y = cur_y;
                            current_line.height = line_h;
                        }

                        InlineItem item;
                        item.type = SegmentType::Text;
                        item.text = ch_str;
                        item.x = cur_x;
                        item.y = cur_y;
                        item.width = ch_bound.Width;
                        item.height = line_h;
                        current_line.items.push_back(item);
                        cur_x += ch_bound.Width;
                    }
                    continue;
                }

                InlineItem item;
                item.type = SegmentType::Text;
                item.text = token_with_space;
                item.x = cur_x;
                item.y = cur_y;
                item.width = t_bound.Width;
                item.height = line_h;
                current_line.items.push_back(item);
                cur_x += t_bound.Width;
            }
        }
    }

    if (!current_line.items.empty()) {
        card.lines.push_back(current_line);
    }

    card.height = pad_t + pad_b + ((float)card.lines.size() * line_h);
    return card;
}

void TextRenderer::draw_message_card(
    Graphics& g,
    HDC hdc,
    const RECT& clip_rect,
    const MessageCardLayout& layout,
    const StreamTheme& theme,
    uint64_t current_time_ms
) {
    float pad_l = theme.message_background.padding.left;
    float pad_t = theme.message_background.padding.top;

    RectF card_rect(layout.x, layout.y, layout.width, layout.height);
    GraphicsPath card_path;
    helper_draw_rounded_rect(card_path, card_rect, theme.message_background.corner_radius);

    // 1. Draw Card Background & Outlines
    if (layout.is_highlight) {
        ColorRGBA high_bg = ColorRGBA::parse(theme.special_events.empty() ? "rgba(249, 168, 212, 0.12)" : theme.special_events[0].highlight.background_color);
        SolidBrush bg_brush(high_bg.to_gdiplus());
        g.FillPath(&bg_brush, &card_path);

        ColorRGBA high_out = ColorRGBA::parse(theme.special_events.empty() ? "#F472B6" : theme.special_events[0].highlight.outline.color);
        Pen out_pen(high_out.to_gdiplus(), 2.0f);
        g.DrawPath(&out_pen, &card_path);
    } else if (theme.message_background.enabled) {
        ColorRGBA bg_col = ColorRGBA::parse(theme.message_background.color);
        SolidBrush bg_brush(bg_col.to_gdiplus());
        g.FillPath(&bg_brush, &card_path);

        if (theme.message_background.outline.enabled && theme.message_background.outline.size > 0.0f) {
            ColorRGBA out_col = ColorRGBA::parse(theme.message_background.outline.color);
            Pen out_pen(out_col.to_gdiplus(), theme.message_background.outline.size);
            g.DrawPath(&out_pen, &card_path);
        }
    }

    // 2. Setup Fonts
    float u_size = std::clamp(theme.username.font.size, 10.0f, 48.0f);
    int u_style = (theme.username.font.weight >= 600) ? FontStyleBold : FontStyleRegular;
    Gdiplus::Font font_user(primary_font_family_.get(), (REAL)u_size, u_style, UnitPixel);

    float t_size = std::clamp(theme.message_text.font.size, 10.0f, 48.0f);
    int t_style = (theme.message_text.font.weight >= 600) ? FontStyleBold : FontStyleRegular;
    Gdiplus::Font font_text(primary_font_family_.get(), (REAL)t_size, t_style, UnitPixel);
    Gdiplus::Font font_indic(indic_font_family_.get(), (REAL)t_size, t_style, UnitPixel);
    Gdiplus::Font font_cjk(cjk_font_family_.get(), (REAL)t_size, t_style, UnitPixel);
    Gdiplus::Font font_fallback(fallback_font_family_.get(), (REAL)t_size, t_style, UnitPixel);

    StringFormat sf(StringFormat::GenericTypographic());
    sf.SetFormatFlags(StringFormatFlagsMeasureTrailingSpaces | StringFormatFlagsNoClip);

    ColorRGBA text_color_rgba = ColorRGBA::parse(theme.message_text.color);
    SolidBrush text_brush(text_color_rgba.to_gdiplus());

    // 3. Draw Inline Items
    for (const auto& line : layout.lines) {
        for (const auto& item : line.items) {
            float draw_x = layout.x + pad_l + item.x;
            float draw_y = layout.y + pad_t + line.y;

            if (item.is_username) {
                if (theme.username.effect.type == "glow") {
                    SolidBrush glow_b(Color(110, item.color.GetR(), item.color.GetG(), item.color.GetB()));
                    g.DrawString(item.text.c_str(), (INT)item.text.size(), &font_user, PointF(draw_x + 1.0f, draw_y + 1.0f), &sf, &glow_b);
                    g.DrawString(item.text.c_str(), (INT)item.text.size(), &font_user, PointF(draw_x - 1.0f, draw_y - 1.0f), &sf, &glow_b);
                }
                SolidBrush u_brush(item.color);
                g.DrawString(item.text.c_str(), (INT)item.text.size(), &font_user, PointF(draw_x, draw_y), &sf, &u_brush);
            } else if (item.type == SegmentType::UnicodeEmoji) {
                // RENDER FULL COLOR EMOJI VIA DIRECTWRITE & DIRECT2D
                if (color_emoji_renderer_.is_ready() && hdc) {
                    color_emoji_renderer_.draw_color_emoji(hdc, clip_rect, item.text, t_size, draw_x, draw_y);
                } else {
                    Gdiplus::Font font_emoji(emoji_font_family_.get(), (REAL)t_size, FontStyleRegular, UnitPixel);
                    SolidBrush e_brush(Color(255, 255, 255, 255));
                    g.DrawString(item.text.c_str(), (INT)item.text.size(), &font_emoji, PointF(draw_x, draw_y), &sf, &e_brush);
                }
            } else if (item.type == SegmentType::CustomEmoji || item.type == SegmentType::AnimatedEmoji) {
                auto asset = g_emoji_cache.get_asset(item.asset_url);
                if (asset && asset->is_loaded) {
                    Gdiplus::Bitmap* frame = asset->get_frame(current_time_ms);
                    if (frame) {
                        g.DrawImage(frame, draw_x, draw_y, item.width, item.height);
                    }
                } else {
                    SolidBrush ph_brush(Color(40, 255, 255, 255));
                    g.FillRectangle(&ph_brush, draw_x, draw_y, item.width, item.height);
                }
            } else {
                Gdiplus::Font* font_to_use = &font_text;
                if (!item.text.empty()) {
                    wchar_t c = item.text[0];
                    if (unicode::is_devanagari(c)) font_to_use = &font_indic;
                    else if (unicode::is_cjk(c)) font_to_use = &font_cjk;
                }

                if (theme.message_text.effect.type == "glow") {
                    SolidBrush glow_txt(Color(90, text_color_rgba.r, text_color_rgba.g, text_color_rgba.b));
                    g.DrawString(item.text.c_str(), (INT)item.text.size(), font_to_use, PointF(draw_x + 1.0f, draw_y + 1.0f), &sf, &glow_txt);
                }

                g.DrawString(item.text.c_str(), (INT)item.text.size(), font_to_use, PointF(draw_x, draw_y), &sf, &text_brush);
            }
        }
    }
}
