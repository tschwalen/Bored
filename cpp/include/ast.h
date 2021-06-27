#pragma once
#include "asteval.h"
#include <vector>
#include <string>
#include <memory> 
#include <utility>

enum class NodeType {
    Program,
    Block,
    AssignOp,
    Declare,
    FunctionDeclare,
    Return,
    IfThen,
    IfElse,
    While,
    BinaryOp,
    UnaryOp,
    FunctionCall,
    Access,
    VariableLookup,
    IntLiteral,
    BoolLiteral,
    RealLiteral,
    StringLiteral,
    VectorLiteral
};

class BaseNode 
{
public:
    virtual NodeType              type() = 0;
    virtual std::string           value() = 0;
    virtual const std::vector<std::shared_ptr<BaseNode>>  children() = 0;
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
};


class AssignOp : public BaseNode 
{
private:
    // these and all pointers in the code should probably be replaced with smart pointers
    std::shared_ptr<BaseNode> lvalue; 
    std::string op;
    std::shared_ptr<BaseNode> expr_node;

    /* note that having to copy objects for the children() call isn't the worst thing in the world since the 
        children() function is only used for testing/debugging of the parser.
    */
public:
    AssignOp(std::shared_ptr<BaseNode> lvalue_, std::string op_, std::shared_ptr<BaseNode> expr_node_)
        : lvalue { lvalue_ }, op { op_ }, expr_node { expr_node_ } {}

    virtual NodeType type() override { return NodeType::AssignOp; }
    virtual std::string value() override { return std::string{"AssignOp " + op + "LValue RValue"}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { lvalue, expr_node };
        return local;
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
};

class FunctionDeclare : public BaseNode 
{
private:
    std::string identifier;
    std::vector<std::string> args;
    std::shared_ptr<BaseNode> body; // always a block I think

    std::string arg_list_to_string() {
        std::string result = "[";
        int index = 0;
        for (auto &argstr : args) {
            result += argstr;
            if (index < args.size() - 1)
                result += ", ";
            ++index;
        }
        result += "]";
        return result;
    }

public:
    FunctionDeclare (std::string identifier_, std::vector<std::string> args_, std::shared_ptr<BaseNode> body_) 
        : identifier { identifier_ }, args { std::move(args_) }, body { body_ } {} 
    
    virtual NodeType type() override { return NodeType::FunctionDeclare; }
    virtual std::string value() override { return std::string{"FunctionDeclare " + identifier + " with " + arg_list_to_string()}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { body };
        return local;
    }
};


class Return : public BaseNode 
{
private:
    std::shared_ptr<BaseNode> expr_node;

public:
    Return (std::shared_ptr<BaseNode> expr_node_) 
        : expr_node { expr_node_ } {}

    virtual NodeType type() override { return NodeType::Return; }
    virtual std::string value() override { return std::string{"Return"}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { expr_node };
        return local;
    }
};

class IfThen : public BaseNode
{
private:
    std::shared_ptr<BaseNode> condition;
    std::shared_ptr<BaseNode> body;
public:
    IfThen (std::shared_ptr<BaseNode> condition_, std::shared_ptr<BaseNode> body_)
        : condition {condition_}, body { body_ } {}

    virtual NodeType type() override { return NodeType::IfThen; }
    virtual std::string value() override { return std::string{"If Then"}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { condition, body };
        return local;
    }
};

class IfElse : public BaseNode
{
private:
    std::shared_ptr<BaseNode> condition;
    std::shared_ptr<BaseNode> then_body;
    std::shared_ptr<BaseNode> else_body;
public:
    IfElse (std::shared_ptr<BaseNode> condition_, std::shared_ptr<BaseNode> then_body_, std::shared_ptr<BaseNode> else_body_)
        : condition {condition_}, then_body { then_body_ }, else_body { else_body_ } {}

    virtual NodeType type() override { return NodeType::IfElse; }
    virtual std::string value() override { return std::string{"If Else"}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { condition, then_body, else_body };
        return local;
    }
};

class While : public BaseNode {
private:
    std::shared_ptr<BaseNode> condition;
    std::shared_ptr<BaseNode> body;
public:
    While (std::shared_ptr<BaseNode> condition_, std::shared_ptr<BaseNode> body_)
            : condition {condition_}, body { body_ } {}
    virtual NodeType type() override { return NodeType::While; }
    virtual std::string value() override { return std::string{"While do"}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { condition, body };
        return local;
    }
};

class BinaryOp : public BaseNode 
{
private:
    std::string op; // TODO: replace with enum
    std::shared_ptr<BaseNode> left_expr;
    std::shared_ptr<BaseNode> right_expr;
public:
    BinaryOp (std::string op_, std::shared_ptr<BaseNode> left_expr_, std::shared_ptr<BaseNode> right_expr_)
        : op { op_ }, left_expr {left_expr_}, right_expr {right_expr_} {}

    virtual NodeType type() override { return NodeType::BinaryOp; }
    virtual std::string value() override { return std::string{"BinaryOp " + op}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { left_expr, right_expr };
        return local;
    }
};

class UnaryOp : public BaseNode
{
private:
    std::string op; // TODO: replace with enum
    std::shared_ptr<BaseNode> right_expr;
public:
    UnaryOp (std::string op_, std::shared_ptr<BaseNode> right_expr_)
        : op { op_ }, right_expr {right_expr_} {}

    virtual NodeType type() override { return NodeType::UnaryOp; }
    virtual std::string value() override { return std::string{"UnaryOp " + op}; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local { right_expr };
        return local;
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
        expr_args.insert(expr_args.begin(), callee);
        return local;
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
};

class IntLiteral : public BaseNode
{
private:
    int literal_value;
public:
    IntLiteral (int literal_value_)
        : literal_value { literal_value_ } {}

    virtual NodeType type() override { return NodeType::IntLiteral; }
    virtual std::string value() override { return std::string{ "Int literal " + std::to_string(literal_value) }; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local;
        return local;
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
    virtual std::string value() override { return std::string{ "Bool literal " + std::to_string(literal_value) }; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local;
        return local;
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
    virtual std::string value() override { return std::string{ "Real literal " + std::to_string(literal_value) }; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local;
        return local;
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
    virtual std::string value() override { return std::string{ "String literal " + literal_value }; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        std::vector<std::shared_ptr<BaseNode>> local;
        return local;
    }
};



/*  Deal with heterogeneous vectors later */
class VectorLiteral : public BaseNode // maybe "vector expression" would be a better way of putting it
{
private:
    std::vector<std::shared_ptr<BaseNode>> contents;
public:
    VectorLiteral (std::vector<std::shared_ptr<BaseNode>> contents_)
        : contents { std::move(contents_) } {}

    virtual NodeType type() override { return NodeType::VectorLiteral; }
    virtual std::string value() override { return std::string{ "VectorLiteral" }; }
    virtual const std::vector<std::shared_ptr<BaseNode>> children() override { 
        return contents;
    }
};