// mini_lang_interpreter.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <cctype>

// ========== Expression ==========
struct Expr {
    virtual ~Expr() = default;
    virtual std::string eval(std::unordered_map<std::string, std::string>& vars) = 0;
};

struct LiteralExpr : Expr {
    std::string value;
    LiteralExpr(const std::string& v) : value(v) {}
    std::string eval(std::unordered_map<std::string, std::string>&) override {
        return value;
    }
};

struct VariableExpr : Expr {
    std::string name;
    VariableExpr(const std::string& n) : name(n) {}
    std::string eval(std::unordered_map<std::string, std::string>& vars) override {
        return vars[name];
    }
};

struct BinaryExpr : Expr {
    std::shared_ptr<Expr> left, right;
    char op;
    BinaryExpr(std::shared_ptr<Expr> l, char o, std::shared_ptr<Expr> r)
        : left(l), op(o), right(r) {}
    std::string eval(std::unordered_map<std::string, std::string>& vars) override {
        std::string a = left->eval(vars);
        std::string b = right->eval(vars);
        if (op == '+') {
            if (isdigit(a[0]) && isdigit(b[0]))
                return std::to_string(std::stoi(a) + std::stoi(b));
            return a + b;
        }
        return "";
    }
};

// ========== Statement ==========
struct Stmt {
    virtual ~Stmt() = default;
    virtual void execute(std::unordered_map<std::string, std::string>& vars) = 0;
};

struct AssignStmt : Stmt {
    std::string name;
    std::shared_ptr<Expr> expr;
    AssignStmt(const std::string& n, std::shared_ptr<Expr> e) : name(n), expr(e) {}
    void execute(std::unordered_map<std::string, std::string>& vars) override {
        vars[name] = expr->eval(vars);
    }
};

struct PrintStmt : Stmt {
    std::shared_ptr<Expr> expr;
    PrintStmt(std::shared_ptr<Expr> e) : expr(e) {}
    void execute(std::unordered_map<std::string, std::string>& vars) override {
        std::cout << expr->eval(vars) << std::endl;
    }
};

struct IfStmt : Stmt {
    std::shared_ptr<Expr> condition;
    std::vector<std::shared_ptr<Stmt>> body;
    IfStmt(std::shared_ptr<Expr> cond, std::vector<std::shared_ptr<Stmt>> b) : condition(cond), body(b) {}
    void execute(std::unordered_map<std::string, std::string>& vars) override {
        if (condition->eval(vars) != "0") {
            for (auto& stmt : body) stmt->execute(vars);
        }
    }
};

struct WhileStmt : Stmt {
    std::shared_ptr<Expr> condition;
    std::vector<std::shared_ptr<Stmt>> body;
    WhileStmt(std::shared_ptr<Expr> cond, std::vector<std::shared_ptr<Stmt>> b) : condition(cond), body(b) {}
    void execute(std::unordered_map<std::string, std::string>& vars) override {
        while (condition->eval(vars) != "0") {
            for (auto& stmt : body) stmt->execute(vars);
        }
    }
};

// ========== Parser ==========
std::shared_ptr<Expr> parse_expr(std::istringstream& iss) {
    std::string token;
    iss >> token;
    if (token.empty()) return nullptr;

    if (token[0] == '"') {
        std::string str = token;
        while (str.back() != '"' && iss) {
            std::string next;
            iss >> next;
            str += " " + next;
        }
        return std::make_shared<LiteralExpr>(str.substr(1, str.length() - 2));
    }

    if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
        return std::make_shared<LiteralExpr>(token);
    }

    char next = iss.peek();
    if (next == '+') {
        iss.get();
        auto rhs = parse_expr(iss);
        return std::make_shared<BinaryExpr>(std::make_shared<VariableExpr>(token), '+', rhs);
    }
    return std::make_shared<VariableExpr>(token);
}

std::shared_ptr<Stmt> parse_stmt(const std::string& line);

std::vector<std::shared_ptr<Stmt>> parse_block(std::istringstream& iss) {
    std::vector<std::shared_ptr<Stmt>> stmts;
    std::string line;
    while (std::getline(iss, line, ';')) {
        if (line.empty()) continue;
        auto stmt = parse_stmt(line);
        if (stmt) stmts.push_back(stmt);
    }
    return stmts;
}

std::shared_ptr<Stmt> parse_stmt(const std::string& line) {
    std::istringstream iss(line);
    std::string first;
    iss >> first;
    if (first == "print") {
        char open;
        iss >> open;
        auto expr = parse_expr(iss);
        return std::make_shared<PrintStmt>(expr);
    } else if (first == "if") {
        auto cond = parse_expr(iss);
        std::string block;
        std::getline(iss, block, '{');
        std::getline(iss, block, '}');
        std::istringstream block_ss(block);
        return std::make_shared<IfStmt>(cond, parse_block(block_ss));
    } else if (first == "while") {
        auto cond = parse_expr(iss);
        std::string block;
        std::getline(iss, block, '{');
        std::getline(iss, block, '}');
        std::istringstream block_ss(block);
        return std::make_shared<WhileStmt>(cond, parse_block(block_ss));
    } else {
        std::string eq;
        iss >> eq;
        if (eq == "=") {
            auto expr = parse_expr(iss);
            return std::make_shared<AssignStmt>(first, expr);
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Stmt>> parse_file(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    std::vector<std::shared_ptr<Stmt>> stmts;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string part;
        while (std::getline(ss, part, ';')) {
            if (part.empty()) continue;
            auto stmt = parse_stmt(part);
            if (stmt) stmts.push_back(stmt);
        }
    }
    return stmts;
}

// ========== Main ==========
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./interpreter <filename>\n";
        return 1;
    }
    auto stmts = parse_file(argv[1]);
    std::unordered_map<std::string, std::string> vars;
    for (auto& stmt : stmts) stmt->execute(vars);
    return 0;
}
