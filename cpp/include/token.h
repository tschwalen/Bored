#pragma once
#include <string>


enum class TokenType {
    keyword, 
    identifier, 
    symbol, 
    bool_literal,
    int_literal,
    real_literal,
    string_literal,
    eof
};



struct Token {
    std::string sval;
    TokenType type;
};

const Token EOF_TOKEN {"", TokenType::eof};

std::string tokenTypeString(TokenType tt);