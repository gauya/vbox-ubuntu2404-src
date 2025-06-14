#ifndef MYLANG_LEXER_H
#define MYLANG_LEXER_H

#include <string>
#include <vector>
#include <stdexcept> // 예외 처리를 위해
#include <unordered_map>

namespace MyLang {

// 토큰의 종류를 정의하는 열거형 (enum)
enum class TokenType {
    KEYWORD,      // if, else, while, func 등
    IDENTIFIER,   // 변수명, 함수명 등
    NUMBER,       // 숫자 리터럴 (123, 4.5)
    OPERATOR,     // +, -, *, /, =, ==, >, < 등
    PAREN_OPEN,   // (
    PAREN_CLOSE,  // )
    BRACE_OPEN,   // {
    BRACE_CLOSE,  // }
    SEMICOLON,    // ;
    STRING_LITERAL, // "hello"
    COMMA, //
    END_OF_FILE   // 파일의 끝을 나타내는 특별한 토큰
    // ... 필요한 다른 토큰 타입 추가
};

// 각 토큰을 표현하는 구조체 또는 클래스
struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;

    // 생성자
    Token(TokenType type, const std::string& value, int line, int column)
        : type(type), value(value), line(line), column(column) {}

    // 디버깅을 위한 출력 (선택 사항)
    std::string toString() const;
};

// 렉서 클래스
class Lexer {
public:
    // 생성자: 소스 코드를 입력받음
    explicit Lexer(const std::string& source_code);

    // 다음 토큰을 반환하고 내부 포인터 이동
    Token getNextToken();

    // 모든 토큰을 리스트로 반환 (한 번에 렉싱)
    std::vector<Token> tokenize();

private:
    std::string source;
    int current_pos;
    int current_line;
    int current_column;

    // 현재 위치의 문자를 반환하고 포인터 이동
    char advance();
    // 현재 위치의 문자를 반환 (이동 없음)
    char peek() const;
    // 특정 문자만큼 포인터 건너뛰기
    void skipWhitespace();
    void skipComment();
    // 숫자, 식별자, 문자열 등을 파싱하는 헬퍼 함수
    Token parseNumber();
    Token parseIdentifierOrKeyword();
    Token parseString();

    // 토큰 타입 맵 (문자열 -> TokenType)
    static const std::unordered_map<std::string, TokenType> keywords; // 키워드 맵 (정적 멤버)
};

} // namespace MyLang

#endif // MYLANG_LEXER_H
