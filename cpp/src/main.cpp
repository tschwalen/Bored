#include "asteval.h"
#include "ast.h"
#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <string>
#include <iostream>
#include <memory>
#include <fstream>

using std::string;

enum Command { lex, parse, exec, compile };

void do_main(int argc, const char* argv[], Command cmd) {
    if ( !(argc > 2) ) return;

    string source;
    string source_file = argv[2];
    std::ifstream ifs(source_file);
    source.assign( (std::istreambuf_iterator<char>(ifs) ), (std::istreambuf_iterator<char>() ) );

    std::vector<Token> tokens = lex_string(source);

    // if selected command is lex, print the tokens and then exit
    if ( cmd == lex ) {
        tuple_print(tokens);
        return;
    }

    // parse_tokens will print the AST if selected command is parse
    std::shared_ptr<BaseNode> ast = parse_tokens(tokens, cmd == parse);
    if (cmd == parse) return;

    // run the interpreter if exec is selected
    if (cmd == exec) {
        run_ast_interpreter(ast);
        return;
    }

}

/*
*
*  args: [ lex | parse | exec | compile | help ] "path/to/file"
*  (More options to come, but I like this for now) 
*
*/
int main( int argc, const char* argv[] ) {
    if ( argc > 1 ) {
        string primary_cmd = argv[1];

        // should be refactored at some point
        if ( primary_cmd == "lex" ) {
            do_main(argc, argv, lex);
            
        } else
        if ( primary_cmd == "parse" ) {
            do_main(argc, argv, parse);

        } else
        if ( primary_cmd == "exec" ) {
            do_main(argc, argv, exec);
        } else 
        if ( primary_cmd == "compile" ) {
            std::cout << "Not implemented" << std::endl;
            return -1;
        } else {
            std::cout << "Structure args in the form of: [ lex | parse | exec | compile | help ] \"path/to/file\" " << std::endl;
        }
    }
    return 0;
}
