#include "first_calculation.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <stdexcept>
#include <cctype>

using namespace std;

using Production = vector<string>;
using Grammar = map<string, vector<Production>>;
using FirstSets = map<string, set<string>>;

string trimText(const string& text) {
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

vector<string> tokenizeProduction(const string& text) {
    vector<string> symbols;
    string value = trimText(text);

    if (value == "epsilon" || value == "e") {
        return symbols;
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

        symbols.push_back(symbol);
    }

    return symbols;
}

set<string> calculateSequenceFirst(
    const Production& production,
    const FirstSets& firstSets
) {
    set<string> result;

    if (production.empty()) {
        result.insert("epsilon");
        return result;
    }

    bool allNullable = true;

    for (const string& symbol : production) {

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

void firstCalculationModule() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    Grammar grammar;
    FirstSets firstSets;

    string line;

    cout << "\n============================================\n";
    cout << "             FIRST CALCULATION              \n";
    cout << "============================================\n";

    cout << "\nEnter grammar productions one per line.\n";
    cout << "Example:\n";
    cout << "A -> aB | b\n";
    cout << "B -> c | epsilon\n";
    cout << "Type END on a separate line when finished.\n\n";

    try {
        while (true) {
            cout << "Production: ";

            if (!getline(cin, line)) {
                return;
            }

            line = trimText(line);

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

            string lhs = trimText(line.substr(0, arrow));
            string rhs = trimText(line.substr(arrow + 2));

            vector<string> lhsTokens = tokenizeProduction(lhs);

            if (lhsTokens.size() != 1 ||
                !isNonTerminal(lhsTokens[0])) {
                throw invalid_argument(
                    "The left side must be one non-terminal, such as A or A'."
                );
            }

            if (rhs.empty()) {
                throw invalid_argument(
                    "Production cannot have an empty right side. Use epsilon."
                );
            }

            string nonTerminal = lhsTokens[0];

            if (grammar.find(nonTerminal) == grammar.end()) {
                grammar[nonTerminal] = {};
            }

            size_t start = 0;

            while (true) {
                size_t separator = rhs.find('|', start);

                string alternative;

                if (separator == string::npos) {
                    alternative = trimText(rhs.substr(start));
                } else {
                    alternative = trimText(
                        rhs.substr(start, separator - start)
                    );
                }

                if (alternative.empty()) {
                    throw invalid_argument(
                        "Empty alternative found. Use epsilon explicitly."
                    );
                }

                Production production =
                    tokenizeProduction(alternative);

                grammar[nonTerminal].push_back(production);

                if (separator == string::npos) {
                    break;
                }

                start = separator + 1;
            }
        }

        if (grammar.empty()) {
            throw invalid_argument("No grammar productions were entered.");
        }

        // Check that every referenced non-terminal has a production.
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

        // Initialize FIRST sets.
        for (const auto& rule : grammar) {
            firstSets[rule.first] = {};
        }

        // Calculate FIRST sets until no set changes.
        bool changed = true;

        while (changed) {
            changed = false;

            for (const auto& rule : grammar) {
                const string& nonTerminal = rule.first;

                for (const Production& production : rule.second) {
                    set<string> newItems =
                        calculateSequenceFirst(production, firstSets);

                    for (const string& item : newItems) {
                        if (firstSets[nonTerminal].insert(item).second) {
                            changed = true;
                        }
                    }
                }
            }
        }

        cout << "\nOriginal Grammar:\n";
        cout << "============================================\n";

        for (const auto& rule : grammar) {
            cout << rule.first << " -> ";

            for (size_t i = 0; i < rule.second.size(); i++) {
                if (i > 0) {
                    cout << " | ";
                }

                if (rule.second[i].empty()) {
                    cout << "epsilon";
                } else {
                    for (const string& symbol : rule.second[i]) {
                        cout << symbol;
                    }
                }
            }

            cout << "\n";
        }

        cout << "============================================\n";

        cout << "\nFIRST Sets:\n";
        cout << "============================================\n";

        for (const auto& entry : firstSets) {
            cout << "FIRST(" << entry.first << ") = { ";

            size_t index = 0;

            for (const string& symbol : entry.second) {
                if (index > 0) {
                    cout << ", ";
                }

                cout << symbol;
                index++;
            }

            cout << " }\n";
        }

        cout << "============================================\n";
        cout << "FIRST Calculation completed successfully.\n";
        cout << "Processed " << grammar.size()
             << " non-terminal(s).\n";

    } catch (const exception& error) {
        cout << "\nError: " << error.what() << "\n";
        cout << "FIRST Calculation could not be completed.\n";
    }
}
