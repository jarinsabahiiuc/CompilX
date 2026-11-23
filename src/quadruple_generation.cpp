#include "quadruple_generation.h"

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cctype>
#include <limits>
#include <stdexcept>

using namespace std;

namespace Quadruple {

struct Token {
    string value;
    string type;
};

struct Quad {
    string op;
    string arg1;
    string arg2;
    string result;
};

vector<Token> tokens;
vector<Quad> quadruples;
size_t position = 0;
int tempCount = 0;

vector<Token> tokenize(const string& expression) {
    vector<Token> result;

    for (size_t i = 0; i < expression.size();) {
        unsigned char ch = static_cast<unsigned char>(expression[i]);

        if (isspace(ch)) {
            i++;
            continue;
        }

        if (isalpha(ch) || expression[i] == '_') {
            string identifier;

            while (i < expression.size()) {
                unsigned char current =
                    static_cast<unsigned char>(expression[i]);

                if (!isalnum(current) && expression[i] != '_') {
                    break;
                }

                identifier += expression[i++];
            }

            result.push_back({identifier, "identifier"});
        }
        else if (isdigit(ch)) {
            string number;
            bool decimal = false;

            while (i < expression.size()) {
                unsigned char current =
                    static_cast<unsigned char>(expression[i]);

                if (isdigit(current)) {
                    number += expression[i++];
                }
                else if (expression[i] == '.' && !decimal) {
                    decimal = true;
                    number += expression[i++];
                }
                else {
                    break;
                }
            }

            if (number.back() == '.') {
                throw invalid_argument("Invalid numeric constant.");
            }

            result.push_back({number, "number"});
        }
        else if (string("+-*/%()=").find(expression[i]) != string::npos) {
            result.push_back({string(1, expression[i]), "operator"});
            i++;
        }
        else {
            throw invalid_argument(
                "Invalid character: " + string(1, expression[i])
            );
        }
    }

    return result;
}

string newTemp() {
    return "t" + to_string(++tempCount);
}

bool match(const string& value) {
    if (position < tokens.size() &&
        tokens[position].value == value) {
        position++;
        return true;
    }

    return false;
}

void addQuad(
    const string& op,
    const string& arg1,
    const string& arg2,
    const string& result
) {
    quadruples.push_back({op, arg1, arg2, result});
}

string parseExpression();

string parsePrimary() {
    if (position >= tokens.size()) {
        throw invalid_argument("Unexpected end of expression.");
    }

    if (match("(")) {
        string value = parseExpression();

        if (!match(")")) {
            throw invalid_argument("Missing closing parenthesis.");
        }

        return value;
    }

    if (tokens[position].type == "identifier" ||
        tokens[position].type == "number") {
        return tokens[position++].value;
    }

    throw invalid_argument("Invalid expression operand.");
}

string parseUnary() {
    if (match("-")) {
        string operand = parseUnary();
        string result = newTemp();

        addQuad("uminus", operand, "-", result);

        return result;
    }

    if (match("+")) {
        return parseUnary();
    }

    return parsePrimary();
}

string parseTerm() {
    string left = parseUnary();

    while (position < tokens.size()) {
        string op = tokens[position].value;

        if (op != "*" && op != "/" && op != "%") {
            break;
        }

        position++;

        string right = parseUnary();
        string result = newTemp();

        addQuad(op, left, right, result);

        left = result;
    }

    return left;
}

string parseExpression() {
    string left = parseTerm();

    while (position < tokens.size()) {
        string op = tokens[position].value;

        if (op != "+" && op != "-") {
            break;
        }

        position++;

        string right = parseTerm();
        string result = newTemp();

        addQuad(op, left, right, result);

        left = result;
    }

    return left;
}

void generate(const string& statement) {
    tokens = tokenize(statement);
    quadruples.clear();
    position = 0;
    tempCount = 0;

    if (tokens.size() < 3 ||
        tokens[0].type != "identifier" ||
        tokens[1].value != "=") {
        throw invalid_argument(
            "Use assignment format: variable = expression."
        );
    }

    string destination = tokens[0].value;
    position = 2;

    string result = parseExpression();

    if (position != tokens.size()) {
        throw invalid_argument(
            "Unexpected symbol: " + tokens[position].value
        );
    }

    addQuad("=", result, "-", destination);
}

void display() {
    cout << "\nQUADRUPLE TABLE\n";
    cout << "------------------------------------------------------------\n";

    cout << left
         << setw(8) << "No."
         << setw(14) << "Operator"
         << setw(14) << "Arg 1"
         << setw(14) << "Arg 2"
         << "Result\n";

    cout << "------------------------------------------------------------\n";

    for (size_t i = 0; i < quadruples.size(); i++) {
        const Quad& q = quadruples[i];

        cout << left
             << setw(8) << i + 1
             << setw(14) << q.op
             << setw(14) << q.arg1
             << setw(14) << q.arg2
             << q.result << "\n";
    }

    cout << "------------------------------------------------------------\n";
    cout << "Total quadruples: " << quadruples.size() << "\n";
}

} // namespace Quadruple

void quadrupleGenerationModule() {
    using namespace Quadruple;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "\n============================================\n";
    cout << "           QUADRUPLE GENERATION             \n";
    cout << "============================================\n";

    cout << "\nSupported operators: +  -  *  /  %\n";
    cout << "Parentheses and unary minus are supported.\n";
    cout << "Example: a = b + c * d\n";

    string statement;

    cout << "\nEnter assignment statement: ";

    if (!getline(cin, statement)) {
        return;
    }

    try {
        generate(statement);
        display();

        cout << "\nQuadruple generation completed successfully.\n";
    }
    catch (const exception& error) {
        cout << "\nError: " << error.what() << "\n";
        cout << "Quadruple generation failed.\n";
    }
}
