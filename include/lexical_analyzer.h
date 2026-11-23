#ifndef LEXICAL_ANALYZER_H
#define LEXICAL_ANALYZER_H

#include <string>
#include <vector>

struct Token {
    std::string lexeme;
    std::string type;
};

std::vector<Token> analyzeLexically(const std::string& source);

#endif
