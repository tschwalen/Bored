#include "ast.h"
#include "asteval.h"
#include "interpreter.h"
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

/* 
*   Utility Functions
*/
bool is_gnr(KvazzResult &kr) {
    return kr.flag == KvazzFlag::Good && kr.kvazz_value.type == KvazzType::Nothing;
}

bool test(KvazzResult kr) {
    /*
     *  Truthy-falsey test for every valid type a kvazz expression could evaluate to.
    */
    auto type = kr.kvazz_value.type;
    auto var_value = kr.kvazz_value.value;

    switch(type) {
        case KvazzType::Bool:
            {
                return std::get<bool>(var_value);
            }
        case KvazzType::Int:
            {
                auto int_value = std::get<int>(var_value);
                return int_value == 0 ? false : true;
            }
        case KvazzType::String:
            {
                auto str_value = std::get<string>(var_value);
                return str_value.length() < 0 ? false : true;
            }
        case KvazzType::Hevec:
            {
                auto vec_value = std::get<vector<KvazzValue>>(var_value);
                return vec_value.length() < 0 ? false : true;
            }
        case KvazzType::Real:
            {
                auto real_value = std::get<double>(var_value);
                // floating point truthyness seems like a bad idea, but
                // I'll put it here for the sake of completeness
                return real_value == 0.0 ? false : true;
            }
        case KvazzType::Nothing:
        case KvazzType::Builtin:
        case KvazzType::LValue:
        default:
            {
                // give more info in the future
                std::cerr << "Attempted bool test on invalid expression value." << std::endl;
            }
    }
    return false;
}

KvazzValue NOTHING = KvazzValue { KvazzType::Nothing, 0 };

KvazzResult GOOD_NO_RETURN = KvazzResult { NOTHING, KvazzFlag::Good };



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
KvazzResult call_function(KvazzFunction &fn, vector<KvazzValue> &arg_values, shared_ptr<Env> env) {
    // TEMP
    return GOOD_NO_RETURN;
}

/*
*  AST-eval functions
*/

KvazzResult Interpreter::eval(Program &node, std::shared_ptr<Env> env) {
    for (auto nd : node.children()) {
        nd->eval(*this, env);
    }

    auto main_found = env->table.find("main");
    if (main_found != env->table.end()) {
        auto main_method = std::get<KvazzFunction>(main_found->second.contents);
        vector<KvazzValue> args;
        call_function(main_method, args, env);
    }
    return GOOD_NO_RETURN;
}

KvazzResult Interpreter::eval(Block &node, std::shared_ptr<Env> env) {
    auto local_env = std::make_shared<Env>(env, unordered_map<string, EnvEntry>());

    for (auto nd : node.children()) {
        auto result = nd->eval(*this, local_env);
        if (result.flag == KvazzFlag::Return)
            return result;
    }
    return GOOD_NO_RETURN;
}


KvazzResult Interpreter::eval(AssignOp &node, std::shared_ptr<Env> env);

KvazzResult Interpreter::eval(Declare &node, std::shared_ptr<Env> env);
KvazzResult Interpreter::eval(FunctionDeclare &node, std::shared_ptr<Env> env);

KvazzResult Interpreter::eval(Return &node, std::shared_ptr<Env> env) {
    auto expression_result = node.expr_node->eval(*this, env);
    expression_result.flag = KvazzFlag::Return;
    return expression_result;
}

KvazzResult Interpreter::eval(IfThen &node, std::shared_ptr<Env> env) {
    if (test(node.condition->eval(*this, env))) {
        return node.body->eval(*this, env);
    }
    return GOOD_NO_RETURN;
}

KvazzResult Interpreter::eval(IfElse &node, std::shared_ptr<Env> env) {
    if (test(node.condition->eval(*this, env))) {
        return node.then_body->eval(*this, env);
    }
    else {
        return node.else_body->eval(*this, env);
    }
}

KvazzResult Interpreter::eval(While &node, std::shared_ptr<Env> env) {
    while (test(node.condition->eval(*this, env))) {
        auto maybe_result = node.body->eval(*this, env);
        if(maybe_result.flag == KvazzFlag::Return)
            return maybe_result;
    }
    return GOOD_NO_RETURN;
}

KvazzResult Interpreter::eval(BinaryOp &node, std::shared_ptr<Env> env);
KvazzResult Interpreter::eval(UnaryOp &node, std::shared_ptr<Env> env);
KvazzResult Interpreter::eval(FunctionCall &node, std::shared_ptr<Env> env);
KvazzResult Interpreter::eval(Access &node, std::shared_ptr<Env> env);
KvazzResult Interpreter::eval(VariableLookup &node, std::shared_ptr<Env> env);
KvazzResult Interpreter::eval(IntLiteral &node, std::shared_ptr<Env> env);
KvazzResult Interpreter::eval(BoolLiteral &node, std::shared_ptr<Env> env);
KvazzResult Interpreter::eval(RealLiteral &node, std::shared_ptr<Env> env);
KvazzResult Interpreter::eval(StringLiteral &node, std::shared_ptr<Env> env);
KvazzResult Interpreter::eval(VectorLiteral &node, std::shared_ptr<Env> env);


//KvazzResult eval_assignop(shared_ptr<BaseNode> node, shared_ptr<Env> env) {
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
//};





