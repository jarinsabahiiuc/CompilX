#include "comment_remover.h"

#include <string>

using namespace std;

enum State {
    NORMAL,
    SINGLE_LINE_COMMENT,
    MULTI_LINE_COMMENT,
    STRING_LITERAL,
    CHARACTER_LITERAL
};

string removeComments(const string& source) {
    string result;
    State state = NORMAL;

    for (size_t i = 0; i < source.length(); i++) {
        char current = source[i];
        char next = (i + 1 < source.length()) ? source[i + 1] : '\0';

        switch (state) {

            case NORMAL:
                if (current == '/' && next == '/') {
                    state = SINGLE_LINE_COMMENT;
                    i++;
                }
                else if (current == '/' && next == '*') {
                    state = MULTI_LINE_COMMENT;
                    i++;
                }
                else if (current == '"') {
                    state = STRING_LITERAL;
                    result += current;
                }
                else if (current == '\'') {
                    state = CHARACTER_LITERAL;
                    result += current;
                }
                else {
                    result += current;
                }
                break;

            case SINGLE_LINE_COMMENT:
                if (current == '\n') {
                    result += '\n';
                    state = NORMAL;
                }
                break;

            case MULTI_LINE_COMMENT:
                if (current == '*' && next == '/') {
                    state = NORMAL;
                    i++;
                }
                else if (current == '\n') {
                    result += '\n';
                }
                break;

            case STRING_LITERAL:
                result += current;

                if (current == '\\' && i + 1 < source.length()) {
                    result += source[++i];
                }
                else if (current == '"') {
                    state = NORMAL;
                }
                break;

            case CHARACTER_LITERAL:
                result += current;

                if (current == '\\' && i + 1 < source.length()) {
                    result += source[++i];
                }
                else if (current == '\'') {
                    state = NORMAL;
                }
                break;
        }
    }

    return result;
}
