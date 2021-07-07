#include "ast.h"
#include "asteval.h"
#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <iostream>
#include <unordered_map>

using std::unordered_map; 
using std::shared_ptr;
using std::vector;
using std::string;

/*

Note: 

I've reached a point in the design of the interpreter where I've realized that
I need to rethink some design decisions. I think the best practice thing to
do at this point is to implement the AST interpreter using a visitor design
pattern.

If this fails, then I supposed every AST node could have an "eval" method,
but this is pretty nasty from a separation of concerns perspective.


Visitor design pattern:

Interpreter is now a class with a Visit/Dispatch method for every node type
- Basically the methods I have written get repurposed 
- Added benefit of encapsulating the global_env in the class, cleaner than
    having a global variable
- Built-ins, metadata, etc can be static fields

Visitor interface can be repurposed to write a single-pass AST compiler too


*/

enum built_in_function_ids {
    _print,
    _lengthof,
    _hevec
    /*
    _printf,
    _println,
    _hovec
    */
};

unordered_map<string, int> built_in_function_table = {
    {"print", _print},
    {"lengthof", _lengthof},
    {"hevec", _hevec},
};


Env global_env {
    nullptr, 
    unordered_map<string, EnvEntry>()
};


LookupResult lookup(string identifier, shared_ptr<Env> env) {
    auto result = built_in_function_table.find(identifier);
    if (result != built_in_function_table.end()) {
        return LookupResult {
            EnvEntry {
                EnvResultType::Builtin,
                KvazzValue {
                    KvazzType::Builtin,
                    result->second
                }
            },
            nullptr
        };
    }

    auto curr_env = env;
    while (curr_env != nullptr) {
        auto result = curr_env->table.find(identifier);
        if (result != curr_env->table.end())
            return LookupResult { result->second, curr_env };
        curr_env = curr_env->parent;
    }
    
    std::cerr << "Lookup of identifier " << identifier << "failed." << std::endl; 
    return LookupResult {
        EnvEntry {
            EnvResultType::Value,
            NOTHING
        },
        curr_env
    };
}


/*
*  AST-eval functions
*/
KvazzResult eval_program(std::shared_ptr<BaseNode> node);
KvazzResult eval_node(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult call_function(KvazzFunction fn, std::vector<KvazzValue>, std::shared_ptr<Env> env);
KvazzResult eval_fn_call(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult eval_bin_op(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult eval_un_op(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult eval_declare(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);

KvazzResult eval_assignop(shared_ptr<BaseNode> node, shared_ptr<Env> env) {
    /*
    if node->lvalue is a VariableLookup of var x:
        get env e in which x resides (either tightest bound or global if sigiled)
        if node->op is standard assign:
            e[x] = eval(node.rvalue)
        else:
            e[x] = e[x] ~node->op~ eval(node.rvalue)

    else if node->lvalue is a Access:
        get data structure s being indexed into
        get KvazzValue i being used to index into s
        get KvazzValue r, the result of eval(node->rvalue)
        
        if i is not an int:
            <not implemented yet>
            allow only if s is a map data structure

        if s is a string:
            allow only standard assign

        if s is a homogenous vector:
            <not implemented yet>
            ensure that the stored value is an acceptable type

        else:
            s is a hevector
            if node->op is standard assign
                s[i] = r
            else:
                s[i] = s[i] ~node->op~ r

    */
};


KvazzResult eval_fn_declare(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult eval_lvalue(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult eval_block(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult eval_return(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult eval_while(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult eval_if_else(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult eval_if_then(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult eval_var_lookup(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult eval_access(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult eval_vector_literal(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult eval_bool_literal(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult eval_int_literal(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);
KvazzResult eval_real_literal(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env);

KvazzResult eval_string_literal(std::shared_ptr<BaseNode> node, std::shared_ptr<Env> env) {

}


/* 
*   Utility Functions
*/
bool is_gnr(KvazzResult kr) {
    return kr.flag == KvazzFlag::Good && kr.return_value.type == KvazzType::Nothing;
}



