#pragma once
#include "ast.h"
#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <unordered_map>

/*
*  AST interpreter type declarations
*/

enum class KvazzFlag {
    Good, Error, Return
};

enum class KvazzType {
    Nothing, Builtin, Int, Real, Bool, String, Hevec
};

enum class EnvResultType {
    Value, Function, Builtin
};

struct KvazzValue
{
    KvazzType type;
    std::variant<int, double, bool, std::string, std::vector<KvazzValue>> value;
};

const KvazzValue NOTHING {
    KvazzType::Nothing,
    0
};

struct KvazzFunction
{
    std::string               name;
    std::vector<std::string>  args;
    std::shared_ptr<BaseNode> body;
};

struct KvazzResult
{
    KvazzValue return_value;
    KvazzFlag  flag;
};

struct EnvEntry 
{
    EnvResultType type;
    std::variant<KvazzValue, KvazzFunction> contents;
};


struct Env 
{
    std::shared_ptr<Env> parent;
    std::unordered_map<std::string, EnvEntry> table;
};

struct LookupResult
{
    EnvEntry result,
    std::shared_ptr<Env> env
};

const KvazzResult GOOD_NO_RETURN {
    NOTHING,
    KvazzFlag::Good
};

bool is_gnr(KvazzResult kr);

LookupResult lookup(std::string identifier, std::shared_ptr<Env> env);


/*
*  AST-eval functions
*/
KvazzResult eval_program(std::shared_ptr<BaseNode>);
KvazzResult eval_node(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult call_function(KvazzFunction fn, std::vector<KvazzValue>, std::shared_ptr<Env> env);
KvazzResult eval_fn_call(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_bin_op(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_un_op(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_declare(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_assignop(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_fn_declare(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);

// needs a different return value
KvazzResult eval_lvalue(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);


KvazzResult eval_block(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_return(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_while(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_if_else(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_if_then(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_var_lookup(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_access(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_vector_literal(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_bool_literal(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_int_literal(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_real_literal(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);
KvazzResult eval_string_literal(std::shared_ptr<BaseNode>, std::shared_ptr<Env> env);













