#pragma once
#include "token.h"
#include <vector> 
#include <string>


bool is_id_char(char c);

std::vector<Token> lex_string ( std::string &source );

void tuple_print( std::vector<Token> &tokens );
