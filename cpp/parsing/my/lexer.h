#ifndef __LEXER__H
#define __LEXER__H

#include <string>
#include <vector>
#include <stdexcept> // 예외 처리를 위해
#include <map>
#include <unordered_map>

#ifdef TEST
#include <iostream>
#endif // TEST

namespace GLexer {

// 토큰이 겹칠수도 있어 차례대로 해석

enum class TokenType : int {
    UNDEF = 0,
    KEYWORD,  // X 
    NAME,     // X
    STRING,   // 
    ALPA,     // constant, name, IDENTIFIER,, KEYWORD, STRING_LITERAL,
    NUMBER,   // constant,
    OPERATOR, // +-*/?=!~^%&
    SCHAR,    // ,.:;@$#
    SPACE,    // isspace()
    COMMENT,  // #, //, --, /* */, <% %>, ;, 
    BLOCK,    // (, {, [, <, ", ', """, ''', /**/, #,
    END_OF_FILE = -1  // EOF
};

// 차례
// eof, comment, space, alpa, number, block, oper, schar 

struct Token {
    TokenType   m_type;
    std::string m_subtype;
    std::string m_value;
    int m_line;
    int m_column;

    // 생성자
    Token() : m_type(TokenType::UNDEF), m_line(0), m_column(0) {}
    Token(TokenType type, const std::string& value, int line, int column)
      : m_type(type), m_value(value), m_line(line), m_column(column) {}

    // 디버깅을 위한 출력 (선택 사항)
    inline std::string toString() const { return m_value; }
};


class Lexer {
private:
  std::string m_str;
  std::vector<Token> m_toks;

  int m_pos;
  int m_line;
  int m_column;

  Token m_token;
public:
  explicit Lexer() 
  : m_pos(0), m_line(1), m_column(0) {};
  explicit Lexer( std::string& str ) 
  : m_str(str), m_pos(0), m_line(1), m_column(0) { tokenize(); };

  bool is_oper_char(int ch);
  bool is_block_char(int ch);
  bool is_special_char(int ch);

  // 다음 토큰을 반환하고 내부 포인터 이동
  Token getToken();
  // 모든 토큰을 리스트로 반환 (한 번에 렉싱)
  std::vector<Token> tokenize();
  
  // 현재 위치의 문자를 반환하고 포인터 이동
  char advance();
  // 현재 위치의 문자를 반환 (이동 없음)
  char peek() const;
  char npeek() const;
  
  // 특정 문자만큼 포인터 건너뛰기
  void skipWhitespace();
  Token parseNumber();
  Token parseName();
  Token parseString();
  Token parseBlock();
  Token parseComment();

  // 토큰 타입 맵 (문자열 -> TokenType)
  static const std::unordered_map<std::string, TokenType> keywords; // 키워드 맵 (정적 멤버)
  static const std::map<TokenType, std::string> tokentype_names;

  int load_file(const char * fn);
};

}

#endif // __LEXER__H
