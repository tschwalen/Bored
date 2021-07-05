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
    Nothing, Int, Real, Bool, String, Hevec
};

struct KvazzValue
{
    KvazzType type;
    std::variant<int, double, bool, std::string, std::vector<KvazzValue>> value;
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
    bool is_function;
    std::variant<KvazzValue, KvazzFunction> contents;
}; 

struct Env 
{
    std::shared_ptr<Env> parent;
    std::unordered_map<std::string, EnvEntry> table;
};

const KvazzResult GOOD_NO_RETURN {
    KvazzValue {
        KvazzType::Nothing,
        0
    },
    KvazzFlag::Good
};

bool is_gnr(KvazzResult kr);










