#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <format>
#include <chrono>

// 직접 만든 렉서와 파서 헤더 파일 포함
#include "lexer.h"
#include "parser.h"

// AST 구조를 출력하기 위한 간단한 헬퍼 함수
void printAST(const MyLang::ASTNode* node, int indent = 0) {
    if (!node) return;

    std::string indent_str(indent * 2, ' ');

    // 각 AST 노드 타입에 따라 출력 로직 구현 (동적 캐스팅 사용)
    if (const MyLang::ProgramNode* program = dynamic_cast<const MyLang::ProgramNode*>(node)) {
        std::cout << indent_str << "Program:\n";
        for (const auto& stmt : program->statements) {
            printAST(stmt.get(), indent + 1);
        }
    } else if (const MyLang::IfStatementNode* if_stmt = dynamic_cast<const MyLang::IfStatementNode*>(node)) {
        std::cout << indent_str << "IfStatement:\n";
        std::cout << indent_str << "  Condition:\n";
        printAST(if_stmt->condition.get(), indent + 2);
        std::cout << indent_str << "  Body:\n";
        printAST(if_stmt->body.get(), indent + 2);
        if (if_stmt->else_body) {
            std::cout << indent_str << "  Else Body:\n";
            printAST(if_stmt->else_body.get(), indent + 2);
        }
    } else if (const MyLang::WhileStatementNode* while_stmt = dynamic_cast<const MyLang::WhileStatementNode*>(node)) {
        std::cout << indent_str << "WhileStatement:\n";
        std::cout << indent_str << "  Condition:\n";
        printAST(while_stmt->condition.get(), indent + 2);
        std::cout << indent_str << "  Body:\n";
        printAST(while_stmt->body.get(), indent + 2);
    } else if (const MyLang::FunctionDeclarationNode* func_decl = dynamic_cast<const MyLang::FunctionDeclarationNode*>(node)) {
        std::cout << indent_str << "FunctionDeclaration: " << func_decl->name << "\n";
        std::cout << indent_str << "  Params: ";
        for (const auto& param : func_decl->params) {
            std::cout << param << " ";
        }
        std::cout << "\n";
        std::cout << indent_str << "  Body:\n";
        printAST(func_decl->body.get(), indent + 2);
    } else if (const MyLang::BinaryExpressionNode* bin_expr = dynamic_cast<const MyLang::BinaryExpressionNode*>(node)) {
        std::cout << indent_str << "BinaryExpression: " << bin_expr->op << "\n";
        std::cout << indent_str << "  Left:\n";
        printAST(bin_expr->left.get(), indent + 2);
        std::cout << indent_str << "  Right:\n";
        printAST(bin_expr->right.get(), indent + 2);
    } else if (const MyLang::NumberLiteralNode* num_lit = dynamic_cast<const MyLang::NumberLiteralNode*>(node)) {
        std::cout << indent_str << "NumberLiteral: " << num_lit->value << "\n";
    } else if (const MyLang::IdentifierNode* id_node = dynamic_cast<const MyLang::IdentifierNode*>(node)) {
        std::cout << indent_str << "Identifier: " << id_node->name << "\n";
    } else {
        // 기타 노드 타입 (예: ExpressionStatementNode, BlockNode 등)
        std::cout << indent_str << "Unknown AST Node Type\n";
    }
}


// g++ -std=c++17 -Wall lexer.cpp parser.cpp main.cpp -o mylang_parser

#include <fstream>
#include <sstream>

int main(int argc,char *argv[]) {
    // 1. 테스트할 소스 코드 정의
    std::string source_code = R"(
        func add(a, b) {
            # This is a comment
            if (a > b) {
                return a + b;
            } else {
                return a - b;
            }
        }

        while (x < 10) {
            x = x + 1;
            if (x == 5) {
                y = y * 2;
            }
        }
        test_var = 123 + 456;
    )";

    try {
    std::ifstream sourf(argv[1]);
    std::cout << "open file : " << std::string(argv[1]) << std::endl;

    std::stringstream buffer;
      buffer << sourf.rdbuf();
      source_code = buffer.str();
      sourf.close();
    } catch (const std::exception& e) {
        std::cout << "예외 발생: " << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "알 수 없는 예외 발생!" << std::endl;
    }
      std::cout << "load file" << std::endl;

    std::cout << "--- Source Code ---" << std::endl;
    std::cout << source_code << std::endl;
    std::cout << "-------------------" << std::endl << std::endl;

    // 2. 렉싱 단계: 소스 코드를 토큰 리스트로 변환
    auto start = std::chrono::steady_clock::now();

    MyLang::Lexer lexer(source_code);
    std::vector<MyLang::Token> tokens = lexer.tokenize();
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "--- Tokens ---" << std::endl;
    try {
        for (const auto& token : tokens) {
            std::cout << format("{:4} {:<12} {:3} {:<15} :",token.line, MyLang::Lexer::tokentype_names.at(token.type), token.typestr, format("<{}>",MyLang::Lexer::tokenSubtype_names.at(token.subtype)));
            std::cout << token.toString() << std::endl;
        }
        std::cout << "--------------" << std::endl << std::endl;

        // 3. 파싱 단계: 토큰 리스트를 AST로 변환
        MyLang::Parser parser(tokens);
        std::unique_ptr<MyLang::ProgramNode> ast_root = parser.parseProgram();

        std::cout << "--- Abstract Syntax Tree (AST) ---" << std::endl;
        printAST(ast_root.get());
        std::cout << "----------------------------------" << std::endl;

    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
//        return 1;
    } catch (const std::exception & e) {
        std::cerr << "<Error :" << e.what() << ">" << std::endl;
//        return 1;
    }

    std::cout << std::format("토크나이징 시간: {}ms\n", duration.count());

    auto now = std::chrono::system_clock::now();
    // time_t로 변환
    //auto time = std::chrono::system_clock::to_time_t(now);
    // 로컬 시간으로 출력
    //std::cout << std::format("현재 시간: {:%Y-%m-%d %H:%M:%S}\n", *std::localtime(&time));
    std::cout << std::format("현재 시간: {:%Y-%m-%d %H:%M:%S}\n", now);

    return 0;
}

// ++ -std=c++17 -Wall lexer.cpp parser.cpp main.cpp -o mylang_parser

