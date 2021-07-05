#include "ast.h"
#include "asteval.h"
#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <unordered_map>

using std::unordered_map; 
using std::shared_ptr;
using std::vector;
using std::string;


Env global_env {
    nullptr, 
    unordered_map<string, EnvEntry>()
};


/* 
*   Utility Functions
*/

bool is_gnr(KvazzResult kr) {
    return kr.flag == KvazzFlag::Good && kr.return_value.type == KvazzType::Nothing;
}



