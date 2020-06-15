#include <stdio.h>
#include <ctype.h>
#include <vector> 
#include <string>
#include <unordered_set> 



std::unordered_set<std::string> keywords        ( {"var", "if", "then", "else", "for", "while", "do", "in"} );
std::unordered_set<std::string> symbols         ( {"{", "}", "(", ")", "[", "]", "<", ">", "+", "-", "*", "/", "%", "!", "?", "=", ".", ",", "&", "|", ";", ":" } );
std::unordered_set<std::string> maybe_multichar ( {"=", "!", ">", "<", "+", "-", "*", "/", "%"} );
std::unordered_set<std::string> multi           ( { "==", "!=", ">=", "<=", "+=", "-=", "*=", "/=", "%=" } );

bool is_id_char(char c) {
    return isdigit(c) || isalpha(c) || c == '_';
}

std::vector<std::pair<std::string, std::string>> lex_string ( std::string &source ) {

    std::vector<std::pair<std::string, std::string>> tokens;
    int index = 0;

    while ( index < source.size() ) {
        
        if ( isspace(source[index]) ) {
            // could treat newlines special, but don't feel like it yet
            while( index < source.size() && isspace(source[index]) ) {
                ++index;
            }
        }
        else {

            // identifiers and keywords (reserved words)
            if ( isalpha(source[index]) || source[index] == '_' ) {
                int end = index + 1;

                // move ahead until the end of the identifier
                while ( end < source.size() && is_id_char(source[end]) ) {
                    ++end;
                }
                std::string word = source.substr(index, end - index);
                std::pair<std::string, std::string> token;

                // unordered_set.contains() only came out in c++20
                if ( keywords.count(word) > 0 ) {
                    token = std::make_pair( word, "keyword" ); 
                }
                else {
                    token = std::make_pair( word, "identifier" );
                }
                tokens.push_back(token);
                index = end;
            }
            // integers and real numbers (floating point)
            else if ( isdigit(source[index]) ) {
                int end = index + 1;

                while ( end < source.size() && isdigit(source[end]) ) {
                    ++end;
                }
                std::pair<std::string, std::string> token;

                if ( source[end] == '.' ) {
                    ++end;
                    while ( end < source.size() && isdigit(source[end]) ) {
                        ++end;
                    }
                    std::string real = source.substr(index, end - index);
                    token = std::make_pair(real, "real-literal");
                }
                else {
                    std::string integer = source.substr(index, end - index);
                    token = std::make_pair(integer, "int-literal");
                }
                tokens.push_back(token);
                index = end;
            }
            // string literals
            else if ( source[index] == '\"' || source[index] == '\'' ) {
                char quote = source[index];
                int end = index + 1;
                while ( end < source.size() && source[end] != quote ) {
                    ++end;
                }
                std::string string_literal = source.substr(index + 1, end - (index + 1));
                std::pair<std::string, std::string> token = std::make_pair(string_literal, "string-literal");
                tokens.push_back(token);
            }
            // symbols and operators
            else if (symbols.count(std::string(1, source[index])) > 0) {
                int end = index + 2;
                std::pair<std::string, std::string> token;
                if ( end <= source.size() && ( multi.count(source.substr(index, end - index)) > 0) ) {
                    std::string symbol = source.substr(index, end - index);
                    token = std::make_pair(symbol, "symbol");
                    index = end;
                }
                else {
                    token = std::make_pair(std::string(1, source[index]), "symbol");
                    ++index;
                }
                tokens.push_back(token);
            }
            else {
                printf("Invalid start of token %c\n", source[index]);
                break;
            }

        }
    }
    return tokens;
}

void pretty_print( std::vector<std::pair<std::string, std::string>> &tokens) {
    printf("[\n");

    for (int i = 0; i < tokens.size(); ++i) {
        const char* token = tokens[i].first.c_str();
        const char* token_type = tokens[i].second.c_str();
        printf("%s : %s\n", token, token_type);
    }

    printf("]\n");
}


int main( int argc, const char* argv[] ) {
    // for now just take an argument string and tokenize it, worry about file IO later
    if( argc > 1 ) {
        std::string source = argv[1];

        std::vector<std::pair<std::string, std::string>> tokens = lex_string(source);

        pretty_print(tokens);
    }
    return 0;
}