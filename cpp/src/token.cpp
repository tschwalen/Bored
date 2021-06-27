#include "token.h"

std::string tokenTypeString(TokenType tt) {
    std::string str_repr;
    switch(tt) {
        case keyword:
            str_repr = "keyword";
            break;
        case identifier:
            str_repr = "identifier";
            break;
        case symbol:
            str_repr = "symbol";
            break;
        case bool_literal:
            str_repr = "bool-literal";
            break;
        case int_literal:
            str_repr = "int-literal";
            break;
        case real_literal:
            str_repr = "real-literal";
            break;
        case string_literal:
            str_repr = "string-literal";
            break;
    }
    return str_repr;
}