#include "code_optimization.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <limits>
#include <stdexcept>

using namespace std;

namespace CodeOptimization {

struct Instruction {
    string lhs;
    string op;
    string arg1;
    string arg2;
};

bool isNumber(const string& value) {
    if (value.empty()) {
        return false;
    }

    char* end = nullptr;
    strtod(value.c_str(), &end);

    return end != value.c_str() && *end == '\0';
}

bool isTemporary(const string& name) {
    return name.size() > 1 &&
           name[0] == 't' &&
           all_of(name.begin() + 1, name.end(),
                  [](unsigned char ch) { return isdigit(ch); });
}

string numberText(double value) {
    ostringstream output;
    output << setprecision(12) << value;
    return output.str();
}

Instruction parseInstruction(const string& line) {
    istringstream stream(line);

    Instruction instruction;
    string equalSign;
    string first;
    string second;
    string third;

    if (!(stream >> instruction.lhs >> equalSign >> first) ||
        equalSign != "=") {
        throw invalid_argument(
            "Invalid TAC instruction. Expected: t1 = a + b"
        );
    }

    if (!(stream >> second)) {
        instruction.op = "=";
        instruction.arg1 = first;
    }
    else if (!(stream >> third)) {
        if (first.size() > 1 && first[0] == '-') {
            instruction.op = "uminus";
            instruction.arg1 = first.substr(1);
        }
        else {
            throw invalid_argument(
                "Invalid unary instruction: " + line
            );
        }
    }
    else {
        string extra;

        if (stream >> extra) {
            throw invalid_argument(
                "Too many operands in instruction: " + line
            );
        }

        if (second != "+" && second != "-" &&
            second != "*" && second != "/" && second != "%") {
            throw invalid_argument(
                "Unsupported operator: " + second
            );
        }

        instruction.op = second;
        instruction.arg1 = first;
        instruction.arg2 = third;
    }

    return instruction;
}

string resolveConstant(
    const string& value,
    const map<string, string>& constants
) {
    auto it = constants.find(value);

    if (it != constants.end()) {
        return it->second;
    }

    return value;
}

bool calculate(
    const string& op,
    double a,
    double b,
    double& result
) {
    if (op == "+") {
        result = a + b;
    }
    else if (op == "-") {
        result = a - b;
    }
    else if (op == "*") {
        result = a * b;
    }
    else if (op == "/") {
        if (b == 0) return false;
        result = a / b;
    }
    else if (op == "%") {
        if (b == 0) return false;
        result = fmod(a, b);
    }
    else {
        return false;
    }

    return true;
}

void constantFoldingAndPropagation(
    vector<Instruction>& code
) {
    map<string, string> constants;

    for (Instruction& instruction : code) {
        instruction.arg1 =
            resolveConstant(instruction.arg1, constants);

        if (instruction.op != "=" &&
            instruction.op != "uminus") {
            instruction.arg2 =
                resolveConstant(instruction.arg2, constants);
        }

        if (instruction.op == "uminus" &&
            isNumber(instruction.arg1)) {
            double value = -strtod(instruction.arg1.c_str(), nullptr);

            instruction.op = "=";
            instruction.arg1 = numberText(value);
        }

        if (instruction.op != "=" &&
            instruction.op != "uminus" &&
            isNumber(instruction.arg1) &&
            isNumber(instruction.arg2)) {

            double a = strtod(instruction.arg1.c_str(), nullptr);
            double b = strtod(instruction.arg2.c_str(), nullptr);
            double result;

            if (calculate(instruction.op, a, b, result)) {
                instruction.op = "=";
                instruction.arg1 = numberText(result);
                instruction.arg2.clear();
            }
        }

        if (instruction.op == "=" && isNumber(instruction.arg1)) {
            constants[instruction.lhs] = instruction.arg1;
        }
        else {
            constants.erase(instruction.lhs);
        }
    }
}

void algebraicSimplification(vector<Instruction>& code) {
    for (Instruction& instruction : code) {
        if (instruction.op == "+") {
            if (instruction.arg2 == "0") {
                instruction.op = "=";
                instruction.arg2.clear();
            }
            else if (instruction.arg1 == "0") {
                instruction.op = "=";
                instruction.arg1 = instruction.arg2;
                instruction.arg2.clear();
            }
        }
        else if (instruction.op == "-") {
            if (instruction.arg2 == "0") {
                instruction.op = "=";
                instruction.arg2.clear();
            }
        }
        else if (instruction.op == "*") {
            if (instruction.arg2 == "1") {
                instruction.op = "=";
                instruction.arg2.clear();
            }
            else if (instruction.arg1 == "1") {
                instruction.op = "=";
                instruction.arg1 = instruction.arg2;
                instruction.arg2.clear();
            }
            else if (instruction.arg1 == "0" ||
                     instruction.arg2 == "0") {
                instruction.op = "=";
                instruction.arg1 = "0";
                instruction.arg2.clear();
            }
        }
        else if (instruction.op == "/") {
            if (instruction.arg2 == "1") {
                instruction.op = "=";
                instruction.arg2.clear();
            }
        }
        else if (instruction.op == "%") {
            if (instruction.arg2 == "1") {
                instruction.op = "=";
                instruction.arg1 = "0";
                instruction.arg2.clear();
            }
        }
    }
}

string expressionKey(const Instruction& instruction) {
    string a = instruction.arg1;
    string b = instruction.arg2;

    if (instruction.op == "+" || instruction.op == "*") {
        if (b < a) {
            swap(a, b);
        }
    }

    return instruction.op + "|" + a + "|" + b;
}

string resolveCopy(
    const string& value,
    const map<string, string>& copies
) {
    string result = value;
    set<string> visited;

    while (copies.find(result) != copies.end() &&
           visited.insert(result).second) {
        result = copies.at(result);
    }

    return result;
}

void copyPropagation(vector<Instruction>& code) {
    map<string, string> copies;
    vector<Instruction> optimized;

    for (Instruction instruction : code) {
        instruction.arg1 = resolveCopy(instruction.arg1, copies);
        instruction.arg2 = resolveCopy(instruction.arg2, copies);

        if (instruction.op == "=" &&
            isTemporary(instruction.lhs)) {
            copies[instruction.lhs] = instruction.arg1;
            continue;
        }

        copies.erase(instruction.lhs);
        optimized.push_back(instruction);
    }

    code = optimized;
}
void commonSubexpressionElimination(
    vector<Instruction>& code
) {
    map<string, string> available;
    map<string, string> replacements;
    vector<Instruction> optimized;

    for (Instruction instruction : code) {
        if (replacements.count(instruction.arg1)) {
            instruction.arg1 = replacements[instruction.arg1];
        }

        if (replacements.count(instruction.arg2)) {
            instruction.arg2 = replacements[instruction.arg2];
        }

        // A new assignment invalidates an older expression
        // that used the previous value of the assigned variable.
        for (auto it = available.begin(); it != available.end();) {
            string key = it->first;
            string variable = instruction.lhs;

            if (key.find("|" + variable + "|") != string::npos ||
                (key.size() > variable.size() &&
                 key.compare(key.size() - variable.size(),
                             variable.size(), variable) == 0)) {
                it = available.erase(it);
            }
            else {
                ++it;
            }
        }

        if (instruction.op == "+" ||
            instruction.op == "-" ||
            instruction.op == "*" ||
            instruction.op == "/" ||
            instruction.op == "%") {

            string key = expressionKey(instruction);
            auto found = available.find(key);

            if (found != available.end()) {
                replacements[instruction.lhs] = found->second;
                continue;
            }

            available[key] = instruction.lhs;
        }

        optimized.push_back(instruction);
    }

    for (Instruction& instruction : optimized) {
        if (replacements.count(instruction.arg1)) {
            instruction.arg1 = replacements[instruction.arg1];
        }

        if (replacements.count(instruction.arg2)) {
            instruction.arg2 = replacements[instruction.arg2];
        }
    }

    code = optimized;
}

void deadCodeElimination(vector<Instruction>& code) {
    set<string> used;
    vector<bool> keep(code.size(), false);

    // Ordinary variable assignments are treated as observable results.
    for (const Instruction& instruction : code) {
        if (!isTemporary(instruction.lhs)) {
            if (instruction.op == "=" ||
                instruction.op == "uminus") {
                used.insert(instruction.arg1);
            }
            else {
                used.insert(instruction.arg1);
                used.insert(instruction.arg2);
            }
        }
    }

    for (int i = static_cast<int>(code.size()) - 1; i >= 0; i--) {
        const Instruction& instruction = code[i];

        if (!isTemporary(instruction.lhs) ||
            used.find(instruction.lhs) != used.end()) {

            keep[i] = true;

            if (instruction.op == "=" ||
                instruction.op == "uminus") {
                used.insert(instruction.arg1);
            }
            else {
                used.insert(instruction.arg1);
                used.insert(instruction.arg2);
            }

            used.erase(instruction.lhs);
        }
    }

    vector<Instruction> optimized;

    for (size_t i = 0; i < code.size(); i++) {
        if (keep[i]) {
            optimized.push_back(code[i]);
        }
    }

    code = optimized;
}

void displayCode(const vector<Instruction>& code) {
    if (code.empty()) {
        cout << "(No instructions)\n";
        return;
    }

    for (size_t i = 0; i < code.size(); i++) {
        const Instruction& instruction = code[i];

        cout << left << setw(4) << to_string(i + 1);

        if (instruction.op == "=") {
            cout << instruction.lhs << " = " << instruction.arg1;
        }
        else if (instruction.op == "uminus") {
            cout << instruction.lhs << " = -" << instruction.arg1;
        }
        else {
            cout << instruction.lhs << " = "
                 << instruction.arg1 << " "
                 << instruction.op << " "
                 << instruction.arg2;
        }

        cout << "\n";
    }
}

} // namespace CodeOptimization

void codeOptimizationModule() {
    using namespace CodeOptimization;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    vector<Instruction> code;
    string line;

    cout << "\n============================================\n";
    cout << "             CODE OPTIMIZATION              \n";
    cout << "============================================\n";

    cout << "\nEnter three-address code, one instruction per line.\n";
    cout << "Example:\n";
    cout << "t1 = 2 * 3\n";
    cout << "t2 = t1 + a\n";
    cout << "result = t2\n";
    cout << "Type END on a separate line when finished.\n\n";

    try {
        while (true) {
            cout << "TAC: ";

            if (!getline(cin, line)) {
                return;
            }

            if (line == "END") {
                break;
            }

            if (line.empty()) {
                continue;
            }

            code.push_back(parseInstruction(line));
        }

        if (code.empty()) {
            throw invalid_argument("No TAC instructions were entered.");
        }

        cout << "\nOriginal Three Address Code:\n";
        cout << "--------------------------------------------\n";
        displayCode(code);

        size_t originalCount = code.size();
        constantFoldingAndPropagation(code);
        algebraicSimplification(code);
        copyPropagation(code);
        commonSubexpressionElimination(code);
        deadCodeElimination(code);

        cout << "\nOptimized Three Address Code:\n";
        cout << "--------------------------------------------\n";
        displayCode(code);

        cout << "--------------------------------------------\n";
        cout << "Original instructions: " << originalCount << "\n";
        cout << "Optimized instructions: " << code.size() << "\n";

        cout << "\nOptimization completed successfully.\n";
    }
    catch (const exception& error) {
        cout << "\nError: " << error.what() << "\n";
        cout << "Code optimization failed.\n";
    }
}




