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
struct KvazzFunction;

enum class KvazzFlag {
    Good, Error, Return
};

enum class KvazzType {
    Nothing, LValue, Builtin, Int, Real, Bool, String, Hevec, Function
};

enum class EnvResultType {
    Value, Function, Builtin
};

struct KvazzValue
{
    KvazzType type;
    std::variant<int, double, bool, std::string, std::vector<KvazzValue>, LValue, KvazzFunction> value;
};

struct KvazzFunction
{
    std::string               name;
    std::vector<std::string>  args;
    std::shared_ptr<BaseNode> body;
};

struct KvazzResult
{
    KvazzValue kvazz_value;
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


class AstEvaluator {
private:
    bool lvalue_flag;

public:
    virtual KvazzResult eval(Program &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(Block &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(AssignOp &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(Declare &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(FunctionDeclare &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(Return &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(IfThen &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(IfElse &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(While &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(BinaryOp &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(UnaryOp &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(FunctionCall &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(Access &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(VariableLookup &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(IntLiteral &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(BoolLiteral &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(RealLiteral &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(StringLiteral &node, std::shared_ptr<Env> env) = 0;
    virtual KvazzResult eval(VectorLiteral &node, std::shared_ptr<Env> env) = 0;
};















