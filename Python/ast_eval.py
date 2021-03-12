
from AST import *
import operator as op

op_table = {
    '+'  : op.add, 
    '-'  : op.sub, 
    '*'  : op.mul, 
    '/'  : op.truediv,
    '%'  : op.mod, 
    '>'  : op.gt, 
    '<'  : op.lt, 
    '>=' : op.ge, 
    '<=' : op.le, 
    '==' : op.eq, 
    '!=' : op.ne,
    '&'  : lambda a, b : a and b,
    '|'  : lambda a, b : a or b
}

unary_op_table = {
    '-' : lambda x : -x,
    '!' : lambda x: not x
}


######### Built-in function executors ##########

def execute_print(args):
    print(*args)

def execute_length(args):
    assert len(args) == 1
    return len(args[0])

def execute_hevec(args):
    ''' 
    args : (size, default_value=None) 
    returns : heterogeneous vector of size <size> with every index containing <default_value>
    '''

    assert len(args) > 0 and len(args) < 3
    if len(args) == 1:
        return [None] * args[0]
    return [args[1]] * args[0]

############ Symbol Table (env) Operations ############


def lookup(identifier, env):
    if identifier in built_ins:
        return ('built-in', identifier, ['args'], built_ins[identifier]), env
    ogenv = env
    while env:
        value = env.get(identifier, None)
        if value != None:
            return value, env
        env = env.get('#parent', None)
    raise Exception('Lookup of identifier %s failed in env %s' % (identifier, ogenv))

def lookup_value(identifier, env):
    return lookup(identifier, env)[0]

############## Interpreter Evaluation Code ##############

symbol_table = {} # starting global symbol table or environment

def eval_ast(prgm):
    global symbol_table

    if prgm.type == 'Program':

        for node in prgm.nodes:
            eval_node(node, symbol_table)

        result = call_function(lookup_value('main', symbol_table), [], symbol_table)
        #print(result)


def eval_node(node, env):
    return visitor_table[node.type](node, env)

def call_function(fn, args, env):
    ''' fn : ('function', <name>, <args>, <body>) '''
    if fn[0] == 'built-in':
        return fn[3](args)

    body = fn[3]
    fn_env = {arg_name : arg_value for arg_name, arg_value in zip(fn[2], args)}
    fn_env['#parent'] = symbol_table # could change this to add closures later, not sure if I want to though

    # allows for recursion (actually, not needed until/unless 1st class functions are implemented)
    # if fn[1] != 'main':
    #     fn_env[fn[1]] = fn


    return eval_node(body, fn_env)[0]

def eval_fn_call(node, env):
    fn = eval_node(node.callee, env)
    args = [eval_node(n, env) for n in node.expr_args]
    return call_function(fn, args, env)
    

def eval_bin_op(node, env):
    left = eval_node(node.left_expr, env)
    right = eval_node(node.right_expr, env)
    return op_table[node.op](left, right)

def eval_un_op(node, env):
    val = eval_node(node.expr_node, env)
    return unary_op_table[node.op](val)

def eval_declare(node, env):
    if node.identifier in env:
        raise Exception( 'Variable %s already defined in scope' % node.identifier )
    env[node.identifier] = eval_node(node.expr_node, env)

def eval_assignop(node, env):
    try:
        item, index, value = eval_lvalue(node.lvalue, env)
    except Exception as ex:
        raise ex
    if node.op == '=':
        item[index] = eval_node(node.expr_node, env)
    else:
        item[index] = op_table[node.op[0]](value, eval_node(node.expr_node, env)) 

def eval_fn_declare(node, env):
    if node.identifier in env:
        raise Exception( 'Variable %s already defined in scope' % node.identifier )
    env[node.identifier] = ('function', node.identifier, node.args, node.body)

def eval_lvalue(node, env):
    if node.type == 'VariableLookup':
        index = node.identifier
        env = symbol_table if node.sigil else env
        value, env = lookup(index, env)
        return env, index, value
    item = eval_node(node.left_expr, env)
    index = eval_node(node.index_expr, env)
    return item, index, item[index]

def eval_block(node, env):
    local_env = {'#parent' : env}
    for stmt in node.stmts:
        v = eval_node(stmt, local_env)
        if type(v) == tuple and v[1] == 'return':
            return v
    return (None, 'Nothing')

def eval_return(node, env):
    return eval_node(node.expr_node, env), 'return'

def eval_while(node, env):
    while eval_node(node.condition, env):
        result = eval_node(node.body, env)
        if result[1] == 'return':
            return result

def eval_if_else(node, env):
    if eval_node(node.condition, env):
        return eval_node(node.then_body, env)
    else:
        return eval_node(node.else_body, env)

def eval_if_then(node, env):
    if eval_node(node.condition, env):
        return eval_node(node.body, env)

def eval_var_lookup(node, env):
    env = symbol_table if node.sigil else env
    return lookup_value(node.identifier, env)

def eval_access(node, env):
    left = eval_node(node.left_expr, env)
    index = eval_node(node.index_expr, env)
    return left[index]

def eval_vector_literal(node, env):

    ## do something about the vector type at some point
    return [eval_node(n, env) for n in node.contents]

def eval_bool_literal(node, env):
    if hasattr(node, 'cached'):
        return node.cached
    b = str.lower(node.literal_value)
    if b == 'true':
        node.cached = True
        return True
    if b == 'false':
        node.cached = False
        return False
    raise Exception('Invalid boolean value: %s' % b)

def eval_int_literal(node, env):
    if hasattr(node, 'cached'):
        return node.cached
    try:
        i = int(node.literal_value)
        node.cached = i
        return i
    except Exception as ex:
        raise ex

def eval_real_literal(node, env):
    if hasattr(node, 'cached'):
        return node.cached
    try:
        i = float(node.literal_value)
        node.cached = i
        return i
    except Exception as ex:
        raise ex

def eval_string_literal(node, env):
    return node.literal_value

visitor_table = {
    'bool-literal' : eval_bool_literal,
    'int-literal' : eval_int_literal,
    'real-literal' : eval_real_literal,
    'string-literal' : eval_string_literal,
    'VectorLiteral' : eval_vector_literal,
    'VariableLookup' : eval_var_lookup,
    'Access' : eval_access,
    'FunctionCall' : eval_fn_call,
    'UnaryOp' : eval_un_op,
    'BinaryOp' : eval_bin_op,
    'While' : eval_while,
    'IfThen' : eval_if_then,
    'IfElse' : eval_if_else,
    'Return' : eval_return,
    'FunctionDeclare' : eval_fn_declare,
    'Declare' : eval_declare,
    'AssignOp' : eval_assignop,
    'Block' : eval_block
}

built_ins = {
    'print' : execute_print,
    'printf': None,
    'println': None,
    'lengthof' : execute_length,
    'hevec': execute_hevec,
    'hovec': None
}