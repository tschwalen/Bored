#pragma once
#include "asteval.h"
#include <vector>
#include <string>
#include <memory> 
#include <utility>

struct Env;
struct KvazzResult;
class AstEvaluator;

enum class NodeType {
    Program, Block, AssignOp, Declare, FunctionDeclare, Return, IfThen,
    IfElse, While, BinaryOp, UnaryOp, FunctionCall, Access, VariableLookup,
    IntLiteral, BoolLiteral, RealLiteral, StringLiteral, VectorLiteral
};

enum class AssignOpType {
    assign, plus, minus, divide, multiply, modulo
};

// 'or' and 'and' become 'pipe' and 'amper' due to c++ reserving these keywords
enum class BinaryOpType {
    pipe, amper, equals, not_equals, less_equals, greater_equals, 
    less_than, greater_than, plus, minus, multiply, divide, 
    modulo
};

// 'not' becomes 'bang' here for similar reasons
enum class UnaryOpType {
    minus, bang
};

AssignOpType get_assign_op (std::string op);
BinaryOpType get_binary_op (std::string op);
UnaryOpType  get_unary_op  (std::string op);

bool is_arithmetic_binop(BinaryOpType);
bool is_comparison_binop(BinaryOpType);
bool is_equality_binop(BinaryOpType);
bool is_logical_binop(BinaryOpType);

std::string arg_list_to_string(std::vector<std::string> args);

/*
* Note: The value() and children() methods of AST node classes are really only used to print out
* a readable representation of the AST, and as such are not written with much consideration to 
* memory efficiency.
*/
class BaseNode 
{
public:
    virtual NodeType              type() = 0;
    virtual std::string           value() = 0;
    virtual const std::vector<std::shared_ptr<BaseNode>>  children() = 0;
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) = 0;
};

class Program : public BaseNode 
{
private:
    std::vector<std::shared_ptr<BaseNode>> nodes;

public:
    virtual NodeType type() override { return NodeType::Program; }
    virtual std::string value() override { return std::string{"Program"}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { return nodes; }

    void add_top_level_stmt( std::shared_ptr<BaseNode> node ) { nodes.push_back(node); }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this, env);
    }
};


class Block : public BaseNode 
{
private:
    std::vector<std::shared_ptr<BaseNode>> stmts;

public:
    Block(std::vector<std::shared_ptr<BaseNode>> stmts_)
        : stmts { std::move(stmts_) } {}
    virtual NodeType type() override { return NodeType::Block; }
    virtual std::string value() override { return std::string{"Block"}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { return stmts; }

    void add_top_level_stmt( std::shared_ptr<BaseNode> node ) { stmts.push_back(node); }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this, env);
    }
};


class AssignOp : public BaseNode 
{
private:
    std::shared_ptr<BaseNode> lvalue; 
    std::string op;
    AssignOpType op_type;
    std::shared_ptr<BaseNode> expr_node;

    /* note that having to copy objects for the children() call isn't the worst thing in the world since the 
        children() function is only used for testing/debugging of the parser.
    */
public:
    AssignOp(std::shared_ptr<BaseNode> lvalue_, std::string op_, std::shared_ptr<BaseNode> expr_node_)
        : lvalue { lvalue_ }, op { op_ }, op_type { get_assign_op(op_) }, expr_node { expr_node_ } {}

    virtual NodeType type() override { return NodeType::AssignOp; }
    virtual std::string value() override { return std::string{"AssignOp " + op + " LValue RValue"}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { lvalue, expr_node };
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this, env);
    }
};

class Declare : public BaseNode 
{
private:
    std::string identifier;
    std::shared_ptr<BaseNode> expr_node;

public:
    Declare(std::string identifier_, std::shared_ptr<BaseNode> expr_node_)
        : identifier { identifier_ }, expr_node {expr_node_} {}

    virtual NodeType type() override { return NodeType::Declare; }
    virtual std::string value() override { return std::string{"Declare " + identifier}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { expr_node };
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this, env);
    }
};

class FunctionDeclare : public BaseNode 
{
private:
    std::string identifier;
    std::vector<std::string> args;
    std::shared_ptr<BaseNode> body;

public:
    FunctionDeclare (std::string identifier_, std::vector<std::string> args_, std::shared_ptr<BaseNode> body_) 
        : identifier { identifier_ }, args { std::move(args_) }, body { body_ } {} 
    
    virtual NodeType type() override { return NodeType::FunctionDeclare; }
    virtual std::string value() override { return std::string{"FunctionDeclare " + identifier + " with " + arg_list_to_string(args)}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { body };
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this, env);
    }
};


class Return : public BaseNode 
{
public:
    std::shared_ptr<BaseNode> expr_node;

    Return (std::shared_ptr<BaseNode> expr_node_) 
        : expr_node { expr_node_ } {}

    virtual NodeType type() override { return NodeType::Return; }
    virtual std::string value() override { return std::string{"Return"}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { expr_node };
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this, env);
    }
};

class IfThen : public BaseNode
{
public:
    std::shared_ptr<BaseNode> condition;
    std::shared_ptr<BaseNode> body;

    IfThen (std::shared_ptr<BaseNode> condition_, std::shared_ptr<BaseNode> body_)
        : condition {condition_}, body { body_ } {}

    virtual NodeType type() override { return NodeType::IfThen; }
    virtual std::string value() override { return std::string{"If then"}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { condition, body };
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this, env);
    }
};

class IfElse : public BaseNode
{
public:
    std::shared_ptr<BaseNode> condition;
    std::shared_ptr<BaseNode> then_body;
    std::shared_ptr<BaseNode> else_body;

    IfElse (std::shared_ptr<BaseNode> condition_, std::shared_ptr<BaseNode> then_body_, std::shared_ptr<BaseNode> else_body_)
        : condition {condition_}, then_body { then_body_ }, else_body { else_body_ } {}

    virtual NodeType type() override { return NodeType::IfElse; }
    virtual std::string value() override { return std::string{"If then else"}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { condition, then_body, else_body };
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this, env);
    }
};

class While : public BaseNode {
public:
    std::shared_ptr<BaseNode> condition;
    std::shared_ptr<BaseNode> body;

    While (std::shared_ptr<BaseNode> condition_, std::shared_ptr<BaseNode> body_)
            : condition {condition_}, body { body_ } {}
    virtual NodeType type() override { return NodeType::While; }
    virtual std::string value() override { return std::string{"While do"}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { condition, body };
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this, env);
    }
};

class BinaryOp : public BaseNode 
{
public:
    std::string op;
    BinaryOpType op_type;
    std::shared_ptr<BaseNode> left_expr;
    std::shared_ptr<BaseNode> right_expr;

    BinaryOp (std::string op_, std::shared_ptr<BaseNode> left_expr_, std::shared_ptr<BaseNode> right_expr_)
        : op { op_ }, op_type { get_binary_op(op_) }, left_expr {left_expr_}, right_expr {right_expr_} {}

    virtual NodeType type() override { return NodeType::BinaryOp; }
    virtual std::string value() override { return std::string{"BinaryOp " + op}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { left_expr, right_expr };
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this, env);
    }
};

class UnaryOp : public BaseNode
{
private:
    std::string op;
    UnaryOpType op_type;
    std::shared_ptr<BaseNode> right_expr;
public:
    UnaryOp (std::string op_, std::shared_ptr<BaseNode> right_expr_)
        : op { op_ }, op_type { get_unary_op(op_) }, right_expr {right_expr_} {}

    virtual NodeType type() override { return NodeType::UnaryOp; }
    virtual std::string value() override { return std::string{"UnaryOp " + op}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { right_expr };
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this, env);
    }
};

class FunctionCall : public BaseNode
{
private:
    std::shared_ptr<BaseNode> callee;
    std::vector<std::shared_ptr<BaseNode>> expr_args;

public:
    FunctionCall (std::shared_ptr<BaseNode> callee_, std::vector<std::shared_ptr<BaseNode>> expr_args_)
        : callee { callee_ }, expr_args { std::move(expr_args_) } {}

    virtual NodeType type() override { return NodeType::FunctionCall; }
    virtual std::string value() override { return std::string{"FunctionCall callee args... "}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local = expr_args;
        local.insert(local.begin(), callee);
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this, env);
    }
};

class Access : public BaseNode
{
private:
    std::shared_ptr<BaseNode> left_expr;
    std::shared_ptr<BaseNode> index_expr;
public:
    Access (std::shared_ptr<BaseNode> left_expr_, std::shared_ptr<BaseNode> index_expr_)
        : left_expr { left_expr_ }, index_expr { index_expr_ } {}

    virtual NodeType type() override { return NodeType::Access; }
    virtual std::string value() override { return std::string{"Access accessee index"}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { left_expr, index_expr };
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this, env);
    }
};

class VariableLookup : public BaseNode
{
private:
    std::string identifier;
    bool sigil;
public:
    VariableLookup(std::string identifier_, bool sigil_)
        : identifier { identifier_ }, sigil { sigil_ } {}

    virtual NodeType type() override { return NodeType::VariableLookup; }
    virtual std::string value() override { return std::string{"VariableLookup" + std::string{sigil ? " $" : " "} + identifier}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local;
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this, env);
    }
};

class IntLiteral : public BaseNode
{
private:
    int literal_value;
public:
    IntLiteral (int literal_value_)
        : literal_value { literal_value_ } {}

    virtual NodeType type() override { return NodeType::IntLiteral; }
    virtual std::string value() override { return std::string{ "int-literal '" + std::to_string(literal_value) + "'" }; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local;
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this);
    }
};

class BoolLiteral : public BaseNode
{
private:
    bool literal_value;
public:
    BoolLiteral (bool literal_value_)
        : literal_value { literal_value_ } {}

    virtual NodeType type() override { return NodeType::BoolLiteral; }
    virtual std::string value() override { return std::string{ "bool-literal " + std::string{literal_value ? "'true'" : "'false'"}}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local;
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this);
    }
};

class RealLiteral : public BaseNode 
{
private:
    double literal_value;
public:
    RealLiteral (double literal_value_)
        : literal_value { literal_value_ } {}

    virtual NodeType type() override { return NodeType::RealLiteral; }
    virtual std::string value() override { return std::string{ "real-literal '" + std::to_string(literal_value) + "'"}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local;
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this);
    }
};

class StringLiteral : public BaseNode 
{
private:
    std::string literal_value;
public:
    StringLiteral (std::string literal_value_)
        : literal_value { literal_value_ } {}

    virtual NodeType type() override { return NodeType::StringLiteral; }
    virtual std::string value() override { return std::string{ "string-literal '" + literal_value + "'"}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local;
        return local;
    }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this);
    }
};

/*  Deal with homogeneous vectors later */
class VectorLiteral : public BaseNode 
{
private:
    std::vector<std::shared_ptr<BaseNode>> contents;
public:
    VectorLiteral (std::vector<std::shared_ptr<BaseNode>> contents_)
        : contents { std::move(contents_) } {}

    virtual NodeType type() override { return NodeType::VectorLiteral; }
    virtual std::string value() override { return std::string{ "VectorLiteral" }; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { return contents; }
    virtual KvazzResult eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) override {
        return ast_eval.eval(*this, env);
    }
};
