#pragma once
#include "token.h"
#include "ast.h"
#include <vector>
#include <utility>

class ParseState 
{
private:
    std::vector<Token> tokens;
    int index;
public:
    ParseState (std::vector<Token> tokens_, int index_=0)
        : tokens { std::move(tokens_) }, index { index_ } {}

    Token currentToken();
    Token peekToken(int n=1);
    Token advance();
    Token matchKeyword(std::string kwrd);
    Token matchTokenType(TokenType ttype);
    Token matchSymbol(std::string smbl);
    Token matchLiteral();
    void  parsingError();
};

std::shared_ptr<BaseNode> parse_program(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_function_declare(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_block(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_statement(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_if(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_while(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_assignment(ParseState &parse_state, std::shared_ptr<BaseNode> lvalue);
std::shared_ptr<BaseNode> parse_declare(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_expr(ParseState &parse_state, int rbp=0);
std::shared_ptr<BaseNode> parse_unary(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_primary(ParseState &parse_state);
std::vector<std::shared_ptr<BaseNode>> parse_function_call(ParseState &parse_state);
std::vector<std::shared_ptr<BaseNode>> parse_expr_list(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_literal(ParseState &parse_state) 

int  binding_power(Token &tok);
void pretty_print_ast(std::shared_ptr<BaseNode> node, std::string _prefix="", bool _last=true);
std::shared_ptr<BaseNode> parse_tokens(std::vector<Token> tokens, bool printout=false);
