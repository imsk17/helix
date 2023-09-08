#include <utility>

#include "allocator.hpp"

#include "tokenization.hpp"

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct NodeBinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr {
    NodeBinExprAdd* add;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*> var;
};

struct NodeExpr {
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStmtExit {
    NodeExpr* expr;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit*, NodeStmtLet*> var;
};

struct NodeProgram {
    std::vector<NodeStmt*> statements;
};

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens))
        , m_allocator(1024 * 1024 * 4)
    {
    }

    std::optional<NodeTerm*> parse_term()
    {
        if (auto int_lit = try_consume(TokenType::int_lit)) {
            auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
            term_int_lit->int_lit = int_lit.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_int_lit;
            return term;
        }
        else if (auto ident = try_consume(TokenType::identifier)) {
            auto term_ident = m_allocator.alloc<NodeTermIdent>();
            term_ident->ident = ident.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_ident;
            return term;
        }
        else {
            return {};
        }
    }

    std::optional<NodeExpr*> parse_expr()

    {
        if (auto term = parse_term()) {
            if (try_consume(TokenType::plus).has_value()) {
                auto bin_expr = m_allocator.alloc<NodeBinExpr>();
                auto bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();
                auto lhs_expr = m_allocator.alloc<NodeExpr>();

                lhs_expr->var = term.value();
                bin_expr_add->lhs = lhs_expr;

                if (auto rhs = parse_expr()) {
                    bin_expr_add->rhs = rhs.value();
                    bin_expr->add = bin_expr_add;
                    auto expr = m_allocator.alloc<NodeExpr>();
                    expr->var = bin_expr;
                    return expr;
                }
                else {
                    std::cerr << "Expected Expression";
                    exit(EXIT_FAILURE);
                }
            }
            else {
                auto expr = m_allocator.alloc<NodeExpr>();
                expr->var = term.value();
                return expr;
            }
        }
        else {
            return {};
        }
    }

    std::optional<NodeStmt*> parse_stmt()
    {
        if (peek().value().type == TokenType::exit && peek(1).has_value()
            && peek(1).value().type == TokenType::open_parenthesis) {
            consume(); // Consume exit token
            consume(); // Consume open parenthesis token

            auto stmt_exit = m_allocator.alloc<NodeStmtExit>();

            if (auto expr = parse_expr()) {
                stmt_exit->expr = expr.value();
            }
            else {
                std::cerr << "Expected `(`" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_parenthesis, "Expected `)`");
            try_consume(TokenType::semi, "Expected `;`");
            auto node_stmt = m_allocator.alloc<NodeStmt>();
            node_stmt->var = stmt_exit;
            return node_stmt;
        }
        else if (
            peek().has_value() && peek().value().type == TokenType::let && peek(1).has_value()
            && peek(1).value().type == TokenType::identifier && peek(2).has_value()
            && peek(2).value().type == TokenType::eq) {
            consume();
            auto stmt = m_allocator.alloc<NodeStmtLet>();
            stmt->ident = consume();
            consume();
            if (auto expr = parse_expr()) {
                stmt->expr = expr.value();
            }
            else {
                std::cerr << "Invalid Expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::semi, "Expected `;`");
            auto node_stmt = m_allocator.alloc<NodeStmt>();
            node_stmt->var = stmt;
            return node_stmt;
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
    ArenaAllocator m_allocator;
    [[nodiscard]] inline std::optional<Token> peek(int offset = 0) const
    {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        }
        else {
            return m_tokens.at(m_index + offset);
        }
    }

    inline Token try_consume(TokenType type, const std::string& error_msg)
    {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        else {
            std::cerr << error_msg << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    inline std::optional<Token> try_consume(TokenType type) {
        if (peek().has_value() && peek().value().type ==type) {
            return consume();
        } else {
            return {};
        }
    }

    inline Token consume()
    {
        return m_tokens.at(m_index++);
    }
};