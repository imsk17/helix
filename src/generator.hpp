#include "parser.hpp"

#if __APPLE__
#define EXIT_SYS_CODE 0x2000001
#elif __linux__
#define EXIT_SYS_CODE 60
#endif

class Generator {
public:
    inline explicit Generator(NodeProgram program)
        : m_program(std::move(program))
    {
    }

    void gen_term(const NodeTerm* term)
    {
        struct TermVisitor {
            Generator* gen;
            void operator()(const NodeTermIntLit* term_int_lit) const
            {
                gen->m_output << "    mov rax, " << term_int_lit->int_lit.value.value() << "\n";
                gen->push("rax");
            }
            void operator()(const NodeTermIdent* term_ident) const
            {
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var& var) {
                    return var.name == term_ident->ident.value.value();
                });
                if (it == gen->m_vars.cend()) {
                    std::cerr << "Undeclared Identifier: " << term_ident->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                    std::stringstream offset;
                offset << "QWORD [rsp + " << (gen->m_stack_size - (*it).stack_location - 1) * 8 << "]\n";
                gen->push(offset.str());
            }
            void operator()(const NodeTermParen* term_paren) const
            {
                gen->gen_expr(term_paren->expr);
            }
        };
        TermVisitor visitor({ .gen = this });
        std::visit(visitor, term->var);
    }

    void gen_bin_expr(const NodeBinExpr* bin_expr)
    {
        struct BinExprVisitor {
            Generator* gen;

            void operator()(const NodeBinExprAdd* expr_add) const
            {
                gen->gen_expr(expr_add->rhs);
                gen->gen_expr(expr_add->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    add rax, rbx\n";
                gen->push("rax");
            }
            void operator()(const NodeBinExprMul* expr_mul) const
            {
                gen->gen_expr(expr_mul->rhs);
                gen->gen_expr(expr_mul->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    mul rbx\n";
                gen->push("rax");
            }
            void operator()(const NodeBinExprSub* expr_sub) const
            {
                gen->gen_expr(expr_sub->rhs);
                gen->gen_expr(expr_sub->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    sub rax, rbx\n";
                gen->push("rax");
            }
            void operator()(const NodeBinExprDiv* expr_div) const
            {
                gen->gen_expr(expr_div->rhs);
                gen->gen_expr(expr_div->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    div rbx\n";
                gen->push("rax");
            }
            void operator()(const NodeBinExprMod* expr_mod) const
            {
                gen->gen_expr(expr_mod->rhs);
                gen->gen_expr(expr_mod->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    div rbx\n";
                gen->push("rdx");
            }
            void operator()(const NodeBinExprGt* expr_gt) const
            {
                gen->gen_expr(expr_gt->rhs);
                gen->gen_expr(expr_gt->lhs);
                gen->m_output << "; preparing gt\n";
                gen->m_output << "    mov rcx, 0\n";
                gen->m_output << "    mov rdx, 1\n";
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    cmp rax, rbx\n";
                gen->m_output << "    cmovg rcx, rdx\n";
                gen->push("rcx");
            }
            void operator()(const NodeBinExprLt* expr_lt) const
            {
                gen->gen_expr(expr_lt->rhs);
                gen->gen_expr(expr_lt->lhs);
                gen->m_output << "; preparing lt\n";
                gen->m_output << "    mov rcx, 0\n";
                gen->m_output << "    mov rdx, 1\n";
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    cmp rax, rbx\n";
                gen->m_output << "    cmovl rcx, rdx\n";
                gen->push("rcx");
            }
            void operator()(const NodeBinExprGte* expr_gte) const
            {
                gen->gen_expr(expr_gte->rhs);
                gen->gen_expr(expr_gte->lhs);
                gen->m_output << "; preparing gte\n";
                gen->m_output << "    mov rcx, 0\n";
                gen->m_output << "    mov rdx, 1\n";
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    cmp rax, rbx\n";
                gen->m_output << "    cmovge rcx, rdx\n";
                gen->push("rcx");
            }
            void operator()(const NodeBinExprLte* expr_lte) const
            {
                gen->gen_expr(expr_lte->rhs);
                gen->gen_expr(expr_lte->lhs);
                gen->m_output << "; preparing lte\n";
                gen->m_output << "    mov rcx, 0\n";
                gen->m_output << "    mov rdx, 1\n";
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    cmp rax, rbx\n";
                gen->m_output << "    cmovle rcx, rdx\n";
                gen->push("rcx");
            }
            void operator()(const NodeBinExprEquality* expr_equality) const
            {
                gen->gen_expr(expr_equality->rhs);
                gen->gen_expr(expr_equality->lhs);
                gen->m_output << "; preparing equality\n";
                gen->m_output << "    mov rcx, 0\n";
                gen->m_output << "    mov rdx, 1\n";
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    cmp rax, rbx\n";
                gen->m_output << "    cmove rcx, rdx\n";
                gen->push("rcx");
            }
            void operator()(const NodeBinExprNotEquality* expr_not_equality) const
            {
                gen->gen_expr(expr_not_equality->rhs);
                gen->gen_expr(expr_not_equality->lhs);
                gen->m_output << "; preparing not equality\n";
                gen->m_output << "    mov rcx, 0\n";
                gen->m_output << "    mov rdx, 1\n";
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    cmp rax, rbx\n";
                gen->m_output << "    cmovne rcx, rdx\n";
                gen->push("rcx");
            }
        };

        BinExprVisitor visitor { .gen = this };
        std::visit(visitor, bin_expr->var);
    }

    void gen_expr(const NodeExpr* expr)
    {
        struct ExpressionVisitor {
            Generator* gen;
            void operator()(const NodeTerm* term) const
            {
                gen->gen_term(term);
            }
            void operator()(const NodeBinExpr* bin_expr) const
            {
                gen->gen_bin_expr(bin_expr);
            };
        };
        ExpressionVisitor visitor { .gen = this };
        std::visit(visitor, expr->var);
    }

    void gen_stmt(const NodeStmt* stmt)
    {
        struct StatementVisitor {
            Generator* gen;
            void operator()(const NodeStmtExit* stmt_exit) const
            {
                gen->gen_expr(stmt_exit->expr);
                gen->m_output << "    mov rax, " << EXIT_SYS_CODE << "\n";
                gen->pop("rdi");
                gen->m_output << "    syscall\n";
            }
            void operator()(const NodeStmtLet* stmt_let) const
            {
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var& var) {
                    return var.name == stmt_let->ident.value.value();
                });
                if (it != gen->m_vars.cend()) {
                    std::cerr << "Identifier already declared: " << stmt_let->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen->m_vars.push_back({ .name = stmt_let->ident.value.value(), .stack_location = gen->m_stack_size });
                gen->gen_expr(stmt_let->expr);
            }
            void operator()(const NodeStmtScope* scope) const
            {
                gen->begin_scope();
                for (const NodeStmt* stmt : scope->stmts) {
                    gen->gen_stmt(stmt);
                }
                gen->end_scope();
            }
        };

        StatementVisitor visitor { .gen = this };
        std::visit(visitor, stmt->var);
    }

    [[nodiscard]] std::string gen_program()
    {
        m_output << "global _main\n_main:\n";

        for (const NodeStmt* stmt : m_program.statements) {
            gen_stmt(stmt);
        }

        m_output << "    mov rax, " << EXIT_SYS_CODE << "\n";
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
        std::string name;
        size_t stack_location;
    };

private:
    void begin_scope()
    {
        m_scopes.push_back(m_vars.size());
    }

    void end_scope()
    {
        size_t pop_count = m_vars.size() - m_scopes.back();
        m_output << "     add rsp, " << pop_count * 8 << "\n";
        m_stack_size -= pop_count;
        for (int i = 0; i < pop_count; i++) {
            m_vars.pop_back();
        }
        m_scopes.pop_back();
    }
    const NodeProgram m_program;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    std::vector<Var> m_vars {};
    std::vector<size_t> m_scopes {};
};