#include "ThemeModel.hpp"
#include "UnicodeUtils.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

// ColorRGBA Implementation
ColorRGBA ColorRGBA::parse(const std::string& raw, ColorRGBA fallback) {
    std::string s;
    for (char c : raw) {
        if (!std::isspace((unsigned char)c)) s += (char)std::tolower((unsigned char)c);
    }
    if (s.empty()) return fallback;

    if (s[0] == '#') {
        std::string hex = s.substr(1);
        if (hex.length() == 3) {
            uint32_t r = std::stoul(std::string(2, hex[0]), nullptr, 16);
            uint32_t g = std::stoul(std::string(2, hex[1]), nullptr, 16);
            uint32_t b = std::stoul(std::string(2, hex[2]), nullptr, 16);
            return ColorRGBA((uint8_t)r, (uint8_t)g, (uint8_t)b, 255);
        } else if (hex.length() == 4) {
            uint32_t r = std::stoul(std::string(2, hex[0]), nullptr, 16);
            uint32_t g = std::stoul(std::string(2, hex[1]), nullptr, 16);
            uint32_t b = std::stoul(std::string(2, hex[2]), nullptr, 16);
            uint32_t a = std::stoul(std::string(2, hex[3]), nullptr, 16);
            return ColorRGBA((uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a);
        } else if (hex.length() == 6) {
            uint32_t val = std::stoul(hex, nullptr, 16);
            return ColorRGBA((uint8_t)((val >> 16) & 0xFF), (uint8_t)((val >> 8) & 0xFF), (uint8_t)(val & 0xFF), 255);
        } else if (hex.length() == 8) {
            uint32_t val = std::stoul(hex, nullptr, 16);
            return ColorRGBA((uint8_t)((val >> 24) & 0xFF), (uint8_t)((val >> 16) & 0xFF), (uint8_t)((val >> 8) & 0xFF), (uint8_t)(val & 0xFF));
        }
    } else if (s.find("rgba(") == 0 || s.find("rgb(") == 0) {
        size_t start = s.find('(');
        size_t end = s.find(')');
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string inner = s.substr(start + 1, end - start - 1);
            std::stringstream ss(inner);
            std::string item;
            std::vector<std::string> parts;
            while (std::getline(ss, item, ',')) {
                parts.push_back(item);
            }
            if (parts.size() >= 3) {
                try {
                    int r = std::stoi(parts[0]);
                    int g = std::stoi(parts[1]);
                    int b = std::stoi(parts[2]);
                    int a = 255;
                    if (parts.size() >= 4) {
                        float fa = std::stof(parts[3]);
                        a = (int)(fa <= 1.0f ? fa * 255.0f : fa);
                    }
                    return ColorRGBA((uint8_t)std::clamp(r, 0, 255), (uint8_t)std::clamp(g, 0, 255), (uint8_t)std::clamp(b, 0, 255), (uint8_t)std::clamp(a, 0, 255));
                } catch (...) {}
            }
        }
    }

    return fallback;
}

std::string ColorRGBA::to_hex_string() const {
    std::ostringstream ss;
    ss << "#" << std::hex << std::uppercase << std::setfill('0');
    ss << std::setw(2) << (int)r << std::setw(2) << (int)g << std::setw(2) << (int)b;
    if (a < 255) {
        ss << std::setw(2) << (int)a;
    }
    return ss.str();
}

std::string ColorRGBA::to_rgba_string() const {
    std::ostringstream ss;
    ss << "rgba(" << (int)r << ", " << (int)g << ", " << (int)b << ", "
       << std::fixed << std::setprecision(2) << (a / 255.0f) << ")";
    return ss.str();
}

Gdiplus::Color ColorRGBA::to_gdiplus(float opacity_multiplier) const {
    uint8_t final_a = (uint8_t)std::clamp((int)(a * opacity_multiplier), 0, 255);
    return Gdiplus::Color(final_a, r, g, b);
}

// JSON conversions
json::Value ThemeMetadata::to_json() const {
    json::Value obj = json::Value::object();
    obj["name"] = name;
    obj["author"] = author;
    obj["version"] = (double)version;
    obj["description"] = description;
    return obj;
}

void ThemeMetadata::from_json(const json::Value& v) {
    if (!v.is_object()) return;
    name = v["name"].as_string(name);
    author = v["author"].as_string(author);
    version = v["version"].as_int(version);
    description = v["description"].as_string(description);
}

json::Value OverlayBackgroundConfig::to_json() const {
    json::Value obj = json::Value::object();
    std::string type_str = "acrylic";
    if (type == OverlayBgType::Mica) type_str = "mica";
    else if (type == OverlayBgType::LiquidGlass) type_str = "liquid_glass";
    else if (type == OverlayBgType::Solid) type_str = "solid";
    else if (type == OverlayBgType::Transparent) type_str = "transparent";
    else if (type == OverlayBgType::LinearGradient) type_str = "linear_gradient";
    else if (type == OverlayBgType::RadialGradient) type_str = "radial_gradient";
    else if (type == OverlayBgType::Image) type_str = "image";

    obj["type"] = type_str;
    obj["color"] = color;
    obj["opacity"] = (double)opacity;

    json::Value grad = json::Value::object();
    grad["enabled"] = gradient_enabled;
    grad["type"] = gradient_type;
    grad["angle"] = (double)gradient_angle;
    json::Value stops = json::Value::array();
    for (const auto& s : gradient_stops) {
        json::Value st = json::Value::object();
        st["offset"] = (double)s.offset;
        st["color"] = s.color.to_hex_string();
        stops.push_back(st);
    }
    grad["stops"] = stops;
    obj["gradient"] = grad;

    json::Value img = json::Value::object();
    img["enabled"] = image_enabled;
    img["path"] = image_path;
    img["fit"] = image_fit;
    img["opacity"] = (double)image_opacity;
    obj["image"] = img;

    json::Value eff = json::Value::object();
    eff["type"] = effect_type;
    eff["blur"] = (double)blur;
    eff["saturation"] = (double)saturation;
    obj["effect"] = eff;

    json::Value out = json::Value::object();
    out["enabled"] = outline_enabled;
    out["size"] = (double)outline_size;
    out["type"] = outline_type;
    out["color"] = outline_color;
    out["glowColor"] = outline_glow_color;
    out["glowBlur"] = (double)outline_glow_blur;
    obj["outline"] = out;

    obj["cornerRadius"] = (double)corner_radius;
    return obj;
}

void OverlayBackgroundConfig::from_json(const json::Value& v) {
    if (!v.is_object()) return;
    std::string t = v["type"].as_string("acrylic");
    if (t == "mica") type = OverlayBgType::Mica;
    else if (t == "liquid_glass") type = OverlayBgType::LiquidGlass;
    else if (t == "solid") type = OverlayBgType::Solid;
    else if (t == "transparent") type = OverlayBgType::Transparent;
    else if (t == "linear_gradient") type = OverlayBgType::LinearGradient;
    else if (t == "radial_gradient") type = OverlayBgType::RadialGradient;
    else if (t == "image") type = OverlayBgType::Image;
    else type = OverlayBgType::Acrylic;

    color = v["color"].as_string(color);
    opacity = (float)v["opacity"].as_double(opacity);

    if (v.contains("gradient")) {
        const auto& g = v["gradient"];
        gradient_enabled = g["enabled"].as_bool(gradient_enabled);
        gradient_type = g["type"].as_string(gradient_type);
        gradient_angle = (float)g["angle"].as_double(gradient_angle);
        if (g.contains("stops") && g["stops"].is_array()) {
            gradient_stops.clear();
            for (size_t i = 0; i < g["stops"].size(); ++i) {
                const auto& st = g["stops"][i];
                GradientStop s;
                s.offset = (float)st["offset"].as_double(0.0);
                s.color = ColorRGBA::parse(st["color"].as_string("#FFFFFF"));
                gradient_stops.push_back(s);
            }
        }
    }

    if (v.contains("image")) {
        const auto& img = v["image"];
        image_enabled = img["enabled"].as_bool(image_enabled);
        image_path = img["path"].as_string(image_path);
        image_fit = img["fit"].as_string(image_fit);
        image_opacity = (float)img["opacity"].as_double(image_opacity);
    }

    if (v.contains("effect")) {
        const auto& eff = v["effect"];
        effect_type = eff["type"].as_string(effect_type);
        blur = (float)eff["blur"].as_double(blur);
        saturation = (float)eff["saturation"].as_double(saturation);
    }

    if (v.contains("outline")) {
        const auto& out = v["outline"];
        outline_enabled = out["enabled"].as_bool(outline_enabled);
        outline_size = (float)out["size"].as_double(outline_size);
        outline_type = out["type"].as_string(outline_type);
        outline_color = out["color"].as_string(outline_color);
        outline_glow_color = out["glowColor"].as_string(outline_glow_color);
        outline_glow_blur = (float)out["glowBlur"].as_double(outline_glow_blur);
    }

    corner_radius = (float)v["cornerRadius"].as_double(corner_radius);
}

json::Value ScrollbarConfig::to_json() const {
    json::Value obj = json::Value::object();
    obj["visible"] = visible;
    std::string mode_str = "clickthrough_off_hover";
    if (interaction_mode == ScrollbarInteractionMode::Always) mode_str = "always";
    else if (interaction_mode == ScrollbarInteractionMode::Disabled) mode_str = "disabled";
    obj["interactionMode"] = mode_str;
    obj["width"] = (double)width;
    obj["color"] = color;
    obj["type"] = type;
    obj["glowColor"] = glow_color;
    obj["cornerRadius"] = (double)corner_radius;
    return obj;
}

void ScrollbarConfig::from_json(const json::Value& v) {
    if (!v.is_object()) return;
    visible = v["visible"].as_bool(visible);
    std::string m = v["interactionMode"].as_string("clickthrough_off_hover");
    if (m == "always") interaction_mode = ScrollbarInteractionMode::Always;
    else if (m == "disabled") interaction_mode = ScrollbarInteractionMode::Disabled;
    else interaction_mode = ScrollbarInteractionMode::ClickthroughOffHover;

    width = (float)v["width"].as_double(width);
    color = v["color"].as_string(color);
    type = v["type"].as_string(type);
    glow_color = v["glowColor"].as_string(glow_color);
    corner_radius = (float)v["cornerRadius"].as_double(corner_radius);
}

json::Value FontConfig::to_json() const {
    json::Value obj = json::Value::object();
    obj["family"] = family;
    obj["size"] = (double)size;
    obj["weight"] = (double)weight;
    return obj;
}

void FontConfig::from_json(const json::Value& v) {
    if (!v.is_object()) return;
    family = v["family"].as_string(family);
    size = (float)v["size"].as_double(size);
    weight = v["weight"].as_int(weight);
}

json::Value TextEffectConfig::to_json() const {
    json::Value obj = json::Value::object();
    obj["type"] = type;
    obj["shadow"] = shadow;
    return obj;
}

void TextEffectConfig::from_json(const json::Value& v) {
    if (!v.is_object()) return;
    type = v["type"].as_string(type);
    shadow = v["shadow"].as_string(shadow);
}

json::Value UsernameConfig::to_json() const {
    json::Value obj = json::Value::object();
    obj["placement"] = (placement == UsernamePlacement::AboveMessage ? "above_message" : "before_message");
    obj["font"] = font.to_json();

    json::Value col = json::Value::object();
    std::string c_mode = "persistent_user_color";
    if (color_mode == UsernameColorMode::Single) c_mode = "single";
    else if (color_mode == UsernameColorMode::RandomPalette) c_mode = "random_palette";
    col["type"] = c_mode;
    col["value"] = color_value;
    json::Value pal = json::Value::array();
    for (const auto& p : palette) pal.push_back(p);
    col["palette"] = pal;
    obj["color"] = col;

    obj["effect"] = effect.to_json();
    obj["lineSpacing"] = (double)line_spacing;
    obj["padding"] = padding.to_json();
    obj["margin"] = margin.to_json();
    return obj;
}

void UsernameConfig::from_json(const json::Value& v) {
    if (!v.is_object()) return;
    std::string pl = v["placement"].as_string("before_message");
    placement = (pl == "above_message" ? UsernamePlacement::AboveMessage : UsernamePlacement::BeforeMessage);
    font.from_json(v["font"]);

    if (v.contains("color")) {
        const auto& c = v["color"];
        std::string cm = c["type"].as_string("persistent_user_color");
        if (cm == "single") color_mode = UsernameColorMode::Single;
        else if (cm == "random_palette") color_mode = UsernameColorMode::RandomPalette;
        else color_mode = UsernameColorMode::PersistentUserColor;

        color_value = c["value"].as_string(color_value);
        if (c.contains("palette") && c["palette"].is_array()) {
            palette.clear();
            for (size_t i = 0; i < c["palette"].size(); ++i) {
                palette.push_back(c["palette"][i].as_string("#F9A8D4"));
            }
        }
    }

    effect.from_json(v["effect"]);
    line_spacing = (float)v["lineSpacing"].as_double(line_spacing);
    padding.from_json(v["padding"]);
    margin.from_json(v["margin"]);
}

json::Value MessageTextConfig::to_json() const {
    json::Value obj = json::Value::object();
    obj["font"] = font.to_json();
    obj["color"] = color;
    obj["effect"] = effect.to_json();
    obj["lineSpacing"] = (double)line_spacing;
    obj["padding"] = padding.to_json();
    obj["margin"] = margin.to_json();
    return obj;
}

void MessageTextConfig::from_json(const json::Value& v) {
    if (!v.is_object()) return;
    font.from_json(v["font"]);
    color = v["color"].as_string(color);
    effect.from_json(v["effect"]);
    line_spacing = (float)v["lineSpacing"].as_double(line_spacing);
    padding.from_json(v["padding"]);
    margin.from_json(v["margin"]);
}

json::Value OutlineConfig::to_json() const {
    json::Value obj = json::Value::object();
    obj["enabled"] = enabled;
    obj["size"] = (double)size;
    obj["type"] = type;
    obj["color"] = color;
    obj["glowColor"] = glow_color;
    return obj;
}

void OutlineConfig::from_json(const json::Value& v) {
    if (!v.is_object()) return;
    enabled = v["enabled"].as_bool(enabled);
    size = (float)v["size"].as_double(size);
    type = v["type"].as_string(type);
    color = v["color"].as_string(color);
    glow_color = v["glowColor"].as_string(glow_color);
}

json::Value MessageBackgroundConfig::to_json() const {
    json::Value obj = json::Value::object();
    obj["enabled"] = enabled;
    obj["color"] = color;
    obj["outline"] = outline.to_json();
    obj["cornerRadius"] = (double)corner_radius;
    obj["padding"] = padding.to_json();
    obj["margin"] = margin.to_json();
    return obj;
}

void MessageBackgroundConfig::from_json(const json::Value& v) {
    if (!v.is_object()) return;
    enabled = v["enabled"].as_bool(enabled);
    color = v["color"].as_string(color);
    outline.from_json(v["outline"]);
    corner_radius = (float)v["cornerRadius"].as_double(corner_radius);
    padding.from_json(v["padding"]);
    margin.from_json(v["margin"]);
}

json::Value SpecialEventTrigger::to_json() const {
    json::Value obj = json::Value::object();
    obj["type"] = type;
    obj["value"] = value;
    return obj;
}

void SpecialEventTrigger::from_json(const json::Value& v) {
    if (!v.is_object()) return;
    type = v["type"].as_string(type);
    value = v["value"].as_string(value);
}

json::Value SpecialEventHighlight::to_json() const {
    json::Value obj = json::Value::object();
    obj["backgroundColor"] = background_color;
    obj["outline"] = outline.to_json();
    obj["cornerRadius"] = (double)corner_radius;
    return obj;
}

void SpecialEventHighlight::from_json(const json::Value& v) {
    if (!v.is_object()) return;
    background_color = v["backgroundColor"].as_string(background_color);
    outline.from_json(v["outline"]);
    corner_radius = (float)v["cornerRadius"].as_double(corner_radius);
}

json::Value SpecialEventConfig::to_json() const {
    json::Value obj = json::Value::object();
    obj["id"] = id;
    obj["name"] = name;
    obj["enabled"] = enabled;
    obj["trigger"] = trigger.to_json();
    obj["highlight"] = highlight.to_json();
    return obj;
}

void SpecialEventConfig::from_json(const json::Value& v) {
    if (!v.is_object()) return;
    id = v["id"].as_string(id);
    name = v["name"].as_string(name);
    enabled = v["enabled"].as_bool(enabled);
    trigger.from_json(v["trigger"]);
    highlight.from_json(v["highlight"]);
}

// StreamTheme Implementation


std::string StreamTheme::to_json_string(bool pretty) const {
    json::Value root = json::Value::object();
    root["metadata"] = metadata.to_json();
    root["overlay"] = json::Value::object();
    root["overlay"]["background"] = overlay.to_json();
    root["scrollbar"] = scrollbar.to_json();
    root["username"] = username.to_json();
    root["messageText"] = message_text.to_json();
    root["messageBackground"] = message_background.to_json();

    json::Value ev_arr = json::Value::array();
    for (const auto& ev : special_events) {
        ev_arr.push_back(ev.to_json());
    }
    root["specialEvents"] = ev_arr;

    return root.serialize(pretty ? 2 : 0);
}

bool StreamTheme::from_json_string(const std::string& json_str, std::string* error_msg) {
    json::Value root = json::parse(json_str);
    if (!root.is_object()) {
        if (error_msg) *error_msg = "Invalid JSON root element (must be object)";
        return false;
    }

    if (root.contains("metadata")) metadata.from_json(root["metadata"]);
    if (root.contains("overlay") && root["overlay"].contains("background")) {
        overlay.from_json(root["overlay"]["background"]);
    } else if (root.contains("overlay")) {
        overlay.from_json(root["overlay"]);
    }

    if (root.contains("scrollbar")) scrollbar.from_json(root["scrollbar"]);
    if (root.contains("username")) username.from_json(root["username"]);
    if (root.contains("messageText")) message_text.from_json(root["messageText"]);
    if (root.contains("messageBackground")) message_background.from_json(root["messageBackground"]);

    if (root.contains("specialEvents") && root["specialEvents"].is_array()) {
        special_events.clear();
        for (size_t i = 0; i < root["specialEvents"].size(); ++i) {
            SpecialEventConfig ev;
            ev.from_json(root["specialEvents"][i]);
            special_events.push_back(ev);
        }
    }

    return validate();
}

static std::string trim_css_val(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n\"'");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n\"';");
    return str.substr(first, (last - first + 1));
}

static float parse_css_num(const std::string& val, float fallback = 0.0f) {
    std::string s = trim_css_val(val);
    if (s.empty()) return fallback;
    size_t end_idx = 0;
    while (end_idx < s.size() && (std::isdigit((unsigned char)s[end_idx]) || s[end_idx] == '.' || s[end_idx] == '-')) {
        end_idx++;
    }
    if (end_idx == 0) return fallback;
    try {
        return std::stof(s.substr(0, end_idx));
    } catch (...) {
        return fallback;
    }
}

std::string StreamTheme::to_css_string() const {
    std::ostringstream ss;
    ss << "/* ==========================================================================\n"
       << "   VANCED CONVERSE THEME\n"
       << "   ========================================================================== */\n\n"
       << "/* Theme Metadata\n"
       << "Theme Name: " << metadata.name << "\n"
       << "Author: " << metadata.author << "\n"
       << "Version: " << metadata.version << "\n"
       << "Description: " << metadata.description << "\n"
       << "*/\n\n"
       << ":root {\n"
       << "    --vc-overlay-bg-color: " << overlay.color << ";\n"
       << "    --vc-overlay-bg-opacity: " << overlay.opacity << ";\n"
       << "    --vc-overlay-bg-blur: " << overlay.blur << "px;\n"
       << "    --vc-overlay-radius: " << overlay.corner_radius << "px;\n\n"
       << "    --vc-overlay-border-color: " << overlay.outline_color << ";\n"
       << "    --vc-overlay-border-width: " << overlay.outline_size << "px;\n"
       << "    --vc-overlay-glow-color: " << overlay.outline_glow_color << ";\n"
       << "    --vc-overlay-glow-radius: " << overlay.outline_glow_blur << "px;\n\n"
       << "    --vc-username-font: \"" << username.font.family << "\", sans-serif;\n"
       << "    --vc-username-size: " << username.font.size << "px;\n"
       << "    --vc-username-weight: " << username.font.weight << ";\n"
       << "    --vc-username-color: " << username.color_value << ";\n"
       << "    --vc-username-glow-color: " << username.effect.shadow << ";\n\n"
       << "    --vc-message-font: \"" << message_text.font.family << "\", sans-serif;\n"
       << "    --vc-message-size: " << message_text.font.size << "px;\n"
       << "    --vc-message-weight: " << message_text.font.weight << ";\n"
       << "    --vc-message-color: " << message_text.color << ";\n"
       << "    --vc-message-line-spacing: " << message_text.line_spacing << "px;\n\n"
       << "    --vc-message-bg-color: " << message_background.color << ";\n"
       << "    --vc-message-radius: " << message_background.corner_radius << "px;\n"
       << "    --vc-message-border-color: " << message_background.outline.color << ";\n"
       << "    --vc-message-border-width: " << message_background.outline.size << "px;\n\n"
       << "    --vc-scrollbar-width: " << scrollbar.width << "px;\n"
       << "    --vc-scrollbar-thumb-color: " << scrollbar.color << ";\n"
       << "    --vc-scrollbar-radius: " << scrollbar.corner_radius << "px;\n\n"
       << "    --vc-motion-duration: 300ms;\n"
       << "    --vc-motion-ease: cubic-bezier(0, 0, 0.2, 1);\n\n";

    for (const auto& ev : special_events) {
        if (ev.id == "question") {
            ss << "    --vc-event-question-bg: " << ev.highlight.background_color << ";\n"
               << "    --vc-event-question-border: " << ev.highlight.outline.color << ";\n"
               << "    --vc-event-question-border-width: " << ev.highlight.outline.size << "px;\n";
        }
    }

    ss << "}\n";
    return ss.str();
}

bool StreamTheme::from_css_string(const std::string& css_str, std::string* error_msg) {
    if (css_str.empty()) {
        if (error_msg) *error_msg = "CSS string is empty";
        return false;
    }

    // 1. Extract Metadata from Comments
    auto find_meta_field = [&](const std::string& key, std::string& out) {
        size_t pos = css_str.find(key);
        if (pos != std::string::npos) {
            size_t start = pos + key.length();
            size_t end = css_str.find_first_of("\r\n*", start);
            if (end != std::string::npos) {
                out = trim_css_val(css_str.substr(start, end - start));
            }
        }
    };

    find_meta_field("Theme Name:", metadata.name);
    find_meta_field("Author:", metadata.author);
    find_meta_field("Description:", metadata.description);

    std::string ver_str;
    find_meta_field("Version:", ver_str);
    if (!ver_str.empty()) {
        try { metadata.version = std::stoi(ver_str); } catch (...) {}
    }

    // 2. Parse Properties line by line
    std::stringstream ss(css_str);
    std::string line;

    while (std::getline(ss, line)) {
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string prop = trim_css_val(line.substr(0, colon));
        std::string val = trim_css_val(line.substr(colon + 1));

        if (prop.empty() || val.empty()) continue;

        // Overlay Variables
        if (prop == "--vc-overlay-bg-color") {
            overlay.color = val;
        } else if (prop == "--vc-overlay-bg-opacity") {
            overlay.opacity = parse_css_num(val, overlay.opacity);
        } else if (prop == "--vc-overlay-bg-blur") {
            overlay.blur = parse_css_num(val, overlay.blur);
        } else if (prop == "--vc-overlay-radius") {
            overlay.corner_radius = parse_css_num(val, overlay.corner_radius);
        } else if (prop == "--vc-overlay-border-color") {
            overlay.outline_color = val;
            overlay.outline_enabled = true;
        } else if (prop == "--vc-overlay-border-width") {
            overlay.outline_size = parse_css_num(val, overlay.outline_size);
            overlay.outline_enabled = (overlay.outline_size > 0.0f);
        } else if (prop == "--vc-overlay-glow-color") {
            overlay.outline_glow_color = val;
        } else if (prop == "--vc-overlay-glow-radius") {
            overlay.outline_glow_blur = parse_css_num(val, overlay.outline_glow_blur);
        }

        // Username Variables
        else if (prop == "--vc-username-font") {
            size_t comma = val.find(',');
            std::string font_fam = (comma != std::string::npos) ? val.substr(0, comma) : val;
            username.font.family = trim_css_val(font_fam);
        } else if (prop == "--vc-username-size") {
            username.font.size = parse_css_num(val, username.font.size);
        } else if (prop == "--vc-username-weight") {
            username.font.weight = (int)parse_css_num(val, (float)username.font.weight);
        } else if (prop == "--vc-username-color") {
            username.color_value = val;
        } else if (prop == "--vc-username-glow-color") {
            username.effect.type = "glow";
            username.effect.shadow = "0 0 4px " + val;
        }

        // Message Variables
        else if (prop == "--vc-message-font") {
            size_t comma = val.find(',');
            std::string font_fam = (comma != std::string::npos) ? val.substr(0, comma) : val;
            message_text.font.family = trim_css_val(font_fam);
        } else if (prop == "--vc-message-size") {
            message_text.font.size = parse_css_num(val, message_text.font.size);
        } else if (prop == "--vc-message-weight") {
            message_text.font.weight = (int)parse_css_num(val, (float)message_text.font.weight);
        } else if (prop == "--vc-message-color") {
            message_text.color = val;
        } else if (prop == "--vc-message-line-spacing") {
            message_text.line_spacing = parse_css_num(val, message_text.line_spacing);
        }

        // Message Background Variables
        else if (prop == "--vc-message-bg-color") {
            message_background.color = val;
            message_background.enabled = (val != "transparent" && val != "none");
        } else if (prop == "--vc-message-radius") {
            message_background.corner_radius = parse_css_num(val, message_background.corner_radius);
        } else if (prop == "--vc-message-border-color") {
            message_background.outline.color = val;
            message_background.outline.enabled = (val != "transparent" && val != "none");
        } else if (prop == "--vc-message-border-width") {
            message_background.outline.size = parse_css_num(val, message_background.outline.size);
            message_background.outline.enabled = (message_background.outline.size > 0.0f);
        }

        // Scrollbar Variables
        else if (prop == "--vc-scrollbar-width") {
            scrollbar.width = parse_css_num(val, scrollbar.width);
        } else if (prop == "--vc-scrollbar-thumb-color") {
            scrollbar.color = val;
        } else if (prop == "--vc-scrollbar-radius") {
            scrollbar.corner_radius = parse_css_num(val, scrollbar.corner_radius);
        }

        // Special Events
        else if (prop == "--vc-event-question-bg") {
            for (auto& ev : special_events) {
                if (ev.id == "question") ev.highlight.background_color = val;
            }
        } else if (prop == "--vc-event-question-border") {
            for (auto& ev : special_events) {
                if (ev.id == "question") {
                    ev.highlight.outline.color = val;
                    ev.highlight.outline.enabled = true;
                }
            }
        } else if (prop == "--vc-event-question-border-width") {
            for (auto& ev : special_events) {
                if (ev.id == "question") ev.highlight.outline.size = parse_css_num(val, 2.0f);
            }
        }
    }

    return validate();
}


bool StreamTheme::validate(std::vector<std::string>* warnings) {
    bool is_valid = true;

    // Clamp and sanitize
    overlay.opacity = std::clamp(overlay.opacity, 0.0f, 1.0f);
    overlay.blur = std::clamp(overlay.blur, 0.0f, 100.0f);
    overlay.corner_radius = std::clamp(overlay.corner_radius, 0.0f, 60.0f);

    username.font.size = std::clamp(username.font.size, 8.0f, 72.0f);
    message_text.font.size = std::clamp(message_text.font.size, 8.0f, 72.0f);

    username.line_spacing = std::clamp(username.line_spacing, -50.0f, 50.0f);
    message_text.line_spacing = std::clamp(message_text.line_spacing, -50.0f, 50.0f);

    message_background.corner_radius = std::clamp(message_background.corner_radius, 0.0f, 50.0f);

    if (username.palette.empty()) {
        username.palette.push_back("#F9A8D4");
        if (warnings) warnings->push_back("Username palette was empty; restored default pink color.");
    }

    return is_valid;
}

std::string StreamTheme::get_user_color(const std::string& author_id, const std::string& author_name) const {
    if (username.color_mode == UsernameColorMode::Single) {
        return username.color_value;
    }

    if (username.palette.empty()) return "#F9A8D4";

    // Deterministic hash based on identity
    std::string key = author_id.empty() ? author_name : author_id;
    std::hash<std::string> hasher;
    size_t idx = hasher(key) % username.palette.size();
    return username.palette[idx];
}

bool StreamTheme::is_special_event_triggered(const std::string& event_id, const std::wstring& msg_text) const {
    for (const auto& ev : special_events) {
        if (ev.enabled && ev.id == event_id) {
            std::wstring trigger_val = unicode::utf8_to_utf16(ev.trigger.value);
            if (trigger_val.empty()) continue;

            if (ev.trigger.type == "contains_character" || ev.trigger.type == "contains_text") {
                if (msg_text.find(trigger_val) != std::wstring::npos) return true;
            } else if (ev.trigger.type == "starts_with") {
                if (msg_text.rfind(trigger_val, 0) == 0) return true;
            }
        }
    }
    return false;
}

StreamTheme StreamTheme::create_midnight_pink() {
    StreamTheme t;
    t.metadata.name = "Midnight Pink";
    t.metadata.author = "Antigravity";
    t.metadata.version = 1;
    t.metadata.description = "Dark acrylic overlay with pink highlights matching reference theme";

    t.overlay.type = OverlayBgType::Acrylic;
    t.overlay.color = "#14141E";
    t.overlay.opacity = 0.4f;
    t.overlay.blur = 15.0f;
    t.overlay.saturation = 120.0f;
    t.overlay.corner_radius = 20.0f;

    t.scrollbar.visible = false;
    t.scrollbar.interaction_mode = ScrollbarInteractionMode::ClickthroughOffHover;
    t.scrollbar.color = "#F9A8D4";

    t.username.placement = UsernamePlacement::BeforeMessage;
    t.username.font = { "Montserrat Alternates", 16.0f, 600 };
    t.username.color_mode = UsernameColorMode::PersistentUserColor;
    t.username.color_value = "#F9A8D4";
    t.username.palette = { "#F9A8D4", "#F472B6", "#FB7185", "#38BDF8", "#34D399", "#A78BFA" };
    t.username.effect = { "glow", "0 0 4px rgba(249, 168, 212, 0.7)" };

    t.message_text.font = { "Montserrat Alternates", 16.0f, 400 };
    t.message_text.color = "#E5E7EB";
    t.message_text.effect = { "solid", "" };

    t.message_background.enabled = true;
    t.message_background.color = "rgba(255, 255, 255, 0.02)";
    t.message_background.corner_radius = 15.0f;
    t.message_background.padding = { 12.0f, 16.0f, 12.0f, 16.0f };
    t.message_background.margin = { 10.0f, 0.0f, 10.0f, 0.0f };

    SpecialEventConfig q;
    q.id = "question";
    q.name = "Question Highlight";
    q.enabled = true;
    q.trigger = { "contains_character", "?" };
    q.highlight.background_color = "rgba(249, 168, 212, 0.12)";
    q.highlight.outline = { true, 2.0f, "solid", "#F472B6", "#F472B6" };
    q.highlight.corner_radius = 15.0f;
    t.special_events = { q };

    return t;
}

StreamTheme StreamTheme::create_cyberpunk_neon() {
    StreamTheme t;
    t.metadata.name = "Cyberpunk Neon";
    t.metadata.author = "Antigravity";
    t.metadata.version = 1;
    t.metadata.description = "Vibrant neon cyan & magenta aesthetic with glass reflection";

    t.overlay.type = OverlayBgType::LiquidGlass;
    t.overlay.color = "#0B0E17";
    t.overlay.opacity = 0.55f;
    t.overlay.blur = 20.0f;
    t.overlay.corner_radius = 18.0f;

    t.username.font = { "Consolas", 15.0f, 700 };
    t.username.color_mode = UsernameColorMode::PersistentUserColor;
    t.username.palette = { "#00F0FF", "#FF007F", "#FFE600", "#7928CA", "#00FF66" };
    t.username.effect = { "glow", "0 0 6px rgba(0, 240, 255, 0.8)" };

    t.message_text.font = { "Segoe UI", 15.0f, 400 };
    t.message_text.color = "#F0F6FC";

    t.message_background.enabled = true;
    t.message_background.color = "rgba(18, 24, 38, 0.65)";
    t.message_background.outline = { true, 1.0f, "solid", "#00F0FF", "#00F0FF" };
    t.message_background.corner_radius = 10.0f;

    return t;
}

StreamTheme StreamTheme::create_frost_glass() {
    StreamTheme t;
    t.metadata.name = "Frost Glass";
    t.metadata.author = "Antigravity";
    t.metadata.version = 1;
    t.metadata.description = "Clean light translucent frosted ice theme";

    t.overlay.type = OverlayBgType::Acrylic;
    t.overlay.color = "#E8EEF5";
    t.overlay.opacity = 0.35f;
    t.overlay.blur = 25.0f;
    t.overlay.corner_radius = 24.0f;

    t.username.font = { "Outfit", 15.0f, 600 };
    t.username.color_mode = UsernameColorMode::PersistentUserColor;
    t.username.palette = { "#0284C7", "#0D9488", "#4F46E5", "#D97706", "#DC2626" };

    t.message_text.font = { "Outfit", 15.0f, 400 };
    t.message_text.color = "#0F172A";

    t.message_background.enabled = true;
    t.message_background.color = "rgba(255, 255, 255, 0.45)";
    t.message_background.corner_radius = 16.0f;

    return t;
}

StreamTheme StreamTheme::create_sunset_glow() {
    StreamTheme t;
    t.metadata.name = "Sunset Glow";
    t.metadata.author = "Antigravity";
    t.metadata.version = 1;
    t.metadata.description = "Warm orange & purple dusk gradient aesthetic";

    t.overlay.type = OverlayBgType::Acrylic;
    t.overlay.color = "#1F121E";
    t.overlay.opacity = 0.5f;
    t.overlay.blur = 18.0f;
    t.overlay.corner_radius = 20.0f;

    t.username.font = { "Segoe UI", 16.0f, 600 };
    t.username.palette = { "#FB923C", "#F472B6", "#E879F9", "#FBBF24", "#F87171" };
    t.username.effect = { "glow", "0 0 5px rgba(251, 146, 60, 0.7)" };

    t.message_text.font = { "Segoe UI", 15.0f, 400 };
    t.message_text.color = "#FFF1F2";

    t.message_background.enabled = true;
    t.message_background.color = "rgba(255, 255, 255, 0.04)";
    t.message_background.corner_radius = 14.0f;

    return t;
}

StreamTheme StreamTheme::create_monochrome() {
    StreamTheme t;
    t.metadata.name = "Monochrome Minimal";
    t.metadata.author = "Antigravity";
    t.metadata.version = 1;
    t.metadata.description = "Ultra sleek stealth black & white aesthetic";

    t.overlay.type = OverlayBgType::Solid;
    t.overlay.color = "#000000";
    t.overlay.opacity = 0.6f;
    t.overlay.blur = 0.0f;
    t.overlay.corner_radius = 12.0f;

    t.username.font = { "Segoe UI", 15.0f, 700 };
    t.username.color_mode = UsernameColorMode::Single;
    t.username.color_value = "#FFFFFF";

    t.message_text.font = { "Segoe UI", 15.0f, 400 };
    t.message_text.color = "#D1D5DB";

    t.message_background.enabled = true;
    t.message_background.color = "rgba(255, 255, 255, 0.06)";
    t.message_background.corner_radius = 8.0f;

    return t;
}
