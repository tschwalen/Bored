#include "parser.h"
#include "token.h"

#include <string>
#include <iostream>

Token ParseState::currentToken () {
    if (index >= tokens.size())
        return Token { std::string(""), eof };
    return tokens[index];
}

Token ParseState::peekToken (int n=1) {
    int i { n + 1 };
    if ( i >= tokens.size())
        return Token { std::string(""), eof };
    return tokens[i];
}

Token ParseState::advance() {
    Token ct = currentToken();
    ++index;
    return ct;
}

Token ParseState::matchKeyword(std::string &kwrd) {
    Token ct = currentToken();
    std::string tv = ct.sval;
    TokenType   tt = ct.type;
    
    if ( tt == keyword && kwrd == tv )
        return advance();

    std::cout << "Expected keyword " << kwrd <<  " , encountered " 
        << tokenTypeString(tt) << " with value " << tv << std::endl;
    parsingError();
    return Token { std::string(""), eof };
}

Token ParseState::matchTokenType(TokenType ttype) {
    Token ct = currentToken();

    if ( ct.type == ttype )
        return advance();
    
    std::cout << "Expected token of type " << tokenTypeString(ttype) <<  " , encountered " 
        << tokenTypeString(ct.type) << " with value " << ct.sval << std::endl;
    parsingError();
    return Token { std::string(""), eof };
}

Token ParseState::matchSymbol(std::string smbl) {
    Token ct = currentToken();

    if ( ct.type == symbol && ct.sval == smbl ) 
        return advance();
    
    std::cout << "Expected symbol " << smbl <<  " , encountered " 
        << tokenTypeString(ct.type) << " with value " << ct.sval << std::endl;
    parsingError();
    return Token { std::string(""), eof };
}

Token ParseState::matchLiteral() {
    Token ct = currentToken();
    TokenType tt = ct.type;
    if ( tt == bool_literal || tt == int_literal || tt == real_literal || tt == string_literal )
        return advance();

    std::cout << "Expected literal value, encountered "  << tokenTypeString(ct.type) 
        << " with value " << ct.sval << std::endl;
    parsingError();
    return Token { std::string(""), eof };
}

void ParseState::parsingError() {
    std::cout << "Parsing error encountered, terminating." << std::endl;
    exit(-1);
}
