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
    NodeType type{};

    virtual std::vector<BaseNode> children() = 0
    virtual std::string           value() = 0;
};