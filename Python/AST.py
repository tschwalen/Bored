



def _compare_node_sequence( seq1, seq2 ):
    ''' used by equals methods '''
    return len(seq1) == len(seq2) and all( map(lambda t : t[0] == t[1], zip(seq1, seq2)) )
    
# AST node types
class Node:
    def __init__(self):
        self.type = "none"

    def children(self):
        return []
    
    def eq_type(self, other):
        ''' used by the __eq__ method of subclasses '''
        return type(self) == type(other) and self.type == other.type

    def eval(self):
        return None


class Program(Node):
    def __init__(self):
        self.type = "Program"
        self.nodes = []

    def add_top_level_stmt(self, node):
        self.nodes.append( node )

    def value(self):
        return self.type

    def children(self):
        return self.nodes

    def __repr__(self):
        return repr(self.nodes)

    def __eq__( self, other ):
        return self.eq_type(other) and _compare_node_sequence(self.nodes, other.nodes)

class Block(Node):
    def __init__(self, stmts):
        self.type = "Block"
        self.stmts = stmts

    def add_stmt(self, node):
        self.stmts.append( node )

    def value(self):
        return self.type

    def children(self):
        return self.stmts

    def __repr__(self):
        return repr(self.stmts)

    def __eq__( self, other ):
        return self.eq_type(other) and _compare_node_sequence(self.stmts, other.stmts)

class AssignOp(Node):
    def __init__(self, lvalue, op, expr_node):
        self.type = "AssignOp"
        self.lvalue = lvalue
        self.op = op
        self.expr_node = expr_node

    def value(self):
        return "{} {} LValue RValue".format(self.type, self.op)

    def children(self):
        return [self.lvalue, self.expr_node]

    def __eq__( self, other ):
        return self.eq_type(other) and self.lvalue == other.lvalue and self.expr_node == other.expr_node and self.op == other.op



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
    
    def __eq__( self, other ):
        return self.eq_type(other) and self.identifier == other.identifier and self.expr_node == other.expr_node

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

    def __eq__( self, other ):
        return self.eq_type(other) and self.identifier == other.identifier and self.args == other.args and self.body == other.body

class Return(Node):
    def __init__(self, expr_node):
        self.type = "Return"
        self.expr_node = expr_node

    def value(self):
        return "Return"

    def children(self):
        return [self.expr_node]

    def __eq__( self, other ):
        return self.eq_type(other) and self.expr_node == other.expr_node


class IfThen(Node):
    def __init__(self, condition, body):
        self.type = "IfThen"
        self.condition = condition
        self.body = body

    def value(self):
        return "If then"

    def children(self):
        return [self.condition] + [self.body]

    def __eq__( self, other ):
        return self.eq_type(other) and self.condition == other.condition and self.body == other.body

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

    def __eq__( self, other ):
        return self.eq_type(other) and self.condition == other.condition and self.then_body == other.then_body and self.else_body == other.else_body

class While(Node):
    def __init__(self, condition, body):
        self.type = "While"
        self.condition = condition
        self.body = body

    def value(self):
        return "While do"

    def children(self):
        return [self.condition] + [self.body]

    def __eq__( self, other ):
        return self.eq_type(other) and self.condition == other.condition and self.body == other.body

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

    def __eq__( self, other ):
        return self.eq_type(other) and self.op == other.op and self.left_expr == other.left_expr and self.right_expr == other.right_expr

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

    def __eq__( self, other ):
        return self.eq_type(other) and self.op == other.op and self.expr_node == other.expr_node

class FunctionCall(Node):
    def __init__(self, callee, expr_args):
        self.type = "FunctionCall"
        self.callee = callee
        self.expr_args = expr_args

    def value(self):
        return "{} callee args...".format(self.type, self.callee)

    def children(self):
        return [self.callee] + self.expr_args

    def __str__(self):
        return "({}, {}, args: {})".format(self.type, self.callee, str(self.expr_args))

    def __eq__( self, other ):
        return self.eq_type(other) and self.callee == other.callee and _compare_node_sequence(self.expr_args, other.expr_args)
    
class Access(Node):
    def __init__(self, left_expr, index_expr):
        self.type = 'Access'
        self.left_expr = left_expr
        self.index_expr = index_expr

    def value(self):
        return "{} accessee index".format(self.type)

    def children(self):
        return [self.left_expr, self.index_expr]

    def __str__(self):
        return "({}, {}, {})".format(self.type, self.left_expr, self.index_expr)

    def __eq__( self, other ):
        return self.eq_type(other) and self.left_expr == other.left_expr and self.index_expr == other.index_expr

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

    def __eq__( self, other ):
        return self.eq_type(other) and self.identifier == other.identifier

class Literal(Node):
    def __init__(self, literal_type, value):
        self.type = literal_type
        self.literal_value = value

    def value(self):
        return "{} \'{}\'".format(self.type, self.literal_value)

    def children(self):
        return []

    def __str__(self):
        return "({}, {})".format(self.literal_value, self.type)

    def __eq__( self, other ):
        return self.eq_type(other) and self.literal_value == other.literal_value


class VectorLiteral(Node):
    def __init__(self, vector_type, contents):
        self.type = 'VectorLiteral'
        self.vector_type = vector_type
        self.contents = contents

    def value(self):
        return "{} {}".format(self.type, self.vector_type)

    def children(self):
        return self.contents

    def __eq__( self, other ):
        return self.eq_type(other) and self.vector_type == other.vector_type and _compare_node_sequence(self.contents, other.contents)