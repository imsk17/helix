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

struct NodeBinExprMul {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprDiv {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprSub {
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct NodeBinExprMod {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd*, NodeBinExprMul*, NodeBinExprSub*, NodeBinExprDiv*, NodeBinExprMod*> var;
};

struct NodeTermParen {
    NodeExpr* expr;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
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
        else if (auto open_paren = try_consume(TokenType::open_parenthesis)) {
            auto expr = parse_expr();
            if (!expr.has_value()) {
                std::cerr << "Expected Expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_parenthesis, "Expected `)`");
            auto term_paren = m_allocator.alloc<NodeTermParen>();
            term_paren->expr = expr.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_paren;
            return term;
        }
        else {
            return {};
        }
    }

    std::optional<NodeExpr*> parse_expr(int min_prec = 0)
    {
        std::optional<NodeTerm*> term_lhs = parse_term();
        if (!term_lhs.has_value()) {
            return {};
        }
        auto expr_lhs = m_allocator.alloc<NodeExpr>();
        expr_lhs->var = term_lhs.value();
        while (true) {
            std::optional<Token> curr_token = peek();
            std::optional<int> prec;
            if (curr_token.has_value()) {
                prec = bin_prec(curr_token->type);
                if (!prec.has_value() || prec < min_prec) {
                    break;
                }
            }
            else {
                break;
            }
            Token op = consume();
            int next_min_prec = prec.value() + 1;
            auto expr_rhs = parse_expr(next_min_prec);
            if (!expr_rhs.has_value()) {
                std::cerr << "Unable to parse expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            auto expr = m_allocator.alloc<NodeBinExpr>();
            auto expr_lhs2 = m_allocator.alloc<NodeExpr>();
            if (op.type == TokenType::plus) {
                auto add = m_allocator.alloc<NodeBinExprAdd>();
                expr_lhs2->var = expr_lhs->var;
                add->lhs = expr_lhs2;
                add->rhs = expr_rhs.value();
                expr->var = add;
            }
            else if (op.type == TokenType::sub) {
                auto sub = m_allocator.alloc<NodeBinExprSub>();
                expr_lhs2->var = expr_lhs->var;
                sub->lhs = expr_lhs2;
                sub->rhs = expr_rhs.value();
                expr->var = sub;
            }
            else if (op.type == TokenType::div) {
                auto div = m_allocator.alloc<NodeBinExprDiv>();
                expr_lhs2->var = expr_lhs->var;
                div->lhs = expr_lhs2;
                div->rhs = expr_rhs.value();
                expr->var = div;
            }
            else if (op.type == TokenType::star) {
                auto mul = m_allocator.alloc<NodeBinExprMul>();
                expr_lhs2->var = expr_lhs->var;
                mul->lhs = expr_lhs2;
                mul->rhs = expr_rhs.value();
                expr->var = mul;
            }
            else if (op.type == TokenType::modulo) {
                auto mod = m_allocator.alloc<NodeBinExprMod>();
                expr_lhs2->var = expr_lhs->var;
                mod->lhs = expr_lhs2;
                mod->rhs = expr_rhs.value();
                expr->var = mod;
            }
            else {
                assert(false); // Should not be reachable;
            }
            expr_lhs->var = expr;
        }
        return expr_lhs;
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