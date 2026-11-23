#include "follow_calculation.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <stdexcept>
#include <cctype>
#include <limits>

using namespace std;

using FollowProduction = vector<string>;
using FollowGrammar = map<string, vector<FollowProduction>>;
using FollowSets = map<string, set<string>>;

string followTrim(const string& text) {
    size_t start = text.find_first_not_of(" \t\r\n");

    if (start == string::npos) {
        return "";
    }

    size_t end = text.find_last_not_of(" \t\r\n");

    return text.substr(start, end - start + 1);
}

bool followIsNonTerminal(const string& symbol) {
    return !symbol.empty() &&
           symbol[0] >= 'A' &&
           symbol[0] <= 'Z';
}

vector<string> followTokenize(const string& text) {
    vector<string> symbols;
    string value = followTrim(text);

    if (value == "epsilon") {
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

set<string> followFirstOfSequence(
    const FollowProduction& sequence,
    const FollowSets& firstSets
) {
    set<string> result;

    if (sequence.empty()) {
        result.insert("epsilon");
        return result;
    }

    bool allNullable = true;

    for (const string& symbol : sequence) {

        if (!followIsNonTerminal(symbol)) {
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

void followCalculationModule() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    FollowGrammar grammar;
    FollowSets firstSets;
    FollowSets followSets;

    string line;
    string startSymbol;

    cout << "\n============================================\n";
    cout << "             FOLLOW CALCULATION             \n";
    cout << "============================================\n";

    cout << "\nEnter grammar productions one per line.\n";
    cout << "Example:\n";
    cout << "E -> TR\n";
    cout << "R -> +TR | epsilon\n";
    cout << "T -> i\n";
    cout << "Type END on a separate line when finished.\n\n";

    try {
        while (true) {
            cout << "Production: ";

            if (!getline(cin, line)) {
                return;
            }

            line = followTrim(line);

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

            string lhs = followTrim(line.substr(0, arrow));
            string rhs = followTrim(line.substr(arrow + 2));

            vector<string> lhsTokens = followTokenize(lhs);

            if (lhsTokens.size() != 1 ||
                !followIsNonTerminal(lhsTokens[0])) {
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
                    alternative = followTrim(rhs.substr(start));
                } else {
                    alternative = followTrim(
                        rhs.substr(start, separator - start)
                    );
                }

                if (alternative.empty()) {
                    throw invalid_argument(
                        "Empty alternative found. Use epsilon explicitly."
                    );
                }

                grammar[nonTerminal].push_back(
                    followTokenize(alternative)
                );

                if (separator == string::npos) {
                    break;
                }

                start = separator + 1;
            }
        }

        if (grammar.empty()) {
            throw invalid_argument("No grammar productions were entered.");
        }

        // Validate referenced non-terminals.
        for (const auto& rule : grammar) {
            for (const FollowProduction& production : rule.second) {
                for (const string& symbol : production) {
                    if (followIsNonTerminal(symbol) &&
                        grammar.find(symbol) == grammar.end()) {
                        throw invalid_argument(
                            "Missing production for non-terminal: " + symbol
                        );
                    }
                }
            }
        }

        // Initialize FIRST and FOLLOW sets.
        for (const auto& rule : grammar) {
            firstSets[rule.first] = {};
            followSets[rule.first] = {};
        }

        // Calculate FIRST sets.
        bool firstChanged = true;

        while (firstChanged) {
            firstChanged = false;

            for (const auto& rule : grammar) {
                for (const FollowProduction& production : rule.second) {
                    set<string> items =
                        followFirstOfSequence(production, firstSets);

                    for (const string& item : items) {
                        if (firstSets[rule.first].insert(item).second) {
                            firstChanged = true;
                        }
                    }
                }
            }
        }

        // The start symbol always contains the end marker.
        followSets[startSymbol].insert("$");

        // Calculate FOLLOW sets until no set changes.
        bool followChanged = true;

        while (followChanged) {
            followChanged = false;

            for (const auto& rule : grammar) {
                const string& lhs = rule.first;

                for (const FollowProduction& production : rule.second) {

                    for (size_t i = 0; i < production.size(); i++) {
                        const string& current = production[i];

                        if (!followIsNonTerminal(current)) {
                            continue;
                        }

                        FollowProduction remaining;

                        for (size_t j = i + 1; j < production.size(); j++) {
                            remaining.push_back(production[j]);
                        }

                        set<string> firstOfRemaining =
                            followFirstOfSequence(remaining, firstSets);

                        // Add FIRST(beta) except epsilon.
                        for (const string& item : firstOfRemaining) {
                            if (item != "epsilon") {
                                if (followSets[current].insert(item).second) {
                                    followChanged = true;
                                }
                            }
                        }

                        // If beta is nullable, add FOLLOW(lhs).
                        if (firstOfRemaining.find("epsilon") !=
                            firstOfRemaining.end()) {

                            for (const string& item : followSets[lhs]) {
                                if (followSets[current].insert(item).second) {
                                    followChanged = true;
                                }
                            }
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

        cout << "\nFOLLOW Sets:\n";
        cout << "============================================\n";

        for (const auto& entry : followSets) {
            cout << "FOLLOW(" << entry.first << ") = { ";

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
        cout << "FOLLOW Calculation completed successfully.\n";
        cout << "Start Symbol: " << startSymbol << "\n";
        cout << "Processed " << grammar.size()
             << " non-terminal(s).\n";

    } catch (const exception& error) {
        cout << "\nError: " << error.what() << "\n";
        cout << "FOLLOW Calculation could not be completed.\n";
    }
}
