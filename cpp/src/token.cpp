#include "token.h"

std::string tokenTypeString(TokenType tt) {
    std::string str_repr;
    switch(tt) {
        case TokenType::keyword:
            str_repr = "keyword";
            break;
        case TokenType::identifier:
            str_repr = "identifier";
            break;
        case TokenType::symbol:
            str_repr = "symbol";
            break;
        case TokenType::bool_literal:
            str_repr = "bool-literal";
            break;
        case TokenType::int_literal:
            str_repr = "int-literal";
            break;
        case TokenType::real_literal:
            str_repr = "real-literal";
            break;
        case TokenType::string_literal:
            str_repr = "string-literal";
            break;
    }
    return str_repr;
}