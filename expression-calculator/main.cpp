#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <variant>
#include <iomanip>

enum class TokenType { Number, Plus, Minus, Multiply, Divide, Modulo, Power, LParen, RParen };

struct Token {
    TokenType type;
    double value = 0;

    std::string to_string() const {
        switch (type) {
            case TokenType::Number:   return std::to_string(value);
            case TokenType::Plus:     return "+";
            case TokenType::Minus:    return "-";
            case TokenType::Multiply: return "*";
            case TokenType::Divide:   return "/";
            case TokenType::Modulo:   return "%";
            case TokenType::Power:    return "^";
            case TokenType::LParen:   return "(";
            case TokenType::RParen:   return ")";
        }
        return "?";
    }
};

class Tokenizer {
public:
    static std::vector<Token> tokenize(const std::string& expr) {
        std::vector<Token> tokens;
        size_t i = 0;

        while (i < expr.size()) {
            if (std::isspace(expr[i])) {
                ++i;
                continue;
            }

            if (std::isdigit(expr[i]) || expr[i] == '.') {
                tokens.push_back(parse_number(expr, i));
                continue;
            }

            // Unary minus: at start, after operator, or after '('
            if (expr[i] == '-' && (tokens.empty() ||
                tokens.back().type == TokenType::LParen ||
                is_operator(tokens.back().type))) {
                ++i;
                if (i < expr.size() && (std::isdigit(expr[i]) || expr[i] == '.')) {
                    Token num = parse_number(expr, i);
                    num.value = -num.value;
                    tokens.push_back(num);
                } else if (i < expr.size() && expr[i] == '(') {
                    // -(...) => push -1 * (
                    tokens.push_back({TokenType::Number, -1});
                    tokens.push_back({TokenType::Multiply});
                } else {
                    throw std::runtime_error("Unexpected character after unary minus");
                }
                continue;
            }

            switch (expr[i]) {
                case '+': tokens.push_back({TokenType::Plus}); break;
                case '-': tokens.push_back({TokenType::Minus}); break;
                case '*': tokens.push_back({TokenType::Multiply}); break;
                case '/': tokens.push_back({TokenType::Divide}); break;
                case '%': tokens.push_back({TokenType::Modulo}); break;
                case '^': tokens.push_back({TokenType::Power}); break;
                case '(': tokens.push_back({TokenType::LParen}); break;
                case ')': tokens.push_back({TokenType::RParen}); break;
                default:
                    throw std::runtime_error(
                        std::string("Unexpected character: '") + expr[i] + "'");
            }
            ++i;
        }

        return tokens;
    }

private:
    static Token parse_number(const std::string& expr, size_t& i) {
        size_t start = i;
        bool has_dot = false;
        while (i < expr.size() && (std::isdigit(expr[i]) || expr[i] == '.')) {
            if (expr[i] == '.') {
                if (has_dot) throw std::runtime_error("Invalid number: multiple decimal points");
                has_dot = true;
            }
            ++i;
        }
        double val = std::stod(expr.substr(start, i - start));
        return {TokenType::Number, val};
    }

    static bool is_operator(TokenType t) {
        return t == TokenType::Plus || t == TokenType::Minus ||
               t == TokenType::Multiply || t == TokenType::Divide ||
               t == TokenType::Modulo || t == TokenType::Power;
    }
};

class ShuntingYard {
public:
    // Returns tokens in Reverse Polish Notation
    static std::vector<Token> to_rpn(const std::vector<Token>& tokens) {
        std::vector<Token> output;
        std::stack<Token> ops;

        for (auto& tok : tokens) {
            if (tok.type == TokenType::Number) {
                output.push_back(tok);
            } else if (tok.type == TokenType::LParen) {
                ops.push(tok);
            } else if (tok.type == TokenType::RParen) {
                while (!ops.empty() && ops.top().type != TokenType::LParen) {
                    output.push_back(ops.top());
                    ops.pop();
                }
                if (ops.empty()) throw std::runtime_error("Mismatched parentheses");
                ops.pop();
            } else {
                while (!ops.empty() && ops.top().type != TokenType::LParen &&
                       should_pop(ops.top().type, tok.type)) {
                    output.push_back(ops.top());
                    ops.pop();
                }
                ops.push(tok);
            }
        }

        while (!ops.empty()) {
            if (ops.top().type == TokenType::LParen) {
                throw std::runtime_error("Mismatched parentheses");
            }
            output.push_back(ops.top());
            ops.pop();
        }

        return output;
    }

private:
    static int precedence(TokenType t) {
        switch (t) {
            case TokenType::Plus:
            case TokenType::Minus:    return 1;
            case TokenType::Multiply:
            case TokenType::Divide:
            case TokenType::Modulo:   return 2;
            case TokenType::Power:    return 3;
            default:                  return 0;
        }
    }

    // ^ is right-associative; all others are left-associative
    static bool should_pop(TokenType stack_op, TokenType new_op) {
        int sp = precedence(stack_op);
        int np = precedence(new_op);
        if (new_op == TokenType::Power) return sp > np;
        return sp >= np;
    }
};

class Evaluator {
public:
    static double evaluate(const std::vector<Token>& rpn) {
        std::stack<double> values;

        for (auto& tok : rpn) {
            if (tok.type == TokenType::Number) {
                values.push(tok.value);
                continue;
            }

            if (values.size() < 2) {
                throw std::runtime_error("Invalid expression");
            }
            double b = values.top(); values.pop();
            double a = values.top(); values.pop();

            switch (tok.type) {
                case TokenType::Plus:     values.push(a + b); break;
                case TokenType::Minus:    values.push(a - b); break;
                case TokenType::Multiply: values.push(a * b); break;
                case TokenType::Divide:
                    if (b == 0) throw std::runtime_error("Division by zero");
                    values.push(a / b);
                    break;
                case TokenType::Modulo:
                    if (b == 0) throw std::runtime_error("Modulo by zero");
                    values.push(std::fmod(a, b));
                    break;
                case TokenType::Power:
                    values.push(std::pow(a, b));
                    break;
                default:
                    throw std::runtime_error("Unknown operator in RPN");
            }
        }

        if (values.size() != 1) {
            throw std::runtime_error("Invalid expression");
        }
        return values.top();
    }
};

std::string format_result(double val) {
    if (val == static_cast<long long>(val) && std::abs(val) < 1e15) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(0) << val;
        return oss.str();
    }
    std::ostringstream oss;
    oss << std::setprecision(10) << val;
    return oss.str();
}

void print_tokens(const std::vector<Token>& tokens, const std::string& label) {
    std::cout << label << ": ";
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) std::cout << " ";
        std::cout << tokens[i].to_string();
    }
    std::cout << "\n";
}

double calculate(const std::string& expr, bool verbose) {
    auto tokens = Tokenizer::tokenize(expr);
    if (tokens.empty()) throw std::runtime_error("Empty expression");

    if (verbose) {
        print_tokens(tokens, "Tokens");
    }

    auto rpn = ShuntingYard::to_rpn(tokens);

    if (verbose) {
        print_tokens(rpn, "RPN   ");
    }

    double result = Evaluator::evaluate(rpn);

    if (verbose) {
        std::cout << "Result: " << format_result(result) << "\n";
    }

    return result;
}

void repl(bool verbose) {
    std::cout << "Expression Calculator (type 'quit' to exit)\n\n";

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;

        std::string trimmed = line;
        while (!trimmed.empty() && std::isspace(trimmed.front())) trimmed.erase(trimmed.begin());
        while (!trimmed.empty() && std::isspace(trimmed.back())) trimmed.pop_back();

        if (trimmed.empty()) continue;
        if (trimmed == "quit" || trimmed == "exit" || trimmed == "q") break;

        try {
            double result = calculate(trimmed, verbose);
            if (!verbose) {
                std::cout << "= " << format_result(result) << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}

void print_usage(const char* prog) {
    std::cerr << "Usage:\n";
    std::cerr << "  " << prog << "                  Interactive REPL mode\n";
    std::cerr << "  " << prog << " \"expression\"     Evaluate single expression\n";
    std::cerr << "  " << prog << " -v \"expression\"   Verbose mode (show steps)\n";
    std::cerr << "  " << prog << " -v                 Verbose REPL mode\n";
}

int main(int argc, char* argv[]) {
    bool verbose = false;
    std::string expression;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else {
            expression = arg;
        }
    }

    if (expression.empty()) {
        repl(verbose);
    } else {
        try {
            double result = calculate(expression, verbose);
            if (!verbose) {
                std::cout << format_result(result) << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

    return 0;
}
