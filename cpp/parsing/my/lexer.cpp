#include <iostream>  // std::cerr, std::cout 등
#include <fstream>   // std::ifstream
#include <string>    // std::string
#include <sstream>   // std::stringstream
#include <format>
#include <string.h>
#include "lexer.h"

namespace MyLang {

#define __C__      1
#define __PYTHON__ 2
#define __BASH__   3
#define __SQL__    4
#define __CPP__    5

#define LANGMODE  __CPP__

struct BlockComment {
  const char *l, *r;
};

/////////////////////////////////////////////////////////////////////////////////////////
#ifndef LANGMODE
const std::map<TokenType, std::string> Lexer::tokentype_names = {
  { TokenType:: UNDEF, "undef" },
  { TokenType:: NAME, "name" },
  { TokenType:: CONST, "string" },
  { TokenType:: NUMBER, "number" },
  { TokenType:: OPERATOR,"operator" },
  { TokenType:: SCHAR, "sp_char" },
  { TokenType:: COMMENT, "comment" },
  { TokenType:: SPACE, "space" },
  { TokenType:: BLOCK, "block" },
  { TokenType:: END_OF_FILE, "endoffile" }
};

// Lexer 클래스 멤버 변수 초기화
const std::unordered_map<std::string, TokenSubtype> Lexer::keywords = {
  {"if", TokenSubtype::KEYWORD},
  {"else", TokenSubtype::KEYWORD},
  {"do", TokenSubtype::KEYWORD},
  {"while", TokenSubtype::KEYWORD},
  {"for", TokenSubtype::KEYWORD},
  {"switch", TokenSubtype::KEYWORD},
  {"case", TokenSubtype::KEYWORD},
  {"break", TokenSubtype::KEYWORD},
  {"default", TokenSubtype::KEYWORD},
  {"auto", TokenSubtype::KEYWORD},
  {"def", TokenSubtype::KEYWORD},
  {"try", TokenSubtype::KEYWORD},
  {"catch", TokenSubtype::KEYWORD},
  {"throw", TokenSubtype::KEYWORD},
  {"function", TokenSubtype::KEYWORD},

  {"include", TokenSubtype::PREKEY},
  {"define", TokenSubtype::PREKEY},
  {"undef", TokenSubtype::PREKEY},
  {"ifdef", TokenSubtype::PREKEY},
  {"endif", TokenSubtype::PREKEY},

  {"void", TokenSubtype::DATTYPE},
  {"char", TokenSubtype::DATTYPE},
  {"unsigned", TokenSubtype::DATTYPE},
  {"int", TokenSubtype::DATTYPE},
  {"long", TokenSubtype::DATTYPE},
  {"float", TokenSubtype::DATTYPE},
  {"double", TokenSubtype::DATTYPE},
  {"bool", TokenSubtype::DATTYPE},

  {"true", TokenSubtype::DATTYPE},
  {"false", TokenSubtype::DATTYPE},

  {"struct", TokenSubtype::DATTYPE},
  {"class", TokenSubtype::DATTYPE},
  {"enum", TokenSubtype::DATTYPE},
  {"typedef", TokenSubtype::DATTYPE},

  {"const", TokenSubtype::DATTYPE},
  {"static", TokenSubtype::DATTYPE},
  {"namespace", TokenSubtype::DATTYPE},
  {"extern", TokenSubtype::DATTYPE},
  {"inline", TokenSubtype::DATTYPE},
  
  {"private", TokenSubtype::DATTYPE},
  {"protected", TokenSubtype::DATTYPE},
  {"public", TokenSubtype::DATTYPE},

  {"return", TokenSubtype::KEYWORD} 
};

const std::map<TokenSubtype, std::string> Lexer::tokenSubtype_names = {
	{ TokenSubtype::UNDEF, "undef" },
	{ TokenSubtype::NAME, "name" },
	{ TokenSubtype::KEYWORD, "keyword" },
	{ TokenSubtype::IDENTIFIER, "identifier" },
	{ TokenSubtype::DATTYPE, "dattype" },
	{ TokenSubtype::PREKEY, "prekey" },
	{ TokenSubtype::CONST, "const" },
	{ TokenSubtype::STRING_LITERAL, "string_literal" },
	{ TokenSubtype::NUMBER, "number" },
	{ TokenSubtype::INTEGER, "integer" },
	{ TokenSubtype::FLOAT, "float" },
	{ TokenSubtype::BIN_NUM, "bin_num" },
	{ TokenSubtype::OCT_NUM, "oct_num" },
	{ TokenSubtype::HEX_NUM, "hex_num" },
	{ TokenSubtype::OPERATOR, "operator" },
	{ TokenSubtype::ARTHMETIC_OP, "arthmetic_op" },
	{ TokenSubtype::ASSIGN_OP, "assign_op" },
	{ TokenSubtype::SIGN_OP, "sign_op" },
	{ TokenSubtype::INCRE_OP, "incre_op" },
	{ TokenSubtype::RELATIVE_OP, "relation_op" },
	{ TokenSubtype::LOGIC_OP, "logic_op" },
	{ TokenSubtype::BITWISE_OP, "bitwise_op" },
	{ TokenSubtype::STRUCT_OP, "struct_op" },
	{ TokenSubtype::SCOPE_OP, "scope_op" },
	{ TokenSubtype::UNARY_OP, "unary_op" },
	{ TokenSubtype::ETC_OP, "etc_op" },
	{ TokenSubtype::SCHAR, "schar" },
	{ TokenSubtype::SPACE, "space" },
	{ TokenSubtype::COMMENT, "comment" },
	{ TokenSubtype::LINE_COMMENT, "line_comment" },
	{ TokenSubtype::BLOCK_COMMENT, "block_comment" },
	{ TokenSubtype::BLOCK, "block" },
	{ TokenSubtype::PAREN_OPEN, "paren_open" },
	{ TokenSubtype::PAREN_CLOSE, "paren_close" },
	{ TokenSubtype::BRACE_OPEN, "brace_open" },
	{ TokenSubtype::BRACE_CLOSE, "brace_close" },
	{ TokenSubtype::SQUARE_BRACKET_OPEN, "square_bracket_open" },
	{ TokenSubtype::SQUARE_BRACKET_CLOSE, "square_bracket_close" },
	{ TokenSubtype::ANGLE_BRACKET_OPEN, "angle_bracket_open" },
	{ TokenSubtype::ANGLE_BRACKET_CLOSE, "angle_bracket_close" },
	{ TokenSubtype::END_OF_FILE,"end_of_file"} 
};

const std::map<std::string, TokenSubtype, std::greater<> > Lexer::operators_subtype = {
  { "<<=", TokenSubtype::BITWISE_OP },
  { ">>=", TokenSubtype::BITWISE_OP },
  { "==",  TokenSubtype::RELATIVE_OP },
  { ">=",  TokenSubtype::RELATIVE_OP },
  { "<=",  TokenSubtype::RELATIVE_OP },
  { "!=",  TokenSubtype::RELATIVE_OP },
  { "||",  TokenSubtype::LOGIC_OP },
  { "&&",  TokenSubtype::LOGIC_OP },
  { "++",  TokenSubtype::INCRE_OP },
  { "--",  TokenSubtype::INCRE_OP },
  { "::",  TokenSubtype::SCOPE_OP },
  { "+=",  TokenSubtype::ASSIGN_OP },
  { "-=",  TokenSubtype::ASSIGN_OP },
  { "*=",  TokenSubtype::ASSIGN_OP },
  { "/=",  TokenSubtype::ASSIGN_OP },
  { "%=",  TokenSubtype::ARTHMETIC_OP },
  { "|=",  TokenSubtype::BITWISE_OP },
  { "&=",  TokenSubtype::BITWISE_OP },
  { "^=",  TokenSubtype::BITWISE_OP },
  { "->",  TokenSubtype::STRUCT_OP },
  { ">>",  TokenSubtype::SHIFT_OP },
  { "<<",  TokenSubtype::SHIFT_OP },
  { "+",   TokenSubtype::ARTHMETIC_OP },
  { "-",   TokenSubtype::ARTHMETIC_OP },
  { "*",   TokenSubtype::ARTHMETIC_OP },
  { "/",   TokenSubtype::ARTHMETIC_OP },
  { "%",   TokenSubtype::ARTHMETIC_OP },
  { "!",   TokenSubtype::LOGIC_OP },
  { ".",   TokenSubtype::STRUCT_OP },
  { "=",   TokenSubtype::ASSIGN_OP },
  { ">",   TokenSubtype::RELATIVE_OP },
  { "<",   TokenSubtype::RELATIVE_OP },
  { "|",   TokenSubtype::BITWISE_OP },
  { "&",   TokenSubtype::BITWISE_OP },
  { "^",   TokenSubtype::BITWISE_OP },
  { "~",   TokenSubtype::BITWISE_OP },
  { "?",   TokenSubtype::LOGIC_OP },
//  { "+",   TokenSubtype::UNARY_OP },
//  { "-",   TokenSubtype::UNARY_OP },
  { "", TokenSubtype::END_OF_FILE }
};

const char *_comment_line_strs[] =  { "//", "#", "--", ";", NULL };
const _comment_block BlockComment = { "/*", "*/" };

const char *_comment_block_strs[] =  { "/*", "*/", NULL };
const char __operator_chars[] = "+-*/%!~^|&=<>:?~.";

const char __comment_chars[] = "/*-;#";
const char __block_chars[] = "(){}[]<>"; // /**/, <%%>
const char __oper_chars[] = "+-*/%!~|&^=<>?:";
const char __special_chars[] = ",.:;@$`";

#else

#if ( LANGMODE == __CPP__ )
#include "cpp.def"
#elif ( LANGMODE == __PYTHON__ )
#include "python.def"
#elif ( LANGMODE == __SQL__ )
#include "sql.def"
#elif ( LANGMODE == __BASH__ )
#include "bash.def"
#endif

#endif // #ifndef LANGMODE

//=====================================================================================
bool comp_str_charp(const std::string& str, size_t at, const char *chs) {
    // 1. const char* (chs)의 nullptr 검사
    if (chs == nullptr) {
        return false;
    }

    size_t str_len = str.length();
    size_t chs_len = strlen(chs);

    // 2. 비교하려는 범위가 원본 문자열의 경계를 벗어나는지 검사
    //    at이 str_len보다 크거나 같으면, 시작 위치 자체가 유효하지 않음
    //    (at + chs_len)이 str_len보다 크면, chs가 str의 끝을 넘어감
    if (at >= str_len || (at + chs_len) > str_len) {
        return false;
    }

    // 3. std::string의 내부 데이터에 접근하기 위해 .data() 사용
    //    .data()는 C++11부터 null-terminated C-string을 반환함을 보장
    const char* p_str = str.data() + at; // str의 시작 지점 + at 만큼 이동한 포인터

    // chs 문자열을 순회하기 위한 임시 포인터
    const char* p_chs = chs;

    // 4. 문자열 비교 루프
    while (*p_chs != '\0') { // chs의 끝(null-terminator)에 도달할 때까지
        if (*p_str != *p_chs) {
            return false; // 문자가 다르면 바로 false 반환
        }
        p_str++; // 다음 문자로 이동
        p_chs++; // 다음 문자로 이동
    }

    return true; // 모든 문자가 일치했으므로 true 반환
}

int find_index(const char* pp, const std::string& str, size_t pos=0) {
  if( !pp || ((strlen(pp)+pos) >= str.length()) )
    return -1;

  size_t idx = str.find(pp, pos);
  if ( idx != std::string::npos ) { // -1
    return (int)(idx - pos);
  }
  return -1;
}

int find_index(const char* pp[], const std::string& str, size_t pos=0) {
  if( !pp || (pos >= str.length()) )
    return -1;

  size_t idx;
  for (int i = 0; pp[i]; ++i) {
      if ((idx=str.find(pp[i], pos)) != std::string::npos) {
          return (int)(idx - pos);
      }
  }
  return -1;
}

int find_index(const char* p, int ch) {
  if( !p )
    return -1;

  for( int i=0; p[i]; i++ ) {
    if(ch == p[i]) return i;
  }
  return -1;
}


bool is_block_char( int ch ) {
  return ( strchr( __block_chars, ch ))? true : false;
}

bool is_oper_char( int ch ) {
  return ( strchr( __oper_chars, ch ))? true : false;
}
bool is_special_char( int ch ) {
  return ( strchr( __special_chars, ch ))? true : false;
}
bool is_comment_char( int ch ) {
  return ( strchr( __comment_chars, ch ))? true : false;
}

TokenSubtype Token::set_subtype() {
  int idx;
  char ch;
  switch(type) {
  case TokenType::NAME:
    try {
      subtype = MyLang::Lexer::keywords.at(value);
    } catch(...) {
      subtype = TokenSubtype::NAME;
    }
    typestr = ""; // == value
    break;
  case TokenType::NUMBER:
    typestr = "";
    subtype = TokenSubtype::NUMBER;
    break;
  case TokenType::OPERATOR:
    // already 
#ifdef DEBUG    
    std::cout << "(" << typestr << ") " << std::endl;
#endif
    break;
  case TokenType::SCHAR:
    typestr = value;
    subtype = TokenSubtype::SCHAR;
    break;
  case TokenType::COMMENT:
    subtype = TokenSubtype::COMMENT;
    if(( idx=find_index( _comment_line_strs, typestr )) != -1 ) {
      typestr = _comment_line_strs[idx];
      subtype = TokenSubtype::LINE_COMMENT;
    } else {
      typestr = _comment_block.l;
      subtype = TokenSubtype::BLOCK_COMMENT;
    } 
    break;
  case TokenType::BLOCK:
    typestr = value.at(0);
    ch = value.at(0);
    subtype = TokenSubtype::BLOCK;
    if ( ch == '(') { subtype = TokenSubtype::PAREN_OPEN; } 
    else if ( ch == ')') { subtype = TokenSubtype::PAREN_CLOSE; }
    else if ( ch == '{') { subtype = TokenSubtype::BRACE_OPEN; }
    else if ( ch == '}') { subtype = TokenSubtype::BRACE_CLOSE; }
    else if ( ch == '[') { subtype = TokenSubtype::SQUARE_BRACKET_OPEN; }
    else if ( ch == ']') { subtype = TokenSubtype::SQUARE_BRACKET_CLOSE; }
    else if ( ch == '<') { subtype = TokenSubtype::ANGLE_BRACKET_OPEN; }
    else if ( ch == '>') { subtype = TokenSubtype::ANGLE_BRACKET_CLOSE; }
    else {
#ifdef DEBUG
  std::cout << "Token Block Unknown : " << value << " --" << std::endl;
#endif
    }
    break;
  case TokenType::SPACE:
    typestr = std::string(1,value.at(0)); // value.at(0)
    subtype = TokenSubtype::SPACE;
    break;
  default:
    typestr = "";
    subtype = TokenSubtype::UNDEF;
  }

#ifdef DEBUG
  try {
  std::cout << "[" << value << "] " << (int)subtype << ":";  
  std::cout << Lexer::tokenSubtype_names.at(subtype) << std::endl;
  } catch(std::exception & e) {
    std::cerr << e.what() << std::endl;
  }
#endif

  return subtype;
}

Token::Token( TokenType type, const std::string& value, size_t line, size_t column ) 
  : type(type), subtype((TokenSubtype)type), value(value), line(line), column(column) {
  set_subtype();
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

char Lexer::nchar() {
  char c = m_str[m_pos];

  if ( c == '\\' ) {
    m_pos++; m_column++;
    char nc = peek();
    if( strchr("fb\"\'nrt\\", nc) ) {
      
      switch(nc) {
      case 'b': c = '\b'; break;
      case 'n': c = '\n'; break;
      case 'r': c = '\r'; break;
      case 't': c = '\t'; break;
      case '\\': c = '\\'; break;
      case '\"': c = '\"'; break;
      case '\'': c = '\''; break;

      default:
        c = nc;
        break;
      }
    } else {
      // error
    }
  }
  return c;
}

//  return lines
void Lexer::skipWhitespace() {
  while (m_pos < m_str.length() && std::isspace(peek())) {
    advance();
  }
}

////////////////////////////////////////////////////////////////////////////////

// 이 함수를 부르기전에 comment 시작문자임을 확인해야한다.
Token Lexer::parseComment() {
  size_t start_col = m_column;
  size_t start_line = m_line;

  std::string str = "";
  int i;

  if ( (i=find_index(_comment_line_strs, m_str, m_pos)) == 0 ) {
    while (m_pos < m_str.length() && peek() != '\n') {
      str += advance();
    }
    return Token(TokenType::COMMENT, str, m_line, start_col,_comment_line_strs[i], TokenSubtype::LINE_COMMENT);
  } 

  if ( (i = find_index(_comment_block.l, m_str, m_pos)) == 0 ) { 
    int len = m_str.find(_comment_block.r, m_pos + strlen(_comment_block.l));
    if( len == -1 ) {
      throw std::runtime_error("left block comment string not found");
    }
    len += strlen(_comment_block.r) - m_pos;
    while( len-- > 0 ) {
      str += advance();
    }

    return Token(TokenType::COMMENT, str, start_line, start_col,_comment_block.l, TokenSubtype::BLOCK_COMMENT);
  }
  // 오른쪽 블럭을 못찾음
  throw std::runtime_error("not comment [" +  std::to_string(start_col) + "]");
}


Token Lexer::parseNumber() {
  std::string str;
  char ch = peek();
  char nh = npeek();
  Token tok;

  tok.type = TokenType::NUMBER;
  tok.typestr = 'd';  
  tok.subtype = TokenSubtype::INTEGER;
  tok.line = m_line;
  tok.column = m_column;


  if ( ch == '0' && ( nh == 'b' || nh == 'x' || (nh >= '0' && nh <= '7') ) ) {
    tok.value += advance();

    if( nh == 'x' ) {  // hexa
      tok.typestr = 'x';  
      tok.value += advance();
      tok.subtype = TokenSubtype::HEX_NUM;

      while (m_pos < m_str.length() && std::isxdigit(peek())) {
        tok.value += advance();
      }

    } else if( nh == 'b' ) {  // bin
      tok.typestr = 'b';  
      tok.value += advance();
      tok.subtype = TokenSubtype::BIN_NUM;
    
      while (m_pos < m_str.length() && ( peek() == '0' || peek() == '1')) {
        tok.value += advance();
      }
    } else {           // octal
      tok.typestr = 'o';  
      tok.subtype = TokenSubtype::OCT_NUM;
    
      while (m_pos < m_str.length() && ( peek() >= '0' && peek() <= '7')) {
        tok.value += advance();
      }
    }
  } else {
    while (m_pos < m_str.length() && std::isdigit(peek())) {
      tok.value += advance();
    }

    if( peek() == '.' ) { // float
      tok.value += advance();
      tok.typestr = 'f';
      tok.subtype = TokenSubtype::FLOAT;

      while (m_pos < m_str.length() && std::isdigit(peek())) {
        tok.value += advance();
      }
    } 
  }

  return Token(tok);
}

__attribute__((weak)) Token Lexer::parseName() {
  int start_col = m_column;
  std::string ident_str;
  while (m_pos < m_str.length() && (std::isalnum(peek()) || peek() == '_')) {
    ident_str += advance();
  }

  return Token(TokenType::NAME, ident_str, m_line, start_col); // 식별자 토큰
}

Token Lexer::parseOperator() {
  int start_col = m_column;
  char ch = peek(); 
  char nh = npeek();

  if ( (ch == '+' || ch == '-') && ( std::isalnum(nh) || nh == '_') ) {
    m_column++;
    m_pos++;
    return Token(TokenType::OPERATOR, std::string(1,ch), m_line, start_col, std::string(1,ch), TokenSubtype::UNARY_OP);
  }
  std::string str;

  // m_strs + m_pos; 으로부터 연속되는 연산자를 (최대 3개까지) 가져옴 <- __operator_chars
  // check =+, =-, ::<, |+,....
  // opierators_subtype에 있는지 차례로 찾음 (map}

  int len=0;
  TokenSubtype st;
  for( size_t ix=m_pos; (ix < m_str.length()) && strchr( __operator_chars, (int)m_str[ix] ) ; ix++ ) {
    str += m_str[ix];
    len++;
  }

  for( ; (len > 0); len--) {
    auto it = operators_subtype.find(str);
    if (it != operators_subtype.end()) {
      
      st = it->second;
      m_column += len;
      m_pos += len;

      return Token(TokenType::OPERATOR, str, m_line, start_col, str, st);
    } 
    str.pop_back(); // str.resize(str.length()-1)
  }
    // Error not operator
  return Token(TokenType::UNDEF, "", 0,0);
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
  while (m_pos < m_str.length() && peek() != end_char) {
    str_val += advance();
  }
  str_val += advance(); // end_char  
  return Token(TokenType::BLOCK, str_val, m_line, start_col);
}

// python """, '''
Token Lexer::parseString() {
  int start_col = m_column;
  char quote_char = advance();  
  
  std::string str_val;
  char ch;
  while( m_pos < m_str.length() && peek() != quote_char ) {
    ch = advance();
    str_val += ch;
    if (ch == '\\') {
      str_val += advance();  
    }
  }
  advance();
  return Token(TokenType::CONST, str_val, m_line, start_col);
}

////////////////////////////////////////////////////////////////////////////////////
#include <format>
#include <stdio.h> // for static keywords map
Token Lexer::getToken() {
  skipWhitespace(); // 공백 무시
  
  Token tok;
  int start_col = m_column; // 토큰 시작 컬럼 저장
  char c = peek();

//std::cout << std::format("{} : {}\n", c,n);

  if ( (m_pos+1) > m_str.length() ||  c == '\0' ) {
    return Token(TokenType::END_OF_FILE, "", m_line, m_column);
  }
  if ( c == '\\' ) {
    std::string sval;
    sval += c;
    sval += npeek();
    m_column += 2;
    m_pos += 2;
    return Token(TokenType:: SCHAR, sval, m_line, start_col );
  }
  if( strchr(__comment_chars, c) ) {
    if( (find_index( _comment_line_strs, m_str, m_pos) == 0) 
    || (find_index( _comment_block.l, m_str, m_pos ) == 0)
    || (find_index( _comment_block.r, m_str, m_pos ) == 0))

      return parseComment();
  }
  if ( c == '\"' || c == '\'' || c == '`' ) {
    return parseString();
  }

  if (std::isdigit(c)) { // +n, (p * -n)
    return parseNumber();
  } 
  if (std::isalpha(c) || c == '_') {
    return parseName();
  } 
  if ( is_block_char(c) ) {
    //return parseBlock(); 블록문자가 단일문자가 아닌곳에서
    advance();
    return Token( TokenType::BLOCK, std::string(1,c), m_line, start_col );
  }
  if ( is_oper_char(c) ) {
    tok = parseOperator();

    if( tok.type != TokenType::UNDEF ) return Token(tok);
  }
  if ( is_special_char(c) ) {
    // return parseSpecialchar(); #if, @deco,..
    advance();
    return Token( TokenType::SCHAR, std::string(1,c), m_line, start_col);
  }
  
  std::string str_val;
  while( m_pos < m_str.length() ) {
    if ( peek() < ' ' || peek() > '~' ) {
      str_val += advance();
    } else break;
  }

  return Token(TokenType:: UNDEF, str_val, m_line, start_col );

  // 알 수 없는 문자
//  throw std::runtime_error("Unexpected character \'" + std::string(1, c) + "\' at line " + std::to_string(m_line) + ", column " + std::to_string(m_column));
}


std::vector<Token> Lexer::tokenize() {
  Token tok;
#ifdef TEST    
  int cnt=0;
#endif

  do {
    tok = getToken();
#ifdef TEST    
    try {
    std::cout << std::format("{:04} {:04} {:<12}.{:>3} {:15} : {}\n", 
      cnt++,tok.line, tokentype_names.at(tok.type), tok.typestr, 
      tokenSubtype_names.at(tok.subtype), tok.value );
    } catch( const std::exception& e) {
      std::cout << e.what() << std::endl;
      std::cout << static_cast<int>(tok.subtype) << " : " << tok.value << std::endl;
    }
#endif
    m_toks.push_back(tok);
  } while (tok.type != TokenType::END_OF_FILE);
  return m_toks;
}

int Lexer::load_file(const char* fn) {
    std::ifstream file(fn);
    std::stringstream buffer;
    buffer << file.rdbuf();
    m_str = buffer.str();
    
    return (int)m_str.length();
}
 
} // namespace GLexer

#ifdef TEST

int main(int argc, const char*argv[]) {
  MyLang::Lexer lex;
  lex.load_file(argv[1]);

  lex.tokenize();
#ifdef TEST  
  std::cout << "=======================================================" << std::endl;
  std::cout << std::format("file size={}, lines={}, tokens={}\n", lex.length(), lex.line(), lex.tokens());
  std::cout << "=======================================================" << std::endl;
#endif
}
#endif // TEST

