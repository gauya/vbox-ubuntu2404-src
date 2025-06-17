#ifndef __PARSE__H
#define __PARSE__H

#include <string>
#include <vector>

#include <lexer>

namespace GParser {

class Parser {
private:
  size_t m_cp;
  std::vector<Token> m_toks;

public:
  explicit Parser()
    : m_cp(0) {}
  explicit Parser( std::vector<Token> toks)
    : m_cp(toks) {}


};

} // GParser

#endif // __PARSE_H_
