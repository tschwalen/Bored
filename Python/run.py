import sys

import lexer
import parse
from ast_eval import eval_ast


if __name__ == "__main__":
    opts = [opt for opt in sys.argv[1:] if opt.startswith("-")]
    args = [arg for arg in sys.argv[1:] if not arg.startswith("-")]  

    command = args[0]

    if command == "help":
        print("Structure args in the form of: [ lex | parse | exec | compile | help ] \"path/to/file\"")

    path = args[1]

    tokens = lexer.lex_file(path)
    if command ==  "lex": 
        lexer.tuple_print(tokens)
        sys.exit(0)    

    ast = parse.parse_tokens(tokens, False)
    if command == "parse":
        parse.pretty_print_ast(ast)
        sys.exit(0)   

    if command == "exec":
        eval_ast(ast)
        sys.exit(0)   

    if command == "compile":
        print ("not implemented")

