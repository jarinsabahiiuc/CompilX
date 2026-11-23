#include "lexical_analyzer.h"

#include <cctype>
#include <set>
#include <string>
#include <vector>

using namespace std;

vector<Token> analyzeLexically(const string& source) {

    set<string> keywords = {
        "int", "float", "double", "char", "void",
        "if", "else", "while", "for", "do",
        "switch", "case", "break", "continue",
        "return", "default", "const", "long",
        "short", "unsigned", "signed", "struct",
        "class", "public", "private", "protected",
        "static", "bool", "true", "false",
        "using", "namespace", "include", "define"
    };

    set<string> operators = {
        "+", "-", "*", "/", "%", "=",
        "==", "!=", "<", ">", "<=", ">=",
        "++", "--", "+=", "-=", "*=", "/=",
        "&&", "||", "!", "&", "|", "^",
        "~", "<<", ">>", "->", "."
    };

    set<string> specialSymbols = {
        ";", ",", "(", ")", "{", "}",
        "[", "]", ":", "?"
    };

    set<string> doubleOperators = {
        "==", "!=", "<=", ">=", "++", "--",
        "+=", "-=", "*=", "/=", "&&", "||",
        "<<", ">>", "->"
    };

    vector<Token> tokens;
    size_t i = 0;

    while (i < source.length()) {

        if (isspace(static_cast<unsigned char>(source[i]))) {
            i++;
            continue;
        }

        string lexeme;

        if (isalpha(static_cast<unsigned char>(source[i])) || source[i] == '_') {

            while (i < source.length() &&
                   (isalnum(static_cast<unsigned char>(source[i])) || source[i] == '_')) {
                lexeme += source[i];
                i++;
            }

            if (keywords.count(lexeme)) {
                tokens.push_back({lexeme, "Keyword"});
            }
            else {
                tokens.push_back({lexeme, "Identifier"});
            }
        }

        else if (isdigit(static_cast<unsigned char>(source[i]))) {

            bool hasDecimal = false;

            while (i < source.length()) {

                if (isdigit(static_cast<unsigned char>(source[i]))) {
                    lexeme += source[i];
                    i++;
                }
                else if (source[i] == '.' && !hasDecimal &&
                         i + 1 < source.length() &&
                         isdigit(static_cast<unsigned char>(source[i + 1]))) {
                    lexeme += source[i];
                    hasDecimal = true;
                    i++;
                }
                else {
                    break;
                }
            }

            tokens.push_back({lexeme, "Constant"});
        }

        else if (source[i] == '"' || source[i] == '\'') {

            char quote = source[i];
            lexeme += source[i];
            i++;

            while (i < source.length()) {

                lexeme += source[i];

                if (source[i] == '\\' && i + 1 < source.length()) {
                    i++;
                    lexeme += source[i];
                }
                else if (source[i] == quote) {
                    i++;
                    break;
                }

                i++;
            }

            if (quote == '"') {
                tokens.push_back({lexeme, "String Literal"});
            }
            else {
                tokens.push_back({lexeme, "Character Literal"});
            }
        }

        else {

            if (i + 1 < source.length()) {

                string possibleOperator = source.substr(i, 2);

                if (doubleOperators.count(possibleOperator)) {
                    tokens.push_back({possibleOperator, "Operator"});
                    i += 2;
                    continue;
                }
            }

            lexeme += source[i];

            if (operators.count(lexeme)) {
                tokens.push_back({lexeme, "Operator"});
            }
            else if (specialSymbols.count(lexeme)) {
                tokens.push_back({lexeme, "Special Symbol"});
            }
            else {
                tokens.push_back({lexeme, "Unknown"});
            }

            i++;
        }
    }

    return tokens;
}
