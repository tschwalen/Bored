#include "ast.h"
#include "asteval.h"
#include <string>
#include <iostream>
#include <vector>

using std::string;

string arg_list_to_string(std::vector<string> args) {
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

AssignOpType get_assign_op (string op) {
    if      (op == "=")  return AssignOpType::assign;
    else if (op == "+=") return AssignOpType::plus;
    else if (op == "-=") return AssignOpType::minus;
    else if (op == "/=") return AssignOpType::divide;
    else if (op == "*=") return AssignOpType::multiply;
    else if (op == "%=") return AssignOpType::modulo;

    std::cerr << "Invalid assign op: " << op << std::endl;
    return AssignOpType::assign;
}

BinaryOpType get_binary_op (string op) {
    if      (op == "|")  return BinaryOpType::pipe;
    else if (op == "&")  return BinaryOpType::amper;
    else if (op == "==") return BinaryOpType::equals;
    else if (op == "!=") return BinaryOpType::not_equals;
    else if (op == "<=") return BinaryOpType::less_equals;
    else if (op == ">=") return BinaryOpType::greater_equals;
    else if (op == "<")  return BinaryOpType::less_than;
    else if (op == ">")  return BinaryOpType::greater_than;
    else if (op == "+")  return BinaryOpType::plus;
    else if (op == "-")  return BinaryOpType::minus;
    else if (op == "*")  return BinaryOpType::multiply;
    else if (op == "/")  return BinaryOpType::divide;
    else if (op == "%")  return BinaryOpType::modulo;

    std::cerr << "Invalid binary op: " << op << std::endl;
    return BinaryOpType::pipe;
}

UnaryOpType get_unary_op (string op) {
    if      (op == "-") return UnaryOpType::minus;
    else if (op == "!") return UnaryOpType::bang;

    std::cerr << "Invalid unary op: " << op << std::endl;
    return UnaryOpType::minus;
}





// unused, thought I may need them for binop eval for interpreter, not sure now
bool is_arithmetic_binop(BinaryOpType op) {
    return op == BinaryOpType::plus ||
            op == BinaryOpType::minus ||
            op ==  BinaryOpType::multiply ||
            op ==  BinaryOpType::divide ||
            op ==  BinaryOpType::modulo;
}

bool is_comparison_binop(BinaryOpType op) {
    return op == BinaryOpType::less_equals ||
            op == BinaryOpType::greater_equals ||
            op ==  BinaryOpType::less_than ||
            op ==  BinaryOpType::greater_than;
}

bool is_equality_binop(BinaryOpType op) {
    return op == BinaryOpType::equals ||
            op == BinaryOpType::not_equals;
}

bool is_logical_binop(BinaryOpType op) {
    return op == BinaryOpType::pipe ||
            op == BinaryOpType::amper;
}

// ast eval methods
KvazzResult Program::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}

KvazzResult Block::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}

KvazzResult AssignOp::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}

KvazzResult Declare::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}

KvazzResult FunctionDeclare::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}

KvazzResult Return::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}

KvazzResult IfThen::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}

KvazzResult IfElse::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}

KvazzResult While::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}


KvazzResult BinaryOp::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}


KvazzResult UnaryOp::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}


KvazzResult FunctionCall::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}

KvazzResult Access::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}

KvazzResult VariableLookup::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}

KvazzResult IntLiteral::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}

KvazzResult BoolLiteral::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}

KvazzResult RealLiteral::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}

KvazzResult StringLiteral::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}

KvazzResult VectorLiteral::eval(AstEvaluator &ast_eval, std::shared_ptr<Env> env) {
    return ast_eval.eval(this, env);
}
