#pragma once
#include <vector>
#include <string>


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
    virtual std::vector<BaseNode>* children() = 0;
};

class Program : public BaseNode 
{
private:
    std::vector<BaseNode> nodes;

public:
    virtual NodeType type() override { return NodeType::Program; }
    virtual std::string value() override { return std::string{"Program"}; }
    virtual std::vector<BaseNode>* children() override { return &nodes; }

    void add_top_level_stmt( BaseNode &node ) { nodes.push_back(node); }
}


class Block : public BaseNode 
{
private:
    std::vector<BaseNode> stmts;

public:
    virtual NodeType type() override { return NodeType::Block; }
    virtual std::string value() override { return std::string{"Block"}; }
    virtual std::vector<BaseNode>* children() override { return &stmts; }

    void add_top_level_stmt( BaseNode &node ) { stmts.push_back(node); }
}


class AssignOp : public BaseNode 
{
private:
    BaseNode* lvalue;
    std::string op;
    BaseNode* expr_node;

    /* note that having to copy objects for the children() call isn't the worst thing in the world since the 
        children() function is only used for testing/debugging of the parser.
    */
public:
    virtual NodeType type() override { return NodeType::AssignOp; }
    virtual std::string value() override { return std::string{"AssignOp"}; }
    virtual std::vector<BaseNode>* children() override { 
        return new std::vector<BaseNode> { *lvalue, *expr_node };
    }
}

class Declare : public BaseNode {}
class FunctionDeclare : public BaseNode {}
class Return : public BaseNode {}
class IfThen : public BaseNode {}
class IfElse : public BaseNode {}
class While : public BaseNode {}
class BinaryOp : public BaseNode {}
class UnaryOp : public BaseNode {}
class FunctionCall : public BaseNode {}
class Access : public BaseNode {}
class VariableLookup : public BaseNode {}
class IntLiteral : public BaseNode {}
class BoolLiteral : public BaseNode {}
class RealLiteral : public BaseNode {}
class StringLiteral : public BaseNode {}
class VectorLiteral : public BaseNode {}

