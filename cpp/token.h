#pragma once
#include <string>


enum TokenType {
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

std::string tokenTypeString(TokenType tt);