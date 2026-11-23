#include "three_address_code.h"

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <cctype>
#include <stdexcept>
#include <limits>

using namespace std;

namespace TAC {

struct Token {
    string value;
    bool operand;
};

bool isOperator(const string& value) {
    return value == "+" || value == "-" ||
           value == "*" || value == "/" ||
           value == "%" || value == "u-";
}

int precedence(const string& op) {
    if (op == "u-") return 3;
    if (op == "*" || op == "/" || op == "%") return 2;
    if (op == "+" || op == "-") return 1;
    return 0;
}

bool isRightAssociative(const string& op) {
    return op == "u-";
}

vector<Token> tokenizeExpression(const string& expression) {
    vector<Token> tokens;

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

            tokens.push_back({identifier, true});
        }
        else if (isdigit(ch) ||
                 (expression[i] == '.' &&
                  i + 1 < expression.size() &&
                  isdigit(static_cast<unsigned char>(expression[i + 1])))) {

            string number;
            bool decimalPoint = false;

            while (i < expression.size()) {
                char current = expression[i];

                if (isdigit(static_cast<unsigned char>(current))) {
                    number += current;
                    i++;
                }
                else if (current == '.' && !decimalPoint) {
                    decimalPoint = true;
                    number += current;
                    i++;
                }
                else {
                    break;
                }
            }

            tokens.push_back({number, true});
        }
        else if (string("+-*/%()").find(expression[i]) != string::npos) {
            tokens.push_back({string(1, expression[i]), false});
            i++;
        }
        else {
            throw invalid_argument(
                string("Unsupported character: ") + expression[i]
            );
        }
    }

    if (tokens.empty()) {
        throw invalid_argument("Expression cannot be empty.");
    }

    return tokens;
}

vector<string> convertToPostfix(const vector<Token>& tokens) {
    vector<string> output;
    stack<string> operators;

    bool expectOperand = true;

    for (const Token& token : tokens) {
        if (token.operand) {
            if (!expectOperand) {
                throw invalid_argument(
                    "Missing operator between operands."
                );
            }

            output.push_back(token.value);
            expectOperand = false;
        }
        else if (token.value == "(") {
            if (!expectOperand) {
                throw invalid_argument(
                    "Missing operator before opening parenthesis."
                );
            }

            operators.push("(");
        }
        else if (token.value == ")") {
            if (expectOperand) {
                throw invalid_argument(
                    "Empty or incomplete parentheses."
                );
            }

            while (!operators.empty() && operators.top() != "(") {
                output.push_back(operators.top());
                operators.pop();
            }

            if (operators.empty()) {
                throw invalid_argument("Mismatched parentheses.");
            }

            operators.pop();
            expectOperand = false;
        }
        else {
            string currentOperator = token.value;

            if (expectOperand) {
                if (currentOperator != "-") {
                    throw invalid_argument(
                        "Unexpected operator: " + currentOperator
                    );
                }

                currentOperator = "u-";
            }

            while (!operators.empty() &&
                   operators.top() != "(" &&
                   (precedence(operators.top()) >
                        precedence(currentOperator) ||
                    (precedence(operators.top()) ==
                        precedence(currentOperator) &&
                     !isRightAssociative(currentOperator)))) {

                output.push_back(operators.top());
                operators.pop();
            }

            operators.push(currentOperator);
            expectOperand = true;
        }
    }

    if (expectOperand) {
        throw invalid_argument(
            "Expression ends with an operator or is incomplete."
        );
    }

    while (!operators.empty()) {
        if (operators.top() == "(") {
            throw invalid_argument("Mismatched parentheses.");
        }

        output.push_back(operators.top());
        operators.pop();
    }

    return output;
}

vector<string> generateTAC(
    const vector<string>& postfix,
    const string& target
) {
    stack<string> operands;
    vector<string> instructions;

    int temporaryCount = 1;

    for (const string& token : postfix) {
        if (!isOperator(token)) {
            operands.push(token);
        }
        else if (token == "u-") {
            if (operands.empty()) {
                throw invalid_argument("Invalid unary expression.");
            }

            string operand = operands.top();
            operands.pop();

            string temporary = "t" + to_string(temporaryCount++);

            instructions.push_back(
                temporary + " = -" + operand
            );

            operands.push(temporary);
        }
        else {
            if (operands.size() < 2) {
                throw invalid_argument("Invalid arithmetic expression.");
            }

            string right = operands.top();
            operands.pop();

            string left = operands.top();
            operands.pop();

            string temporary = "t" + to_string(temporaryCount++);

            instructions.push_back(
                temporary + " = " + left + " " +
                token + " " + right
            );

            operands.push(temporary);
        }
    }

    if (operands.size() != 1) {
        throw invalid_argument("Invalid expression.");
    }

    instructions.push_back(
        target + " = " + operands.top()
    );

    return instructions;
}

} // namespace TAC

void threeAddressCodeModule() {
    using namespace TAC;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "\n============================================\n";
    cout << "          THREE ADDRESS CODE GENERATION     \n";
    cout << "============================================\n";

    cout << "\nSupported operators: +  -  *  /  %\n";
    cout << "Parentheses and unary minus are supported.\n";
    cout << "Example: a = b + c * d\n\n";

    string statement;

    cout << "Enter assignment statement: ";

    if (!getline(cin, statement)) {
        return;
    }

    try {
        size_t equalPosition = statement.find('=');

        if (equalPosition == string::npos ||
            statement.find('=', equalPosition + 1) != string::npos) {
            throw invalid_argument(
                "Use exactly one assignment operator (=)."
            );
        }

        string target = statement.substr(0, equalPosition);
        string expression = statement.substr(equalPosition + 1);

        // Trim spaces around the assignment target.
        size_t first = target.find_first_not_of(" \t\r\n");

        if (first == string::npos) {
            throw invalid_argument("Assignment target is missing.");
        }

        size_t last = target.find_last_not_of(" \t\r\n");
        target = target.substr(first, last - first + 1);

        if (!(isalpha(static_cast<unsigned char>(target[0])) ||
              target[0] == '_')) {
            throw invalid_argument("Invalid assignment target.");
        }

        for (char ch : target) {
            unsigned char current = static_cast<unsigned char>(ch);

            if (!isalnum(current) && ch != '_') {
                throw invalid_argument("Invalid assignment target.");
            }
        }

        vector<Token> tokens = tokenizeExpression(expression);
        vector<string> postfix = convertToPostfix(tokens);

        vector<string> instructions =
            generateTAC(postfix, target);

        cout << "\nThree Address Code:\n";
        cout << "--------------------------------------------\n";

        for (size_t i = 0; i < instructions.size(); i++) {
            cout << i + 1 << ". " << instructions[i] << "\n";
        }

        cout << "--------------------------------------------\n";
        cout << "Total instructions: " << instructions.size() << "\n";
        cout << "TAC generation completed successfully.\n";

    }
    catch (const exception& error) {
        cout << "\nError: " << error.what() << "\n";
        cout << "Three Address Code generation failed.\n";
    }
}
