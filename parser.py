"""
program -> top_lvl_stmt program
        |  top_lvl_stmt

top_lvl_stmt -> 'var' <identifier> '=' expr ';'
             |  func_dec

func_dec -> 'function' <identifier> '(' arg_list ')' block
         |  'function' <identifier> '(' ')' block

arg_list -> <identifier> ',' arg_list
         |  <identifier>

block -> '{' stmt_sequence '}'

stmt_sequence -> stmt stmt_sequence
              |  stmt

stmt -> 'var' <identifier> '=' expr ';'
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

expr -> expr '|' and_expr
     |  and_expr

and_expr -> and_expr '&' comparison
         |  comparison

comparison -> comparison == term 
           |  comparison != term  
           |  comparison > term  
           |  comparison < term  
           |  comparison >= term  
           |  comparison <= term  
           |  term  

term -> term '+' factor
     |  term '-' factor
     |  factor

factor -> factor '*' unary    
       |  factor '/' unary
       |  factor '%' unary    
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

# may seem pointless, but will make the code more readible
def tokenRep(t):
    return t[0]
def tokenType(t):
    return t[1]

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

class Assign(Node):
    def __init__(self, id, expr_node):
        self.type = "Assign"
        self.id = id
        self.expr_node = expr_node

class Declare(Node):
    def __init__(self, id, expr_node):
        self.type = "Declare"
        self.id = id
        self.expr_node = expr_node

class FunctionDeclare(Node):
    def __init__(self, id, args, body):
        self.type = "FunctionDeclare"
        self.id = id
        self.args = args
        self.body = body

class Return(Node):
    def __init__(self, expr_node):
        self.type = "Return"
        self.expr_node = expr_node

class AssignOp(Node):
    def __init__(self, id, op, expr_node):
        self.type = "AssignOp"
        self.id = id
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

class UnaryOp(Node):
    def __init__(self, op, expr_node):
        self.type = "UnaryOp"
        self.op = op
        self.expr_node = expr_node

class FunctionCall(Node):
    def __init__(self, id, expr_args):
        self.type = "FunctionCall"
        self.id = id
        self.expr_args = expr_args


if __name__ == "__main__":

    with open('tokens.txt', 'r') as file:
        data = file.read().replace('\n', '')
    
    # wow dangerous
    tokens = eval(data)
    print(tokens)