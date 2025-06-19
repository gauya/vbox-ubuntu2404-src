// parser.cpp
#include "parser.h"
#include <iostream>
#include <stdexcept>
#include <utility> // std::move를 위해

namespace MyLang {

// ... (Parser 생성자 및 expect, currentToken, advance 함수는 동일) ...

std::string tokenTypeToString(TokenType type) {
  return "";
}

// ... (parseProgram 함수는 동일) ...

std::unique_ptr<ASTNode> Parser::parseStatement() {
    if (currentToken().subtype == TokenSubtype::KEYWORD) {
        if (currentToken().value == "if") {
            return parseIfStatement();
        } else if (currentToken().value == "while") {
            return parseWhileStatement();
        } else if (currentToken().value == "func") {
            return parseFunctionDeclaration();
        } else if (currentToken().value == "return") {
            expect(TokenSubtype::KEYWORD, "return");
            auto expr = parseExpression();
            expect(TokenType::SCHAR,";");
            return std::make_unique<ReturnStatementNode>(std::move(expr));
        } else {
          // error 
        }
    }
    auto expr = parseExpression();
    expect(TokenType::SCHAR, ";");
    return std::make_unique<ExpressionStatementNode>(std::move(expr));
}

// --- 변경: parseBlock()의 반환 타입을 일치시킴 ---
std::unique_ptr<BlockNode> Parser::parseBlock() {
    expect(TokenSubtype::BRACE_OPEN); // '{'
    auto block_node = std::make_unique<BlockNode>();
    while (currentToken().subtype != TokenSubtype::BRACE_CLOSE &&
           currentToken().type != TokenType::END_OF_FILE) {
        block_node->statements.push_back(parseStatement());
    }
    expect(TokenSubtype::BRACE_CLOSE); // '}'
    return block_node;
}
// --- 여기까지 변경 ---

std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    expect(TokenSubtype::KEYWORD, "if");
    expect(TokenSubtype::PAREN_OPEN);
    auto condition = parseExpression();
    expect(TokenSubtype::PAREN_CLOSE);

    // --- 변경: body와 else_body에 직접 할당 가능 ---
    auto body = parseBlock(); // 이제 unique_ptr<BlockNode>를 반환
    
    std::unique_ptr<BlockNode> else_body = nullptr;
    if (currentToken().subtype == TokenSubtype::KEYWORD && currentToken().value == "else") {
        expect(TokenSubtype::KEYWORD, "else");
        else_body = parseBlock(); // 이제 unique_ptr<BlockNode>를 반환
    }

    auto if_node = std::make_unique<IfStatementNode>();
    if_node->condition = std::move(condition);
    if_node->body = std::move(body);
    if_node->else_body = std::move(else_body);
    return if_node;
}

std::unique_ptr<ASTNode> Parser::parseWhileStatement() {
    expect(TokenSubtype::KEYWORD, "while");
    expect(TokenSubtype::PAREN_OPEN);
    auto condition = parseExpression();
    expect(TokenSubtype::PAREN_CLOSE);

    // --- 변경: body에 직접 할당 가능 ---
    auto body = parseBlock();

    auto while_node = std::make_unique<WhileStatementNode>();
    while_node->condition = std::move(condition);
    while_node->body = std::move(body);
    return while_node;
}

std::unique_ptr<ASTNode> Parser::parseFunctionDeclaration() {
    expect(TokenSubtype::KEYWORD, "func");
    Token name_token = expect(TokenSubtype::IDENTIFIER);
    expect(TokenSubtype::PAREN_OPEN);
    
    std::vector<std::string> params;
    if (currentToken().subtype == TokenSubtype::IDENTIFIER) {
        params.push_back(expect(TokenSubtype::IDENTIFIER).value);
        while (currentToken().type == TokenType::OPERATOR && currentToken().value == ",") {
            expect(TokenType::OPERATOR, ",");
            params.push_back(expect(TokenSubtype::IDENTIFIER).value);
        }
    }
    expect(TokenSubtype::PAREN_CLOSE);

    // --- 변경: body에 직접 할당 가능 ---
    auto body = parseBlock();

    auto func_node = std::make_unique<FunctionDeclarationNode>();
    func_node->name = name_token.value;
    func_node->params = std::move(params);
    func_node->body = std::move(body);
    return func_node;
}

// ASTNode::getNodeType()은 virtual이므로, dynamic_cast를 대체하는 데 유용합니다.
// (main.cpp의 printAST 함수에서 활용)

Parser::Parser(std::vector<Token> tokens)
    : tokens(std::move(tokens)), current_token_index(0) {
    if (this->tokens.empty()) {
        this->tokens.push_back(Token(TokenType::END_OF_FILE, "", 0, 0, {}));
    }
}

const Token& Parser::currentToken() const {
    if (current_token_index >= tokens.size()) {
        return tokens.back();
    }
    return tokens[current_token_index];
}

void Parser::advance() {
    if (current_token_index < tokens.size()) {
        current_token_index++;
    }
}

Token Parser::expect(TokenType type) {
    const Token& current = currentToken();
    if (current.type == type) {
        Token consumed_token = current;
        advance();
        return consumed_token;
    } else {
        throw std::runtime_error("Syntax Error: Expected " + tokenTypeToString(type) +
                                 ", got " + current.toString() +
                                 " at line " + std::to_string(current.line) +
                                 ", column " + std::to_string(current.column));
    }
}

Token Parser::expect(TokenType type, const std::string& value) {
    const Token& current = currentToken();
    if (current.type == type && current.value == value) {
        Token consumed_token = current;
        advance();
        return consumed_token;
    } else {
        throw std::runtime_error("Syntax Error: Expected '" + value + "' (type " + tokenTypeToString(type) +
                                 "), got " + current.toString() +
                                 " at line " + std::to_string(current.line) +
                                 ", column " + std::to_string(current.column));
    }
}

Token Parser::expect(TokenSubtype type, const std::string& value) {
    const Token& current = currentToken();
    if (current.subtype == type && (value == "" || current.value == value)) {
        Token consumed_token = current;
        advance();
        return consumed_token;
    } else {
        throw std::runtime_error("Syntax Error: Expected '" + value + "' (type " + tokenTypeToString(current.type) +
                                 "), got " + current.toString() +
                                 " at line " + std::to_string(current.line) +
                                 ", column " + std::to_string(current.column));
    }
}


std::unique_ptr<ProgramNode> Parser::parseProgram() {
    auto program = std::make_unique<ProgramNode>();
    while (currentToken().type != TokenType::END_OF_FILE) {
        program->statements.push_back(parseStatement());
    }
    return program;
}

int getOperatorPrecedence(const std::string& op) {
    if (op == "+" || op == "-") return 1;
    if (op == "*" || op == "/") return 2;
    if (op == "==" || op == ">" || op == "<" || op == "=") return 0; // 할당 연산자 우선순위 추가
    return -1;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    return parseBinaryExpression(0);
}

std::unique_ptr<ASTNode> Parser::parseBinaryExpression(int precedence) {
    auto left = parsePrimaryExpression();

    while (currentToken().type == TokenType::OPERATOR &&
           getOperatorPrecedence(currentToken().value) >= precedence) {
        std::string op = expect(TokenType::OPERATOR).value;
        int next_precedence = getOperatorPrecedence(op);
        // 왼쪽 결합성 연산자 (대부분의 이진 연산자: +, -, *, /)
        // 같은 우선순위의 다음 연산자는 현재 연산자와 같거나 낮은 우선순위로 파싱
        auto right = parseBinaryExpression(next_precedence + 1);

        auto binary_node = std::make_unique<BinaryExpressionNode>();
        binary_node->left = std::move(left);
        binary_node->op = op;
        binary_node->right = std::move(right);
        left = std::move(binary_node);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parsePrimaryExpression() {
    const Token& current = currentToken();
    if (current.type == TokenType::NUMBER) {
        // NumberLiteralNode의 생성자에 int 인자를 전달
        return std::make_unique<NumberLiteralNode>(std::stoi(expect(TokenType::NUMBER).value));
    } else if (current.subtype == TokenSubtype::IDENTIFIER) {
        // IdentifierNode의 생성자에 std::string 인자를 전달
        return std::make_unique<IdentifierNode>(expect(TokenSubtype::IDENTIFIER).value);
    } else if (current.subtype == TokenSubtype::PAREN_OPEN) {
        expect(TokenSubtype::PAREN_OPEN);
        auto expr = parseExpression();
        expect(TokenSubtype::PAREN_CLOSE);
        return expr;
    } else if (current.subtype == TokenSubtype::STRING_LITERAL) { // 문자열 리터럴 추가
        // StringLiteralNode 정의 필요. 여기서는 임시로 IdentifierNode 사용
        return std::make_unique<IdentifierNode>(expect(TokenSubtype::STRING_LITERAL).value); // TODO: StringLiteralNode로 변경
    }
    // TODO: 함수 호출 등 추가

    throw std::runtime_error("Syntax Error: Unexpected token in primary expression: " + current.toString());
}

// ... (getOperatorPrecedence, parseExpression, parseBinaryExpression, parsePrimaryExpression 함수는 동일) ...

} // namespace MyLang
