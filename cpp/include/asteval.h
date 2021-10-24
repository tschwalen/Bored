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

struct LValue;

enum class KvazzFlag {
    Good, Error, Return
};

enum class KvazzType {
    Nothing, LValue, Builtin, Int, Real, Bool, String, Hevec
};

enum class EnvResultType {
    Value, Function, Builtin
};

struct KvazzValue
{
    KvazzType type;
    std::variant<int, double, bool, std::string, std::vector<KvazzValue>, LValue> value;
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

struct LValue {
    KvazzType type; // Nothing : env
    std::variant<std::string, std::shared_ptr<Env>, std::vector<KvazzValue>> lvalue;
    std::variant<int, std::string> index;
};

struct LookupResult
{
    EnvEntry result;
    std::shared_ptr<Env> env;
};

KvazzValue VALUE_NOTHING();

KvazzResult RESULT_GOOD_NO_RETURN();


class AstEvaluator {
private:
    bool lvalue_flag;

public:
    KvazzResult eval(Program &node, std::shared_ptr<Env> env);
    KvazzResult eval(Block &node, std::shared_ptr<Env> env);
    KvazzResult eval(AssignOp &node, std::shared_ptr<Env> env);
    KvazzResult eval(Declare &node, std::shared_ptr<Env> env);
    KvazzResult eval(FunctionDeclare &node, std::shared_ptr<Env> env);
    KvazzResult eval(Return &node, std::shared_ptr<Env> env);
    KvazzResult eval(IfThen &node, std::shared_ptr<Env> env);
    KvazzResult eval(IfElse &node, std::shared_ptr<Env> env);
    KvazzResult eval(While &node, std::shared_ptr<Env> env);
    KvazzResult eval(BinaryOp &node, std::shared_ptr<Env> env);
    KvazzResult eval(UnaryOp &node, std::shared_ptr<Env> env);
    KvazzResult eval(FunctionCall &node, std::shared_ptr<Env> env);
    KvazzResult eval(Access &node, std::shared_ptr<Env> env);
    KvazzResult eval(VariableLookup &node, std::shared_ptr<Env> env);
    KvazzResult eval(IntLiteral &node);
    KvazzResult eval(BoolLiteral &node);
    KvazzResult eval(RealLiteral &node);
    KvazzResult eval(StringLiteral &node);
    KvazzResult eval(VectorLiteral &node, std::shared_ptr<Env> env);
};



bool is_gnr(KvazzResult kr);

LookupResult lookup(std::string identifier, std::shared_ptr<Env> env);

KvazzResult call_function(KvazzFunction &fn, std::vector<KvazzValue> &arg_values, std::shared_ptr<Env> env) {

/*
*  AST-eval functions
*/















