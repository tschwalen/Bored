#include "lexer.h"
#include "token.h"
#include <iostream>
#include <fstream>
#include <unordered_set> 

using std::string;
using std::vector;
using std::unordered_set;

unordered_set<string> keywords        ( {"var", "if", "then", "else", "for", "while", "do", "in", "function", "return"} );
unordered_set<string> symbols         ( {"{", "}", "(", ")", "[", "]", "<", ">", "+", "-", "*", "/", "%", "!", "?", "=", ".", ",", "&", "|", ";", ":", "$"  } );
unordered_set<string> multi           ( { "==", "!=", ">=", "<=", "+=", "-=", "*=", "/=", "%=", "<[", "]>" } );

bool is_id_char(char c) {
    return isdigit(c) || isalpha(c) || c == '_';
}

vector<Token> lex_string ( string &source ) {

    vector<Token> tokens;
    int index = 0;

    while ( index < source.size() ) {
        
        if ( isspace(source[index]) ) {
            // could treat newlines special, but don't feel like it yet
            while( index < source.size() && isspace(source[index]) ) {
                ++index;
            }
        }
        else {
            /*
            *
            *   Handle identifiers and reserved words
            *
            */
            if ( isalpha(source[index]) || source[index] == '_' ) {
                int end = index + 1;

                // move ahead until the end of the identifier
                while ( end < source.size() && is_id_char(source[end]) ) {
                    ++end;
                }
                string word = source.substr(index, end - index);
                Token token;

                // unordered_set.contains() only came out in c++20, so count > 0 is a less-readable way to check set membership
                if ( keywords.count(word) > 0 ) {
                    token = Token { word, TokenType::keyword };
                }
                else if (word == "true" || word == "false") {
                    token = Token { word, TokenType::bool_literal };
                }
                else {
                    token = Token { word, TokenType::identifier };
                }
                tokens.push_back(token);
                index = end;
            }
            /*
            *
            *   Handle numeric literals (ints & floats)
            *
            */
            else if ( isdigit(source[index]) ) {
                int end = index + 1;

                while ( end < source.size() && isdigit(source[end]) ) {
                    ++end;
                }
                Token token;

                if ( source[end] == '.' ) {
                    ++end;
                    while ( end < source.size() && isdigit(source[end]) ) {
                        ++end;
                    }
                    string real = source.substr(index, end - index);
                    token = Token { real, TokenType::real_literal };
                }
                else {
                    string integer = source.substr(index, end - index);
                    token = Token { integer, TokenType::int_literal };
                }
                tokens.push_back(token);
                index = end;
            }
            /*
            *
            *   Handle single and double-quoted strings
            *
            */
            else if ( source[index] == '\"' || source[index] == '\'' ) {
                char quote = source[index];
                int end = index + 1;
                while ( end < source.size() && source[end] != quote ) {
                    ++end;
                }
                string string_value = source.substr(index + 1, end - (index + 1));
                Token token = Token { string_value, TokenType::string_literal };
                tokens.push_back(token);
                index = end + 1;
            }
            /*
            *
            *   Handle operators and other punctuation-based symbols
            *
            */
            else if (symbols.count(string(1, source[index])) > 0) {
                int end = index + 2;
                Token token;
                if ( end <= source.size() && ( multi.count(source.substr(index, end - index)) > 0) ) {
                    string symbol_value = source.substr(index, end - index);
                    token = Token { symbol_value, TokenType::symbol };
                    index = end;
                }
                else {
                    token =  Token { string(1, source[index]), TokenType::symbol };
                    ++index;
                }
                tokens.push_back(token);
            }
            /*
            *
            *   Handle single and multiline comments
            *
            */
            else if ( source[index] == '~' ) {
                int comment_index = index + 1;
                if ( source[comment_index] == '~' ) {
                    // multiline comment
                    ++comment_index;
                    while (source.substr(comment_index, 2) != "~~") {
                        ++comment_index;
                    }
                    index = comment_index + 2;
                }
                else {
                    while ( source[comment_index] != '\n' ) {
                        comment_index += 1;
                    }
                    index = comment_index;
                }
            }
            else {
                // not even going to think about error recovery
                std::cout << "Invalid start of token " << source[index] << std::endl;
                break;
            }

        }
    }
    return tokens;
}

void tuple_print( vector<Token> &tokens ) {
    std::cout << "[" << std::endl;
    int i = 0;
    for (auto &tok : tokens) {
        if ( i != 0) {
            std::cout << "," << std::endl;
        }
        std::cout << "(\"" << tok.sval << "\", \"" << tokenTypeString(tok.type) << "\")";
        ++i;
    }
    std::cout << std::endl << "]" << std::endl;
}

#ifdef LEXER
int main( int argc, const char* argv[] ) {
    if( argc > 1 ) {
        string source = argv[1];

        // if file flag is used, read from file instead 
        if( source == "-f" && argc > 2 ) {
            /* at some point, should read files in chunks since in practice putting a whole file in memory could
            *  get impractical.
            */
            string source_file = argv[2];
            std::ifstream ifs(source_file);
            source.assign( (std::istreambuf_iterator<char>(ifs) ), (std::istreambuf_iterator<char>() ) );
        } 

        vector<Token> tokens = lex_string(source);
        tuple_print(tokens);
    }
    return 0;
};
#endif
