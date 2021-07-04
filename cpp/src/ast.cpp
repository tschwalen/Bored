#include "ast.h"
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