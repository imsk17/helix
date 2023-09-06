#pragma once

#include "string"
#include "vector"

enum class TokenType { exit, int_lit, semi };

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
                    continue;
                }
                else {
                    std::cerr << "You Messed Up.";
                    exit(EXIT_FAILURE);
                }
            }
            else if (std::isdigit(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value())) {
                    buf.push_back(consume());
                }
                tokens.push_back({ .type = TokenType::int_lit, .value = buf });
                buf.clear();
                continue;
            }
            else if (peek().value() == ';') {
                consume();
                tokens.push_back({ .type = TokenType::semi });
            }
            else if (std::isspace(peek().value())) {
                consume();
                continue;
            }
            else {
                std::cerr<< "You Messed Up.";
                exit(EXIT_FAILURE);
            }
        }
        m_index = 0;
        return tokens;
    }

private:
    const std::string m_str;
    int m_index = 0 ;
    [[nodiscard]] inline std::optional<char> peek(int ahead = 1) const
    {
        if (m_index + ahead > m_str.length()) {
            return {};
        }
        else {
            return m_str.at(m_index);
        }
    }

    inline char consume()
    {
        return m_str.at(m_index++);
    }
};