// parser.h
#ifndef MYLANG_PARSER_H
#define MYLANG_PARSER_H

#include <vector>
#include <memory>
#include <string>
#include "lexer.h"

namespace MyLang {

struct ASTNode {
    virtual ~ASTNode() = default;
    virtual std::string getNodeType() const { return "ASTNode"; }
};

struct ProgramNode : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
    std::string getNodeType() const override { return "ProgramNode"; }
};

struct BlockNode : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
    std::string getNodeType() const override { return "BlockNode"; }
};

struct IfStatementNode : ASTNode {
    std::unique_ptr<ASTNode> condition;
    // --- 변경: body와 else_body의 타입이 BlockNode로 명확해졌으므로 변경 ---
    std::unique_ptr<BlockNode> body;
    std::unique_ptr<BlockNode> else_body; // else 블록은 선택 사항
    // --- 여기까지 변경 ---
    std::string getNodeType() const override { return "IfStatementNode"; }
};

struct WhileStatementNode : ASTNode {
    std::unique_ptr<ASTNode> condition;
    // --- 변경: body의 타입을 BlockNode로 변경 ---
    std::unique_ptr<BlockNode> body;
    // --- 여기까지 변경 ---
    std::string getNodeType() const override { return "WhileStatementNode"; }
};

struct FunctionDeclarationNode : ASTNode {
    std::string name;
    std::vector<std::string> params;
    // --- 변경: body의 타입을 BlockNode로 변경 ---
    std::unique_ptr<BlockNode> body;
    // --- 여기까지 변경 ---
    std::string getNodeType() const override { return "FunctionDeclarationNode"; }
};

struct BinaryExpressionNode : ASTNode {
    std::unique_ptr<ASTNode> left;
    std::string op;
    std::unique_ptr<ASTNode> right;
    std::string getNodeType() const override { return "BinaryExpressionNode"; }
};

struct NumberLiteralNode : ASTNode {
    int value;
    explicit NumberLiteralNode(int val) : value(val) {}
    std::string getNodeType() const override { return "NumberLiteralNode"; }
};

struct IdentifierNode : ASTNode {
    std::string name;
    explicit IdentifierNode(const std::string& n) : name(n) {}
    std::string getNodeType() const override { return "IdentifierNode"; }
};

struct ReturnStatementNode : ASTNode {
    std::unique_ptr<ASTNode> expression;
    explicit ReturnStatementNode(std::unique_ptr<ASTNode> expr) : expression(std::move(expr)) {}
    std::string getNodeType() const override { return "ReturnStatementNode"; }
};

struct ExpressionStatementNode : ASTNode {
    std::unique_ptr<ASTNode> expression;
    explicit ExpressionStatementNode(std::unique_ptr<ASTNode> expr) : expression(std::move(expr)) {}
    std::string getNodeType() const override { return "ExpressionStatementNode"; }
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::unique_ptr<ProgramNode> parseProgram();

private:
    std::vector<Token> tokens;
    size_t current_token_index;

    const Token& currentToken() const;
    void advance();
    Token expect(TokenType type);
    Token expect(TokenType type, const std::string& value);

    std::unique_ptr<ASTNode> parseStatement();
    // --- 변경: parseBlock()의 선언을 일치시킴 ---
    std::unique_ptr<BlockNode> parseBlock();
    // --- 여기까지 변경 ---
    std::unique_ptr<ASTNode> parseIfStatement();
    std::unique_ptr<ASTNode> parseWhileStatement();
    std::unique_ptr<ASTNode> parseFunctionDeclaration();
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseBinaryExpression(int precedence = 0);
    std::unique_ptr<ASTNode> parsePrimaryExpression();
};

} // namespace MyLang

#endif // MYLANG_PARSER_H
