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
        self.children = []

    # consider removing
    def add_top_level_stmt(self, node):
        self.children.append( node )

    def __repr__(self):
        return repr(self.children)

class Assign(Node):
    def __init__(self, identifier, expr_node):
        self.type = "Assign"
        self.identifier = identifier
        self.expr_node = expr_node

class Declare(Node):
    def __init__(self, identifier, expr_node):
        self.type = "Declare"
        self.identifier = identifier
        self.expr_node = expr_node

    def __repr__(self):
        return "{} {} = \n{}".format(self.type, self.identifier, repr(self.expr_node))

class FunctionDeclare(Node):
    def __init__(self, identifier, args, body):
        self.type = "FunctionDeclare"
        self.identifier = identifier
        self.args = args
        self.body = body

class Return(Node):
    def __init__(self, expr_node):
        self.type = "Return"
        self.expr_node = expr_node

class AssignOp(Node):
    def __init__(self, identifier, op, expr_node):
        self.type = "AssignOp"
        self.identifier = identifier
        self.op = op
        self.expr_node = expr_node

class IfThen(Node):
    def __init__(self, condition, body):
        self.type = "IfThen"
        self.condition = condition
        self.body = body

class IfElse(Node):
    def __init__(self, condition, then_body, else_body):
        self.type = "IfElse"
        self.condition = condition
        self.then_body = then_body
        self.else_body = else_body

class While(Node):
    def __init__(self, condition, body):
        self.type = "While"
        self.condition = condition
        self.body = body

class BinaryOp(Node):
    def __init__(self, op, left_expr, right_expr):
        self.type = "BinaryOp"
        self.op = op
        self.left_expr = left_expr
        self.right_expr = right_expr

    def __repr__(self):
        return "({}\n left: {}\n right: {}\n)".format(self.op, self.left_expr, self.right_expr)

class UnaryOp(Node):
    def __init__(self, op, expr_node):
        self.type = "UnaryOp"
        self.op = op
        self.expr_node = expr_node

    def __repr__(self):
        return "({}\n {})".format(self.op, self.expr_node)

class FunctionCall(Node):
    def __init__(self, identifier, expr_args):
        self.type = "FunctionCall"
        self.identifier = identifier
        self.expr_args = expr_args

    def __str__(self):
        return "({}, {}, args: {})".format(self.type, self.identifier, str(self.expr_args))
    

class VariableLookup(Node):
    def __init__(self, identifier):
        self.type = "VariableLookup"
        self.identifier = identifier

    def __str__(self):
        return "({}, {})".format(self.type, self.identifier)

class Literal(Node):
    def __init__(self, literal_type, value):
        self.literal_type = literal_type
        self.value = value

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
            pass
        else:
            parse_error("Encountered unexpected token \'{}\' while parsing top-level statement".format(tv))


        # might be unneeded
        ct = currentToken()

    return ast_root


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
    
    

if __name__ == "__main__":
    
    with open('tokens2.txt', 'r') as file:
        data = file.read().replace('\n', '')
    
    # wow dangerous
    tokens = eval(data)
    #print(tokens)
    ast = parse_program()

    print(ast)
