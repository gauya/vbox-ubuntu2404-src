#include <lexer.h>
#include <iostream>  // std::cerr, std::cout 등
#include <fstream>   // std::ifstream
#include <string>    // std::string
#include <sstream>   // std::stringstream
#include <format>
#include <string.h>

namespace GLexer {

// Lexer 클래스 멤버 변수 초기화
const std::unordered_map<std::string, TokenType> Lexer::keywords = {
  {"if", TokenType::KEYWORD},
  {"else", TokenType::KEYWORD},
  {"while", TokenType::KEYWORD},
  {"function", TokenType::KEYWORD},
  {"switch", TokenType::KEYWORD},
  {"case", TokenType::KEYWORD},
  {"break", TokenType::KEYWORD},
  {"for", TokenType::KEYWORD},
  {"default", TokenType::KEYWORD},
  {"auto", TokenType::KEYWORD},

  {"void", TokenType::VARTYPE},
  {"char", TokenType::VARTYPE},
  {"unsigned", TokenType::VARTYPE},
  {"int", TokenType::VARTYPE},
  {"long", TokenType::VARTYPE},
  {"float", TokenType::VARTYPE},
  {"double", TokenType::VARTYPE},
  {"bool", TokenType::VARTYPE},

  {"true", TokenType::VARTYPE},
  {"false", TokenType::VARTYPE},

  {"struct", TokenType::VARTYPE},
  {"class", TokenType::VARTYPE},
  {"enum", TokenType::VARTYPE},
  {"typedef", TokenType::VARTYPE},

  {"const", TokenType::VARTYPE},
  {"static", TokenType::VARTYPE},
  {"namespace", TokenType::VARTYPE},
  {"extern", TokenType::VARTYPE},
  {"inline", TokenType::VARTYPE},

  {"return", TokenType::KEYWORD} // 예시에 return 추가
};

const std::map<TokenType, std::string> Lexer::tokentype_names = {
  { TokenType:: UNDEF, "undef" },
  { TokenType:: KEYWORD,"keyword" },
  { TokenType:: VARTYPE,"vartype" },
  { TokenType:: NAME, "name" },
  { TokenType:: STRING, "string" },
  { TokenType:: ALPA, "alpa" },
  { TokenType:: NUMBER, "number" },
  { TokenType:: OPERATOR,"operator" },
  { TokenType:: SCHAR, "sp_char" },
  { TokenType:: COMMENT, "comment" },
  { TokenType:: SPACE, "space" },
  { TokenType:: BLOCK, "block" },
  { TokenType:: END_OF_FILE, "endoffile" }
};

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

//////////////////////////////////////////////////////////////////////////
#if 0
#include <stdint.h>

#define isodigit(a) ((a) >= '0' && (a) <= '7')
#define isbdigit(a) ((a) == '0' || (a) == '1')

const char __default_white_space[] = " \t\n\r";

const char* whitespace_skip(const char *str) {
  if( !str )
    return (const char*)0;

  while( *str && strchr(__default_white_space,*str) ) str++;

  return str;
}

//  return lines
void Lexer::skipWhitespace() {
  while (m_pos < m_str.length() && std::isspace(peek())) {
    advance();
  }
}

int stoi(const char *s) {
	s = whitespace_skip(s);
	if(!s) return 0;

	int minus = 1;
	uint32_t val = 0;

	if(*s == '-' || *s == '+')  {
		if(*s++ == '-') minus = -1;
	}

	for(;*s >= '0' && *s <= '9'; s++) {
		val = (val * 10) + (*s - '0');
	}
	//if( val & 0x80000000 ) // error

	return (minus ==-1)? -(int)val : (int)val;
}

int64_t stol(const char *s) {
	s = whitespace_skip(s);
	if(!s) return 0;

	int minus = 1;
	uint64_t val = 0;

	if(*s == '-' || *s == '+')  {
		if(*s++ == '-') minus = -1;
	}

	for(;*s >= '0' && *s <= '9'; s++) {
		val = (val * 10) + (*s - '0');
	}

	return (minus == -1)? -(int64_t)val : (uint64_t)val;
}

uint32_t stob(const char *s) {
	s = whitespace_skip(s);
	if(!s) return 0;

	if(*s == '\0') {
		s++;
		if(*s == 'b' || *s == 'B') s++;
	}

	uint32_t val = 0;
	const char *n = s;

	while(isbdigit(*n)) n++;
	n--;

	for(uint32_t u=0; n>=s; n--, u <<= 1) {
		if( *n == '1' ) val += u;
	}

	return val;
}

uint32_t stoo(const char *s) {
	s = whitespace_skip(s);
	if(!s) return 0;

	uint32_t val = 0;
	const char *n = s;

	while(isodigit(*n)) n++;
	n--;

	for(uint32_t u=0; n>=s; n--, u <<= 3) {
		val += (*n-'0') * u;
	}
	return val;
}

uint32_t stox(const char *s) {
	s = whitespace_skip(s);
	if(!s) return 0;

	if(*s == '\0') {
		s++;
		if(*s == 'x' || *s == 'X') s++;
	}

	uint32_t val = 0;
	const char *n = s;

	while(isxdigit(*n)) n++;
	n--;

	for(uint32_t u=0; n>=s; n--, u <<= 4) {
		val += (*n-'0') * u;
	}
	return val;
}

double stof(const char *s) {
	s = whitespace_skip(s);
	if(!s) return 0.;

	int minus = 1;
	double v = 0;

	if(*s == '-' || *s == '+')  {
		if(*s++ == '-') minus = -1;
	}

	for(;*s && (*s >= '0' && *s <= '9'); s++) {
		v = (v * 10) + (*s - '0');
	}

	if(*s++ == '.') {
		int c;
		double d=1.0;
		for(c = 0; *s >= '0' && *s <= '9'; s++,c++) {
			d *= 0.1;
			v += d * (*s - '0');
		}
	}

	return (minus == -1)? -v : v;
}
#endif // 1
////////////////////////////////////////////////////////////////////////////////

// 이 함수를 부르기전에 comment문자임을 확인해야한다.
// //, /*, <%과 같은 두자리 이상의 문자로 된 comment는?
Token Lexer::parseComment() {
  // 한줄 주석 처리
  size_t start_col = m_column;
  size_t start_line = m_line;

  std::string str;
  char c = peek();
  if ((c == '#') || (c == '/' && npeek() == '/')) {
    while (m_pos < m_str.length() && peek() != '\n') {
      str += advance();
    }
    return Token(TokenType::COMMENT, str, m_line, start_col, (c=='#')? std::string(1,c) : "//");
  } 
  // 여러줄 주석
  if( c == '/' && npeek() == '*') {
    str += advance();
    str += advance();

    while (m_pos < m_str.length() && !(peek() == '*' && npeek() == '/')) {
      str += advance();
    }
    if ( (m_pos + 1) < m_str.length() ) {
      str += advance();
      str += advance();
    }

    return Token(TokenType::COMMENT, str, start_line, start_col, "/*");
  }
  throw std::runtime_error("not comment [" +  std::to_string(start_col) + "]");
}

Token Lexer::parseNumber() {
  int start_col = m_column;
  std::string num_str;
  while (m_pos < m_str.length() && std::isdigit(peek())) {
    num_str += advance();
  }
  // 간단한 정수만 처리
  return Token(TokenType::NUMBER, num_str, m_line, start_col, "");
}

Token Lexer::parseName() {
  int start_col = m_column;
  std::string ident_str;
  while (m_pos < m_str.length() && (std::isalnum(peek()) || peek() == '_')) {
    ident_str += advance();
  }

  // 키워드인지 확인
  auto it = keywords.find(ident_str);
//  std::cout << "===========<>" << ident_str << std::endl;
  if (it != keywords.end()) {
    return Token(it->second, ident_str, m_line, start_col,""); // 키워드 토큰
  } else {
    return Token(TokenType::NAME, ident_str, m_line, start_col, ""); // 식별자 토큰
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
  while (m_pos < m_str.length() && peek() != end_char) {
    str_val += advance();
  }
  str_val += advance(); // end_char  
  return Token(TokenType::BLOCK, str_val, m_line, start_col, std::string(1,quote_char));
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
  return Token(TokenType::STRING, str_val, m_line, start_col, {quote_char});
}

const char __block_chars[] = "(){}[]<>"; // /**/, <%%>
bool Lexer::is_block_char( int ch ) {
  return ( strchr( __block_chars, ch ))? true : false;
}

const char __oper_chars[] = "+-*/%!~|&^=<>?:";
bool Lexer::is_oper_char( int ch ) {
  return ( strchr( __oper_chars, ch ))? true : false;
}
const char __special_chars[] = ",.:;@$`";
bool Lexer::is_special_char( int ch ) {
  return ( strchr( __special_chars, ch ))? true : false;
}

const char *__operation_keys[] = {
  "++","--",
  "<=",">=","==","!=",
  "||","&&",
  "<<=",">>=",
  "<<",">>",
  "::",
  "+","-","*","/","%","?",
  "=","+=","-=","*=","/=","%=","^=","|=","&=","^=",
  "|","&","^",
  NULL
};

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

int find_operation_key_at_pos(const std::string& str, size_t idx) {
    // 1. 시작 위치 (idx) 유효성 검사
    if (idx >= str.length()) {
        return -1; // 시작 위치가 문자열 범위를 벗어나면 바로 실패
    }

    // 2. __operation_keys 배열 순회
    for (int i = 0; __operation_keys[i] != NULL; ++i) {
        const char* op_key = __operation_keys[i];
        size_t op_len = strlen(op_key); // 연산자 문자열의 길이

        // 3. str의 남은 길이가 현재 연산자 문자열보다 짧으면 비교 불가능
        if ((str.length() - idx) < op_len) {
            continue; // 다음 연산자 키로 넘어감
        }

        // 4. std::string::compare를 사용하여 부분 문자열 비교
        //    str.compare(pos, len, c_str) : str의 pos부터 len 길이만큼의 부분 문자열과 c_str 비교
        //    같으면 0을 반환합니다.
        if (str.compare(idx, op_len, op_key) == 0) {
            return i; // 일치하는 연산자 키를 찾았으므로 해당 인덱스 반환
        }
    }

    return -1; // 모든 연산자 키를 검사했지만 일치하는 것을 찾지 못함
}

#include <stdio.h> // for static keywords map
Token Lexer::getToken() {
  skipWhitespace(); // 공백 무시
  
  int start_col = m_column; // 토큰 시작 컬럼 저장
  char c = peek();

  if ( (m_pos+1) > m_str.length() ||  c == '\0' ) {
    return Token(TokenType::END_OF_FILE, "", m_line, m_column,"");
  }
  if ( c == '\\' ) {
    std::string sval;
    sval += c;
    sval += npeek();
    m_column += 2;
    m_pos += 2;
    return Token(TokenType:: SCHAR, sval, m_line, start_col, sval );
  }
  if( (c == '#') 
    || ( c == '/' && npeek() == '/') 
    || ( c == '/' && npeek() == '*')) { 
    
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
    return Token( TokenType::BLOCK, std::string(1,c), m_line, start_col, {c} );
  }
  if ( is_oper_char(c) ) {
    int ok = find_operation_key_at_pos(m_str, m_pos);
    if ( ok == -1 ) {
      // error
      advance();
      return Token( TokenType::OPERATOR, std::string(1,c), m_line, start_col,{c});
    }
    size_t len = strlen( __operation_keys[ok] );
    size_t pp = m_pos;
    m_pos += len; // advance();
    m_column += len;
    return Token( TokenType::OPERATOR, m_str.substr(pp, len), m_line, start_col,{c});
  }
  if ( is_special_char(c) ) {
    advance();
    return Token( TokenType::SCHAR, std::string(1,c), m_line, start_col,{c});
  }
  
  std::string str_val;
  while( m_pos < m_str.length() ) {
    if ( peek() < ' ' || peek() > '~' ) {
      str_val += advance();
    } else break;
  }

  return Token(TokenType:: UNDEF, str_val, m_line, start_col, "" );

  // 알 수 없는 문자
//  throw std::runtime_error("Unexpected character \'" + std::string(1, c) + "\' at line " + std::to_string(m_line) + ", column " + std::to_string(m_column));
}


std::vector<Token> Lexer::tokenize() {
  int cnt=0;
  Token tok;
  do {
    tok = getToken();
    std::cout << std::format("{:04} {:04} {:<12}.{:3} : {}\n", 
      cnt++,tok.m_line, tokentype_names.at(tok.m_type), tok.m_subtype, tok.m_value );
    m_toks.push_back(tok);
  } while (tok.m_type != TokenType::END_OF_FILE);
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

