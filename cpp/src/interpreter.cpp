#include "ast.h"
#include "asteval.h"
#include "interpreter.h"
#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <sstream>

using std::unordered_map; 
using std::shared_ptr;
using std::vector;
using std::string;

/////////////////////////////////////////////////////////////////////////////////////
// USEFUL CONSTANTS
//
/////////////////////////////////////////////////////////////////////////////////////
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

// enum-to-string function for KvazzType
string kvazztype_as_string(KvazzType t) {
    // could probably be rewritten into table lookup
    switch(t) {
        case KvazzType::Bool:
        {
           return "Bool";
        }
        case KvazzType::Nothing:
        {
            return "Nothing";
        }
        case KvazzType::Int:
        {
            return "Int";
        }
        case KvazzType::Real:
        {
            return "Real";
        }
        case KvazzType::String:
        {
            return "String";
        }
        case KvazzType::Hevec:
        {
            return "Hevec";
        }
        case KvazzType::LValue:
        {
            return "LValue";
        }
        case KvazzType::Function:
        {
            return "Function";
        }
        case KvazzType::Builtin:
        {
            return "Builtin";
        }
    }
    return "";
}

string kvazzvalue_as_string(KvazzValue &item) {
    std::stringstream result;

    switch(item.type) {
        case KvazzType::Bool:
        {
            result << (std::get<bool>(item.value) ? "true" : "false");
            break;
        }
        case KvazzType::Nothing:
        {
            result << "Nothing";
            break;
        }
        case KvazzType::Int:
        {
            result << std::get<int>(item.value);
            break;
        }
        case KvazzType::Real:
        {
            result << std::get<double>(item.value);
            break;
        }
        case KvazzType::String:
        {
            result << std::get<string>(item.value);
            break;
        }
        case KvazzType::Hevec:
        {
            auto v = std::get<vector<KvazzValue>>(item.value);
            result << "[";
            for(int i = 0; i < v.size(); ++i) {
                result << kvazzvalue_as_string(v[i]);
                if (i != v.size() - 1) {
                    result << ", ";
                }
            }
            result << "]";
            break;
        }
        case KvazzType::LValue:
        {
            // shouldn't really happen
            result << "LValue";
            break;
        }
        case KvazzType::Function:
        {
            KvazzFunction kf = std::get<KvazzFunction>(item.value);
            result << "Function<" << kf.name << "(";
            for(int i = 0; i < kf.args.size(); ++i) {
                result << kf.args[i];
                if (i != kf.args.size() - 1) {
                    result << ", ";
                }
            }
            result << ")>";
            break;
        }
        case KvazzType::Builtin:
        {
            result << "Builtin<" << built_in_function_as_string(std::get<int>(item.value)) << ">";
            break;
        }
    }
    return result.str();
}

/////////////////////////////////////////////////////////////////////////////////////
// MAKE GOOD RESULT
//
/////////////////////////////////////////////////////////////////////////////////////

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
    switch(value.type) {
        case KvazzType::Bool:
            {
                return make_good_result(std::get<bool>(value.value));
            }
        case KvazzType::Int:
            {
                return make_good_result(std::get<int>(value.value));
            }
        case KvazzType::String:
            {
                return make_good_result(std::get<string>(value.value));
            }
        case KvazzType::Hevec:
            {
                return make_good_result(std::get<vector<KvazzValue>>(value.value));
            }
        case KvazzType::Real:
            {
                return make_good_result(std::get<double>(value.value));
            }
        case KvazzType::Nothing:
        case KvazzType::Builtin:
        case KvazzType::LValue:
        default:
            {
                std::cerr << "Attempted to make_good_value on invalid KvazzType.\n";
            }
    }
    return ERROR_NO_VALUE;
}

/* 
*   Utility Functions
*/

// UNUSED
bool is_gnr(KvazzResult &kr) {
    return kr.flag == KvazzFlag::Good && kr.kvazz_value.type == KvazzType::Nothing;
}

// UNUSED
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
                return vec_value.size() < 0 ? false : true;
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

/////////////////////////////////////////////////////////////////////////////////////
// KVAZZVALUE OPERATORS
//
/////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////
// BUILT-IN FUNCTIONS
//
/////////////////////////////////////////////////////////////////////////////////////

KvazzResult execute_built_in_print(vector<KvazzValue> &args) {
    // emulate python's print for now to keep both interpreters acting the same.
    // Other print functions can act different.
    int i = 0;
    while (i < args.size() - 1) {
        auto &item = args[i];
        std::cout << kvazzvalue_as_string(item);
        std::cout << " ";
        ++i;
    }
    auto &last = args[i];
    std::cout << kvazzvalue_as_string(last);
    std::cout << "\n";
    return GOOD_NO_VALUE;
}

KvazzResult execute_built_in_lengthof(vector<KvazzValue> &args) {
    // number of args must equal 1
    if (args.size() != 1) {
        std::cerr
            << "Wrong number of arguments passed to built-in function lengthof."
            << "Expected: 1, Received: " << args.size() << "\n";
    }
    else {
        auto &arg = args[0];

        // arg must be some non-scalar type (only vector or string for now)
        if (arg.type == KvazzType::Hevec) {
            vector<KvazzValue> the_vector = std::get<vector<KvazzValue>>(arg.value);
            int length = the_vector.size();
            return make_good_result(length);
        }
        if (arg.type == KvazzType::String) {
            string the_string = std::get<string>(arg.value);
            int length = the_string.size();
            return make_good_result(length);
        }
        std::cerr
            << "Unsupported type for lengthof. Expected non-scalar type, Received: "
            << kvazztype_as_string(arg.type) << "\n";
    }
    return ERROR_NO_VALUE;
}

KvazzResult execute_built_in_hevec(vector<KvazzValue> &args) {
    if (args.size() > 0 && args.size() < 3) {
        auto length_kvalue = args[0];
        // length must be of type int
        if (length_kvalue.type != KvazzType::Int) {
            std::cerr
                << "Invalid type given for hevec length. Expected: Int, Received: "
                << kvazztype_as_string(length_kvalue.type) << "\n";
            goto _HEVEC_ERROR;
        }
        int length = std::get<int>(length_kvalue.value);
        auto default_kvalue = args.size() == 2 ? args[1] : NOTHING;
        vector<KvazzValue> new_hevec(length);
        for (int i = 0; i < length; ++i) {
            new_hevec[i] = default_kvalue;
        }
        return make_good_result(new_hevec);
    }
    else {
        std::cerr
            << "Invalid number of args passed to built-in-function hevec. "
            << "Expected: 1 or 2, Received: " << args.size() << "\n";
    }
    _HEVEC_ERROR:
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

string built_in_function_as_string(int id) {
    switch (id) {
        case _print:
            return "print";
        case _lengthof:
            return "lengthof";
        case _hevec:
            return "hevec";
    }
    return "INVALID_BUILTIN";
}

/////////////////////////////////////////////////////////////////////////////////////
// INTERPRETER
//
/////////////////////////////////////////////////////////////////////////////////////

// maybe this should be part of the interpreter class

shared_ptr<Env> null_env = shared_ptr<Env>(nullptr);
shared_ptr<Env> global_env = std::make_shared<Env>(
    std::move(null_env),
    unordered_map<string, EnvEntry>{}
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
    auto function_env = std::make_shared<Env> ( std::move(global_env), function_env_map );
    return fn.body->eval(interpreter, function_env);
}

KvazzResult call_builtin_function (
        int builtin_fn_id,
        vector<KvazzValue> &arg_values) {

    switch (builtin_fn_id) {
        case _print:
            return execute_built_in_print(arg_values);
        case _lengthof:
            return execute_built_in_lengthof(arg_values);
        case _hevec:
            return execute_built_in_hevec(arg_values);
        default:{}
    }
    std::cerr << "Tried calling unknown built-in function with id: " << builtin_fn_id << " \n";
    return ERROR_NO_VALUE;
}

/*
*  AST-eval Interpreter class methods
*/
KvazzResult Interpreter::eval(BaseNode *node, shared_ptr<Env> env) {
    std::cerr << "Eval not implemented for BaseNode\n";
    return ERROR_NO_VALUE;
}

KvazzResult Interpreter::eval(Program *node, shared_ptr<Env> env) {
    for (auto nd : node->children()) {
        nd->eval(*this, env);
    }

    auto main_found = env->table.find("main");
    if (main_found != env->table.end()) {
        auto main_method = std::get<KvazzFunction>(main_found->second.contents);
        vector<KvazzValue> args;
        call_function(main_method, args, *this);
    }
    return GOOD_NO_VALUE;
}

KvazzResult Interpreter::eval(Block *node, shared_ptr<Env> env) {
    auto local_env = std::make_shared<Env>(std::move(env), unordered_map<string, EnvEntry>{});
    for (auto nd : node->children()) {
        auto result = nd->eval(*this, local_env);
        if (result.flag == KvazzFlag::Return)
            return result;
    }
    return GOOD_NO_VALUE;
}

KvazzResult Interpreter::eval(AssignOp *node, shared_ptr<Env> env) {

    // set the lvalue flag so that the next eval will return an lvalue
    this->lvalue_flag = true;
    LValue lvalue = std::get<LValue>(node->lvalue->eval(*this, env).kvazz_value.value);
    KvazzValue new_value = node->expr_node->eval(*this, env).kvazz_value;

    if (node->op_type != AssignOpType::assign) {
        KvazzValue old_value = node->lvalue->eval(*this, env).kvazz_value;
        switch(node->op_type) {
            case AssignOpType::minus:
            {
                new_value = kvazzvalue_minus(old_value, new_value).kvazz_value;
                break;
            }
            case AssignOpType::divide:
            {
                new_value = kvazzvalue_divide(old_value, new_value).kvazz_value;
                break;
            }
            case AssignOpType::multiply:
            {
                new_value = kvazzvalue_multiply(old_value, new_value).kvazz_value;
                break;
            }
            case AssignOpType::modulo:
            {
                new_value = kvazzvalue_modulo(old_value, new_value).kvazz_value;
                break;
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
        the_vector.at(index) = std::move(new_value);
    }
    else {
        auto the_env = std::get<shared_ptr<Env>>(lvalue.lvalue);
        auto index = std::get<string>(lvalue.index);
        the_env->table[index] = EnvEntry { EnvResultType::Value, new_value };
    }

    return GOOD_NO_VALUE;
}

KvazzResult Interpreter::eval(Declare *node, shared_ptr<Env> env) {
    auto result = env->table.find(node->identifier);
    if (result == env->table.end()) {
        auto kv = node->expr_node->eval(*this, env).kvazz_value;
        env->table[node->identifier] = EnvEntry { EnvResultType::Value, std::move(kv) };
        return GOOD_NO_VALUE;
    }
    std::cerr << "Identifier \'" << node->identifier << "\' already defined in this scope\n";
    return ERROR_NO_VALUE;
}

KvazzResult Interpreter::eval(FunctionDeclare *node, shared_ptr<Env> env) {
    auto result = env->table.find(node->identifier);
    if (result == env->table.end()) {
        env->table[node->identifier] = EnvEntry {
            EnvResultType::Function,
            KvazzFunction {
                node->identifier,
                std::move(node->args),
                node->body
            }
        };
        return GOOD_NO_VALUE;
    }
    std::cerr << "Identifier \'" << node->identifier << "\' already defined in this scope\n";
    return ERROR_NO_VALUE;
}

KvazzResult Interpreter::eval(Return *node, shared_ptr<Env> env) {
    auto expression_result = node->expr_node->eval(*this, env);
    expression_result.flag = KvazzFlag::Return;
    return expression_result;
}

KvazzResult Interpreter::eval(IfThen *node, shared_ptr<Env> env) {
    if (truthy_test(node->condition->eval(*this, env))) {
        return node->body->eval(*this, env);
    }
    return GOOD_NO_VALUE;
}

KvazzResult Interpreter::eval(IfElse *node, shared_ptr<Env> env) {
    if (truthy_test(node->condition->eval(*this, env))) {
        return node->then_body->eval(*this, env);
    }
    else {
        return node->else_body->eval(*this, env);
    }
}

KvazzResult Interpreter::eval(While *node, shared_ptr<Env> env) {
    while (truthy_test(node->condition->eval(*this, env))) {
        auto maybe_result = node->body->eval(*this, env);
        if(maybe_result.flag == KvazzFlag::Return)
            return maybe_result;
    }
    return GOOD_NO_VALUE;
}

KvazzResult Interpreter::eval(BinaryOp *node, shared_ptr<Env> env) {
    auto left = node->left_expr->eval(*this, env);
    auto right = node->right_expr->eval(*this, env);

    if (left.flag == KvazzFlag::Error || right.flag == KvazzFlag::Error) {
        // might should do a system exit here... not sure, better error handling will come
        return KvazzResult { NOTHING, KvazzFlag::Error };
    }

    // these should mostly all be broken out into their own functions once the logic has been figured out
    // since there's potential for a lot of code to be in here.
    // I want operators to have meaning on various types e.g. '+' is both arithemetic addition as well
    // as string and maybe array concat as well. So lots of type checking code will be involved in
    // operator code.
    switch(node->op_type) {
        case BinaryOpType::pipe:
        {
            // defined on all types through truthy/falsey -ness
            // logical OR
            return make_good_result(truthy_test(left) || truthy_test(right));
        }
        case BinaryOpType::amper:
        {
            // defined on all types through truthy/falsey -ness
            // logical AND
            return make_good_result(truthy_test(left) && truthy_test(right));
        }
        case BinaryOpType::equals:
        {
            // defined on all types
            return make_good_result(kvazzvalue_equals(left.kvazz_value, right.kvazz_value));
        }
        case BinaryOpType::not_equals:
        {
            // defined on all types
            return make_good_result(!kvazzvalue_equals(left.kvazz_value, right.kvazz_value));
        }
        case BinaryOpType::less_equals:
        {
            // defined for numeric types
            return make_good_result(kvazzvalue_less_equals(left.kvazz_value, right.kvazz_value));
        }
        case BinaryOpType::greater_equals:
        {
            // defined for numeric types
            return make_good_result(kvazzvalue_greater_equals(left.kvazz_value, right.kvazz_value));
        }
        case BinaryOpType::less_than:
        {
            // defined for numeric types
            return make_good_result(kvazzvalue_less_than(left.kvazz_value, right.kvazz_value));
        }
        case BinaryOpType::greater_than:
        {
            // defined for numeric types
            return make_good_result(kvazzvalue_greater_than(left.kvazz_value, right.kvazz_value));
        }
        case BinaryOpType::plus:
        {
            // defined for numeric types (as arithmetic plus), strings, and vectors (as concat)
            return kvazzvalue_plus(left.kvazz_value, right.kvazz_value);
        }
        case BinaryOpType::minus:
        {
            // defined for numeric types
            return kvazzvalue_minus(left.kvazz_value, right.kvazz_value);
        }
        case BinaryOpType::multiply:
        {
            // defined for numeric types
            return kvazzvalue_multiply(left.kvazz_value, right.kvazz_value);
        }
        case BinaryOpType::divide:
        {
            // defined for numeric types
            return kvazzvalue_divide(left.kvazz_value, right.kvazz_value);
        }
        case BinaryOpType::modulo:
        {
            // defined on integers only
            return kvazzvalue_modulo(left.kvazz_value, right.kvazz_value);
        }
    }
    return ERROR_NO_VALUE;
}

KvazzResult Interpreter::eval(UnaryOp *node, shared_ptr<Env> env) {
    auto right = node->right_expr->eval(*this, env);
    if (right.flag == KvazzFlag::Error) {
        // might should do a system exit here... not sure, better error handling will come
        return KvazzResult { NOTHING, KvazzFlag::Error };
    }
    switch(node->op_type) {
        case UnaryOpType::bang:
        {
            // defined on all valid types
            return make_good_result(truthy_test(right));
        }
        case UnaryOpType::minus:
        {
            // defined on numeric types
            return Kvazzvalue_unary_minus(right.kvazz_value);
        }
    }
    return ERROR_NO_VALUE;

}

KvazzResult Interpreter::eval(FunctionCall *node, shared_ptr<Env> env) {
    auto callee_expr_result = node->callee->eval(*this, env);
    if (callee_expr_result.flag != KvazzFlag::Error) {
        vector<KvazzValue> arg_values;
        for (auto &expr_arg : node->expr_args)
            arg_values.push_back(expr_arg->eval(*this, env).kvazz_value);

        if (callee_expr_result.kvazz_value.type == KvazzType::Function) {
            auto function = std::get<KvazzFunction>(callee_expr_result.kvazz_value.value);
            return call_function(function, arg_values, *this);
        }
        if (callee_expr_result.kvazz_value.type == KvazzType::Builtin) {
            auto builtin_function_id = std::get<int>(callee_expr_result.kvazz_value.value);
            return call_builtin_function(builtin_function_id, arg_values);
        }
    }

    return ERROR_NO_VALUE;
}

KvazzResult Interpreter::eval(Access *node, shared_ptr<Env> env) {
    // Take note that the lvalue flag was set, and then turn it off so that subsequent eval
    // calls don't return an lvalue, e.g. vector[index1][index2]
    auto was_lvalue_flag_set = this->lvalue_flag;
    this->lvalue_flag = false;

    auto left_expr_result = node->left_expr->eval(*this, env);
    auto index_expr_result = node->index_expr->eval(*this, env);
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

KvazzResult Interpreter::eval(VariableLookup *node, shared_ptr<Env> env) {
    auto was_lvalue_flag_set = this->lvalue_flag;
    this->lvalue_flag = false;

    auto env_to_use = node->sigil ? global_env : env;
    auto lookup_result = lookup(node->identifier, env_to_use);
    if (lookup_result.result.type == EnvResultType::Value) {
        if (was_lvalue_flag_set) {
            LValue lvalue {KvazzType::Nothing, lookup_result.env, node->identifier};
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

KvazzResult Interpreter::eval(IntLiteral *node, shared_ptr<Env> env) {
    return make_good_result(node->literal_value);
}

KvazzResult Interpreter::eval(BoolLiteral *node, shared_ptr<Env> env) {
    return make_good_result(node->literal_value);
}

KvazzResult Interpreter::eval(RealLiteral *node, shared_ptr<Env> env) {
    return make_good_result(node->literal_value);
}
KvazzResult Interpreter::eval(StringLiteral *node, shared_ptr<Env> env) {
    return make_good_result(node->literal_value);
}
KvazzResult Interpreter::eval(VectorLiteral *node, shared_ptr<Env> env) {
    vector<KvazzValue> results;
    for(auto nd : node->contents) {
        auto result = nd->eval(*this, env);
        if (result.flag == KvazzFlag::Good) {
            results.push_back(result.kvazz_value);
        }
    }
    return make_good_result(std::move(results));
}

// Entry point method
void run_ast_interpreter(std::shared_ptr<BaseNode> ast) {
    Interpreter i;
    auto result = ast->eval(i, global_env);
    // Todo: print something about the result?
}
