#pragma once

#include "string"
#include "vector"

enum class TokenType {
    exit,
    int_lit,
    semi,
    open_parenthesis,
    close_parenthesis,
    identifier,
    let,
    eq,
    plus,
    star,
    div,
    sub,
    modulo,
    lt,
    gt,
    lte,
    gte,
    equality,
    not_equality,
    open_curly,
    close_curly
};

bool is_binary_operation(TokenType type)
{
    switch (type) {
    case TokenType::div:
    case TokenType::sub:
    case TokenType::plus:
    case TokenType::star:
    case TokenType::modulo:
    case TokenType::lt:
    case TokenType::gt:
    case TokenType::lte:
    case TokenType::gte:
    case TokenType::equality:
    case TokenType::not_equality:
        return true;
    default:
        return false;
    }
}

std::optional<int> bin_prec(TokenType type)
{
    switch (type) {
    case TokenType::sub:
    case TokenType::plus:
        return 0;
    case TokenType::div:
    case TokenType::star:
    case TokenType::modulo:
        return 1;
    case TokenType::lt:
    case TokenType::gt:
    case TokenType::lte:
    case TokenType::gte:
        return 2;
    case TokenType::equality:
    case TokenType::not_equality:
        return 3;
    default:
        return {};
    }
}

struct Token {
    TokenType type;
    std::optional<std::string> value {};
};

class Tokenizer {
public:
    inline explicit Tokenizer(std::string src)
        : m_str(std::move(src))
    {
    }

    inline std::vector<Token> tokenize()
    {
        std::vector<Token> tokens;
        std::string buf;
        while (peek().has_value()) {
            if (std::isalpha(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && isalnum(peek().value())) {
                    buf.push_back(consume());
                }
                if (buf == "exit") {
                    tokens.push_back({ .type = TokenType::exit });
                    buf.clear();
                }
                else if (buf == "let") {
                    tokens.push_back({ .type = TokenType::let });
                    buf.clear();
                }
                else {
                    tokens.push_back({ .type = TokenType::identifier, .value = buf });
                    buf.clear();
                }
            }
            else if (peek().value() == '(') {
                consume();
                tokens.push_back({ .type = TokenType::open_parenthesis });
            }
            else if (peek().value() == '+') {
                consume();
                tokens.push_back({ .type = TokenType::plus });
            }
            else if (peek().value() == '<') {
                if (peek(1).has_value() && peek(1).value() == '=') {
                    consume();
                    consume();
                    tokens.push_back({ .type = TokenType::lte });
                    continue;
                }
                consume();
                tokens.push_back({ .type = TokenType::lt });
            }
            else if (peek().value() == '{') {
                consume();
                tokens.push_back({ .type = TokenType::open_curly });
            }
            else if (peek().value() == '}') {
                consume();
                tokens.push_back({ .type = TokenType::close_curly });
            }
            else if (peek().value() == '>') {
                if (peek(1).has_value() && peek(1).value() == '=') {
                    consume();
                    consume();
                    tokens.push_back({ .type = TokenType::gte });
                    continue;
                }
                consume();
                tokens.push_back({ .type = TokenType::gt });
            }
            else if (peek().value() == '*') {
                consume();
                tokens.push_back({ .type = TokenType::star });
            }
            else if (peek().value() == '!' && peek(1).has_value() && peek(1).value() == '=') {
                consume();
                consume();
                tokens.push_back({ .type = TokenType::not_equality });
            }
            else if (peek().value() == '%') {
                consume();
                tokens.push_back({ .type = TokenType::modulo });
            }
            else if (peek().value() == '/') {
                // Single Line Comments `//`
                if (peek(1).has_value() && peek(1).value() == '/') {
                    consume(); // Consume first slash
                    consume(); // Consume second slash
                    while (peek().has_value() && peek().value() != '\n') {
                        consume();
                    }
                    continue;
                }
                consume();
                tokens.push_back({ .type = TokenType::div });
            }
            else if (peek().value() == '-') {
                consume();
                tokens.push_back({ .type = TokenType::sub });
            }
            else if (peek().value() == '=') {
                if (peek(1).has_value() && peek(1).value() == '=') {
                    consume();
                    consume();
                    tokens.push_back({ .type = TokenType::equality });
                    continue;
                }
                consume();
                tokens.push_back({ .type = TokenType::eq });
            }
            else if (peek().value() == ')') {
                consume();
                tokens.push_back({ .type = TokenType::close_parenthesis });
            }
            else if (std::isdigit(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value())) {
                    buf.push_back(consume());
                }
                tokens.push_back({ .type = TokenType::int_lit, .value = buf });
                buf.clear();
            }
            else if (peek().value() == ';') {
                consume();
                tokens.push_back({ .type = TokenType::semi });
            }
            else if (std::isspace(peek().value())) {
                consume();
            }
            else {
                std::cerr << "You Messed Up.";
                exit(EXIT_FAILURE);
            }
        }
        m_index = 0;
        return tokens;
    }

private:
    const std::string m_str;
    int m_index = 0;
    [[nodiscard]] inline std::optional<char> peek(int offset = 0) const
    {
        if (m_index + offset >= m_str.length()) {
            return {};
        }
        else {
            return m_str.at(m_index + offset);
        }
    }

    inline char consume()
    {
        return m_str.at(m_index++);
    }
};