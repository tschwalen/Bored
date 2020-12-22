import sys

import lexer
import parse
from ast_eval import eval_ast


if __name__ == "__main__":
    opts = [opt for opt in sys.argv[1:] if opt.startswith("-")]
    args = [arg for arg in sys.argv[1:] if not arg.startswith("-")]

    path = args[0]

    tokens = lexer.lex_file(path)
    ast = parse.parse_tokens(tokens, True)
    eval_ast(ast)
