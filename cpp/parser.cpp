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
    else {
        std::cout << "Expected keyword " << kwrd <<  " , encountered " 
            << tokenTypeString(tt) << " with value " << tv << std::endl;
        parsingError();
        return Token { std::string(""), eof };
    }

}

void ParseState::parsingError() {
    std::cout << "Parsing error encountered, terminating." << std::endl;
    exit(-1);
}
