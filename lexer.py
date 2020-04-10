import sys


# lexer helper methods

keywords = {}
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


def is_id_char(c):
    """returns true if the passed character is a valid identifier character"""
    return c.isalnum() or (c == "_")


def lex_string(source):
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
            if source[i].isalpha():
                j = i + 1
                while j < len(source) and is_id_char(source[j]):
                    j += 1

                word = source[i : j]
                
                if word in keywords.keys():
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
    
    print(tokens)
                
                 


if __name__ == "__main__":

    opts = [opt for opt in sys.argv[1:] if opt.startswith("-")]
    args = [arg for arg in sys.argv[1:] if not arg.startswith("-")]

    
    if "-c" in opts:
        if len(args) == 0:
            raise SystemExit("Command mode requires at least one argument")
        else:
            for arg in args:
                lex_string(arg)
    
    # otherwise read in a file and lex that
    else:
        pass