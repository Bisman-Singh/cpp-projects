#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>

class MarkdownParser {
public:
    std::string parse(const std::string& markdown) {
        std::istringstream stream(markdown);
        std::string line;
        std::string html;
        std::vector<std::string> lines;

        while (std::getline(stream, line)) {
            lines.push_back(line);
        }

        size_t i = 0;
        while (i < lines.size()) {
            // Code block
            if (lines[i].substr(0, 3) == "```") {
                std::string lang;
                if (lines[i].size() > 3) {
                    lang = lines[i].substr(3);
                }
                std::string code;
                ++i;
                while (i < lines.size() && lines[i].substr(0, 3) != "```") {
                    if (!code.empty()) code += "\n";
                    code += escape_html(lines[i]);
                    ++i;
                }
                if (i < lines.size()) ++i;
                if (lang.empty()) {
                    html += "<pre><code>" + code + "</code></pre>\n";
                } else {
                    html += "<pre><code class=\"language-" + lang + "\">" + code + "</code></pre>\n";
                }
                continue;
            }

            // Blank line
            if (lines[i].empty()) {
                ++i;
                continue;
            }

            // Horizontal rule
            if (is_horizontal_rule(lines[i])) {
                html += "<hr>\n";
                ++i;
                continue;
            }

            // Heading
            if (lines[i][0] == '#') {
                html += parse_heading(lines[i]) + "\n";
                ++i;
                continue;
            }

            // Blockquote
            if (lines[i].size() >= 2 && lines[i][0] == '>') {
                std::string bq_content;
                while (i < lines.size() && !lines[i].empty() && lines[i][0] == '>') {
                    std::string bq_line = lines[i].substr(1);
                    if (!bq_line.empty() && bq_line[0] == ' ') bq_line = bq_line.substr(1);
                    if (!bq_content.empty()) bq_content += "\n";
                    bq_content += bq_line;
                    ++i;
                }
                html += "<blockquote><p>" + parse_inline(bq_content) + "</p></blockquote>\n";
                continue;
            }

            // Unordered list
            if (is_unordered_list_item(lines[i])) {
                html += "<ul>\n";
                while (i < lines.size() && is_unordered_list_item(lines[i])) {
                    std::string item = lines[i].substr(2);
                    html += "  <li>" + parse_inline(item) + "</li>\n";
                    ++i;
                }
                html += "</ul>\n";
                continue;
            }

            // Ordered list
            if (is_ordered_list_item(lines[i])) {
                html += "<ol>\n";
                while (i < lines.size() && is_ordered_list_item(lines[i])) {
                    auto dot_pos = lines[i].find(". ");
                    std::string item = lines[i].substr(dot_pos + 2);
                    html += "  <li>" + parse_inline(item) + "</li>\n";
                    ++i;
                }
                html += "</ol>\n";
                continue;
            }

            // Paragraph: collect consecutive non-special lines
            std::string para;
            while (i < lines.size() && !lines[i].empty() &&
                   lines[i][0] != '#' && !is_horizontal_rule(lines[i]) &&
                   lines[i][0] != '>' && !is_unordered_list_item(lines[i]) &&
                   !is_ordered_list_item(lines[i]) && lines[i].substr(0, 3) != "```") {
                if (!para.empty()) para += " ";
                para += lines[i];
                ++i;
            }
            html += "<p>" + parse_inline(para) + "</p>\n";
        }

        return html;
    }

private:
    static std::string escape_html(const std::string& text) {
        std::string result;
        result.reserve(text.size());
        for (char c : text) {
            switch (c) {
                case '&': result += "&amp;"; break;
                case '<': result += "&lt;"; break;
                case '>': result += "&gt;"; break;
                case '"': result += "&quot;"; break;
                default: result += c;
            }
        }
        return result;
    }

    static bool is_horizontal_rule(const std::string& line) {
        std::string trimmed;
        for (char c : line) {
            if (c != ' ') trimmed += c;
        }
        if (trimmed.size() < 3) return false;
        return (trimmed.find_first_not_of('-') == std::string::npos) ||
               (trimmed.find_first_not_of('*') == std::string::npos) ||
               (trimmed.find_first_not_of('_') == std::string::npos);
    }

    static bool is_unordered_list_item(const std::string& line) {
        return line.size() >= 2 && (line[0] == '-' || line[0] == '*' || line[0] == '+') && line[1] == ' ';
    }

    static bool is_ordered_list_item(const std::string& line) {
        size_t i = 0;
        while (i < line.size() && std::isdigit(line[i])) ++i;
        return i > 0 && i < line.size() - 1 && line[i] == '.' && line[i + 1] == ' ';
    }

    std::string parse_heading(const std::string& line) {
        size_t level = 0;
        while (level < line.size() && level < 6 && line[level] == '#') ++level;
        std::string text = line.substr(level);
        while (!text.empty() && text[0] == ' ') text = text.substr(1);
        return "<h" + std::to_string(level) + ">" + parse_inline(text) +
               "</h" + std::to_string(level) + ">";
    }

    std::string parse_inline(const std::string& text) {
        std::string result;
        size_t i = 0;

        while (i < text.size()) {
            // Links: [text](url)
            if (text[i] == '[') {
                auto close_bracket = text.find(']', i);
                if (close_bracket != std::string::npos && close_bracket + 1 < text.size() &&
                    text[close_bracket + 1] == '(') {
                    auto close_paren = text.find(')', close_bracket + 2);
                    if (close_paren != std::string::npos) {
                        std::string link_text = text.substr(i + 1, close_bracket - i - 1);
                        std::string url = text.substr(close_bracket + 2, close_paren - close_bracket - 2);
                        result += "<a href=\"" + url + "\">" + escape_html(link_text) + "</a>";
                        i = close_paren + 1;
                        continue;
                    }
                }
            }

            // Inline code
            if (text[i] == '`') {
                auto end = text.find('`', i + 1);
                if (end != std::string::npos) {
                    result += "<code>" + escape_html(text.substr(i + 1, end - i - 1)) + "</code>";
                    i = end + 1;
                    continue;
                }
            }

            // Bold: **text**
            if (i + 1 < text.size() && text[i] == '*' && text[i + 1] == '*') {
                auto end = text.find("**", i + 2);
                if (end != std::string::npos) {
                    result += "<strong>" + parse_inline(text.substr(i + 2, end - i - 2)) + "</strong>";
                    i = end + 2;
                    continue;
                }
            }

            // Italic: *text*
            if (text[i] == '*') {
                auto end = text.find('*', i + 1);
                if (end != std::string::npos && text[end - 1] != '*') {
                    result += "<em>" + parse_inline(text.substr(i + 1, end - i - 1)) + "</em>";
                    i = end + 1;
                    continue;
                }
            }

            // Escape special HTML chars in regular text
            switch (text[i]) {
                case '&': result += "&amp;"; break;
                case '<': result += "&lt;"; break;
                case '>': result += "&gt;"; break;
                default: result += text[i];
            }
            ++i;
        }

        return result;
    }
};

std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void write_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot write to file: " + path);
    }
    file << content;
}

void print_usage(const char* prog) {
    std::cerr << "Usage: " << prog << " <input.md> [-o output.html]\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string input_path = argv[1];
    std::string output_path;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            output_path = argv[++i];
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    try {
        std::string markdown = read_file(input_path);
        MarkdownParser parser;
        std::string html = parser.parse(markdown);

        if (output_path.empty()) {
            std::cout << html;
        } else {
            write_file(output_path, html);
            std::cout << "Written to " << output_path << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
