import sys
from AST import *

class ParseState:
    def __init__(self, tokens, index=0, error_fn=None):
        self.tokens = tokens
        self.index = index
        self.error_fn = error_fn
        #print(" ".join([t[0] for t in tokens]))
        #print(tokens)


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
        #print([t[0] for t in self.tokens[self.index:]])
        ct = self.currentToken()
        #print('advanced past %s' % ct[0])
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
        #print('matchTokenType called with %s' % ttype)
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

precedence = {
    '|' : 1,
    '&' : 1,
    "==" : 3, "!=" : 3, "<=" : 3, ">=" : 3, "<" : 3, ">" : 3,
    "+" : 4, "-" : 4,
    "*" : 5, "/" : 5, "%" : 5
}

# may seem pointless, but will make the code more readible. Used sparingly
def tokenValue(t):
    return t[0]
def tokenType(t):
    return t[1]


def parse_program(parse_state):
    ast_root = Program()

    tv, tt = parse_state.currentToken() # tokenvalue, tokentype
    while tt != "EOF":
        if tv == "var":
            ast_node = parse_declare(parse_state)
            ast_root.add_top_level_stmt( ast_node )
        elif tv == "function":
            ast_node = parse_function_declare(parse_state)
            ast_root.add_top_level_stmt( ast_node )
        else:
            parse_state.parse_error("Encountered unexpected token \'{}\' while parsing top-level statement".format(tv))

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
            stmt = parse_primary(parse_state)
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

    parse_state.parse_error("Invalid start of statement, encountered {}".format(tv))

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
    
    parse_state.parse_error("Expected assignment, modifier, or function to follow identifier. Encountered {} {}".format(identifier, tv))

def parse_declare(parse_state):
    parse_state.matchKeyword("var")
    identifier = tokenValue( parse_state.matchTokenType("identifier") )
    parse_state.matchSymbol("=")
    expression = parse_expr(parse_state)
    parse_state.matchSymbol(";")

    return Declare(identifier=identifier, expr_node=expression)

def binding_power(tok):
    t = tokenValue(tok)
    return precedence.get(t, -1)

def parse_expr(parse_state, rbp = 0):
    left_expr = parse_primary(parse_state)

    # here we go into pratt parsing
    while ( binding_power(parse_state.currentToken()) > rbp):
        op = parse_state.currentToken()
        parse_state.advance()
        left_expr = BinaryOp(op=tokenValue(op), 
                             left_expr=left_expr, 
                             right_expr=parse_expr(parse_state, binding_power(op)))
    
    return left_expr

def parse_unary(parse_state):
    tv = tokenValue(parse_state.currentToken())
    parse_state.matchSymbol(tv)
    expr_node = parse_primary(parse_state)
    return UnaryOp(op=tv, expr_node=expr_node)

def parse_primary(parse_state):
    ct = parse_state.currentToken()

    if tokenValue(ct) == "-":
        return parse_unary(parse_state)

    primary_expr = None
    if tokenValue(ct) == "(":
        parse_state.matchSymbol("(")
        parenthesized_expr = parse_expr(parse_state)
        parse_state.matchSymbol(")")
        primary_expr = parenthesized_expr
    elif tokenType(ct) == "identifier":
        identifier = tokenValue( parse_state.matchTokenType("identifier") )
        primary_expr = VariableLookup(identifier=identifier)

    if primary_expr != None:
        ct = parse_state.currentToken()
        while True:
            if tokenValue(ct) == '(':
                fn_call_args = parse_function_call(parse_state)
                primary_expr = FunctionCall(callee=primary_expr, expr_args=fn_call_args)
            elif tokenValue(ct) == '[':
                parse_state.matchSymbol('[')
                index_expr = parse_expr(parse_state)
                parse_state.matchSymbol(']')
                primary_expr = Access(left_expr=primary_expr, index_expr=index_expr)
            else:
                return primary_expr
            ct = parse_state.currentToken()


    value, literal_type = parse_state.matchLiteral()
    return Literal(literal_type=literal_type, value=value)


def parse_function_call(parse_state):
    parse_state.matchSymbol("(")

    if tokenValue( parse_state.currentToken() ) == ")":
        expr_args = []
    else:
        expr_args = parse_expr_list(parse_state)

    parse_state.matchSymbol(")")
    return expr_args

def parse_expr_list(parse_state):
    expr = parse_expr(parse_state)

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
    

def parse_tokens(tokens):
    parse_state = ParseState(tokens)
    ast = parse_program(parse_state)
    pretty_print_ast(ast)
    return ast

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


'''
First built-ins

Strings: immutable heterogenous char vector

    var message = 'Similar to how they are in every language';
    message[3]
    >>> 'i'

Heterogeneous vector : hevec:

    var example = ['string', 445, [1, 2, 3]]

    example[1] = 'different value';
    example[2,2] = 4;
    >>> ['string', 'different value', [1, 2, 4]]

    * as name implies, can contain all different types and can be nested jaggedly
    * us references, access time is slow and you lose out on spatial locality, but flexibility is improved

Homogenous vector : hovec:

    var num_example = <1, 2, 3>;
    var two_dimensional = < <3, 3, 3>, <3, 0, 3> >

    num_example[0]
    >>> 1
    two_dimensional[2, 1]
    >>> 0

    * homogenous vector -> contains all the same type
    *                   -> can be nested, but cannot be jagged
    *                   -> stored contiguously in memory

-----------------Next step after above is implemented-------------------
Everything here is tentative

Associative Array : assarr

decisions to make: 
- syntax
- Heterogeneous or homogenous type
- implementation


Classes, records, structs, etc.

- Aggregate data types are extremely useful. 
- What I'm thinking right now is to have something akin to C/C++ structs
    - make class methods and constructors optional, so that you can easily make a struct/namedtuple data type
    - but also make class methods and constructors syntactically pleasant so that object oriented code is easy to write

- Inheritance/Polymorphism/Prototype rules
    - I like how python does it




'''

'''
Change AssignOp to have LValue instead of identifier member
Make an LValue node, can be:
|- an identifier
|- an access expression. so an identifier (and maybe also an expression or primary later? foo()[bar] = ...?)
    followed by a set of brackets with an expressionlist enclosed

Add a some new logic to parse_primary:
- parse access expressions, should be very similar to function calls
- parse literals for homogenous and heterogenous vectors. We already have expressionlist parsing code
    so adding this should be trivial

Then just make sure to add nodes for the new types above.


'''