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
    virtual const std::vector<BaseNode*>  &children() = 0;
};

class Program : public BaseNode 
{
private:
    std::vector<BaseNode*> nodes;

public:
    virtual NodeType type() override { return NodeType::Program; }
    virtual std::string value() override { return std::string{"Program"}; }
    virtual const std::vector<BaseNode*> &children() override { return nodes; }

    void add_top_level_stmt( BaseNode &node ) { nodes.push_back(&node); }
};


class Block : public BaseNode 
{
private:
    std::vector<BaseNode*> stmts;

public:
    virtual NodeType type() override { return NodeType::Block; }
    virtual std::string value() override { return std::string{"Block"}; }
    virtual const std::vector<BaseNode*> &children() override { return stmts; }

    void add_top_level_stmt( BaseNode &node ) { stmts.push_back(&node); }
}


class AssignOp : public BaseNode 
{
private:
    // these and all pointers in the code should probably be replaced with smart pointers
    BaseNode* lvalue; 
    std::string op;
    BaseNode* expr_node;

    /* note that having to copy objects for the children() call isn't the worst thing in the world since the 
        children() function is only used for testing/debugging of the parser.
    */
public:
    AssignOp(BaseNode* left_value, std::string oper, BaseNode* expression_node) {
        lvalue = left_value;
        op = oper;
        expr_node = expression_node;
    }

    virtual NodeType type() override { return NodeType::AssignOp; }
    virtual std::string value() override { return std::string{"AssignOp " + op + "LValue RValue"}; }
    virtual const std::vector<BaseNode*> &children() override { 
        std::vector<BaseNode*>* local = new std::vector<BaseNode*> { lvalue, expr_node };
        return *local;
    }
};

class Declare : public BaseNode 
{
private:
    std::string identifier;
    BaseNode* expr_node;

public:
    Declare(std::string id, BaseNode* expression_node) {
        identifier = id;
        expr_node = expression_node;
    }

    virtual NodeType type() override { return NodeType::Declare; }
    virtual std::string value() override { return std::string{"Declare " + identifier}; }
    virtual const std::vector<BaseNode*> &children() override { 
        std::vector<BaseNode*>* local = new std::vector<BaseNode*> { expr_node };
        return *local;
    }
};

class FunctionDeclare : public BaseNode 
{
private:
    std::string identifier;
    std::vector<std::string> args;
    BaseNode* body; // always a block I think

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
    virtual NodeType type() override { return NodeType::FunctionDeclare; }
    virtual std::string value() override { return std::string{"FunctionDeclare " + identifier + " with " + arg_list_to_string()}; }
    virtual const std::vector<BaseNode*> &children() override { 
        std::vector<BaseNode*>* local = new std::vector<BaseNode*> { body };
        return *local;
    }
};


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

