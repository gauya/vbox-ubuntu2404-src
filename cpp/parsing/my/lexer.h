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

namespace MyLang {

// 토큰이 겹칠수도 있어 차례대로 해석

enum class TokenType : int {
    UNDEF = 0,
    NAME,     // X
    STRING,   // 
    NUMBER,   // constant,
    OPERATOR, // +-*/?=!~^%&
    SCHAR,    // ,.:;@$#
    SPACE,    // isspace()
    COMMENT,  // #, //, --, /* */, <% %>, ;, 
    BLOCK,    // (, {, [, <, ", ', """, ''', /**/, #,
    END_OF_FILE  // EOF
};

enum class TokenSubtype : int {
    UNDEF = 0,
    NAME,     // ============================================
    KEYWORD,  // if, while
    IDENTIFIER, // function, variable name
    DATTYPE,  // int
    PREKEY,   // #include

    STRING,   //           =================================================== 
    STRING_LITERAL,
    
    NUMBER,   // constant, =====================================================
    INTEGER,  // decimal, 
    FLOAT,    // 
    BIN_NUM,  // 0b
    OCT_NUM,  // 0
    HEX_NUM,  // 0x
    
    OPERATOR, // +-*/?=!~^%& ================================================
    ARTHMETIC_OP,
    ASSIGN_OP,  // =, -=, +=, *=. /=, %=
    SIGN_OP,    // +, -
    INCRE_OP,   // ++,--
    RELATIVE_OP,// >, <, >=, <=, !=, ==
    LOGIC_OP,   // ||, &&, !
    BITWISE_OP, // |, &, ^
    STRUCT_OP,  // ., ->
    SCOPE_OP,   // ::
    ETC_OP,  // sizeof, new, delete 
    
    SCHAR,    // ,.:;@$# =======================================================
    
    SPACE,    // isspace() \b, \t, \n, \r, , \f,   
    
    COMMENT,  // #, //, --, /* */, <% %>, ;, ========================================
    LINE_COMMENT,  // #, //, --
    BLOCK_COMMENT, // /**/
    
    BLOCK,    // (, {, [, <, ", ', , ''', /**/, #, ================================
    PAREN_OPEN,
    PAREN_CLOSE,
    BRACE_OPEN,
    BRACE_CLOSE,
    SQUARE_BRACKET_OPEN,
    SQUARE_BRACKET_CLOSE,
    ANGLE_BRACKET_OPEN,
    ANGLE_BRACKET_CLOSE,
    
    END_OF_FILE  // EOF
};

// 차례
// eof, comment, space, alpa, number, block, oper, schar 

struct Token {
    TokenType   type;
    TokenSubtype subtype;
    std::string typestr;
    std::string value;
    size_t line;
    size_t column;

    // 생성자
    Token() : type(TokenType::UNDEF), subtype(TokenSubtype::UNDEF), line(0), column(0) {}
    Token(TokenType type, const std::string& value, size_t line, size_t column);
    Token(TokenType type, const std::string& value, size_t line, size_t column, const std::string& ts, TokenSubtype) : type(type), subtype(subtype), typestr(ts), value(value), line(line), column(column) {};

    TokenSubtype set_subtype();
    // 디버깅을 위한 출력 (선택 사항)
    inline std::string toString() const { return value; }
};


class Lexer {
private:
  std::string m_str;
  std::vector<Token> m_toks;

  size_t m_pos;
  size_t m_line;
  size_t m_column;

public:
  explicit Lexer() 
  : m_pos(0), m_line(1), m_column(0) {};
  explicit Lexer( std::string& str ) 
  : m_str(str), m_pos(0), m_line(1), m_column(0) { tokenize(); };

  bool is_oper_char(int ch);
  bool is_block_char(int ch);
  bool is_special_char(int ch);
  inline size_t length() const { return m_str.length(); }
  inline size_t tokens() const { return m_toks.size(); }
  inline size_t line() const { return m_line; }

  // 다음 토큰을 반환하고 내부 포인터 이동
  Token getToken();
  // 모든 토큰을 리스트로 반환 (한 번에 렉싱)
  std::vector<Token> tokenize();
  
  // 현재 위치의 문자를 반환하고 포인터 이동
  char advance();
  char nchar();
  // 현재 위치의 문자를 반환 (이동 없음)
  char peek() const;
  char npeek() const;
  
  // 특정 문자만큼 포인터 건너뛰기
  void skipWhitespace();
  Token parseOperator();
  Token parseNumber();
  Token parseName();
  Token parseString();
  Token parseBlock();
  Token parseComment();

  //std::vector<Token>operator=() { return m_toks; }
  const std::vector<Token>& getTokens() const { return m_toks; }

  // 토큰 타입 맵 (문자열 -> TokenType)
  static const std::unordered_map<std::string, TokenSubtype> keywords; // 키워드 맵 (정적 멤버)
  static const std::map<TokenType, std::string> tokentype_names;
  static const std::map<TokenSubtype, std::string> tokenSubtype_names;
  static const std::map<std::string, TokenSubtype, std::greater<> > operators_subtype;
  int load_file(const char * fn);
};

}

#endif // __LEXER__H
