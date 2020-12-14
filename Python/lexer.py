import sys


# lexer helper methods

keywords = {"var", "if", "then", "else", "for", "while", "do", "in", "function", "return"}


symbols = {
    "{" : "left-brace",
    "}" : "right-brace",
    "(" : "left-paren",
    ")" : "right-paren",
    "[" : "left-bracket",
    "]" : "right-bracket",
    "<" : "less-than",
    ">" : "greater-than",
    "+" : "plus",
    "-" : "minus",
    "*" : "star",
    "/" : "slash",
    "%" : "percent",
    "!" : "not",
    "?" : "question",
    "=" : "equals",
    "." : "dot",
    "," : "comma",
    "&" : "and", 
    "|" : "or",
    ";" : "semicolon",
    ":" : "colon"
}

maybe_multichar = {
    "=", "!", ">", "<", "+", "-", "*", "/", "%"
}

multi = {
    "==" : "equals-equals",
    "!=" : "not-equals",
    ">=" : "greater-equals",
    "<=" : "less-equals",
    "+=" : "plus-equals",
    "-=" : "minus-equals",
    "*=" : "star-equals",
    "/=" : "slash-equals",
    "%=" : "percent-equals"
}

def pretty_print(ts):
    print("[")
    for t in ts:
        print(f"{t[0]} : {t[1]}")
    print("]")

def tuple_print( tokens ):
    print("[")

    i = 0
    while i < (len(tokens) - 1):
        print("(\"%s\", \"%s\")," % (tokens[i][1], tokens[i][0]))
        i += 1
    print("(\"%s\", \"%s\")" % (tokens[i][1], tokens[i][0]))

    print("]")


def is_id_char(c):
    """returns true if the passed character is a valid identifier character"""
    return c.isalnum() or (c == "_")


def lex_string(source, print_tokens=False):
    global keywords
    global symbols
    
    tokens = []
    i = 0

    while(i < len(source)):

        if source[i].isspace():
            while i < len(source) and source[i].isspace():
                i += 1
        else:
            # identifiers
            if source[i].isalpha() or source[i] == "_":
                j = i + 1
                while j < len(source) and is_id_char(source[j]):
                    j += 1

                word = source[i : j]
                
                if word in keywords:
                    tokens.append( ('keyword', word) )
                else:
                    tokens.append( ('identifier', source[i : j]) )
                i = j
            
            elif source[i].isnumeric():
                j = i + 1
                while j < len(source) and source[j].isnumeric():
                    j += 1
                if source[j] == ".":
                    j += 1
                    while j < len(source) and source[j].isnumeric():
                        j += 1
                    tokens.append( ('real-literal', source[i : j]) )
                    i = j
                else:
                    tokens.append( ('int-literal', source[i : j]) )
                    i = j

            elif source[i] in "\'\"":
                quote = source[i]
                j = i + 1
                while source[j] != quote:
                    j += 1
                tokens.append( ('string-literal', source[ i + 1 : j ]) )
                i = j + 1

            elif source[i] in symbols.keys():
                
                j = i + 2
                if j <= len(source) and source[i : j] in multi.keys():
                    symbol = source[i : j]
                    tokens.append( (multi[symbol], symbol) )
                    i = j
                else:
                    symbol = source[i : i+1]
                    tokens.append( (symbols[symbol], symbol) )
                    i += 1
            else:
                raise Exception("Invalid token start: " + source[i])
    
    #pretty_print(tokens)
    if print_tokens:
        tuple_print(tokens)

    # swap tuple order so that it works with the lexer and matches the order of the c++ lexer
    return list( map(lambda t : (t[1], t[0]), tokens) )
                
                 


if __name__ == "__main__":

    opts = [opt for opt in sys.argv[1:] if opt.startswith("-")]
    args = [arg for arg in sys.argv[1:] if not arg.startswith("-")]

    
    if "-c" in opts:
        if len(args) == 0:
            raise SystemExit("Command mode requires at least one argument")
        else:
            lex_string(' '.join(args), print_tokens=True)
    
    # otherwise read in a file and lex that
    else:
        pass