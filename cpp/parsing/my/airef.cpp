R"(  
  // 키워드인지 확인
  auto it = keywords.find(ident_str);
//  std::cout << "===========<>" << ident_str << std::endl;
  if (it != keywords.end()) {
    return Token(it->second, ident_str, m_line, start_col,""); // 키워드 토큰
  } else {
    return Token(TokenType::NAME, ident_str, m_line, start_col, ""); // 식별자 토큰
  }
  )";
}
*/

/*
const std::map<std::string, TokenSubtype, std::greater<>> operators_subtype = {
    {"<<=", TokenSubtype::LEFT_SHIFT_ASSIGN},
    {">>=", TokenSubtype::RIGHT_SHIFT_ASSIGN},
    {"+=", TokenSubtype::ADD_ASSIGN},
    // ... 다른 연산자들 ...
    {"<", TokenSubtype::LESS}
};
*/

/*
// chatgpt
Token Lexer::parse_operator() {
    const size_t max_check_len = 3; // 예: 최대 연산자 길이
    size_t maxlen = 0;
    TokenSubtype matched_subtype;
    std::string matched_op;

    // 최대 길이까지 lookahead
    for (size_t len = 1; len <= max_check_len && pos + len <= source.size(); ++len) {
        std::string op = source.substr(pos, len);
        auto it = operators_subtype.find(op);
        if (it != operators_subtype.end()) {
            maxlen = len;
            matched_op = op;
            matched_subtype = it->second;
        }
    }

    // 매치된 연산자가 없다면 실패
    if (maxlen == 0) {
        return make_error_token("Unknown operator");
    }

    // 문맥 검사 예시: '<-3' 과 같은 경우 '<'만 연산자여야 함
    if (matched_op == "<-" && pos + 2 < source.size()) {
        char next_char = source[pos + 2];
        if (isdigit(next_char)) {
            // '-'는 음수의 일부로 보고 '<'만 처리
            matched_op = "<";
            matched_subtype = operators_subtype["<"];
            maxlen = 1;
        }
    }

    Token token;
    token.type = TokenType::Operator;
    token.subtype = matched_subtype;
    token.lexeme = matched_op;
    token.pos = pos;

    pos += maxlen;
    return token;
}

// perplexity
Token parse_operator(const std::string& source, size_t& pos) {
    size_t start = pos;
    size_t max_len = std::min(source.size() - pos, max_operator_length);

    // 최대 길이부터 1까지 탐색
    for (size_t len = max_len; len >= 1; --len) {
        std::string candidate = source.substr(pos, len);
        auto it = operators_subtype.find(candidate);

        if (it != operators_subtype.end()) {
            pos += len;  // 연산자 길이만큼 위치 이동
            return Token(TokenType::OPERATOR, candidate, ..., it->second);
        }
    }

    // 단일 문자 연산자도 없으면 오류
    throw std::runtime_error("Unknown operator");
}

// deepseek 
//const std::regex operator_regex(
//    R"((\+\+|--|==|!=|<=|>=|&&|\|\||<<|>>|[-+*/%&|^<>!=~?:]))"
//);

// const std::map<std::string, TokenSubtype> Lexer::opierators_subtype

