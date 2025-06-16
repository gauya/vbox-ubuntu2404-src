#include <lexer.h>
#include <iostream>  // std::cerr, std::cout 등
#include <fstream>   // std::ifstream
#include <string>    // std::string
#include <sstream>   // std::stringstream
#include <format>

namespace GLexer {

// Lexer 클래스 멤버 변수 초기화
const std::unordered_map<std::string, TokenType> Lexer::keywords = {
  {"if", TokenType::KEYWORD},
  {"else", TokenType::KEYWORD},
  {"while", TokenType::KEYWORD},
  {"func", TokenType::KEYWORD},
  {"return", TokenType::KEYWORD} // 예시에 return 추가
};

const std::map<TokenType, std::string> Lexer::tokentype_names = {
  { TokenType:: UNDEF, "undef" },
  { TokenType:: KEYWORD,"keyword" },
  { TokenType:: NAME, "name" },
  { TokenType:: STRING, "string" },
  { TokenType:: ALPA, "alpa" },
  { TokenType:: NUMBER, "number" },
  { TokenType:: OPERATOR,"operator" },
  { TokenType:: SCHAR, "special_char" },
  { TokenType:: COMMENT, "comment" },
  { TokenType:: SPACE, "space" },
  { TokenType:: BLOCK, "block" },
  { TokenType:: END_OF_FILE, "endoffile" }
};

// peek()에서 '\' 를 처리해야할까
char Lexer::peek() const {
  if (m_pos < (int)m_str.length()) {
    return m_str[m_pos];
  }
  return '\0'; // EOF
}

char Lexer::npeek() const {
  if ((m_pos+1) < (int)m_str.length()) {
    return m_str[m_pos+1];
  }
  return '\0'; // EOF
}

char Lexer::advance() {
  if (m_pos < (int)m_str.length()) {
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
  while (m_pos < (int)m_str.length() && std::isspace(peek())) {
    advance();
  }
}

// 이 함수를 부르기전에 comment문자임을 확인해야한다.
// //, /*, <%과 같은 두자리 이상의 문자로 된 comment는?
Token Lexer::parseComment() {
  // 한줄 주석 처리
  int start_col = m_column;
  std::string str;
  if ((peek() == '#') || (peek() == '/' && npeek() == '/')) {
    while (m_pos < (int)m_str.length() && peek() != '\n') {
      str += advance();
    }
    return Token(TokenType::COMMENT, str, m_line, start_col);
  } 
  // 여러줄 주석
  throw std::runtime_error("not comment [" +  std::to_string(start_col) + "]");
}

Token Lexer::parseNumber() {
  int start_col = m_column;
  std::string num_str;
  while (m_pos < (int)m_str.length() && std::isdigit(peek())) {
    num_str += advance();
  }
  // 간단한 정수만 처리
  return Token(TokenType::NUMBER, num_str, m_line, start_col);
}

Token Lexer::parseName() {
  int start_col = m_column;
  std::string ident_str;
  while (m_pos < (int)m_str.length() && (std::isalnum(peek()) || peek() == '_')) {
    ident_str += advance();
  }

  // 키워드인지 확인
  auto it = keywords.find(ident_str);
  if (it != keywords.end()) {
    return Token(it->second, ident_str, m_line, start_col); // 키워드 토큰
  } else {
    return Token(TokenType::NAME, ident_str, m_line, start_col); // 식별자 토큰
  }
}

Token Lexer::parseBlock() {
  int start_col = m_column;
  char quote_char = peek(); // advance(); // 

  if ( ! is_block_char(quote_char) ) {
    throw std::runtime_error("Unterminated string literal at line " + std::to_string(m_line) + ", column " + std::to_string(start_col));
  }
  char end_char = (quote_char == '(')? ')' :
    (quote_char == '{')? '}' :
    (quote_char == '[')? ']' : '>' ;

  std::string str_val;
  while (m_pos < (int)m_str.length() && peek() != end_char) {
    str_val += advance();
  }
  str_val += advance(); // end_char  
  return Token(TokenType::BLOCK, str_val, m_line, start_col);
}

Token Lexer::parseString() {
  int start_col = m_column;
  char quote_char = advance();  
  
  std::string str_val;
  while( m_pos < (int)m_str.length() && peek() != quote_char ) {
    str_val += advance();
  }
  advance();
  return Token(TokenType::STRING, str_val, m_line, start_col);
}

#include <string.h>

const char __block_chars[] = "(){}[]<>";
bool Lexer::is_block_char( int ch ) {
  return ( strchr( __block_chars, ch ))? true : false;
}

const char __oper_chars[] = "+-*/%!~|&^=<>?";
bool Lexer::is_oper_char( int ch ) {
  return ( strchr( __oper_chars, ch ))? true : false;
}
const char __special_chars[] = ",.:;@$`";
bool Lexer::is_special_char( int ch ) {
  return ( strchr( __special_chars, ch ))? true : false;
}


#include <stdio.h> // for static keywords map
Token Lexer::getToken() {
  skipWhitespace(); // 공백 무시
  
  int start_col = m_column; // 토큰 시작 컬럼 저장
  char c = peek();

  if ( (m_pos+1) > (int)m_str.length() ||  c == '\0' ) {
    return Token(TokenType::END_OF_FILE, "", m_line, m_column);
  }
  if( (c == '#') 
    || ( c == '/' && npeek() == '/') 
    || ( c == '/' && npeek() == '*') 
    || ( c == '*' && npeek() == '/')) {
    
    return parseComment();
  }
  if ( c == '\"' || c == '\'' || c == '`' ) {
    return parseString();
  }

  if (std::isdigit(c)) {
    return parseNumber();
  } 
  if (std::isalpha(c) || c == '_') {
    return parseName();
  } 
  if ( is_block_char(c) ) {
    //return parseBlock();
    advance();
    return Token( TokenType::BLOCK, std::string(1,c), m_line, start_col);
  }
  if ( is_oper_char(c) ) {
    advance();
    return Token( TokenType::OPERATOR, std::string(1,c), m_line, start_col);
  }
  if ( is_special_char(c) ) {
    advance();
    return Token( TokenType::SCHAR, std::string(1,c), m_line, start_col);
  }
  
  std::string str_val;
  while( m_pos < (int)m_str.length() ) {
    if ( peek() < ' ' || peek() > '~' ) {
      str_val += advance();
    } else break;
  }

  return Token(TokenType:: UNDEF, str_val, m_line, m_column );

  // 알 수 없는 문자
//  throw std::runtime_error("Unexpected character \'" + std::string(1, c) + "\' at line " + std::to_string(m_line) + ", column " + std::to_string(m_column));
}


std::vector<Token> Lexer::tokenize() {
  int cnt=0;
  while (m_token.m_type != TokenType::END_OF_FILE) {
    m_token = getToken();
    
    std::cout << std::format("{:04} {:04} {:<12} : {}\n", cnt++,m_token.m_line, tokentype_names.at(m_token.m_type), m_token.m_value );
    if(cnt > 2800) break;
    m_toks.push_back(m_token);
  }
  return m_toks;
}
 
int Lexer::load_file(const char *fn) {
  std::string path = fn;
  std::ifstream ifile(path);

  if ( ! ifile.is_open()) {    // !ifile.is_open()
      std::cerr << "Failed to open " << path << "\n";
      return -1;
  }

  std::stringstream buffer;
  buffer << ifile.rdbuf();
  m_str = buffer.str();
  
  ifile.close();
  

  std::cout << m_str;
  std::cout << "Load file(" << path << ") size = " << std::to_string(m_str.length()) << std::endl;
  std::cout << "=======================================================" << std::endl;

  return (int)m_str.length();
}

} // namespace GLexer

#ifdef TEST
int main(int argc, const char*argv[]) {
  GLexer::Lexer lex;
  lex.load_file(argv[1]);


  lex.tokenize();
}
#endif // TEST

