#include "parser.h"
#include "token.h"
#include "ast.h"

#include <string>
#include <iostream>
#include <memory> 

using std::vector;
using std::string;

Token ParseState::currentToken () {
    if (index >= tokens.size())
        return Token { string(""), eof };
    return tokens[index];
}

Token ParseState::peekToken (int n=1) {
    int i { n + 1 };
    if ( i >= tokens.size())
        return Token { string(""), eof };
    return tokens[i];
}

Token ParseState::advance() {
    Token ct = currentToken();
    ++index;
    return ct;
}

Token ParseState::matchKeyword(string &kwrd) {
    Token ct = currentToken();
    std::string tv = ct.sval;
    TokenType   tt = ct.type;
    
    if ( tt == keyword && kwrd == tv )
        return advance();

    std::cout << "Expected keyword " << kwrd <<  " , encountered " 
        << tokenTypeString(tt) << " with value " << tv << std::endl;
    parsingError();
    return Token { string(""), eof };
}

Token ParseState::matchTokenType(TokenType ttype) {
    Token ct = currentToken();

    if ( ct.type == ttype )
        return advance();
    
    std::cout << "Expected token of type " << tokenTypeString(ttype) <<  " , encountered " 
        << tokenTypeString(ct.type) << " with value " << ct.sval << std::endl;
    parsingError();
    return Token { string(""), eof };
}

Token ParseState::matchSymbol(string smbl) {
    Token ct = currentToken();

    if ( ct.type == symbol && ct.sval == smbl ) 
        return advance();
    
    std::cout << "Expected symbol " << smbl <<  " , encountered " 
        << tokenTypeString(ct.type) << " with value " << ct.sval << std::endl;
    parsingError();
    return Token { string(""), eof };
}

Token ParseState::matchLiteral() {
    Token ct = currentToken();
    TokenType tt = ct.type;
    if ( tt == bool_literal || tt == int_literal || tt == real_literal || tt == string_literal )
        return advance();

    std::cout << "Expected literal value, encountered "  << tokenTypeString(ct.type) 
        << " with value " << ct.sval << std::endl;
    parsingError();
    return Token { string(""), eof };
}

void ParseState::parsingError() {
    std::cout << "Parsing error encountered, terminating." << std::endl;
    exit(-1);
}


std::shared_ptr<BaseNode> parse_program(ParseState &parse_state) {
    auto ast_root = std::make_shared<Program>();

    Token ct = parse_state.currentToken(); 
    while (ct.type != eof) {
        if ( ct.sval == "var" ) {
            auto ast_node = parse_declare(parse_state);
            ast_root->add_top_level_stmt(ast_node);
        }
        else if (ct.sval == "function") {
            auto ast_node = parse_function_declare(parse_state);
            ast_root->add_top_level_stmt(ast_node);
        }
        else {
            std::cout << "Encountered unexpected token " << ct.sval 
                << " while parsing top-level statement." << std::endl;
            parse_state.parsingError();
        }
        ct = parse_state.currentToken();
    }
    return ast_root;
}

std::shared_ptr<BaseNode> parse_function_declare(ParseState &parse_state) {
    parse_state.matchKeyword( string{"function"} );
    Token identifier_token = parse_state.matchTokenType( identifier );
    parse_state.matchSymbol( string{"("} );

    vector<string> arg_names;
    if ( parse_state.currentToken().sval != ")" ) {
        do {
            Token arg = parse_state.matchTokenType( identifier );
            arg_names.push_back(arg.sval);
        } 
        while ( 
            // use short-circuiting here to advance the parser state only if the comma is encountered
            ( parse_state.currentToken().sval == "," ) && 
            ( parse_state.advance().type != eof ) 
        );
    }

    parse_state.matchSymbol( string{")"} );
    auto body = parse_block(parse_state);
    return std::make_shared<FunctionDeclare>(identifier_token.sval, arg_names, body);
}


std::shared_ptr<BaseNode> parse_block(ParseState &parse_state) {
    parse_state.matchSymbol( string{"{"} );
    vector<std::shared_ptr<BaseNode>> stmts { parse_statement(parse_state) };

    while ( parse_state.currentToken().sval != "}" )
        stmts.push_back( parse_statement(parse_state) );

    parse_state.matchSymbol( string{"}"} );
    return std::make_shared<Block>(stmts);
}

std::shared_ptr<BaseNode> parse_statement(ParseState &parse_state) {
    auto current_token = parse_state.currentToken();

    if ( current_token.type == identifier || current_token.sval == "$" ) {
        auto lvalue = parse_primary(parse_state);

        if ( lvalue->type() == NodeType::FunctionCall ) {
            parse_state.matchSymbol(string{";"});
            return lvalue;
        }
        return parse_assignment(parse_state, lvalue);
    }

    if ( current_token.sval == "var" ) 
        return parse_declare(parse_state);
    if ( current_token.sval == "if" )       
        return parse_if(parse_state);
    if ( current_token.sval == "while" )    
        return parse_while(parse_state);

    if ( current_token.sval == "return" ) {
        parse_state.matchKeyword( string{"return"} );
        auto expr = parse_expr(parse_state);
        parse_state.matchSymbol( string{";"});
        return std::make_shared<Return>(expr);
    } 

    // if this point was reached, something went wrong
    std::cout << "Invalid start of statement, encountered " << current_token.sval << std::endl;
    parse_state.parsingError();
    // garbage return here to satisfy the compiler
    return std::make_shared<Return>();
}

std::shared_ptr<BaseNode> parse_if(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_while(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_assignment(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_declare(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_expr(ParseState &parse_state, int rbp=0);
std::shared_ptr<BaseNode> parse_unary(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_primary(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_function_call(ParseState &parse_state);
std::shared_ptr<BaseNode> parse_expr_list(ParseState &parse_state);
