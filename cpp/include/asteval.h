#pragma once
#include <string>
#include <variant>
#include <vector>


enum class KvazzFlag {
    Good, Error, Return
};

enum class KvazzType {
    Int, Real, Bool, String, Vector
};

class KvazzValue 
{
private:
    KvazzType type;
    std::variant<int, double, bool, std::string, std::vector<KvazzValue>> contents;
};

class KvazzResult
{
private:
    KvazzFlag flag;
    KvazzValue value;
};