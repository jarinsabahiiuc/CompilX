#include "predictive_parsing.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <iomanip>
#include <stdexcept>
#include <cctype>
#include <limits>

using namespace std;

namespace Predictive {

using Production = vector<string>;
using Grammar = map<string, vector<Production>>;
using SymbolSets = map<string, set<string>>;
using ParsingTable = map<pair<string, string>, Production>;

string trim(const string& text) {
    size_t start = text.find_first_not_of(" \t\r\n");

    if (start == string::npos) {
        return "";
    }

    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

bool isNonTerminal(const string& symbol) {
    return !symbol.empty() &&
           symbol[0] >= 'A' &&
           symbol[0] <= 'Z';
}

vector<string> tokenize(const string& text) {
    vector<string> result;
    string value = trim(text);

    if (value == "epsilon") {
        return result;
    }

    for (size_t i = 0; i < value.size(); i++) {
        unsigned char ch = static_cast<unsigned char>(value[i]);

        if (isspace(ch)) {
            throw invalid_argument(
                "Do not put spaces between grammar symbols."
            );
        }

        string symbol(1, value[i]);

        if (value[i] >= 'A' && value[i] <= 'Z') {
            while (i + 1 < value.size() && value[i + 1] == '\'') {
                symbol += "'";
                i++;
            }
        }

        result.push_back(symbol);
    }

    return result;
}

string productionText(const Production& production) {
    if (production.empty()) {
        return "epsilon";
    }

    string result;

    for (const string& symbol : production) {
        result += symbol;
    }

    return result;
}

set<string> firstOfSequence(
    const Production& sequence,
    const SymbolSets& firstSets
) {
    set<string> result;

    if (sequence.empty()) {
        result.insert("epsilon");
        return result;
    }

    bool allNullable = true;

    for (const string& symbol : sequence) {
        if (!isNonTerminal(symbol)) {
            result.insert(symbol);
            allNullable = false;
            break;
        }

        auto it = firstSets.find(symbol);

        if (it == firstSets.end()) {
            allNullable = false;
            break;
        }

        for (const string& item : it->second) {
            if (item != "epsilon") {
                result.insert(item);
            }
        }

        if (it->second.find("epsilon") == it->second.end()) {
            allNullable = false;
            break;
        }
    }

    if (allNullable) {
        result.insert("epsilon");
    }

    return result;
}

void calculateFirst(
    const Grammar& grammar,
    SymbolSets& firstSets
) {
    for (const auto& rule : grammar) {
        firstSets[rule.first] = {};
    }

    bool changed = true;

    while (changed) {
        changed = false;

        for (const auto& rule : grammar) {
            for (const Production& production : rule.second) {
                set<string> items =
                    firstOfSequence(production, firstSets);

                for (const string& item : items) {
                    if (firstSets[rule.first].insert(item).second) {
                        changed = true;
                    }
                }
            }
        }
    }
}

void calculateFollow(
    const Grammar& grammar,
    const string& startSymbol,
    const SymbolSets& firstSets,
    SymbolSets& followSets
) {
    for (const auto& rule : grammar) {
        followSets[rule.first] = {};
    }

    followSets[startSymbol].insert("$");

    bool changed = true;

    while (changed) {
        changed = false;

        for (const auto& rule : grammar) {
            const string& lhs = rule.first;

            for (const Production& production : rule.second) {
                for (size_t i = 0; i < production.size(); i++) {
                    const string& current = production[i];

                    if (!isNonTerminal(current)) {
                        continue;
                    }

                    Production remaining;

                    for (size_t j = i + 1; j < production.size(); j++) {
                        remaining.push_back(production[j]);
                    }

                    set<string> items =
                        firstOfSequence(remaining, firstSets);

                    for (const string& item : items) {
                        if (item != "epsilon") {
                            if (followSets[current].insert(item).second) {
                                changed = true;
                            }
                        }
                    }

                    if (items.find("epsilon") != items.end()) {
                        for (const string& item : followSets[lhs]) {
                            if (followSets[current].insert(item).second) {
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
}

string stackText(const vector<string>& parsingStack) {
    string result;

    for (auto it = parsingStack.rbegin();
         it != parsingStack.rend(); ++it) {
        if (!result.empty()) {
            result += " ";
        }

        result += *it;
    }

    return result;
}

string remainingInput(
    const vector<string>& input,
    size_t position
) {
    string result;

    for (size_t i = position; i < input.size(); i++) {
        result += input[i];
    }

    return result;
}

} // namespace Predictive

void predictiveParsingModule() {
    using namespace Predictive;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    Grammar grammar;
    SymbolSets firstSets;
    SymbolSets followSets;
    ParsingTable table;

    string line;
    string startSymbol;

    cout << "\n============================================\n";
    cout << "             PREDICTIVE PARSING             \n";
    cout << "============================================\n";

    cout << "\nEnter grammar productions one per line.\n";
    cout << "Example:\n";
    cout << "E -> TR\n";
    cout << "R -> +TR | epsilon\n";
    cout << "T -> i\n";
    cout << "Type END on a separate line when finished.\n\n";

    try {
        // Read grammar.
        while (true) {
            cout << "Production: ";

            if (!getline(cin, line)) {
                return;
            }

            line = trim(line);

            if (line == "END") {
                break;
            }

            if (line.empty()) {
                continue;
            }

            size_t arrow = line.find("->");

            if (arrow == string::npos ||
                line.find("->", arrow + 2) != string::npos) {
                throw invalid_argument(
                    "Invalid production. Use the format A -> aB | b."
                );
            }

            string lhs = trim(line.substr(0, arrow));
            string rhs = trim(line.substr(arrow + 2));

            vector<string> lhsTokens = tokenize(lhs);

            if (lhsTokens.size() != 1 ||
                !isNonTerminal(lhsTokens[0])) {
                throw invalid_argument(
                    "The left side must be one non-terminal, such as A or A'."
                );
            }

            if (rhs.empty()) {
                throw invalid_argument(
                    "Use epsilon instead of an empty production."
                );
            }

            string nonTerminal = lhsTokens[0];

            if (startSymbol.empty()) {
                startSymbol = nonTerminal;
            }

            if (grammar.find(nonTerminal) == grammar.end()) {
                grammar[nonTerminal] = {};
            }

            size_t start = 0;

            while (true) {
                size_t separator = rhs.find('|', start);

                string alternative;

                if (separator == string::npos) {
                    alternative = trim(rhs.substr(start));
                } else {
                    alternative = trim(
                        rhs.substr(start, separator - start)
                    );
                }

                if (alternative.empty()) {
                    throw invalid_argument(
                        "Empty alternative found. Use epsilon explicitly."
                    );
                }

                grammar[nonTerminal].push_back(tokenize(alternative));

                if (separator == string::npos) {
                    break;
                }

                start = separator + 1;
            }
        }

        if (grammar.empty()) {
            throw invalid_argument("No grammar productions were entered.");
        }

        // Validate non-terminal references.
        for (const auto& rule : grammar) {
            for (const Production& production : rule.second) {
                for (const string& symbol : production) {
                    if (isNonTerminal(symbol) &&
                        grammar.find(symbol) == grammar.end()) {
                        throw invalid_argument(
                            "Missing production for non-terminal: " + symbol
                        );
                    }
                }
            }
        }

        // Calculate FIRST and FOLLOW.
        calculateFirst(grammar, firstSets);

        calculateFollow(
            grammar,
            startSymbol,
            firstSets,
            followSets
        );

        // Build the parsing table.
        for (const auto& rule : grammar) {
            const string& lhs = rule.first;

            for (const Production& production : rule.second) {
                set<string> items =
                    firstOfSequence(production, firstSets);

                for (const string& terminal : items) {
                    if (terminal != "epsilon") {
                        auto key = make_pair(lhs, terminal);

                        if (table.find(key) != table.end()) {
                            throw runtime_error(
                                "Grammar is not LL(1). Conflict at M[" +
                                lhs + ", " + terminal + "]."
                            );
                        }

                        table[key] = production;
                    }
                }

                if (items.find("epsilon") != items.end()) {
                    for (const string& terminal : followSets[lhs]) {
                        auto key = make_pair(lhs, terminal);

                        if (table.find(key) != table.end()) {
                            throw runtime_error(
                                "Grammar is not LL(1). Conflict at M[" +
                                lhs + ", " + terminal + "]."
                            );
                        }

                        table[key] = production;
                    }
                }
            }
        }

        // Display FIRST sets.
        cout << "\nFIRST Sets:\n";

        for (const auto& entry : firstSets) {
            cout << "FIRST(" << entry.first << ") = { ";

            size_t index = 0;

            for (const string& symbol : entry.second) {
                if (index++ > 0) {
                    cout << ", ";
                }

                cout << symbol;
            }

            cout << " }\n";
        }

        // Display FOLLOW sets.
        cout << "\nFOLLOW Sets:\n";

        for (const auto& entry : followSets) {
            cout << "FOLLOW(" << entry.first << ") = { ";

            size_t index = 0;

            for (const string& symbol : entry.second) {
                if (index++ > 0) {
                    cout << ", ";
                }

                cout << symbol;
            }

            cout << " }\n";
        }

        cout << "\nGrammar is LL(1). Parsing table created successfully.\n";

        // Read input string.
        string inputText;

        cout << "\nEnter input string to parse: ";

        if (!getline(cin, inputText)) {
            return;
        }

        if (inputText.find('$') != string::npos) {
            throw invalid_argument(
                "Do not include the end marker '$' in the input string."
            );
        }

        vector<string> input = tokenize(inputText);

        // Validate input terminals.
        set<string> terminalSet;

        for (const auto& rule : grammar) {
            for (const Production& production : rule.second) {
                for (const string& symbol : production) {
                    if (!isNonTerminal(symbol)) {
                        terminalSet.insert(symbol);
                    }
                }
            }
        }

        for (const string& symbol : input) {
            if (terminalSet.find(symbol) == terminalSet.end()) {
                throw invalid_argument(
                    "Unknown input terminal: " + symbol
                );
            }
        }

        input.push_back("$");

        // Initialize parsing stack.
        vector<string> parsingStack;

        parsingStack.push_back("$");
        parsingStack.push_back(startSymbol);

        size_t position = 0;
        int step = 1;
        const int MAX_STEPS = 10000;

        bool accepted = false;
        bool failed = false;

        cout << "\nPREDICTIVE PARSING TRACE\n";
        cout << "============================================================\n";

        cout << left
             << setw(7) << "Step"
             << setw(24) << "Stack"
             << setw(20) << "Input"
             << "Action\n";

        cout << "------------------------------------------------------------\n";

        while (!parsingStack.empty() && step <= MAX_STEPS) {
            string top = parsingStack.back();
            string currentInput = input[position];

            string action;

            if (top == "$" && currentInput == "$") {
                action = "ACCEPT";
                accepted = true;

                cout << left
                     << setw(7) << step
                     << setw(24) << stackText(parsingStack)
                     << setw(20) << remainingInput(input, position)
                     << action << "\n";

                break;
            }

            if (!isNonTerminal(top)) {
                if (top == currentInput) {
                    action = "Match " + top;
                    parsingStack.pop_back();
                    position++;
                } else {
                    action = "ERROR: Terminal mismatch";
                    failed = true;
                }
            } else {
                auto it = table.find({top, currentInput});

                if (it == table.end()) {
                    action = "ERROR: No table entry";
                    failed = true;
                } else {
                    Production production = it->second;

                    action = top + " -> " + productionText(production);

                    parsingStack.pop_back();

                    for (auto rit = production.rbegin();
                         rit != production.rend(); ++rit) {
                        parsingStack.push_back(*rit);
                    }
                }
            }

            cout << left
                 << setw(7) << step
                 << setw(24) << stackText(parsingStack);

            // Display input from the current position.
            cout << setw(20) << remainingInput(input, position);
            cout << action << "\n";

            if (failed) {
                break;
            }

            step++;
        }

        cout << "============================================================\n";

        if (step > MAX_STEPS) {
            cout << "\nParsing stopped: maximum step limit exceeded.\n";
        } else if (accepted) {
            cout << "\nResult: INPUT ACCEPTED.\n";
        } else {
            cout << "\nResult: INPUT REJECTED.\n";
        }

        cout << "Predictive Parsing completed.\n";

    } catch (const exception& error) {
        cout << "\nError: " << error.what() << "\n";
        cout << "Predictive Parsing could not be completed.\n";
    }
}
