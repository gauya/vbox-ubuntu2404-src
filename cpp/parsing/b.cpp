#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

// ====================== Expression ======================
struct Expr {
    virtual int eval(std::unordered_map<std::string, int>& vars) = 0;
    virtual ~Expr() = default;
};

struct NumberExpr : Expr {
    int value;
    NumberExpr(int v) : value(v) {}
    int eval(std::unordered_map<std::string, int>&) override {
        return value;
    }
};

struct VariableExpr : Expr {
    std::string name;
    VariableExpr(const std::string& n) : name(n) {}
    int eval(std::unordered_map<std::string, int>& vars) override {
        if (vars.find(name) == vars.end()) {
            std::cerr << "Error: undefined variable '" << name << "'\n";
            return 0;
        }
        return vars[name];
    }
};

struct BinaryExpr : Expr {
    std::shared_ptr<Expr> left, right;
    char op;
    BinaryExpr(std::shared_ptr<Expr> l, char o, std::shared_ptr<Expr> r)
        : left(l), op(o), right(r) {}
    int eval(std::unordered_map<std::string, int>& vars) override {
        int a = left->eval(vars);
        int b = right->eval(vars);
        switch (op) {
            case '+': return a + b;
            case '-': return a - b;
            case '*': return a * b;
            case '/': return b != 0 ? a / b : 0;
            default:
                std::cerr << "Unknown binary operator\n";
                return 0;
        }
    }
};

// ====================== Statement ======================
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

// ====================== Evaluator ======================
class Evaluator {
    std::unordered_map<std::string, int> vars;
public:
    void run(const std::vector<std::shared_ptr<Stmt>>& stmts) {
        for (auto& stmt : stmts) {
            stmt->execute(vars);
        }
    }
};

// ====================== Main ======================
int main() {
    std::vector<std::shared_ptr<Stmt>> program;

    // x = 10;
    program.push_back(std::make_shared<AssignStmt>(
        "x", std::make_shared<NumberExpr>(10)));

    // y = x + 20;
    program.push_back(std::make_shared<AssignStmt>(
        "y", std::make_shared<BinaryExpr>(
            std::make_shared<VariableExpr>("x"),
            '+',
            std::make_shared<NumberExpr>(20))));

    // print(y);
    program.push_back(std::make_shared<PrintStmt>(
        std::make_shared<VariableExpr>("y")));

    Evaluator eval;
    eval.run(program);

    return 0;
}

