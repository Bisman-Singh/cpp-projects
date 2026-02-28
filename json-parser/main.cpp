#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <iomanip>
#include <cmath>

class JsonValue;
using JsonNull = std::nullptr_t;
using JsonBool = bool;
using JsonInt = long long;
using JsonFloat = double;
using JsonString = std::string;
using JsonArray = std::vector<std::shared_ptr<JsonValue>>;
using JsonObject = std::vector<std::pair<std::string, std::shared_ptr<JsonValue>>>;

class JsonValue {
public:
    using Value = std::variant<JsonNull, JsonBool, JsonInt, JsonFloat, JsonString, JsonArray, JsonObject>;
    Value data;

    JsonValue() : data(nullptr) {}
    explicit JsonValue(Value v) : data(std::move(v)) {}

    bool is_null() const { return std::holds_alternative<JsonNull>(data); }
    bool is_bool() const { return std::holds_alternative<JsonBool>(data); }
    bool is_int() const { return std::holds_alternative<JsonInt>(data); }
    bool is_float() const { return std::holds_alternative<JsonFloat>(data); }
    bool is_string() const { return std::holds_alternative<JsonString>(data); }
    bool is_array() const { return std::holds_alternative<JsonArray>(data); }
    bool is_object() const { return std::holds_alternative<JsonObject>(data); }

    std::string to_string(int indent = 0, int current = 0) const {
        std::ostringstream oss;
        std::string pad(static_cast<size_t>(current), ' ');
        std::string inner(static_cast<size_t>(current + indent), ' ');

        if (is_null()) {
            oss << "null";
        } else if (is_bool()) {
            oss << (std::get<JsonBool>(data) ? "true" : "false");
        } else if (is_int()) {
            oss << std::get<JsonInt>(data);
        } else if (is_float()) {
            double val = std::get<JsonFloat>(data);
            if (val == std::floor(val) && std::abs(val) < 1e15) {
                oss << std::fixed << std::setprecision(1) << val;
            } else {
                oss << val;
            }
        } else if (is_string()) {
            oss << "\"" << escape_string(std::get<JsonString>(data)) << "\"";
        } else if (is_array()) {
            const auto& arr = std::get<JsonArray>(data);
            if (arr.empty()) {
                oss << "[]";
            } else {
                oss << "[\n";
                for (size_t i = 0; i < arr.size(); ++i) {
                    oss << inner << arr[i]->to_string(indent, current + indent);
                    if (i + 1 < arr.size()) oss << ",";
                    oss << "\n";
                }
                oss << pad << "]";
            }
        } else if (is_object()) {
            const auto& obj = std::get<JsonObject>(data);
            if (obj.empty()) {
                oss << "{}";
            } else {
                oss << "{\n";
                for (size_t i = 0; i < obj.size(); ++i) {
                    oss << inner << "\"" << escape_string(obj[i].first) << "\": "
                        << obj[i].second->to_string(indent, current + indent);
                    if (i + 1 < obj.size()) oss << ",";
                    oss << "\n";
                }
                oss << pad << "}";
            }
        }
        return oss.str();
    }

private:
    static std::string escape_string(const std::string& s) {
        std::string result;
        result.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '\"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c;
            }
        }
        return result;
    }
};

class JsonParser {
public:
    std::shared_ptr<JsonValue> parse(const std::string& input) {
        src_ = input;
        pos_ = 0;
        skip_whitespace();
        auto result = parse_value();
        skip_whitespace();
        if (pos_ < src_.size()) {
            throw std::runtime_error("Unexpected trailing characters at position " + std::to_string(pos_));
        }
        return result;
    }

private:
    std::string src_;
    size_t pos_ = 0;

    char peek() const {
        if (pos_ >= src_.size()) throw std::runtime_error("Unexpected end of input");
        return src_[pos_];
    }

    char advance() {
        char c = peek();
        ++pos_;
        return c;
    }

    void expect(char c) {
        char got = advance();
        if (got != c) {
            throw std::runtime_error(
                std::string("Expected '") + c + "' but got '" + got + "' at position " + std::to_string(pos_ - 1));
        }
    }

    void skip_whitespace() {
        while (pos_ < src_.size() && std::isspace(static_cast<unsigned char>(src_[pos_]))) {
            ++pos_;
        }
    }

    std::shared_ptr<JsonValue> parse_value() {
        skip_whitespace();
        char c = peek();

        if (c == 'n') return parse_null();
        if (c == 't' || c == 'f') return parse_bool();
        if (c == '"') return parse_string_value();
        if (c == '[') return parse_array();
        if (c == '{') return parse_object();
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parse_number();

        throw std::runtime_error(
            std::string("Unexpected character '") + c + "' at position " + std::to_string(pos_));
    }

    std::shared_ptr<JsonValue> parse_null() {
        for (char expected : {'n', 'u', 'l', 'l'}) expect(expected);
        return std::make_shared<JsonValue>(JsonValue::Value{nullptr});
    }

    std::shared_ptr<JsonValue> parse_bool() {
        if (peek() == 't') {
            for (char expected : {'t', 'r', 'u', 'e'}) expect(expected);
            return std::make_shared<JsonValue>(JsonValue::Value{true});
        }
        for (char expected : {'f', 'a', 'l', 's', 'e'}) expect(expected);
        return std::make_shared<JsonValue>(JsonValue::Value{false});
    }

    std::string parse_raw_string() {
        expect('"');
        std::string result;
        while (true) {
            char c = advance();
            if (c == '"') break;
            if (c == '\\') {
                char esc = advance();
                switch (esc) {
                    case '"':  result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/':  result += '/'; break;
                    case 'n':  result += '\n'; break;
                    case 'r':  result += '\r'; break;
                    case 't':  result += '\t'; break;
                    case 'b':  result += '\b'; break;
                    case 'f':  result += '\f'; break;
                    case 'u': {
                        std::string hex;
                        for (int i = 0; i < 4; ++i) hex += advance();
                        unsigned long cp = std::stoul(hex, nullptr, 16);
                        if (cp < 0x80) {
                            result += static_cast<char>(cp);
                        } else if (cp < 0x800) {
                            result += static_cast<char>(0xC0 | (cp >> 6));
                            result += static_cast<char>(0x80 | (cp & 0x3F));
                        } else {
                            result += static_cast<char>(0xE0 | (cp >> 12));
                            result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                            result += static_cast<char>(0x80 | (cp & 0x3F));
                        }
                        break;
                    }
                    default:
                        throw std::runtime_error(std::string("Invalid escape: \\") + esc);
                }
            } else {
                result += c;
            }
        }
        return result;
    }

    std::shared_ptr<JsonValue> parse_string_value() {
        return std::make_shared<JsonValue>(JsonValue::Value{parse_raw_string()});
    }

    std::shared_ptr<JsonValue> parse_number() {
        size_t start = pos_;
        bool is_float = false;

        if (peek() == '-') advance();
        if (!std::isdigit(static_cast<unsigned char>(peek()))) {
            throw std::runtime_error("Invalid number at position " + std::to_string(pos_));
        }
        while (pos_ < src_.size() && std::isdigit(static_cast<unsigned char>(src_[pos_]))) advance();

        if (pos_ < src_.size() && src_[pos_] == '.') {
            is_float = true;
            advance();
            if (pos_ >= src_.size() || !std::isdigit(static_cast<unsigned char>(src_[pos_]))) {
                throw std::runtime_error("Invalid number at position " + std::to_string(pos_));
            }
            while (pos_ < src_.size() && std::isdigit(static_cast<unsigned char>(src_[pos_]))) advance();
        }

        if (pos_ < src_.size() && (src_[pos_] == 'e' || src_[pos_] == 'E')) {
            is_float = true;
            advance();
            if (pos_ < src_.size() && (src_[pos_] == '+' || src_[pos_] == '-')) advance();
            while (pos_ < src_.size() && std::isdigit(static_cast<unsigned char>(src_[pos_]))) advance();
        }

        std::string num_str = src_.substr(start, pos_ - start);
        if (is_float) {
            return std::make_shared<JsonValue>(JsonValue::Value{std::stod(num_str)});
        }
        return std::make_shared<JsonValue>(JsonValue::Value{std::stoll(num_str)});
    }

    std::shared_ptr<JsonValue> parse_array() {
        expect('[');
        skip_whitespace();
        JsonArray arr;

        if (peek() != ']') {
            arr.push_back(parse_value());
            skip_whitespace();
            while (peek() == ',') {
                advance();
                arr.push_back(parse_value());
                skip_whitespace();
            }
        }
        expect(']');
        return std::make_shared<JsonValue>(JsonValue::Value{std::move(arr)});
    }

    std::shared_ptr<JsonValue> parse_object() {
        expect('{');
        skip_whitespace();
        JsonObject obj;

        if (peek() != '}') {
            skip_whitespace();
            std::string key = parse_raw_string();
            skip_whitespace();
            expect(':');
            auto val = parse_value();
            obj.emplace_back(std::move(key), std::move(val));
            skip_whitespace();

            while (peek() == ',') {
                advance();
                skip_whitespace();
                key = parse_raw_string();
                skip_whitespace();
                expect(':');
                val = parse_value();
                obj.emplace_back(std::move(key), std::move(val));
                skip_whitespace();
            }
        }
        expect('}');
        return std::make_shared<JsonValue>(JsonValue::Value{std::move(obj)});
    }
};

void test_parse(JsonParser& parser, const std::string& label, const std::string& json) {
    std::cout << "=== " << label << " ===\n";
    std::cout << "Input:  " << json << "\n";
    try {
        auto val = parser.parse(json);
        std::cout << "Parsed:\n" << val->to_string(2) << "\n\n";
    } catch (const std::exception& e) {
        std::cout << "Error:  " << e.what() << "\n\n";
    }
}

int main() {
    JsonParser parser;

    test_parse(parser, "Simple Object",
        R"({"name": "Alice", "age": 30, "active": true})");

    test_parse(parser, "Nested Object",
        R"({"user": {"id": 42, "email": "alice@example.com"}, "roles": ["admin", "editor"]})");

    test_parse(parser, "Array of Mixed Types",
        R"([1, 2.5, "hello", true, false, null, [10, 20]])");

    test_parse(parser, "Complex Nested Structure",
        R"({
            "database": {
                "host": "localhost",
                "port": 5432,
                "credentials": {"user": "admin", "pass": "secret"}
            },
            "features": ["auth", "logging", "cache"],
            "debug": false,
            "version": 3.14,
            "metadata": null
        })");

    test_parse(parser, "Empty Structures",
        R"({"empty_array": [], "empty_object": {}})");

    test_parse(parser, "Unicode and Escapes",
        R"({"greeting": "Hello\nWorld", "tab": "col1\tcol2", "unicode": "\u0048\u0065\u006C\u006C\u006F"})");

    test_parse(parser, "Parse Error (intentional)",
        R"({"key": })");

    test_parse(parser, "Numbers",
        R"({"int": 42, "negative": -17, "float": 3.14159, "exponent": 1.5e10, "zero": 0})");

    return 0;
}
