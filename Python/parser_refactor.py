import sys
from AST import *


class ParseState:
    def __init__(self, tokens, index=0, error_fn=None):
        self.tokens = tokens
        self.index = index
        self.error_fn = error_fn

    def currentToken(self):
        if self.index >= len(self.tokens):
            return ("", "EOF")
        return self.tokens[self.index]

    def peekToken(self, n=1):
        i = self.index + n
        if i >= len(self.tokens):
            return ("", "EOF")
        return self.tokens[i]

    def advance(self):
        ct = self.currentToken()
        self.index += 1
        return ct

    def matchKeyword(self, kwrd):
        ct = self.currentToken()
        tv, tt = ct
        if ct == (kwrd, "keyword"):
            return self.advance()
        else:
            self.parse_error("Expected keyword \'{}\', encountered \'{}\' with value \'{}\'.".format(kwrd, tt, tv))

    def matchTokenType(self, ttype):
        ct = self.currentToken()
        tv, tt = ct
        if tt == ttype:
            return self.advance()
        else:
            self.parse_error("Expected token of type \'{}\', encountered \'{}\' with value \'{}\'.".format(ttype, tt, tv))

    def matchSymbol(self, smbl):
        ct = self.currentToken()
        tv, tt = ct
        if ct == (smbl, "symbol"):
            return self.advance()
        else:
            self.parse_error("Expected symbol \'{}\', encountered \'{}\' with value \'{}\'.".format(smbl, tt, tv))

    def matchLiteral(self):
        ct = self.currentToken()
        tv, tt = ct
        if tt.endswith("literal"):
            return self.advance()
        else:
            self.parse_error("Expected literal value, encountered \'{}\' with value \'{}\'.".format( tt, tv))

    def parse_error(self, msg, *args, **kwargs):
        if self.error_fn != None:
            self.error_fn(msg, args, kwargs)
        else:
            print("Parse error: {}".format(msg))
            raise SystemExit



# may seem pointless, but will make the code more readible. Used sparingly
def tokenValue(t):
    return t[0]
def tokenType(t):
    return t[1]


def parse_program(parse_state):
    ast_root = Program()

    tv, tt = parse_state.currentToken()
    while tt != "EOF":
        if tv == "var":
            ast_node = parse_declare(parse_state)
            ast_root.add_top_level_stmt( ast_node )
        elif tv == "function":
            ast_node = parse_function_declare(parse_state)
            ast_root.add_top_level_stmt( ast_node )
        else:
            parse_state.parse_error("Encountered unexpected token \'{}\' while parsing top-level statement".format(tv))


        # might be unneeded
        tv, tt = parse_state.currentToken()

    return ast_root


def parse_function_declare(parse_state):
    parse_state.matchKeyword("function")

    fn_name, _ = parse_state.matchTokenType("identifier")

    parse_state.matchSymbol("(")

    if tokenValue( parse_state.currentToken() ) == ")":
        arg_names = []
    else:
        arg_names = parse_arg_list(parse_state)
    
    parse_state.matchSymbol(")")

    body = parse_block(parse_state)

    return FunctionDeclare(identifier=fn_name, args=arg_names, body=body)

def parse_arg_list(parse_state):
    arg, _ = parse_state.matchTokenType("identifier")

    if tokenValue( parse_state.currentToken() ) == ",":
        parse_state.matchSymbol(",")
        return [arg] + parse_arg_list(parse_state)

    return [arg]

def parse_block(parse_state):
    # one thing we could do here is accept single statements followed by a semicolon, without requiring braces around it. 
    # this would be a simple matter of checking if the next token was a brace symbol. 
    # I'm undecided on this so far because I'm not sure if I'll like how it looks, and because I'm not sure if I want 
    # to allow single statements in every case where I have a <block> in the grammar. for example, I think it would look
    # strange for function declarations not to include braces.

    parse_state.matchSymbol("{")
    stmts = [ parse_statement(parse_state) ]

    # yes I'm doing it iteratively. Functions tend to have no more than 6 or 7 arguments whereas a block can have dozens of statements
    while tokenValue( parse_state.currentToken() ) != "}":
        stmts.append( parse_statement(parse_state) )

    parse_state.matchSymbol("}")
    return Block(stmts=stmts)

def parse_statement(parse_state):
    ct = parse_state.currentToken()

    if tokenType(ct) == "identifier":
        nt = parse_state.peekToken()
        tv = tokenValue(nt)
        if tv == "=":
            stmt = parse_assign(parse_state)
            return stmt

        if tv == "(":
            stmt = parse_function_call(parse_state)
            parse_state.matchSymbol(";")
            return stmt

        return parse_modifier(parse_state)

    tv = tokenValue(ct)
    
    if tv == "var":
        return parse_declare(parse_state)

    if tv == "return":
        parse_state.matchKeyword("return")
        expr = parse_expr(parse_state)
        parse_state.matchSymbol(";")
        return Return(expr_node=expr)

    if tv == "if":
        return parse_if(parse_state)

    if tv == "while":
        return parse_while(parse_state)

    parse_error("Invalid start of statement, encountered {}".format(tv))

def parse_if(parse_state):

    parse_state.matchKeyword("if")
    condition = parse_expr(parse_state)

    parse_state.matchKeyword("then")
    then_body = parse_block(parse_state)

    if tokenValue(parse_state.currentToken()) == "else":
        parse_state.matchKeyword("else")
        else_body = parse_block(parse_state)
        return IfElse(condition=condition, then_body=then_body, else_body=else_body)

    return IfThen(condition=condition, body=then_body)

def parse_while(parse_state):
    parse_state.matchKeyword("while")
    condition = parse_expr(parse_state)

    parse_state.matchKeyword("do")
    body = parse_block(parse_state)

    return While(condition=condition, body=body)

def parse_assign(parse_state):
    identifier = tokenValue( parse_state.matchTokenType("identifier") )
    parse_state.matchSymbol("=")
    expression = parse_expr(parse_state)

    parse_state.matchSymbol(";")

    return Assign(identifier=identifier, expr_node=expression)


def parse_modifier(parse_state):
    identifier = tokenValue( parse_state.matchTokenType("identifier") )
    tv = tokenValue( parse_state.currentToken() )
    if tv in ["+=", "-=", "/=", "*=", "%="]:
            parse_state.matchSymbol(tv)
            expr = parse_expr(parse_state)
            parse_state.matchSymbol(";")
            return AssignOp(identifier=identifier, op=tv, expr_node=expr)
    
    parse_error("Expected assignment, modifier, or function to follow identifier. Encountered {} {}".format(identifier, tv))

def parse_declare(parse_state):
    parse_state.matchKeyword("var")

    identifier = tokenValue( parse_state.matchTokenType("identifier") )

    parse_state.matchSymbol("=")

    expression = parse_expr(parse_state)

    parse_state.matchSymbol(";")

    return Declare(identifier=identifier, expr_node=expression)

def parse_expr(parse_state):
    left_expr = parse_and_expr(parse_state)

    if tokenValue(parse_state.currentToken()) == "|":
        parse_state.matchSymbol("|")
        right_expr = parse_expr(parse_state)
        return BinaryOp(op="|", left_expr=left_expr, right_expr=right_expr)
    
    return left_expr

def parse_and_expr(parse_state):
    left_expr = parse_comparison(parse_state)

    if tokenValue(parse_state.currentToken()) == "&":
        parse_state.matchSymbol("&")
        right_expr = parse_and_expr(parse_state)
        return BinaryOp(op="&", left_expr=left_expr, right_expr=right_expr)

    return left_expr

def parse_comparison(parse_state):
    left_expr = parse_term(parse_state)
    
    tv = tokenValue(parse_state.currentToken())
    if tv in ["==", "!=", "==", "<=", ">=", "<", ">"]:
        parse_state.matchSymbol(tv)
        right_expr = parse_comparison(parse_state)
        return BinaryOp(op=tv, left_expr=left_expr, right_expr=right_expr)
    else:
        return left_expr

def parse_term(parse_state):
    left_expr = parse_factor(parse_state)

    tv = tokenValue(parse_state.currentToken())
    if tv in ["+", "-"]:
        parse_state.matchSymbol(tv)
        right_expr = parse_term(parse_state)
        return BinaryOp(op=tv, left_expr=left_expr, right_expr=right_expr)
    else:
        return left_expr

def parse_factor(parse_state):
    left_expr = parse_unary(parse_state)

    tv = tokenValue(parse_state.currentToken())
    if tv in ["*", "/", "%"]:
        parse_state.matchSymbol(tv)
        right_expr = parse_factor(parse_state)
        return BinaryOp(op=tv, left_expr=left_expr, right_expr=right_expr)
    else:
        return left_expr

def parse_unary(parse_state):
    tv = tokenValue(parse_state.currentToken())
    if tv in ["+", "-"]:
        parse_state.matchSymbol(tv)
        expr_node = parse_primary(parse_state)
        return UnaryOp(op=tv, expr_node=expr_node)
    return parse_primary(parse_state)

def parse_primary(parse_state):
    ct = parse_state.currentToken()

    if tokenValue(ct) == "(":
        parse_state.matchSymbol("(")
        parenthesized_expr = parse_expr(parse_state)
        parse_state.matchSymbol(")")
        return parenthesized_expr

    if tokenType(ct) == "identifier":
        nt = parse_state.peekToken()
        if tokenValue(nt) == "(":
            return parse_function_call(parse_state)
        identifier = tokenValue( parse_state.matchTokenType("identifier") )
        return VariableLookup(identifier=identifier)

    value, literal_type = parse_state.matchLiteral()
    return Literal(literal_type=literal_type, value=value)


def parse_function_call(parse_state):
    fn_name = tokenValue( parse_state.matchTokenType("identifier") )
    parse_state.matchSymbol("(")

    if tokenValue( parse_state.currentToken() ) == ")":
        expr_args = []
    else:
        expr_args = parse_expr_list(parse_state)

    parse_state.matchSymbol(")")
    return FunctionCall(identifier=fn_name, expr_args=expr_args)

def parse_expr_list(parse_state):
    expr = parse_expr(parse_state)

    # of course this could be done iteratively, but if I'm gonna write a recusive descent parser I may as well commit to it
    if tokenValue( parse_state.currentToken() ) == ",":
        parse_state.matchSymbol(",")
        return [expr] + parse_expr_list(parse_state)

    return [expr]
    
### End of parsing code ###

# adapted from https://vallentin.dev/2016/11/29/pretty-print-tree
def pretty_print_ast(node, _prefix="", _last=True):
    print(_prefix, "`- " if _last else "|- ", node.value(), sep="" )
    _prefix += "   " if _last else "|  "
    child_count = len(node.children())
    for i, child in enumerate(node.children()):
        _last = i == (child_count - 1)
        pretty_print_ast(child, _prefix, _last)
    

if __name__ == "__main__":
    
    args = [arg for arg in sys.argv[1:] if not arg.startswith("-")]

    _file = "../tests/tokens.txt"
    if len(args) > 0:
        _file = args[0]


    with open(_file, 'r') as file:
        data = file.read().replace('\n', '')
    
    # wow dangerous
    tokens = eval(data)
    parse_state = ParseState(tokens)
    #print(tokens)
    ast = parse_program(parse_state)

    pretty_print_ast(ast)