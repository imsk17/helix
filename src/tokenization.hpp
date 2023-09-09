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
    sub
};

bool is_binary_operation(TokenType type)
{
    switch (type) {
    case TokenType::div:
    case TokenType::sub:
    case TokenType::plus:
    case TokenType::star:
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
        return 1;
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
            else if (peek().value() == '*') {
                consume();
                tokens.push_back({ .type = TokenType::star });
            }
            else if (peek().value() == '/') {
                consume();
                tokens.push_back({ .type = TokenType::div });
            }
            else if (peek().value() == '-') {
                consume();
                tokens.push_back({ .type = TokenType::sub });
            }
            else if (peek().value() == '=') {
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