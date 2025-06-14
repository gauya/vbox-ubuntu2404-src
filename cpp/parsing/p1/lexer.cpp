#include "lexer.h"
#include <sstream> // for std::stringstream (toString() helper)
#include <unordered_map> // for static keywords map

namespace MyLang {

// TokenType을 문자열로 변환하는 헬퍼 함수 (디버깅용)
std::string Token::toString() const {
    std::string type_str;
    switch (type) {
        case TokenType::KEYWORD: type_str = "KEYWORD"; break;
        case TokenType::IDENTIFIER: type_str = "IDENTIFIER"; break;
        case TokenType::NUMBER: type_str = "NUMBER"; break;
        case TokenType::OPERATOR: type_str = "OPERATOR"; break;
        case TokenType::PAREN_OPEN: type_str = "PAREN_OPEN"; break;
        case TokenType::PAREN_CLOSE: type_str = "PAREN_CLOSE"; break;
        case TokenType::BRACE_OPEN: type_str = "BRACE_OPEN"; break;
        case TokenType::BRACE_CLOSE: type_str = "BRACE_CLOSE"; break;
        case TokenType::SEMICOLON: type_str = "SEMICOLON"; break;
        case TokenType::STRING_LITERAL: type_str = "STRING_LITERAL"; break;
        case TokenType::COMMA: type_str = "COMMA"; break;
        case TokenType::END_OF_FILE: type_str = "END_OF_FILE"; break;
        default: type_str = "UNKNOWN"; break;
    }
    std::stringstream ss;
    ss << "Token(" << type_str << ", '" << value << "', Line: " << line << ", Col: " << column << ")";
    return ss.str();
}

// Lexer 클래스 멤버 변수 초기화
const std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"if", TokenType::KEYWORD},
    {"else", TokenType::KEYWORD},
    {"while", TokenType::KEYWORD},
    {"func", TokenType::KEYWORD},
    {"return", TokenType::KEYWORD} // 예시에 return 추가
    // ... 다른 키워드 추가
};

Lexer::Lexer(const std::string& source_code)
    : source(source_code), current_pos(0), current_line(1), current_column(0) {}

char Lexer::advance() {
    if (current_pos < source.length()) {
        char c = source[current_pos++];
        if (c == '\n') {
            current_line++;
            current_column = 0;
        } else {
            current_column++;
        }
        return c;
    }
    return '\0'; // EOF
}

char Lexer::peek() const {
    if (current_pos < source.length()) {
        return source[current_pos];
    }
    return '\0'; // EOF
}

void Lexer::skipWhitespace() {
    while (current_pos < source.length() && std::isspace(peek())) {
        advance();
    }
}

void Lexer::skipComment() {
    // '#'으로 시작하는 한 줄 주석 처리
    if (peek() == '#') {
        while (current_pos < source.length() && peek() != '\n') {
            advance();
        }
        // 개행 문자는 렉서가 자동으로 처리하도록 남겨둠
        skipWhitespace();
        /*
        // 주석 끝에 개행 문자가 있다면 소비 (ANDROID NDK 빌드 호환성을 위해 '\0'도 체크)
        if (peek() == '\n' || peek() == '\0') { // <--- '\0' 추가 (안전성 증대)
            advance(); // <--- 개행 문자 소비
        }*/
        
    }
}

Token Lexer::parseNumber() {
    int start_col = current_column;
    std::string num_str;
    while (current_pos < source.length() && std::isdigit(peek())) {
        num_str += advance();
    }
    // 간단한 정수만 처리
    return Token(TokenType::NUMBER, num_str, current_line, start_col);
}

Token Lexer::parseIdentifierOrKeyword() {
    int start_col = current_column;
    std::string ident_str;
    while (current_pos < source.length() && (std::isalnum(peek()) || peek() == '_')) {
        ident_str += advance();
    }

    // 키워드인지 확인
    auto it = keywords.find(ident_str);
    if (it != keywords.end()) {
        return Token(it->second, ident_str, current_line, start_col); // 키워드 토큰
    } else {
        return Token(TokenType::IDENTIFIER, ident_str, current_line, start_col); // 식별자 토큰
    }
}

Token Lexer::parseString() {
    int start_col = current_column;
    char quote_char = advance(); // '\"' 소비
    std::string str_val;
    while (current_pos < source.length() && peek() != quote_char && peek() != '\n') {
        str_val += advance();
    }
    if (peek() == '\"') {
        advance(); // '\"' 소비
        return Token(TokenType::STRING_LITERAL, str_val, current_line, start_col);
    } else {
        throw std::runtime_error("Unterminated string literal at line " + std::to_string(current_line) + ", column " + std::to_string(start_col));
    }
}

#include <stdio.h> // for static keywords map
Token Lexer::getNextToken() {
    while (current_pos < source.length()) {
        skipWhitespace(); // 공백 무시
        skipComment();    // 주석 무시
        if (current_pos >= source.length()) break; // 공백/주석만 남았으면 종료

        char c = peek();
        int start_col = current_column; // 토큰 시작 컬럼 저장

        if (std::isdigit(c)) {
            return parseNumber();
        } else if (std::isalpha(c) || c == '_') {
            return parseIdentifierOrKeyword();
        } else if (c == '\"' || c == '\'') {
            return parseString();
        } else if (c == '(') {
            advance(); return Token(TokenType::PAREN_OPEN, "(", current_line, start_col);
        } else if (c == ')') {
            advance(); return Token(TokenType::PAREN_CLOSE, ")", current_line, start_col);
        } else if (c == '{') {
            advance(); return Token(TokenType::BRACE_OPEN, "{", current_line, start_col);
        } else if (c == '}') {
            advance(); return Token(TokenType::BRACE_CLOSE, "}", current_line, start_col);
        } else if (c == ';') {
            advance(); return Token(TokenType::SEMICOLON, ";", current_line, start_col);
        } else if (c == ',') { // <--- Add this new block for comma!
            advance(); return Token(TokenType::COMMA, ",", current_line, start_col);
        } else if (c == '+') {
            advance(); return Token(TokenType::OPERATOR, "+", current_line, start_col);
        } else if (c == '-') {
            advance(); return Token(TokenType::OPERATOR, "-", current_line, start_col);
        } else if (c == '*') {
            advance(); return Token(TokenType::OPERATOR, "*", current_line, start_col);
        } else if (c == '/') {
            advance(); return Token(TokenType::OPERATOR, "/", current_line, start_col);
        } else if (c == '=') {
            advance();
            if (peek() == '=') { // == 연산자
                advance(); return Token(TokenType::OPERATOR, "==", current_line, start_col);
            }
            return Token(TokenType::OPERATOR, "=", current_line, start_col); // = 할당 연산자
        } else if (c == '>') {
            advance(); return Token(TokenType::OPERATOR, ">", current_line, start_col);
        } else if (c == '<') {
            advance(); return Token(TokenType::OPERATOR, "<", current_line, start_col);
        } 
        else {
            printf("[%c]\n",c);
        }
        // 알 수 없는 문자
        throw std::runtime_error("Unexpected character \'" + std::string(1, c) + "\' at line " + std::to_string(current_line) + ", column " + std::to_string(current_column));
    }
    return Token(TokenType::END_OF_FILE, "", current_line, current_column);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token token = getNextToken();
        tokens.push_back(token);
        if (token.type == TokenType::END_OF_FILE) {
            break;
        }
    }
    return tokens;
}

} // namespace MyLang
