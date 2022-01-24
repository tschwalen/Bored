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

KvazzValue NOTHING = KvazzValue { KvazzType::Nothing, 0 };

KvazzResult ERROR_NO_VALUE = KvazzResult { NOTHING, KvazzFlag::Error };

KvazzResult GOOD_NO_VALUE = KvazzResult { NOTHING, KvazzFlag::Good };

KvazzResult GOOD_BOOL_TRUE = KvazzResult {
    KvazzValue {
        KvazzType::Bool,
        true
    },
    KvazzFlag::Good
};

KvazzResult GOOD_BOOL_FALSE = KvazzResult {
    KvazzValue {
        KvazzType::Bool,
        false
    },
    KvazzFlag::Good
};

// Int, Real, Bool, String, Hevec
KvazzResult make_good_result(bool value) {
    return KvazzResult {
        KvazzValue {
            KvazzType::Bool,
            value
        },
        KvazzFlag::Good
    };
}

KvazzResult make_good_result(int value) {
    return KvazzResult {
        KvazzValue {
            KvazzType::Int,
            value
        },
        KvazzFlag::Good
    };
}

KvazzResult make_good_result(double value) {
    return KvazzResult {
        KvazzValue {
            KvazzType::Real,
            value
        },
        KvazzFlag::Good
    };
}

KvazzResult make_good_result(string value) {
    return KvazzResult {
        KvazzValue {
            KvazzType::String,
            value
        },
        KvazzFlag::Good
    };
}

KvazzResult make_good_result(vector<KvazzValue> value) {
    return KvazzResult {
        KvazzValue {
            KvazzType::Hevec,
            value
        },
        KvazzFlag::Good
    };
}

KvazzResult make_good_result(LValue value) {
    return KvazzResult {
        KvazzValue {
            KvazzType::LValue,
            value
        },
        KvazzFlag::Good
    };
}

KvazzResult make_good_result(KvazzFunction value) {
    return KvazzResult {
        KvazzValue {
            KvazzType::Function,
            value
        },
        KvazzFlag::Good
    };
}

KvazzResult make_good_result(KvazzValue value) {
    //switch on every type and then just call make_good_result once you std::get the type
    KvazzResult result;
    switch(value.type) {
        case KvazzType::Bool:
            {
                result = make_good_result(std::get<bool>(value.value));
            }
        case KvazzType::Int:
            {
                result = make_good_result(std::get<int>(value.value));
            }
        case KvazzType::String:
            {
                result = make_good_result(std::get<string>(value.value));
            }
        case KvazzType::Hevec:
            {
                result = make_good_result(std::get<vector<KvazzValue>>(value.value));
            }
        case KvazzType::Real:
            {
                result = make_good_result(std::get<double>(value.value));
            }
        case KvazzType::Nothing:
        case KvazzType::Builtin:
        case KvazzType::LValue:
        default:
            {
                std::cerr << "Attempted to make_good_value on invalid KvazzType.\n";
            }
    }
    return result;
}

/* 
*   Utility Functions
*/
bool is_gnr(KvazzResult &kr) {
    return kr.flag == KvazzFlag::Good && kr.kvazz_value.type == KvazzType::Nothing;
}

bool is_numeric_type(KvazzType t) {
    return t == KvazzType::Int || t == KvazzType::Real;
}

bool truthy_test(KvazzResult kr) {
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
            {
                return false;
            }
        case KvazzType::Builtin:
        case KvazzType::LValue:
        default:
            {
                // give more info in the future
                std::cerr << "Attempted bool test on invalid expression value.\n";
            }
    }
    return false;
}

// these functions can probably go into a different file... maybe asteval.cpp

KvazzResult kvazzvalue_plus(KvazzValue &kv1, KvazzValue &kv2) {
    auto left_type = kv1.type;
    auto right_type = kv2.type;

    if(left_type == right_type) {
        if(left_type == KvazzType::Int) {
            return make_good_result(std::get<int>(kv1.value) + std::get<int>(kv2.value));
        }
        if(left_type == KvazzType::Real) {
            return make_good_result(std::get<double>(kv1.value) + std::get<double>(kv2.value));
        }
        if(left_type == KvazzType::String) {
            return make_good_result(std::get<string>(kv1.value) + std::get<string>(kv2.value));
        }
        if(left_type == KvazzType::Hevec) {
            auto left_vec = std::get<vector<KvazzValue>>(kv1.value);
            auto &right_vec = std::get<vector<KvazzValue>>(kv2.value);
            left_vec.insert( left_vec.end(), right_vec.begin(), right_vec.end() );
            return make_good_result(left_vec);
        }
    }
    else if (left_type == KvazzType::Int) {
        if(right_type == KvazzType::Real) {
            auto left_value = std::get<int>(kv1.value);
            auto right_value = std::get<double>(kv2.value);
            return make_good_result(left_value + right_value);
        }
    }
    else if (left_type == KvazzType::Real) {
        if(right_type == KvazzType::Int) {
            auto left_value = std::get<double>(kv1.value);
            auto right_value = std::get<int>(kv2.value);
            return make_good_result(left_value + right_value);
        }
    }

    return ERROR_NO_VALUE;
}


KvazzResult kvazzvalue_modulo(KvazzValue &kv1, KvazzValue &kv2) {
    if (kv1.type == KvazzType::Int && kv2.type == KvazzType::Int) {
        return make_good_result(std::get<int>(kv1.value) % std::get<int>(kv2.value));
    }
    return ERROR_NO_VALUE;
}

KvazzResult kvazzvalue_divide(KvazzValue &kv1, KvazzValue &kv2) {
    auto left_type = kv1.type;
    auto right_type = kv2.type;
    if (left_type == KvazzType::Int) {
        auto left_value = std::get<int>(kv1.value);
        if(right_type == KvazzType::Int) {
            auto right_value = std::get<int>(kv2.value);
            return make_good_result(left_value / right_value);
        }
        else {
            auto right_value = std::get<double>(kv2.value);
            return make_good_result(left_value / right_value);
        }
    }
    else {
        auto left_value = std::get<double>(kv1.value);
        if(right_type == KvazzType::Int) {
            auto right_value = std::get<int>(kv2.value);
            return make_good_result(left_value / right_value);
        }
        else {
            auto right_value = std::get<double>(kv2.value);
            return make_good_result(left_value / right_value);
        }
    }
    return ERROR_NO_VALUE;
}

KvazzResult kvazzvalue_multiply(KvazzValue &kv1, KvazzValue &kv2) {
    auto left_type = kv1.type;
    auto right_type = kv2.type;
    if (left_type == KvazzType::Int) {
        auto left_value = std::get<int>(kv1.value);
        if(right_type == KvazzType::Int) {
            auto right_value = std::get<int>(kv2.value);
            return make_good_result(left_value * right_value);
        }
        else {
            auto right_value = std::get<double>(kv2.value);
            return make_good_result(left_value * right_value);
        }
    }
    else {
        auto left_value = std::get<double>(kv1.value);
        if(right_type == KvazzType::Int) {
            auto right_value = std::get<int>(kv2.value);
            return make_good_result(left_value * right_value);
        }
        else {
            auto right_value = std::get<double>(kv2.value);
            return make_good_result(left_value * right_value);
        }
    }
    return ERROR_NO_VALUE;
}

KvazzResult kvazzvalue_minus(KvazzValue &kv1, KvazzValue &kv2) {
    auto left_type = kv1.type;
    auto right_type = kv2.type;
    if (left_type == KvazzType::Int) {
        auto left_value = std::get<int>(kv1.value);
        if(right_type == KvazzType::Int) {
            auto right_value = std::get<int>(kv2.value);
            return make_good_result(left_value - right_value);
        }
        else {
            auto right_value = std::get<double>(kv2.value);
            return make_good_result(left_value - right_value);
        }
    }
    else {
        auto left_value = std::get<double>(kv1.value);
        if(right_type == KvazzType::Int) {
            auto right_value = std::get<int>(kv2.value);
            return make_good_result(left_value - right_value);
        }
        else {
            auto right_value = std::get<double>(kv2.value);
            return make_good_result(left_value - right_value);
        }
    }
    return ERROR_NO_VALUE;
}

bool kvazzvalue_less_equals(KvazzValue &kv1, KvazzValue &kv2) {
    auto left_type = kv1.type;
    auto right_type = kv2.type;
    if (left_type == KvazzType::Int) {
        auto left_value = std::get<int>(kv1.value);
        if(right_type == KvazzType::Int) {
            auto right_value = std::get<int>(kv2.value);
            return left_value <= right_value;
        }
        else {
            auto right_value = std::get<double>(kv2.value);
            return left_value <= right_value;
        }
    }
    else {
        auto left_value = std::get<double>(kv1.value);
        if(right_type == KvazzType::Int) {
            auto right_value = std::get<int>(kv2.value);
            return left_value <= right_value;
        }
        else {
            auto right_value = std::get<double>(kv2.value);
            return left_value <= right_value;
        }
    }
    return false;
}

bool kvazzvalue_less_than(KvazzValue &kv1, KvazzValue &kv2) {
    auto left_type = kv1.type;
    auto right_type = kv2.type;
    if (left_type == KvazzType::Int) {
        auto left_value = std::get<int>(kv1.value);
        if(right_type == KvazzType::Int) {
            auto right_value = std::get<int>(kv2.value);
            return left_value < right_value;
        }
        else {
            auto right_value = std::get<double>(kv2.value);
            return left_value < right_value;
        }
    }
    else {
        auto left_value = std::get<double>(kv1.value);
        if(right_type == KvazzType::Int) {
            auto right_value = std::get<int>(kv2.value);
            return left_value < right_value;
        }
        else {
            auto right_value = std::get<double>(kv2.value);
            return left_value < right_value;
        }
    }
    return false;
}

bool kvazzvalue_greater_equals(KvazzValue &kv1, KvazzValue &kv2) {
    auto left_type = kv1.type;
    auto right_type = kv2.type;
    if (left_type == KvazzType::Int) {
        auto left_value = std::get<int>(kv1.value);
        if(right_type == KvazzType::Int) {
            auto right_value = std::get<int>(kv2.value);
            return left_value >= right_value;
        }
        else {
            auto right_value = std::get<double>(kv2.value);
            return left_value >= right_value;
        }
    }
    else {
        auto left_value = std::get<double>(kv1.value);
        if(right_type == KvazzType::Int) {
            auto right_value = std::get<int>(kv2.value);
            return left_value >= right_value;
        }
        else {
            auto right_value = std::get<double>(kv2.value);
            return left_value >= right_value;
        }
    }
    return false;
}

bool kvazzvalue_greater_than(KvazzValue &kv1, KvazzValue &kv2) {
    auto left_type = kv1.type;
    auto right_type = kv2.type;
    if (left_type == KvazzType::Int) {
        auto left_value = std::get<int>(kv1.value);
        if(right_type == KvazzType::Int) {
            auto right_value = std::get<int>(kv2.value);
            return left_value > right_value;
        }
        else {
            auto right_value = std::get<double>(kv2.value);
            return left_value > right_value;
        }
    }
    else {
        auto left_value = std::get<double>(kv1.value);
        if(right_type == KvazzType::Int) {
            auto right_value = std::get<int>(kv2.value);
            return left_value > right_value;
        }
        else {
            auto right_value = std::get<double>(kv2.value);
            return left_value > right_value;
        }
    }
    return false;
}

bool kvazzvalue_equals(KvazzValue &kv1, KvazzValue &kv2) {
    auto left_type = kv1.type;
    auto right_type = kv2.type;

    if (left_type != right_type) {
        if (left_type == KvazzType::Int && right_type == KvazzType::Real) {
            auto left_value = std::get<int>(kv1.value);
            auto right_value = std::get<double>(kv2.value);
            return left_value == right_value;
        }
        else if (left_type == KvazzType::Real && right_type == KvazzType::Int) {
            auto left_value = std::get<double>(kv1.value);
            auto right_value = std::get<int>(kv2.value);
            return left_value == right_value;
        }
        return false;
    }

    // from here on out left_type == right_type
    if (left_type == KvazzType::String) {
        auto left_value = std::get<string>(kv1.value);
        auto right_value = std::get<string>(kv2.value);
        return left_value == right_value;
    }

    if (left_type == KvazzType::Hevec) {
        auto vec1 = std::get<vector<KvazzValue>>(kv1.value);
        auto vec2 = std::get<vector<KvazzValue>>(kv2.value);
        if(vec1.size() != vec2.size()) {
            return false;
        }

        for (int i = 0; i < vec1.size(); ++i) {
            if (!kvazzvalue_equals(vec1[i], vec2[i])) {
                return false;
            }
        }
        return true;
    }
    // last compare case for now. Not sure if I want to be able to compare functions or
    // built ins... comparing AST might be interesting. Another compare operator x =@= y
    // that checks whether or not x and y are the same object might be useful but hard to
    // implement.
    return false;
}

KvazzResult Kvazzvalue_unary_minus(KvazzValue &kv) {
    if (kv.type == KvazzType::Int) {
        return make_good_result( -std::get<int>(kv.value));
    }
    if(kv.type == KvazzType::Real) {
       return make_good_result( -std::get<double>(kv.value));
    }
    return ERROR_NO_VALUE;
}

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

// maybe this should be part of the interpreter class
shared_ptr<Env> global_env = std::make_shared<Env> (
    nullptr, 
    unordered_map<string, EnvEntry>()
);

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


/**
 *  Calls the passed KvazzFunction with the specified args
 */
KvazzResult call_function(
        KvazzFunction &fn,
        vector<KvazzValue> &arg_values,
        /* shared_ptr<Env> env,    // unused for now since all functions are executed with global scope */
        Interpreter &interpreter) {

    unordered_map<string, EnvEntry> function_env_map;
    auto argv_index = 0;
    for (auto arg_name : fn.args) {
        function_env_map.emplace(arg_name, EnvEntry {EnvResultType::Value, arg_values[argv_index]});
        ++argv_index;
    }
    auto function_env = std::make_shared<Env> ( global_env, function_env_map );
    return fn.body->eval(interpreter, function_env);
}

KvazzResult call_builtin_function (
        int builtin_fn_id,
        vector<KvazzValue> &arg_values,
        shared_ptr<Env> env) {

    switch (builtin_fn_id) {
        case _print:
        {}

        case _lengthof:
        {}

        case _hevec:
        {}

        default:{}
    }
    std::cerr << "Tried calling unknown built-in function with id: " << builtin_fn_id << " \n";
    return ERROR_NO_VALUE;
}

/*
*  AST-eval Interpreter class methods
*/
KvazzResult Interpreter::eval(Program &node, shared_ptr<Env> env) {
    for (auto nd : node.children()) {
        nd->eval(*this, env);
    }

    auto main_found = env->table.find("main");
    if (main_found != env->table.end()) {
        auto main_method = std::get<KvazzFunction>(main_found->second.contents);
        vector<KvazzValue> args;
        call_function(main_method, args, this);
    }
    return GOOD_NO_VALUE;
}

KvazzResult Interpreter::eval(Block &node, shared_ptr<Env> env) {
    auto local_env = std::make_shared<Env>(env, unordered_map<string, EnvEntry>());
    for (auto nd : node.children()) {
        auto result = nd->eval(*this, local_env);
        if (result.flag == KvazzFlag::Return)
            return result;
    }
    return GOOD_NO_VALUE;
}

KvazzResult Interpreter::eval(AssignOp &node, shared_ptr<Env> env) {

    // set the lvalue flag so that the next eval will return an lvalue
    this->lvalue_flag = true;
    LValue lvalue = std::get<LValue>(node.lvalue->eval(*this, env).kvazz_value.value);
    KvazzValue new_value = node.expr_node->eval(*this, env).kvazz_value;

    if (node.op_type != AssignOpType::assign) {
        KvazzValue old_value = node.lvalue->eval(*this, env).kvazz_value;
        switch(node.op_type) {
            case AssignOpType::minus:
            {
                new_value = kvazzvalue_minus(old_value, new_value).kvazz_value;
            }
            case AssignOpType::divide:
            {
                new_value = kvazzvalue_divide(old_value, new_value).kvazz_value;
            }
            case AssignOpType::multiply:
            {
                new_value = kvazzvalue_multiply(old_value, new_value).kvazz_value;
            }
            case AssignOpType::modulo:
            {
                new_value = kvazzvalue_modulo(old_value, new_value).kvazz_value;
            }
            default:
            {}
        }
    }

    if (lvalue.type == KvazzType::Hevec) {
        // not sure if this actually modifies the vector due to c++ copies
        // may need to change lvalue to pass a reference or pointer
        auto the_vector = std::get<vector<KvazzValue>>(lvalue.lvalue);
        auto index = std::get<int>(lvalue.index);
        the_vector[index] = new_value;
    }
    else {
        auto the_env = std::get<shared_ptr<Env>>(lvalue.lvalue);
        auto index = std::get<string>(lvalue.index);
        the_env->table[index] = new_value;
    }

    return GOOD_NO_VALUE;
}

KvazzResult Interpreter::eval(Declare &node, shared_ptr<Env> env) {
    auto result = env->table.find(node.identifier);
    if (result == env->table.end()) {
        auto kv = node.expr_node->eval(*this, env).kvazz_value;
        env->table[node.identifier] = EnvEntry { EnvResultType::Value, std::move(kv) };
        return GOOD_NO_VALUE;
    }
    std::cerr << "Identifier \'" << node.identifier << "\' already defined in this scope\n";
    return ERROR_NO_VALUE;
}

KvazzResult Interpreter::eval(FunctionDeclare &node, shared_ptr<Env> env) {
    auto result = env->table.find(node.identifier);
    if (result == env->table.end()) {
        env->table[node.identifier] = EnvEntry {
            EnvResultType::Function,
            KvazzFunction {
                node.identifier,
                std::move(node.args),
                node.body
            }
        };
        return GOOD_NO_VALUE;
    }
    std::cerr << "Identifier \'" << node.identifier << "\' already defined in this scope\n";
    return ERROR_NO_VALUE;
}

KvazzResult Interpreter::eval(Return &node, shared_ptr<Env> env) {
    auto expression_result = node.expr_node->eval(*this, env);
    expression_result.flag = KvazzFlag::Return;
    return expression_result;
}

KvazzResult Interpreter::eval(IfThen &node, shared_ptr<Env> env) {
    if (truthy_test(node.condition->eval(*this, env))) {
        return node.body->eval(*this, env);
    }
    return GOOD_NO_VALUE;
}

KvazzResult Interpreter::eval(IfElse &node, shared_ptr<Env> env) {
    if (truthy_test(node.condition->eval(*this, env))) {
        return node.then_body->eval(*this, env);
    }
    else {
        return node.else_body->eval(*this, env);
    }
}

KvazzResult Interpreter::eval(While &node, shared_ptr<Env> env) {
    while (truthy_test(node.condition->eval(*this, env))) {
        auto maybe_result = node.body->eval(*this, env);
        if(maybe_result.flag == KvazzFlag::Return)
            return maybe_result;
    }
    return GOOD_NO_VALUE;
}

KvazzResult Interpreter::eval(BinaryOp &node, shared_ptr<Env> env) {
    auto left = node.left_expr->eval(*this, env);
    auto right = node.right_expr->eval(*this, env);

    if (left.flag == KvazzFlag::Error || right.flag == KvazzFlag::Error) {
        // might should do a system exit here... not sure, better error handling will come
        return KvazzResult { NOTHING, KvazzFlag::Error };
    }

    // these should mostly all be broken out into their own functions once the logic has been figured out
    // since there's potential for a lot of code to be in here.
    // I want operators to have meaning on various types e.g. '+' is both arithemetic addition as well
    // as string and maybe array concat as well. So lots of type checking code will be involved in
    // operator code.
    KvazzResult result;
    switch(node.op_type) {
        case BinaryOpType::pipe:
        {
            // defined on all types through truthy/falsey -ness
            // logical OR
            result = make_good_result(truthy_test(left) || truthy_test(right));
        }
        case BinaryOpType::amper:
        {
            // defined on all types through truthy/falsey -ness
            // logical AND
            result = make_good_result(truthy_test(left) && truthy_test(right));
        }
        case BinaryOpType::equals:
        {
            // defined on all types
            result = make_good_result(kvazzvalue_equals(left.kvazz_value, right.kvazz_value));
        }
        case BinaryOpType::not_equals:
        {
            // defined on all types
            result = make_good_result(!kvazzvalue_equals(left.kvazz_value, right.kvazz_value));
        }
        case BinaryOpType::less_equals:
        {
            // defined for numeric types
            result = make_good_result(kvazzvalue_less_equals(left.kvazz_value, right.kvazz_value));
        }
        case BinaryOpType::greater_equals:
        {
            // defined for numeric types
            result = make_good_result(kvazzvalue_greater_equals(left.kvazz_value, right.kvazz_value));
        }
        case BinaryOpType::less_than:
        {
            // defined for numeric types
            result = make_good_result(kvazzvalue_less_than(left.kvazz_value, right.kvazz_value));
        }
        case BinaryOpType::greater_than:
        {
            // defined for numeric types
            result = make_good_result(kvazzvalue_greater_than(left.kvazz_value, right.kvazz_value));
        }
        case BinaryOpType::plus:
        {
            // defined for numeric types (as arithmetic plus), strings, and vectors (as concat)
            result = kvazzvalue_plus(left.kvazz_value, right.kvazz_value);
        }
        case BinaryOpType::minus:
        {
            // defined for numeric types
            result = kvazzvalue_minus(left.kvazz_value, right.kvazz_value);
        }
        case BinaryOpType::multiply:
        {
            // defined for numeric types
            result = kvazzvalue_multiply(left.kvazz_value, right.kvazz_value);
        }
        case BinaryOpType::divide:
        {
            // defined for numeric types
            result = kvazzvalue_divide(left.kvazz_value, right.kvazz_value);
        }
        case BinaryOpType::modulo:
        {
            // defined on integers only
            result = kvazzvalue_modulo(left.kvazz_value, right.kvazz_value);
        }
    }
    return result;
}

KvazzResult Interpreter::eval(UnaryOp &node, shared_ptr<Env> env) {
    auto right = node.right_expr->eval(*this, env);
    if (right.flag == KvazzFlag::Error) {
        // might should do a system exit here... not sure, better error handling will come
        return KvazzResult { NOTHING, KvazzFlag::Error };
    }
    KvazzResult result;
    switch(node.op_type) {
        case UnaryOpType::bang:
        {
            // defined on all valid types
            result = make_good_result(truthy_test(right));
        }
        case UnaryOpType::minus:
        {
            // defined on numeric types
            result = Kvazzvalue_unary_minus(right.kvazz_value);
        }
    }
    return result;

}

KvazzResult Interpreter::eval(FunctionCall &node, shared_ptr<Env> env) {
    auto callee_expr_result = node.callee->eval(*this, env);
    if (callee_expr_result.flag != KvazzFlag::Error) {
        vector<KvazzValue> arg_values;
        for (auto &expr_arg : node.expr_args)
            arg_values.push_back(expr_arg->eval(*this, env).kvazz_value);

        if (callee_expr_result.kvazz_value.type == KvazzType::Function) {
            auto function = std::get<KvazzFunction>(callee_expr_result.kvazz_value.value);
            return call_function(function, arg_values, this);
        }
        if (callee_expr_result.kvazz_value.type == KvazzType::Builtin) {
            auto builtin_function_id = std::get<int>(callee_expr_result.kvazz_value.value);
            return call_builtin_function(builtin_function_id, arg_values, global_env);
        }
    }

    // TODO: handle built-in function case
    return ERROR_NO_VALUE;
}

KvazzResult Interpreter::eval(Access &node, shared_ptr<Env> env) {
    // Take note that the lvalue flag was set, and then turn it off so that subsequent eval
    // calls don't return an lvalue, e.g. vector[index1][index2]
    auto was_lvalue_flag_set = this->lvalue_flag;
    this->lvalue_flag = false;

    auto left_expr_result = node.left_expr->eval(*this, env);
    auto index_expr_result = node.index_expr->eval(*this, env);
    if (index_expr_result.kvazz_value.type == KvazzType::Int) {
        if(left_expr_result.kvazz_value.type == KvazzType::Hevec) {
            vector<KvazzValue> the_vec = std::get<vector<KvazzValue>>(left_expr_result.kvazz_value.value);
            auto index = std::get<int>(index_expr_result.kvazz_value.value);
            if (index < the_vec.size()) {
                if (was_lvalue_flag_set) {
                    // make an lvalue and return that instead
                    LValue lvalue { KvazzType::Hevec, the_vec, index };
                    return make_good_result(lvalue);
                } else {
                    return make_good_result(the_vec[index]);
                }
            }
            else {
                std::cerr << "Index " << index << " out of bounds for vector";
            }
        }
        if(left_expr_result.kvazz_value.type == KvazzType::String) {
            string the_string = std::get<string>(left_expr_result.kvazz_value.value);
            auto index = std::get<int>(index_expr_result.kvazz_value.value);
            if (index < the_string.length()) {
                if (was_lvalue_flag_set) {
                    // make an lvalue and return that instead
                    std::cerr << "Strings are immutable, assigning to index is not supported.\n";
                }
                else {
                    return make_good_result(the_string.substr(index, 1));
                }
            }
            else {
                std::cerr << "Index " << index << " out of bounds for \"" << the_string << "\"";
            }
        }
    }
    // when dictionaries are added, other types of index values may be valid too

    return ERROR_NO_VALUE;
}

KvazzResult Interpreter::eval(VariableLookup &node, shared_ptr<Env> env) {
    auto was_lvalue_flag_set = this->lvalue_flag;
    this->lvalue_flag = false;

    auto env_to_use = node.sigil ? global_env : env;
    auto lookup_result = lookup(node.identifier, env_to_use);
    if (lookup_result.result.type == EnvResultType::Value) {
        if (was_lvalue_flag_set) {
            LValue lvalue {KvazzType::Nothing, lookup_result.env, node.identifier};
            return make_good_result(lvalue);
        }
        return make_good_result(std::get<KvazzValue>(lookup_result.result.contents));
    }
    if (lookup_result.result.type == EnvResultType::Function) {
        if (was_lvalue_flag_set) {
            // Maybe in the future this will change
            std::cerr << "Functions cannot be reassigned.\n";
        }
        else {
            return make_good_result(std::get<KvazzFunction>(lookup_result.result.contents));
        }
    }
    if (lookup_result.result.type == EnvResultType::Builtin) {
        if (was_lvalue_flag_set) {
            // Maybe in the future this will change
            std::cerr << "Built-in functions cannot be reassigned.\n";
        }
        else {
            return KvazzResult {
                std::get<KvazzValue>(lookup_result.result.contents),
                KvazzFlag::Good
            };
        }
    }
    return ERROR_NO_VALUE;
}

KvazzResult Interpreter::eval(IntLiteral &node, shared_ptr<Env> env) {
    return make_good_result(node.literal_value);
}

KvazzResult Interpreter::eval(BoolLiteral &node, shared_ptr<Env> env) {
    return make_good_result(node.literal_value);
}

KvazzResult Interpreter::eval(RealLiteral &node, shared_ptr<Env> env) {
    return make_good_result(node.literal_value);
}
KvazzResult Interpreter::eval(StringLiteral &node, shared_ptr<Env> env) {
    return make_good_result(node.literal_value);
}
KvazzResult Interpreter::eval(VectorLiteral &node, shared_ptr<Env> env) {
    vector<KvazzValue> results;
    for(auto nd : node.contents) {
        auto result = nd->eval(*this, env);
        if (result.flag == KvazzFlag::Good) {
            results.push_back(result.kvazz_value);
        }
    }
    return make_good_result(std::move(results));
}
