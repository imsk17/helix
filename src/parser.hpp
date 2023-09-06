#include <utility>

#include "tokenization.hpp"

struct NodeExprIntLit {
    Token int_lit;
};

struct NodeExprIdent {
    Token ident;
};

struct NodeExpr {
    std::variant<NodeExprIntLit, NodeExprIdent> var;
};

struct NodeStmtExit {
    NodeExpr expr;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit, NodeStmtLet> var;
};

struct NodeProgram {
    std::vector<NodeStmt> statements;
};

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens))
    {
    }

    std::optional<NodeExpr> parse_expr()
    {
        if (peek().has_value() && peek().value().type == TokenType::int_lit) {
            return NodeExpr { .var = NodeExprIntLit { .int_lit = consume() } };
        }
        else if (peek().has_value() && peek().value().type == TokenType::identifier) {
            return NodeExpr { .var = NodeExprIdent { .ident = consume() } };
        }
        return {};
    }

    std::optional<NodeStmt> parse_stmt()
    {
        if (peek().value().type == TokenType::exit && peek(1).has_value()
            && peek(1).value().type == TokenType::open_parenthesis) {
            consume(); // Consume exit token
            consume(); // Consume open parenthesis token

            NodeStmtExit stmt_exit;
            if (auto expr = parse_expr()) {
                stmt_exit = { .expr = expr.value() };
            }
            else {
                std::cerr << "Expected `(`" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::close_parenthesis) {
                consume();
            }
            else {
                std::cerr << "Expected `)`" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::semi) {
                consume();
            }
            else {
                std::cerr << "Expected `;`" << std::endl;
                exit(EXIT_FAILURE);
            }
            return NodeStmt { .var = stmt_exit };
        }
        else if (
            peek().has_value() && peek().value().type == TokenType::let && peek(1).has_value()
            && peek(1).value().type == TokenType::identifier && peek(2).has_value()
            && peek(2).value().type == TokenType::eq) {
            consume();
            auto stmt = NodeStmtLet {
                .ident = consume(),
            };
            consume();
            if (auto expr = parse_expr()) {
                stmt.expr = expr.value();
            }
            else {
                std::cerr << "Invalid Expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::semi) {
                consume();
            }
            else {
                std::cerr << "Expected `;`" << std::endl;
                exit(EXIT_FAILURE);
            }
            return NodeStmt { .var = stmt };
        }
        else {
            return {};
        }
    }

    std::optional<NodeProgram> parse_program()
    {
        NodeProgram program;
        while (peek().has_value()) {
            if (auto stm = parse_stmt()) {
                program.statements.push_back(stm.value());
            }
            else {
                std::cerr << "Invalid Statement" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return program;
    }

private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    [[nodiscard]] inline std::optional<Token> peek(int offset = 0) const
    {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        }
        else {
            return m_tokens.at(m_index + offset);
        }
    }

    inline Token consume()
    {
        return m_tokens.at(m_index++);
    }
};