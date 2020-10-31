
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
        return "({}, {})".format(self.literal_value, self.literal_type)