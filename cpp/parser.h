#pragma once
#include "token.h"
#include "ast.h"
#include <vector>


class ParseState 
{
private:
    std::vector<Token> tokens;
    int index;
public:
    ParseState (std::vector<Token> tokens_, int index_)
        : tokens { tokens_ }, index { index_ } {}

    Token currentToken();
    Token peekToken(int n=1);
    Token advance();
    Token matchKeyword(std::string kwrd);
    Token matchTokenType(TokenType ttype);
    Token matchSymbol(std::string smbl);
    Token matchLiteral();
    void  parsingError();
};

BaseNode parse_program(ParseState parse_state);
BaseNode parse_function_declare(ParseState parse_state);
BaseNode parse_arg_list(ParseState parse_state);
BaseNode parse_block(ParseState parse_state);
BaseNode parse_statement(ParseState parse_state);
BaseNode parse_if(ParseState parse_state);
BaseNode parse_while(ParseState parse_state);
BaseNode parse_assignment(ParseState parse_state);
BaseNode parse_declare(ParseState parse_state);
BaseNode parse_expr(ParseState parse_state, int rbp=0);
BaseNode parse_unary(ParseState parse_state);
BaseNode parse_primary(ParseState parse_state);
BaseNode parse_function_call(ParseState parse_state);
BaseNode parse_expr_list(ParseState parse_state);

void pretty_print_ast(BaseNode node, std::string _prefix="", bool _last=true);
BaseNode parse_tokens(Token tokens, bool printout=false);
