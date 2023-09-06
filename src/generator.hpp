#include "parser.hpp"

class Generator {
public:
    inline explicit Generator(NodeProgram program)
        : m_program(std::move(program))
    {
    }

    void gen_expr(const NodeExpr& expr)
    {
        struct ExpressionVisitor {
            Generator* gen;
            void operator()(const NodeExprIntLit& expr_int_lit)
            {
                gen->m_output << "    mov rax, " << expr_int_lit.int_lit.value.value() << "\n";
                gen->push("rax");
            }
            void operator()(const NodeExprIdent& expr_ident)
            {
                if (!gen->m_vars.contains(expr_ident.ident.value.value())) {
                    std::cerr << "Undeclared Identifier: " << expr_ident.ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                else {
                    const auto& var = gen->m_vars.at(expr_ident.ident.value.value());
                    std::stringstream offset;
                    offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_location - 1) * 8 << "]\n";
                    gen->push(offset.str());
                }
            }
        };
        ExpressionVisitor visitor { .gen = this };
        std::visit(visitor, expr.var);
    }

    void gen_stmt(const NodeStmt& stmt)
    {
        struct StatementVisitor {
            Generator* gen;
            void operator()(const NodeStmtExit& stmt_exit) const
            {
                gen->gen_expr(stmt_exit.expr);
                gen->m_output << "    mov rax, 0x2000001\n";
                gen->pop("rdi");
                gen->m_output << "    syscall\n";
            }
            void operator()(const NodeStmtLet& stmt_let)
            {
                if (gen->m_vars.contains(stmt_let.ident.value.value())) {
                    std::cerr << "Identifier already declared: " << stmt_let.ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen->m_vars.insert({ stmt_let.ident.value.value(), Var { .stack_location = gen->m_stack_size } });
                gen->gen_expr(stmt_let.expr);
            }
        };

        StatementVisitor visitor { .gen = this };
        std::visit(visitor, stmt.var);
    }

    [[nodiscard]] std::string gen_program()
    {
        m_output << "global _main\n_main:\n";

        for (const NodeStmt& stmt : m_program.statements) {
            gen_stmt(stmt);
        }

        m_output << "    mov rax, 0x2000001\n";
        m_output << "    mov rdi, 0\n";
        m_output << "    syscall";
        return m_output.str();
    }

    void push(const std::string& reg)
    {
        m_output << "    push " << reg << "\n";
        m_stack_size++;
    }

    void pop(const std::string& reg)
    {
        m_output << "    pop " << reg << "\n";
        m_stack_size--;
    }

    struct Var {
        size_t stack_location;
    };

private:
    const NodeProgram m_program;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    std::unordered_map<std::string, Var> m_vars {};
};