#include <map>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>

enum class TokenType { Identifier, Number, Symbol, Keyword, End };

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
    std::string input;
    size_t pos = 0;

public:
    Lexer(const std::string& src) : input(src) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos < input.size()) {
            if (isspace(input[pos])) {
                ++pos;
            } else if (isalpha(input[pos])) {
                std::string ident;
                while (isalnum(input[pos])) ident += input[pos++];
                if (ident == "function" || ident == "return" || ident == "print")
                    tokens.push_back({TokenType::Keyword, ident});
                else
                    tokens.push_back({TokenType::Identifier, ident});
            } else if (isdigit(input[pos])) {
                std::string num;
                while (isdigit(input[pos])) num += input[pos++];
                tokens.push_back({TokenType::Number, num});
            } else {
                tokens.push_back({TokenType::Symbol, std::string(1, input[pos++])});
            }
        }
        tokens.push_back({TokenType::End, ""});
        return tokens;
    }
};

struct Expr {
    virtual ~Expr() {}
};

struct NumberExpr : Expr {
    int value;
    NumberExpr(int v) : value(v) {}
};

struct VariableExpr : Expr {
    std::string name;
    VariableExpr(const std::string& n) : name(n) {}
};

struct BinaryExpr : Expr {
    std::shared_ptr<Expr> lhs, rhs;
    char op;
    BinaryExpr(std::shared_ptr<Expr> l, char o, std::shared_ptr<Expr> r)
        : lhs(l), op(o), rhs(r) {}
};

struct Stmt {
    virtual void execute(std::unordered_map<std::string, int>& vars) = 0;
    virtual ~Stmt() = default;
};

struct AssignStmt : Stmt {
    std::string name;
    std::shared_ptr<Expr> expr;
    AssignStmt(const std::string& n, std::shared_ptr<Expr> e)
        : name(n), expr(e) {}
    void execute(std::unordered_map<std::string, int>& vars) override {
        vars[name] = expr->eval(vars);
    }
};

struct PrintStmt : Stmt {
    std::shared_ptr<Expr> expr;
    PrintStmt(std::shared_ptr<Expr> e) : expr(e) {}
    void execute(std::unordered_map<std::string, int>& vars) override {
        std::cout << expr->eval(vars) << std::endl;
    }
};

using Statement = std::variant<PrintStmt>; // 추후 Assignment 등 추가 가능

std::shared_ptr<Expr> parseExpr(); // 예제 생략

Statement parseStatement() {
    if (match("print")) {
        expect("(");
        auto expr = parseExpr();
        expect(")");
        expect(";");
        return PrintStmt(expr);
    }
    // ... (assignment 등 추가)
}

std::unordered_map<std::string, int> variables;

int evalExpr(std::shared_ptr<Expr> expr) {
    if (auto num = dynamic_cast<NumberExpr*>(expr.get())) {
        return num->value;
    } else if (auto var = dynamic_cast<VariableExpr*>(expr.get())) {
        return variables[var->name];
    } else if (auto bin = dynamic_cast<BinaryExpr*>(expr.get())) {
        int left = evalExpr(bin->lhs);
        int right = evalExpr(bin->rhs);
        switch (bin->op) {
            case '+': return left + right;
            case '-': return left - right;
            case '*': return left * right;
            case '/': return left / right;
        }
    }
    throw std::runtime_error("Unknown expression");
}

void execute(const Statement& stmt) {
    if (auto print = std::get_if<PrintStmt>(&stmt)) {
        int val = evalExpr(print->expr);
        std::cout << val << std::endl;
    }
}


// ============================
// Expression 계열
// ============================

// 기본 AST 노드
struct ASTNode {
    virtual ~ASTNode() {}
};


struct NumberExpr : ASTNode {
    int value;
    NumberExpr(int v) : value(v) {}
};

struct VariableExpr : ASTNode {
    std::string name;
    VariableExpr(const std::string& n) : name(n) {}
};

struct BinaryExpr : ASTNode {
    std::string op;
    std::shared_ptr<ASTNode> left, right;
    BinaryExpr(const std::string& o, std::shared_ptr<ASTNode> l, std::shared_ptr<ASTNode> r)
        : op(o), left(l), right(r) {}
};

struct FunctionCallExpr : ASTNode {
    std::string name;
    std::vector<std::shared_ptr<ASTNode>> args;
    FunctionCallExpr(const std::string& n, const std::vector<std::shared_ptr<ASTNode>>& a)
        : name(n), args(a) {}
};

// ============================
// Statement 계열
// ============================

struct AssignmentStmt : ASTNode {
    std::string var;
    std::shared_ptr<ASTNode> expr;
    AssignmentStmt(const std::string& v, std::shared_ptr<ASTNode> e)
        : var(v), expr(e) {}
};

struct ReturnStmt : ASTNode {
    std::shared_ptr<ASTNode> expr;
    ReturnStmt(std::shared_ptr<ASTNode> e) : expr(e) {}
};

struct IfStmt : ASTNode {
    std::shared_ptr<ASTNode> condition;
    std::vector<std::shared_ptr<ASTNode>> then_branch;
    std::vector<std::shared_ptr<ASTNode>> else_branch;
    IfStmt(std::shared_ptr<ASTNode> cond,
           const std::vector<std::shared_ptr<ASTNode>>& then_b,
           const std::vector<std::shared_ptr<ASTNode>>& else_b)
        : condition(cond), then_branch(then_b), else_branch(else_b) {}
};

struct WhileStmt : ASTNode {
    std::shared_ptr<ASTNode> condition;
    std::vector<std::shared_ptr<ASTNode>> body;
    WhileStmt(std::shared_ptr<ASTNode> cond, const std::vector<std::shared_ptr<ASTNode>>& b)
        : condition(cond), body(b) {}
};

struct FunctionDefStmt : ASTNode {
    std::string name;
    std::vector<std::string> params;
    std::vector<std::shared_ptr<ASTNode>> body;
    FunctionDefStmt(const std::string& n, const std::vector<std::string>& p,
                    const std::vector<std::shared_ptr<ASTNode>>& b)
        : name(n), params(p), body(b) {}
};


using Value = int;

struct Function {
    std::vector<std::string> params;
    std::vector<std::shared_ptr<ASTNode>> body;
};

struct Context {
    std::map<std::string, Value> variables;
    std::map<std::string, Function> functions;

    Value getVar(const std::string& name) {
        if (variables.count(name)) return variables[name];
        throw std::runtime_error("Undefined variable: " + name);
    }

    void setVar(const std::string& name, Value value) {
        variables[name] = value;
    }

    void defineFunc(const std::string& name, const Function& fn) {
        functions[name] = fn;
    }

    Function getFunc(const std::string& name) {
        if (functions.count(name)) return functions[name];
        throw std::runtime_error("Undefined function: " + name);
    }
};

class Evaluator {
public:
    Value evalExpr(std::shared_ptr<ASTNode> node, Context& ctx) {
        if (auto n = std::dynamic_pointer_cast<NumberExpr>(node)) {
            return n->value;
        } else if (auto v = std::dynamic_pointer_cast<VariableExpr>(node)) {
            return ctx.getVar(v->name);
        } else if (auto b = std::dynamic_pointer_cast<BinaryExpr>(node)) {
            Value left = evalExpr(b->left, ctx);
            Value right = evalExpr(b->right, ctx);
            if (b->op == "+") return left + right;
            if (b->op == "-") return left - right;
            if (b->op == "*") return left * right;
            if (b->op == "/") return left / right;
            if (b->op == "<") return left < right;
            if (b->op == ">") return left > right;
            if (b->op == "==") return left == right;
            throw std::runtime_error("Unknown binary operator: " + b->op);
        } else if (auto f = std::dynamic_pointer_cast<FunctionCallExpr>(node)) {
            Function fn = ctx.getFunc(f->name);
            if (fn.params.size() != f->args.size())
                throw std::runtime_error("Argument count mismatch for function " + f->name);

            // 새로운 함수 실행 컨텍스트
            Context localCtx;
            localCtx.functions = ctx.functions; // 함수는 공유
            for (size_t i = 0; i < f->args.size(); ++i) {
                localCtx.setVar(fn.params[i], evalExpr(f->args[i], ctx));
            }

            for (auto stmt : fn.body) {
                if (auto ret = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
                    return evalExpr(ret->expr, localCtx);
                } else {
                    evalStmt(stmt, localCtx);
                }
            }
            return 0;
        }

        throw std::runtime_error("Unknown expression node");
    }

    void evalStmt(std::shared_ptr<ASTNode> node, Context& ctx) {
        if (auto assign = std::dynamic_pointer_cast<AssignmentStmt>(node)) {
            Value val = evalExpr(assign->expr, ctx);
            ctx.setVar(assign->var, val);
        } else if (auto func = std::dynamic_pointer_cast<FunctionDefStmt>(node)) {
            ctx.defineFunc(func->name, Function{func->params, func->body});
        } else if (auto ifs = std::dynamic_pointer_cast<IfStmt>(node)) {
            if (evalExpr(ifs->condition, ctx)) {
                for (auto stmt : ifs->then_branch) evalStmt(stmt, ctx);
            } else {
                for (auto stmt : ifs->else_branch) evalStmt(stmt, ctx);
            }
        } else if (auto w = std::dynamic_pointer_cast<WhileStmt>(node)) {
            while (evalExpr(w->condition, ctx)) {
                for (auto stmt : w->body) evalStmt(stmt, ctx);
            }
        } else {
            throw std::runtime_error("Unknown statement node");
        }
    }

    void run(const std::vector<std::shared_ptr<ASTNode>>& program) {
        Context ctx;
        for (auto stmt : program) {
            evalStmt(stmt, ctx);
        }

        // 디버그 출력
        for (const auto& [name, val] : ctx.variables) {
            std::cout << name << " = " << val << std::endl;
        }
    }
};

std::string readFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("파일 열기 실패: " + filename);
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "사용법: ./interpreter <파일명>" << std::endl;
        return 1;
    }

    try {
        std::string source = readFile(argv[1]);
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        // TODO: Parser parser(tokens); AST 구성
        // TODO: Evaluator eval; eval.run(ast);

        std::cout << "[*] 토큰 목록:" << std::endl;
        for (const auto& token : tokens) {
            std::cout << " - " << token.value << std::endl;
        }

    } catch (std::exception& e) {
        std::cerr << "에러: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

int _main() {
    Evaluator eval;

    auto fn_add = std::make_shared<FunctionDefStmt>("add", std::vector<std::string>{"a", "b"},
        std::vector<std::shared_ptr<ASTNode>>{
            std::make_shared<ReturnStmt>(
                std::make_shared<BinaryExpr>("+",
                    std::make_shared<VariableExpr>("a"),
                    std::make_shared<VariableExpr>("b")
                )
            )
        });

    auto assign_x = std::make_shared<AssignmentStmt>("x", std::make_shared<NumberExpr>(10));
    auto assign_y = std::make_shared<AssignmentStmt>("y", std::make_shared<NumberExpr>(20));

    auto if_stmt = std::make_shared<IfStmt>(
        std::make_shared<BinaryExpr>("<",
            std::make_shared<VariableExpr>("x"),
            std::make_shared<VariableExpr>("y")),
        std::vector<std::shared_ptr<ASTNode>>{
            std::make_shared<AssignmentStmt>("z",
                std::make_shared<FunctionCallExpr>("add",
                    std::vector<std::shared_ptr<ASTNode>>{
                        std::make_shared<VariableExpr>("x"),
                        std::make_shared<VariableExpr>("y")
                    }))},
        std::vector<std::shared_ptr<ASTNode>>{
            std::make_shared<AssignmentStmt>("z", std::make_shared<NumberExpr>(0))
        });

    auto while_stmt = std::make_shared<WhileStmt>(
        std::make_shared<BinaryExpr>(">",
            std::make_shared<VariableExpr>("z"),
            std::make_shared<NumberExpr>(0)),
        std::vector<std::shared_ptr<ASTNode>>{
            std::make_shared<AssignmentStmt>("z",
                std::make_shared<BinaryExpr>("-",
                    std::make_shared<VariableExpr>("z"),
                    std::make_shared<NumberExpr>(1)))
        });

    eval.run({
        fn_add, assign_x, assign_y, if_stmt, while_stmt
    });
}


