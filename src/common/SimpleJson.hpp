#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <iomanip>
#include <memory>

namespace json {

enum class Type {
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object
};

class Value {
public:
    Type type = Type::Null;
    bool bool_val = false;
    double num_val = 0.0;
    std::string str_val;
    std::vector<Value> arr_val;
    std::map<std::string, Value> obj_val;

    Value() : type(Type::Null) {}
    Value(bool b) : type(Type::Boolean), bool_val(b) {}
    Value(int n) : type(Type::Number), num_val(static_cast<double>(n)) {}
    Value(double n) : type(Type::Number), num_val(n) {}
    Value(const char* s) : type(Type::String), str_val(s ? s : "") {}
    Value(const std::string& s) : type(Type::String), str_val(s) {}

    static Value object() {
        Value v;
        v.type = Type::Object;
        return v;
    }

    static Value array() {
        Value v;
        v.type = Type::Array;
        return v;
    }

    bool is_null() const { return type == Type::Null; }
    bool is_bool() const { return type == Type::Boolean; }
    bool is_number() const { return type == Type::Number; }
    bool is_string() const { return type == Type::String; }
    bool is_array() const { return type == Type::Array; }
    bool is_object() const { return type == Type::Object; }

    bool as_bool(bool default_val = false) const {
        return is_bool() ? bool_val : default_val;
    }

    double as_double(double default_val = 0.0) const {
        return is_number() ? num_val : default_val;
    }

    int as_int(int default_val = 0) const {
        return is_number() ? static_cast<int>(num_val) : default_val;
    }

    std::string as_string(const std::string& default_val = "") const {
        return is_string() ? str_val : default_val;
    }

    bool contains(const std::string& key) const {
        if (!is_object()) return false;
        return obj_val.find(key) != obj_val.end();
    }

    Value& operator[](const std::string& key) {
        if (type != Type::Object) {
            type = Type::Object;
            obj_val.clear();
        }
        return obj_val[key];
    }

    const Value& operator[](const std::string& key) const {
        static const Value null_val;
        if (!is_object()) return null_val;
        auto it = obj_val.find(key);
        return it != obj_val.end() ? it->second : null_val;
    }

    Value& operator[](size_t index) {
        if (type != Type::Array) {
            type = Type::Array;
            arr_val.clear();
        }
        if (index >= arr_val.size()) arr_val.resize(index + 1);
        return arr_val[index];
    }

    const Value& operator[](size_t index) const {
        static const Value null_val;
        if (!is_array() || index >= arr_val.size()) return null_val;
        return arr_val[index];
    }

    void push_back(const Value& v) {
        if (type != Type::Array) {
            type = Type::Array;
            arr_val.clear();
        }
        arr_val.push_back(v);
    }

    size_t size() const {
        if (is_array()) return arr_val.size();
        if (is_object()) return obj_val.size();
        return 0;
    }

    std::string dump(int indent = 0, int current_indent = 0) const {
        std::ostringstream ss;
        std::string ind_str(current_indent, ' ');
        std::string next_ind_str(current_indent + indent, ' ');

        switch (type) {
            case Type::Null:
                ss << "null";
                break;
            case Type::Boolean:
                ss << (bool_val ? "true" : "false");
                break;
            case Type::Number:
                if (num_val == static_cast<int>(num_val))
                    ss << static_cast<int>(num_val);
                else
                    ss << num_val;
                break;
            case Type::String: {
                ss << '"';
                for (char c : str_val) {
                    if (c == '"') ss << "\\\"";
                    else if (c == '\\') ss << "\\\\";
                    else if (c == '\b') ss << "\\b";
                    else if (c == '\f') ss << "\\f";
                    else if (c == '\n') ss << "\\n";
                    else if (c == '\r') ss << "\\r";
                    else if (c == '\t') ss << "\\t";
                    else if (static_cast<unsigned char>(c) < 32) {
                        ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c << std::dec;
                    } else {
                        ss << c;
                    }
                }
                ss << '"';
                break;
            }
            case Type::Array: {
                if (arr_val.empty()) {
                    ss << "[]";
                } else {
                    ss << "[";
                    if (indent > 0) ss << "\n";
                    for (size_t i = 0; i < arr_val.size(); ++i) {
                        if (indent > 0) ss << next_ind_str;
                        ss << arr_val[i].dump(indent, current_indent + indent);
                        if (i + 1 < arr_val.size()) ss << ",";
                        if (indent > 0) ss << "\n";
                    }
                    if (indent > 0) ss << ind_str;
                    ss << "]";
                }
                break;
            }
            case Type::Object: {
                if (obj_val.empty()) {
                    ss << "{}";
                } else {
                    ss << "{";
                    if (indent > 0) ss << "\n";
                    size_t count = 0;
                    for (auto it = obj_val.begin(); it != obj_val.end(); ++it, ++count) {
                        if (indent > 0) ss << next_ind_str;
                        Value key_v(it->first);
                        ss << key_v.dump() << ": ";
                        ss << it->second.dump(indent, current_indent + indent);
                        if (count + 1 < obj_val.size()) ss << ",";
                        if (indent > 0) ss << "\n";
                    }
                    if (indent > 0) ss << ind_str;
                    ss << "}";
                }
                break;
            }
        }
        return ss.str();
    }

    std::string serialize(int indent = 0) const {
        return dump(indent);
    }
};

class Parser {
private:
    std::string src_;
    size_t pos_ = 0;

    void skip_whitespace() {
        while (pos_ < src_.size()) {
            if (std::isspace(static_cast<unsigned char>(src_[pos_]))) {
                pos_++;
            } else if (src_[pos_] == '/' && pos_ + 1 < src_.size() && src_[pos_ + 1] == '/') {
                // Single line comment // ...
                pos_ += 2;
                while (pos_ < src_.size() && src_[pos_] != '\n' && src_[pos_] != '\r') {
                    pos_++;
                }
            } else if (src_[pos_] == '/' && pos_ + 1 < src_.size() && src_[pos_ + 1] == '*') {
                // Multi line comment /* ... */
                pos_ += 2;
                while (pos_ + 1 < src_.size() && !(src_[pos_] == '*' && src_[pos_ + 1] == '/')) {
                    pos_++;
                }
                if (pos_ + 1 < src_.size()) pos_ += 2;
                else pos_ = src_.size();
            } else {
                break;
            }
        }
    }

    char peek() {
        skip_whitespace();
        if (pos_ >= src_.size()) return '\0';
        return src_[pos_];
    }

    char get() {
        skip_whitespace();
        if (pos_ >= src_.size()) return '\0';
        return src_[pos_++];
    }

    Value parse_null() {
        if (src_.compare(pos_, 4, "null") == 0) {
            pos_ += 4;
            return Value();
        }
        return Value();
    }

    Value parse_bool() {
        if (src_.compare(pos_, 4, "true") == 0) {
            pos_ += 4;
            return Value(true);
        }
        if (src_.compare(pos_, 5, "false") == 0) {
            pos_ += 5;
            return Value(false);
        }
        return Value(false);
    }

    Value parse_number() {
        size_t start = pos_;
        if (pos_ < src_.size() && (src_[pos_] == '-' || src_[pos_] == '+')) pos_++;
        while (pos_ < src_.size() && (std::isdigit(static_cast<unsigned char>(src_[pos_])) || src_[pos_] == '.' || src_[pos_] == 'e' || src_[pos_] == 'E')) {
            pos_++;
        }
        std::string num_str = src_.substr(start, pos_ - start);
        try {
            double d = std::stod(num_str);
            return Value(d);
        } catch (...) {
            return Value(0.0);
        }
    }

    Value parse_string() {
        if (get() != '"') return Value("");
        std::string res;
        while (pos_ < src_.size()) {
            char c = src_[pos_++];
            if (c == '"') break;
            if (c == '\\') {
                if (pos_ >= src_.size()) break;
                char esc = src_[pos_++];
                if (esc == '"') res += '"';
                else if (esc == '\\') res += '\\';
                else if (esc == '/') res += '/';
                else if (esc == 'b') res += '\b';
                else if (esc == 'f') res += '\f';
                else if (esc == 'n') res += '\n';
                else if (esc == 'r') res += '\r';
                else if (esc == 't') res += '\t';
                else if (esc == 'u') {
                    if (pos_ + 4 <= src_.size()) {
                        std::string hex_str = src_.substr(pos_, 4);
                        pos_ += 4;
                        try {
                            uint32_t cp = std::stoul(hex_str, nullptr, 16);
                            if (cp < 0x80) {
                                res += (char)cp;
                            } else if (cp < 0x800) {
                                res += (char)(0xC0 | (cp >> 6));
                                res += (char)(0x80 | (cp & 0x3F));
                            } else {
                                res += (char)(0xE0 | (cp >> 12));
                                res += (char)(0x80 | ((cp >> 6) & 0x3F));
                                res += (char)(0x80 | (cp & 0x3F));
                            }
                        } catch (...) {}
                    }
                }
            } else {
                res += c;
            }
        }
        return Value(res);
    }

    Value parse_array() {
        if (get() != '[') return Value();
        Value arr;
        arr.type = Type::Array;
        if (peek() == ']') {
            get();
            return arr;
        }

        while (true) {
            arr.push_back(parse_value());
            char c = peek();
            if (c == ']') {
                get();
                break;
            }
            if (c == ',') {
                get();
            } else {
                break;
            }
        }
        return arr;
    }

    Value parse_object() {
        if (get() != '{') return Value();
        Value obj;
        obj.type = Type::Object;
        if (peek() == '}') {
            get();
            return obj;
        }

        while (true) {
            Value key = parse_string();
            if (get() != ':') break;
            Value val = parse_value();
            obj[key.as_string()] = val;

            char c = peek();
            if (c == '}') {
                get();
                break;
            }
            if (c == ',') {
                get();
            } else {
                break;
            }
        }
        return obj;
    }

public:
    Parser(const std::string& src) : src_(src) {}

    Value parse_value() {
        char c = peek();
        if (c == '{') return parse_object();
        if (c == '[') return parse_array();
        if (c == '"') return parse_string();
        if (c == 't' || c == 'f') return parse_bool();
        if (c == 'n') return parse_null();
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '-' || c == '+') return parse_number();
        return Value();
    }
};

inline Value parse(const std::string& json_str) {
    Parser p(json_str);
    return p.parse_value();
}

} // namespace json
