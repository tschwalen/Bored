#include "parser.h"
#include "token.h"
#include "ast.h"

#include <string>
#include <iostream>
#include <memory> 

using std::vector;
using std::string;
using std::shared_ptr;


/*
*   ParseState Methods
*/

Token ParseState::currentToken () {
    if (index >= tokens.size())
        return EOF_TOKEN;
    return tokens[index];
}

Token ParseState::peekToken (int n) {
    int i { n + 1 };
    if ( i >= tokens.size())
        return EOF_TOKEN;
    return tokens[i];
}

Token ParseState::advance() {
    Token ct = currentToken();
    ++index;
    return ct;
}

Token ParseState::matchKeyword(string kwrd) {
    Token ct = currentToken();
    std::string tv = ct.sval;
    TokenType   tt = ct.type;
    
    if ( tt == TokenType::keyword && kwrd == tv )
        return advance();

    std::cout << "Expected keyword " << kwrd <<  " , encountered " 
        << tokenTypeString(tt) << " with value " << tv << std::endl;
    parsingError();
    return EOF_TOKEN;
}

Token ParseState::matchTokenType(TokenType ttype) {
    Token ct = currentToken();

    if ( ct.type == ttype )
        return advance();
    
    std::cout << "Expected token of type " << tokenTypeString(ttype) <<  " , encountered " 
        << tokenTypeString(ct.type) << " with value " << ct.sval << std::endl;
    parsingError();
    return EOF_TOKEN;
}

Token ParseState::matchSymbol(string smbl) {
    Token ct = currentToken();

    if ( ct.type == TokenType::symbol && ct.sval == smbl ) 
        return advance();
    
    std::cout << "Expected symbol " << smbl <<  " , encountered " 
        << tokenTypeString(ct.type) << " with value " << ct.sval << std::endl;
    parsingError();
    return EOF_TOKEN;
}

Token ParseState::matchLiteral() {
    Token ct = currentToken();
    TokenType tt = ct.type;
    if ( tt == TokenType::bool_literal || tt == TokenType::int_literal || tt == TokenType::real_literal || tt == TokenType::string_literal )
        return advance();

    std::cout << "Expected literal value, encountered "  << tokenTypeString(ct.type) 
        << " with value " << ct.sval << std::endl;
    parsingError();
    return EOF_TOKEN;
}

void ParseState::parsingError() {
    std::cout << "Parsing error encountered, terminating." << std::endl;
    exit(-1);
}


/*
*  Recursive-descent parsing methods
*/

shared_ptr<BaseNode> parse_program(ParseState &parse_state) {
    auto ast_root = std::make_shared<Program>();

    Token ct = parse_state.currentToken(); 
    while (ct.type != TokenType::eof) {
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

shared_ptr<BaseNode> parse_function_declare(ParseState &parse_state) {
    parse_state.matchKeyword( string{"function"} );
    Token identifier_token = parse_state.matchTokenType( TokenType::identifier );
    parse_state.matchSymbol( string{"("} );

    vector<string> arg_names;
    if ( parse_state.currentToken().sval != ")" ) {
        do {
            Token arg = parse_state.matchTokenType( TokenType::identifier );
            arg_names.push_back(arg.sval);
        } 
        while ( 
            // use short-circuiting here to advance the parser state only if the comma is encountered
            ( parse_state.currentToken().sval == "," ) && 
            ( parse_state.advance().type != TokenType::eof ) 
        );
    }

    parse_state.matchSymbol( string{")"} );
    auto body = parse_block(parse_state);
    return std::make_shared<FunctionDeclare>(identifier_token.sval, arg_names, body);
}


shared_ptr<BaseNode> parse_block(ParseState &parse_state) {
    parse_state.matchSymbol( string{"{"} );
    vector<shared_ptr<BaseNode>> stmts { parse_statement(parse_state) };

    while ( parse_state.currentToken().sval != "}" )
        stmts.push_back( parse_statement(parse_state) );

    parse_state.matchSymbol( string{"}"} );
    return std::make_shared<Block>(stmts);
}

shared_ptr<BaseNode> parse_statement(ParseState &parse_state) {
    auto current_token = parse_state.currentToken();

    if ( current_token.type == TokenType::identifier || current_token.sval == "$" ) {
        auto lvalue = parse_primary(parse_state);

        if ( lvalue->type() == NodeType::FunctionCall ) {
            parse_state.matchSymbol(string{";"});
            return lvalue;
        }
        return parse_assignment(parse_state, lvalue);
    }

    shared_ptr<BaseNode> statement;
    if ( current_token.sval == "var" ) { 
        statement =  parse_declare(parse_state);
    }
    else if ( current_token.sval == "if" ) {
        statement = parse_if(parse_state);
    }          
    else if ( current_token.sval == "while" ) {
        statement =  parse_while(parse_state);
    }           
    else if ( current_token.sval == "return" ) {
        parse_state.matchKeyword( string{"return"} );
        auto expr = parse_expr(parse_state);
        parse_state.matchSymbol( string{";"});
        statement = std::make_shared<Return>(expr);
    } 
    else {
        std::cout << "Invalid start of statement, encountered " << current_token.sval << std::endl;
        parse_state.parsingError();
    }

    return statement;
}

shared_ptr<BaseNode> parse_if(ParseState &parse_state){
    parse_state.matchKeyword( string{"if"} );
    auto condition = parse_expr(parse_state);
    parse_state.matchKeyword("then");
    auto then_body = parse_block(parse_state);

    if( parse_state.currentToken().sval == "else"  ) {
        parse_state.matchKeyword("else");
        auto else_body = parse_block(parse_state);
        return std::make_shared<IfElse>(condition, then_body, else_body);
    }
    
    return std::make_shared<IfThen>(condition, then_body);
}

shared_ptr<BaseNode> parse_while(ParseState &parse_state) {
    parse_state.matchKeyword( string{"while"} );
    auto condition = parse_expr(parse_state);
    parse_state.matchKeyword( string{"do"} );
    auto body = parse_block(parse_state);
    return std::make_shared<While>(condition, body);
}

shared_ptr<BaseNode> parse_assignment(ParseState &parse_state, shared_ptr<BaseNode> lvalue) {

    if (lvalue->type() != NodeType::VariableLookup && lvalue->type() != NodeType::Access) {
        std::cout << "Invalid L value:  " << lvalue->value() << std::endl;
        parse_state.parsingError();
    }

    auto token_value = parse_state.currentToken().sval;
    if ( !(token_value == "="  || token_value == "+=" || token_value == "-=" || 
        token_value == "/=" || token_value == "*=" || token_value == "%=" ) ) 
    {
        std::cout << "Expected assignment operator, encountered  " << token_value << std::endl;
        parse_state.parsingError();
    }
    
    parse_state.matchSymbol(token_value);
    auto expr = parse_expr(parse_state);
    parse_state.matchSymbol(";");
    return std::make_shared<AssignOp>(lvalue, token_value, expr);
}

shared_ptr<BaseNode> parse_declare(ParseState &parse_state) {

    parse_state.matchKeyword("var");
    auto id = parse_state.matchTokenType(TokenType::identifier);
    parse_state.matchSymbol("=");
    auto expr = parse_expr(parse_state);
    parse_state.matchSymbol(";");
    return std::make_shared<Declare>(id.sval, expr);
}

shared_ptr<BaseNode> parse_expr(ParseState &parse_state, int rbp) {
    auto left_expr = parse_primary(parse_state);

    // use Pratt parsing for operator precedence in expressions
    while ( binding_power( parse_state.currentToken() ) > rbp ) {
        auto op = parse_state.currentToken();
        parse_state.advance();
        // last arg, right_expr, stores the result of the recursive call here
        left_expr = std::make_shared<BinaryOp>(op.sval, left_expr, parse_expr(parse_state, binding_power(op)));
    }
    return left_expr;
}

shared_ptr<BaseNode> parse_unary(ParseState &parse_state) {
    auto token_value = parse_state.currentToken().sval;
    parse_state.matchSymbol(token_value);
    auto expr_node = parse_primary(parse_state);
    return std::make_shared<UnaryOp>(token_value, expr_node);
}

shared_ptr<BaseNode> parse_primary(ParseState &parse_state) {
    auto current_token = parse_state.currentToken();

    // vector literals
    if ( current_token.sval == "[" || current_token.sval == "<[" ) {
        // parsing both heterogeneous and homogenous vectors the same way, since type enforcement won't happen until later
        auto token_value =  current_token.sval;
        parse_state.matchSymbol(token_value);

        // vector_type unnecessary right now, so commented out for now
        //string vector_type;
        string closing;
        if ( token_value == "[" ) {
            //vector_type = "Heterogeneous";
            closing = "]";
        }
        else {
            //vector_type = "Heterogeneous";
            closing = "]>";   
        }
        auto vector_contents = parse_expr_list(parse_state);
        parse_state.matchSymbol(closing);
        return std::make_shared<VectorLiteral>(vector_contents);
    }

    // unary ops
    if ( current_token.sval == "!" || current_token.sval == "-" ) {
        return parse_unary(parse_state);
    }

    shared_ptr<BaseNode> primary_expr;

    // parenthesized expression
    if ( current_token.sval == "(" ) {
        parse_state.matchSymbol("(");
        auto parenthesized_expr = parse_expr(parse_state);
        parse_state.matchSymbol(")");
        primary_expr = parenthesized_expr;
    }
    // identifier
    else if (current_token.type == TokenType::identifier) {
        auto id = parse_state.matchTokenType(TokenType::identifier).sval;
        primary_expr = std::make_shared<VariableLookup>(id, false);
    }
    // sigiled identifier (global lookup)
    else if ( current_token.sval == "$" ) {
        parse_state.matchSymbol("$");
        auto id = parse_state.matchTokenType(TokenType::identifier).sval;
        primary_expr = std::make_shared<VariableLookup>(id, true);
    }

    if ( primary_expr != nullptr ) {
        // both an identifier or a parenthesized expression could be followed by access brackets or fn call
        auto current_token = parse_state.currentToken();
        while ( true ) {
            if ( current_token.sval == "(" ) {
                auto fn_call_args = parse_function_call(parse_state);
                primary_expr =  std::make_shared<FunctionCall>(primary_expr, fn_call_args);
            }   
            else if ( current_token.sval == "[" ) {
                parse_state.matchSymbol("[");
                auto index_expr = parse_expr(parse_state);
                parse_state.matchSymbol("]");
                primary_expr = std::make_shared<Access>(primary_expr, index_expr);
            }
            else {
                return primary_expr;
            }
            current_token = parse_state.currentToken();
        }
    }

    // if all else fails, we assume it's a literal
    return parse_literal(parse_state);
}

vector<shared_ptr<BaseNode>> parse_function_call(ParseState &parse_state) {
    parse_state.matchSymbol("(");

    vector<shared_ptr<BaseNode>> expr_args;
    if ( parse_state.currentToken().sval != ")" ) {
        expr_args = parse_expr_list(parse_state);
    }
    parse_state.matchSymbol(")");
    return expr_args;
}

vector<shared_ptr<BaseNode>> parse_expr_list(ParseState &parse_state) {
    vector<shared_ptr<BaseNode>> expr_list;

    do {
        auto expr = parse_expr(parse_state);
        expr_list.push_back(expr);
    } while (parse_state.currentToken().sval == ",");

    return expr_list;
}


shared_ptr<BaseNode> parse_literal(ParseState &parse_state) {
    Token literal_token = parse_state.matchLiteral();
    shared_ptr<BaseNode> result;
    switch(literal_token.type) {
        case TokenType::bool_literal:
            {
                // might be overkill, but don't want to fail silently
                bool bool_value = false;
                if ( literal_token.sval == "true") {
                    bool_value = true;
                }
                else if (literal_token.sval == "false") {
                    bool_value = false;
                }
                else {
                    std::cout << "Invalid contents of a boolean literal token : " << literal_token.sval << std::endl;
                    parse_state.parsingError();
                }

                result = std::make_shared<BoolLiteral>(bool_value);
                break;
            }
        case TokenType::int_literal:
            {
                int int_value = std::stoi(literal_token.sval);
                result = std::make_shared<IntLiteral>(int_value);
                break;
            }
        case TokenType::real_literal:
            {
                double double_value = std::stod(literal_token.sval);
                result = std::make_shared<RealLiteral>(double_value);
                break;
            }
        case TokenType::string_literal:
            {
                result = std::make_shared<StringLiteral>(literal_token.sval);
                break;
            }
        default:
            {
                std::cout << "Invalid token type for literal: " << tokenTypeString(literal_token.type) << std::endl;
                parse_state.parsingError();
            }  
    }
    return result;
}


// entry-point for parsing
shared_ptr<BaseNode> parse_tokens(vector<Token> tokens, bool printout) {
    ParseState parse_state { tokens };
    shared_ptr<BaseNode> ast = parse_program(parse_state);
    
    if (printout)
        pretty_print_ast(ast);

    return ast;
}


// operator-precedence lookup 
int binding_power(Token tok) {
    auto tv = tok.sval;
    if ( tv == "|" || tv == "&" ) 
        return 1;
    if ( tv == "==" || tv ==  "!=" || tv ==  "<=" || tv ==  ">=" || tv ==  "<" || tv ==  ">" ) 
        return 3;
    if ( tv == "+" || tv == "-" ) 
        return 4;
    if ( tv == "*" || tv == "/" || tv == "%" ) 
        return 5;
    return -1;
}

// adapted from https://vallentin.dev/2016/11/29/pretty-print-tree
void pretty_print_ast(shared_ptr<BaseNode> node, string _prefix, bool _last) {
    std::cout << _prefix << ( _last ? "`- " : "|- ") << node->value() << std::endl;
    _prefix = _prefix + ( _last ? "   " : "|  " );
    vector<shared_ptr<BaseNode>> children = node->children();
    int child_count = children.size();
    int i = 0;
    for (auto ch : children) {
        _last = ( i == (child_count - 1));
        pretty_print_ast(ch, _prefix, _last);
        ++i;
    }
}

