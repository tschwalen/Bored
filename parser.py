"""
program -> top_lvl_stmt program
        |  top_lvl_stmt

top_lvl_stmt -> declare
             |  func_dec

declare -> 'var' <identifier> '=' expr ';'

func_dec -> 'function' <identifier> '(' arg_list ')' block
         |  'function' <identifier> '(' ')' block

arg_list -> <identifier> ',' arg_list
         |  <identifier>

block -> '{' stmt_sequence '}'

stmt_sequence -> stmt stmt_sequence
              |  stmt

stmt -> declare
     |   <identifier> '=' expr ';'
     |  'return' expr ';'
     |   modifier ';'
     |   func_call ';'
     |   'if' expr 'then' block 
     |   'if' expr 'then' block 'else' block
     |   'while' expr 'do' block

modifier -> <identifier> '+=' expr
         |  <identifier> '-=' expr
         |  <identifier> '/=' expr
         |  <identifier> '*=' expr
         |  <identifier> '%=' expr

func_call -> <identifier> '(' ')'
          |  <identifier> '(' expr_list ')'

expr_list -> expr ',' expr_list
          |  expr

expr -> and_expr '|' expr
     |  and_expr

and_expr -> comparison '&' and_expr
         |  comparison

comparison -> term == comparison 
           |  term != comparison  
           |  term >  comparison 
           |  term <  comparison 
           |  term >= comparison  
           |  term <= comparison  
           |  term  

term -> factor '+' term
     |  factor '-' term 
     |  factor

factor -> unary '*' factor    
       |  unary '/' factor
       |  unary '%' factor     
       |  unary

unary -> '-' primary
      |  '!' primary
      |  primary

primary -> <int-literal>
        |  <real-literal>
        |  <string-literal>
        |  <bool-literal>
        |  <identifier>
        |  '(' expr ')'
        |  func_call

"""

import sys




# AST node types
class Node:
    def __init__(self):
        self.type = "none"
    

class Program(Node):
    def __init__(self):
        self.type = "Program"
        self.nodes = []

    # consider removing
    def add_top_level_stmt(self, node):
        self.nodes.append( node )

    def value(self):
        return self.type

    def children(self):
        return self.nodes

    def __repr__(self):
        return repr(self.nodes)

class Block(Node):
    def __init__(self, stmts):
        self.type = "Block"
        self.stmts = stmts

    # consider removing
    def add_stmt(self, node):
        self.stmts.append( node )

    def value(self):
        return self.type

    def children(self):
        return self.stmts

    def __repr__(self):
        return repr(self.stmts)

class Assign(Node):
    def __init__(self, identifier, expr_node):
        self.type = "Assign"
        self.identifier = identifier
        self.expr_node = expr_node

    def value(self):
        return "{} {}".format(self.type, self.identifier)

    def children(self):
        return [self.expr_node]

class Declare(Node):
    def __init__(self, identifier, expr_node):
        self.type = "Declare"
        self.identifier = identifier
        self.expr_node = expr_node

    def value(self):
        return "{} {}".format(self.type, self.identifier)

    def children(self):
        return [self.expr_node]

    def __repr__(self):
        return "{} {} = \n{}".format(self.type, self.identifier, repr(self.expr_node))

class FunctionDeclare(Node):
    def __init__(self, identifier, args, body):
        self.type = "FunctionDeclare"
        self.identifier = identifier
        self.args = args
        self.body = body

    def value(self):
        return "{} {} with {}".format(self.type, self.identifier, self.args)

    def children(self):
        return [self.body]

class Return(Node):
    def __init__(self, expr_node):
        self.type = "Return"
        self.expr_node = expr_node

    def value(self):
        return "Return"

    def children(self):
        return [self.expr_node]

class AssignOp(Node):
    def __init__(self, identifier, op, expr_node):
        self.type = "AssignOp"
        self.identifier = identifier
        self.op = op
        self.expr_node = expr_node

    def value(self):
        return "{} {} {}".format(self.type, self.identifier, self.op)

    def children(self):
        return [self.expr_node]

class IfThen(Node):
    def __init__(self, condition, body):
        self.type = "IfThen"
        self.condition = condition
        self.body = body

    def value(self):
        return "If then"

    def children(self):
        return [self.condition] + [self.body]

class IfElse(Node):
    def __init__(self, condition, then_body, else_body):
        self.type = "IfElse"
        self.condition = condition
        self.then_body = then_body
        self.else_body = else_body

    def value(self):
        return "If then else"

    def children(self):
        return [self.condition] + [self.then_body] + [self.else_body]

class While(Node):
    def __init__(self, condition, body):
        self.type = "While"
        self.condition = condition
        self.body = body

    def value(self):
        return "While do"

    def children(self):
        return [self.condition] + [self.body]

class BinaryOp(Node):
    def __init__(self, op, left_expr, right_expr):
        self.type = "BinaryOp"
        self.op = op
        self.left_expr = left_expr
        self.right_expr = right_expr

    def value(self):
        return "{} {}".format(self.type, self.op)

    def children(self):
        return [self.left_expr] + [self.right_expr]

    def __repr__(self):
        return "({}\n left: {}\n right: {}\n)".format(self.op, self.left_expr, self.right_expr)

class UnaryOp(Node):
    def __init__(self, op, expr_node):
        self.type = "UnaryOp"
        self.op = op
        self.expr_node = expr_node

    def value(self):
        return "{} {}".format(self.type, self.op)

    def children(self):
        return [self.expr_node]

    def __repr__(self):
        return "({}\n {})".format(self.op, self.expr_node)

class FunctionCall(Node):
    def __init__(self, identifier, expr_args):
        self.type = "FunctionCall"
        self.identifier = identifier
        self.expr_args = expr_args

    def value(self):
        return "{} {}".format(self.type, self.identifier)

    def children(self):
        return self.expr_args

    def __str__(self):
        return "({}, {}, args: {})".format(self.type, self.identifier, str(self.expr_args))
    

class VariableLookup(Node):
    def __init__(self, identifier):
        self.type = "VariableLookup"
        self.identifier = identifier

    def value(self):
        return "{} {}".format(self.type, self.identifier)

    def children(self):
        return []

    def __str__(self):
        return "({}, {})".format(self.type, self.identifier)

class Literal(Node):
    def __init__(self, literal_type, value):
        self.literal_type = literal_type
        self.literal_value = value

    def value(self):
        return "{} {}".format(self.literal_type, self.literal_value)

    def children(self):
        return []

    def __str__(self):
        return "({}, {})".format(self.value, self.literal_type)

# may seem pointless, but will make the code more readible
def tokenValue(t):
    return t[0]
def tokenType(t):
    return t[1]

# Actual Parsing code begins here
tokens = []
index = 0

# return the current token
def currentToken():
    global tokens
    global index
    if index >= len(tokens):
        return ("", "EOF")
    return tokens[index]

def peekToken(n=1):
    global tokens
    global index
    i = index + n
    if i >= len(tokens):
        return ("", "EOF")
    return tokens[i]

# return the current token, and advance the index
def advance():
    global index
    ct = currentToken()
    index += 1
    return ct


def matchKeyword(kwrd):
    ct = currentToken()
    if ct == (kwrd, "keyword"):
        return advance()
    else:
        parse_error("Expected keyword \'{}\', encountered \'{}\' with value \'{}\'.".format(kwrd, tokenType(ct), tokenValue(ct)))

def matchTokenType(ttype):
    ct = currentToken()
    if tokenType(ct) == ttype:
        return advance()
    else:
        parse_error("Expected token of type \'{}\', encountered \'{}\' with value \'{}\'.".format(ttype, tokenType(ct), tokenValue(ct)))

def matchSymbol(smbl):
    ct = currentToken()
    if ct == (smbl, "symbol"):
        return advance()
    else:
        parse_error("Expected symbol \'{}\', encountered \'{}\' with value \'{}\'.".format(smbl, tokenType(ct), tokenValue(ct)))

def matchLiteral():
    ct = currentToken()
    if tokenType(ct).endswith("literal"):
        return advance()
    else:
        parse_error("Expected literal value, encountered \'{}\' with value \'{}\'.".format( tokenType(ct), tokenValue(ct)))

# basic, but will do for now. Don't feel like adding line number reporting, error recovery until later
def parse_error(msg):
    print("Parse error: {}".format(msg))
    raise SystemExit

def parse_program():
    global tokens
    global index

    ast_root = Program()

    ct = currentToken()
    while tokenType( ct ) != "EOF":
        tv = tokenValue(ct)
        if tv == "var":
            ast_node = parse_declare()
            ast_root.add_top_level_stmt( ast_node )
        elif tv == "function":
            ast_node = parse_function_declare()
            ast_root.add_top_level_stmt( ast_node )
        else:
            parse_error("Encountered unexpected token \'{}\' while parsing top-level statement".format(tv))


        # might be unneeded
        ct = currentToken()

    return ast_root


def parse_function_declare():
    matchKeyword("function")

    fn_name = tokenValue( matchTokenType("identifier") )

    matchSymbol("(")

    if tokenValue( currentToken() ) == ")":
        arg_names = []
    else:
        arg_names = parse_arg_list()
    
    matchSymbol(")")

    body = parse_block()

    return FunctionDeclare(identifier=fn_name, args=arg_names, body=body)

def parse_arg_list():
    arg = tokenValue( matchTokenType("identifier") )

    if tokenValue( currentToken() ) == ",":
        matchSymbol(",")
        return [arg] + parse_arg_list()

    return [arg]

def parse_block():
    # one thing we could do here is accept single statements followed by a semicolon, without requiring braces around it. 
    # this would be a simple matter of checking if the next token was a brace symbol. 
    # I'm undecided on this so far because I'm not sure if I'll like how it looks, and because I'm not sure if I want 
    # to allow single statements in every case where I have a <block> in the grammar. for example, I think it would look
    # strange for function declarations not to include braces.

    matchSymbol("{")
    stmts = [ parse_statement() ]

    # yes I'm doing it iteratively. Functions tend to have no more than 6 or 7 arguments whereas a block can have dozens of statements
    while tokenValue( currentToken() ) != "}":
        stmts.append( parse_statement() )

    matchSymbol("}")
    return Block(stmts=stmts)

def parse_statement():
    ct = currentToken()

    if tokenType(ct) == "identifier":
        nt = peekToken()
        tv = tokenValue(nt)
        if tv == "=":
            stmt = parse_assign()
            return stmt

        if tv == "(":
            stmt = parse_function_call()
            matchSymbol(";")
            return stmt

        return parse_modifier()

    tv = tokenValue(ct)
    
    if tv == "var":
        return parse_declare()

    if tv == "return":
        matchKeyword("return")
        expr = parse_expr()
        matchSymbol(";")
        return Return(expr_node=expr)

    if tv == "if":
        return parse_if()

    if tv == "while":
        return parse_while()

    parse_error("Invalid start of statement, encountered {}".format(tv))

def parse_if():

    matchKeyword("if")
    condition = parse_expr()

    matchKeyword("then")
    then_body = parse_block()

    if tokenValue(currentToken()) == "else":
        matchKeyword("else")
        else_body = parse_block()
        return IfElse(condition=condition, then_body=then_body, else_body=else_body)

    return IfThen(condition=condition, body=then_body)

def parse_while():
    matchKeyword("while")

    condition = parse_expr()

    matchKeyword("do")

    body = parse_block()

    return While(condition=condition, body=body)

def parse_assign():
    identifier = tokenValue( matchTokenType("identifier") )

    matchSymbol("=")

    expression = parse_expr()

    matchSymbol(";")

    return Assign(identifier=identifier, expr_node=expression)


def parse_modifier():
    identifier = tokenValue( matchTokenType("identifier") )
    tv = tokenValue( currentToken() )
    if tv in ["+=", "-=", "/=", "*=", "%="]:
            matchSymbol(tv)

            expr = parse_expr()

            matchSymbol(";")
            return AssignOp(identifier=identifier, op=tv, expr_node=expr)
    
    parse_error("Expected assignment, modifier, or function to follow identifier. Encountered {} {}".format(identifier, tv))

def parse_declare():
    matchKeyword("var")

    identifier = tokenValue( matchTokenType("identifier") )

    matchSymbol("=")

    expression = parse_expr()

    matchSymbol(";")

    return Declare(identifier=identifier, expr_node=expression)

def parse_expr():
    left_expr = parse_and_expr()

    if tokenValue(currentToken()) == "|":
        matchSymbol("|")
        right_expr = parse_expr()
        return BinaryOp(op="|", left_expr=left_expr, right_expr=right_expr)
    
    return left_expr

def parse_and_expr():
    left_expr = parse_comparison()

    if tokenValue(currentToken()) == "&":
        matchSymbol("&")
        right_expr = parse_and_expr()
        return BinaryOp(op="&", left_expr=left_expr, right_expr=right_expr)

    return left_expr

def parse_comparison():
    left_expr = parse_term()
    
    tv = tokenValue(currentToken())
    if tv in ["==", "!=", "==", "<=", ">=", "<", ">"]:
        matchSymbol(tv)
        right_expr = parse_comparison()
        return BinaryOp(op=tv, left_expr=left_expr, right_expr=right_expr)
    else:
        return left_expr

def parse_term():
    left_expr = parse_factor()

    tv = tokenValue(currentToken())
    if tv in ["+", "-"]:
        matchSymbol(tv)
        right_expr = parse_term()
        return BinaryOp(op=tv, left_expr=left_expr, right_expr=right_expr)
    else:
        return left_expr

def parse_factor():
    left_expr = parse_unary()

    tv = tokenValue(currentToken())
    if tv in ["*", "/", "%"]:
        matchSymbol(tv)
        right_expr = parse_factor()
        return BinaryOp(op=tv, left_expr=left_expr, right_expr=right_expr)
    else:
        return left_expr

def parse_unary():
    tv = tokenValue(currentToken())
    if tv in ["+", "-"]:
        matchSymbol(tv)
        expr_node = parse_primary()
        return UnaryOp(op=tv, expr_node=expr_node)
    return parse_primary()

def parse_primary():
    ct = currentToken()

    if tokenValue(ct) == "(":
        matchSymbol("(")
        parenthesized_expr = parse_expr()
        matchSymbol(")")
        return parenthesized_expr

    if tokenType(ct) == "identifier":
        nt = peekToken()
        if tokenValue(nt) == "(":
            return parse_function_call()
        identifier = tokenValue( matchTokenType("identifier") )
        return VariableLookup(identifier=identifier)

    value, literal_type = matchLiteral()
    return Literal(literal_type=literal_type, value=value)


def parse_function_call():
    fn_name = tokenValue( matchTokenType("identifier") )
    matchSymbol("(")

    if tokenValue( currentToken() ) == ")":
        expr_args = []
    else:
        expr_args = parse_expr_list()

    matchSymbol(")")
    return FunctionCall(identifier=fn_name, expr_args=expr_args)

def parse_expr_list():
    expr = parse_expr()

    # of course this could be done iteratively, but if I'm gonna write a recusive descent parser I may as well commit to it
    if tokenValue( currentToken() ) == ",":
        matchSymbol(",")
        return [expr] + parse_expr_list()

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

    _file = "tokens.txt"
    if len(args) > 0:
        _file = args[0]


    with open(_file, 'r') as file:
        data = file.read().replace('\n', '')
    
    # wow dangerous
    tokens = eval(data)
    #print(tokens)
    ast = parse_program()

    pretty_print_ast(ast)
