#include <lexer.h>

namespace GLexer {

// Lexer 클래스 멤버 변수 초기화
const std::unordered_map<std::string, TokenType> Lexer::keywords = {
  {"if", TokenType::KEYWORD},
  {"else", TokenType::KEYWORD},
  {"while", TokenType::KEYWORD},
  {"func", TokenType::KEYWORD},
  {"return", TokenType::KEYWORD} // 예시에 return 추가
};

int Lexer::load_file( std::string fn ) {
  return -1; 
}

// peek()에서 '\' 를 처리해야할까
char Lexer::peek() const {
  if (m_pos < m_str.length()) {
    return m_str[m_pos];
  }
  return '\0'; // EOF
}

char Lexer::npeek() const {
  if ((m_pos+1) < m_str.length()) {
    return m_str[m_pos+1];
  }
  return '\0'; // EOF
}

char Lexer::advance() {
  if (m_pos < m_str.length()) {
    char c = m_str[m_pos++];
    if (c == '\n') {
      m_line++;
      m_column = 0;
    } else {
      m_column++;
    }
    return c;
  }
  return '\0'; // EOF
}

//  return lines
void Lexer::skipWhitespace() {
  while (current_pos < source.length() && std::isspace(peek())) {
    advance();
  }
}

// 이 함수를 부르기전에 comment문자임을 확인해야한다.
// //, /*, <%과 같은 두자리 이상의 문자로 된 comment는?
void Lexer::skipComment() {
  // 한줄 주석 처리
  if ((peek() == '#') || (peek() == '/' && npeek() == '/')) {
    int oidx = m_pos;
    m_token.m_line = m_line;
    m_token.m_column = m_column;

    while (m_pos < m_str.length() && peek() != '\n') {
      advance();
    }
    
    m_token.m_value = m_str.substring(oidx, (m_pos - oidx));
    m_token.m_type = TokenType::COMMENT;

    skipWhitespace();

    return m_token;
  } 
  // 여러줄 주석

}

Token Lexer::parseNumber() {
  int start_col = m_column;
  std::string num_str;
  while (m_pos < m_str.length() && std::isdigit(peek())) {
    num_str += advance();
  }
  // 간단한 정수만 처리
  return Token(TokenType::NUMBER, num_str, m_line, start_col);
}

Token Lexer::parseIdentifierOrKeyword() {
  int start_col = m_column;
  std::string ident_str;
  while (m_pos < m_str.length() && (std::isalnum(peek()) || peek() == '_')) {
    ident_str += advance();
  }

  // 키워드인지 확인
  auto it = keywords.find(ident_str);
  if (it != keywords.end()) {
    return Token(it->second, ident_str, m_line, start_col); // 키워드 토큰
  } else {
    return Token(TokenType::IDENTIFIER, ident_str, m_line, start_col); // 식별자 토큰
  }
}

Token Lexer::parseString() {
  int start_col = m_col;
  char quote_char = advance(); // '\"' 소비
  std::string str_val;
  while (m_pos < m_str.length() && peek() != quote_char && peek() != '\n') {
    str_val += advance();
  }
  if (peek() == '\"') {
    advance(); // '\"' 소비
    return Token(TokenType::STRING_LITERAL, str_val, m_line, start_col);
  } else {
    throw std::runtime_error("Unterminated string literal at line " + std::to_string(m_line) + ", column " + std::to_string(start_col));
  }
}

#include <stdio.h> // for static keywords map
Token Lexer::getNextToken() {
  while (m_pos < m_str.length()) {
    skipWhitespace(); // 공백 무시
    skipComment();  // 주석 무시
    if (m_pos >= m_str.length()) break; // 공백/주석만 남았으면 종료

    char c = peek();
    int start_col = m_col; // 토큰 시작 컬럼 저장

    if (std::isdigit(c)) {
      return parseNumber();
    } else if (std::isalpha(c) || c == '_') {
      return parseIdentifierOrKeyword();
    } else if (c == '\"' || c == '\'') {
      return parseString();
    } else if (c == '(') {
      advance(); return Token(TokenType::PAREN_OPEN, "(", m_line, start_col);
    } else if (c == ')') {
      advance(); return Token(TokenType::PAREN_CLOSE, ")", m_line, start_col);
    } else if (c == '{') {
      advance(); return Token(TokenType::BRACE_OPEN, "{", m_line, start_col);
    } else if (c == '}') {
      advance(); return Token(TokenType::BRACE_CLOSE, "}", m_line, start_col);
    } else if (c == ';') {
      advance(); return Token(TokenType::SEMICOLON, ";", m_line, start_col);
    } else if (c == ',') { // <--- Add this new block for comma!
      advance(); return Token(TokenType::COMMA, ",", m_line, start_col);
    } else if (c == '+') {
      advance(); return Token(TokenType::OPERATOR, "+", m_line, start_col);
    } else if (c == '-') {
      advance(); return Token(TokenType::OPERATOR, "-", m_line, start_col);
    } else if (c == '*') {
      advance(); return Token(TokenType::OPERATOR, "*", m_line, start_col);
    } else if (c == '/') {
      advance(); return Token(TokenType::OPERATOR, "/", m_line, start_col);
    } else if (c == '=') {
      advance();
      if (peek() == '=') { // == 연산자

        advance(); return Token(TokenType::OPERATOR, "==", m_line, start_col);
      }
      return Token(TokenType::OPERATOR, "=", m_line, start_col); // = 할당 연산자
    } else if (c == '>') {
      advance(); return Token(TokenType::OPERATOR, ">", m_line, start_col);
    } else if (c == '<') {
      advance(); return Token(TokenType::OPERATOR, "<", m_line, start_col);
    }
    
    else {
      printf("[%c]\n",c);
    }
    // 알 수 없는 문자
    throw std::runtime_error("Unexpected character \'" + std::string(1, c) + "\' at line " + std::to_string(m_line) + ", column " + std::to_string(m_col));
  }
  return Token(TokenType::END_OF_FILE, "", m_line, m_col;
}

std::vector<Token> Lexer::tokenize() {
  while (m_toks.type != TokenType::END_OF_FILE) {
    Token token = getNextToken();
    m_toks.push_back(token);
  }
  return m_toks;
}



} // namespace GLexer
