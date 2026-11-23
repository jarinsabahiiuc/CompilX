#include "ll1_parsing_table.h"

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

namespace LL1 {

using Production = vector<string>;
using Grammar = map<string, vector<Production>>;
using SymbolSets = map<string, set<string>>;
using ParsingTable = map<pair<string, string>, vector<string>>;

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

} // namespace LL1

void ll1ParsingTableModule() {
    using namespace LL1;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    Grammar grammar;
    SymbolSets firstSets;
    SymbolSets followSets;
    ParsingTable table;

    string line;
    string startSymbol;

    cout << "\n============================================\n";
    cout << "             LL(1) PARSING TABLE            \n";
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

        // Validate all referenced non-terminals.
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

        // Calculate FIRST and FOLLOW sets.
        calculateFirst(grammar, firstSets);

        calculateFollow(
            grammar,
            startSymbol,
            firstSets,
            followSets
        );

        // Collect all terminal symbols.
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

        terminalSet.erase("epsilon");
        terminalSet.insert("$");

        vector<string> terminals(
            terminalSet.begin(),
            terminalSet.end()
        );

        // Construct the LL(1) parsing table.
        for (const auto& rule : grammar) {
            const string& lhs = rule.first;

            for (const Production& production : rule.second) {
                string productionString =
                    lhs + " -> " + productionText(production);

                set<string> productionFirst =
                    firstOfSequence(production, firstSets);

                for (const string& terminal : productionFirst) {
                    if (terminal != "epsilon") {
                        table[{lhs, terminal}].push_back(
                            productionString
                        );
                    }
                }

                if (productionFirst.find("epsilon") !=
                    productionFirst.end()) {

                    for (const string& terminal : followSets[lhs]) {
                        table[{lhs, terminal}].push_back(
                            productionString
                        );
                    }
                }
            }
        }

        cout << "\nFIRST Sets:\n";
        cout << "============================================\n";

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

        cout << "\nFOLLOW Sets:\n";
        cout << "============================================\n";

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

        cout << "\nLL(1) PARSING TABLE:\n";
        cout << "============================================\n";

        const int columnWidth = 18;

        cout << left << setw(columnWidth) << "Non-Terminal";

        for (const string& terminal : terminals) {
            cout << setw(columnWidth) << terminal;
        }

        cout << "\n";

        cout << string(
            columnWidth * (terminals.size() + 1),
            '-'
        ) << "\n";

        bool hasConflict = false;

        for (const auto& rule : grammar) {
            cout << left << setw(columnWidth) << rule.first;

            for (const string& terminal : terminals) {
                auto it = table.find({rule.first, terminal});

                if (it == table.end()) {
                    cout << setw(columnWidth) << "-";
                } else if (it->second.size() == 1) {
                    cout << setw(columnWidth) << it->second[0];
                } else {
                    cout << setw(columnWidth) << "CONFLICT";
                    hasConflict = true;
                }
            }

            cout << "\n";
        }

        cout << "============================================\n";

        if (hasConflict) {
            cout << "\nGrammar is NOT LL(1).\n";
            cout << "Multiple productions occupy at least one table cell.\n";

            cout << "\nConflicting entries:\n";

            for (const auto& entry : table) {
                if (entry.second.size() > 1) {
                    cout << "M[" << entry.first.first
                         << ", " << entry.first.second << "]: ";

                    for (size_t i = 0; i < entry.second.size(); i++) {
                        if (i > 0) {
                            cout << " ; ";
                        }

                        cout << entry.second[i];
                    }

                    cout << "\n";
                }
            }
        } else {
            cout << "\nGrammar is LL(1).\n";
            cout << "No parsing table conflicts detected.\n";
        }

        cout << "\nLL(1) Parsing Table construction completed.\n";
        cout << "Start Symbol: " << startSymbol << "\n";

    } catch (const exception& error) {
        cout << "\nError: " << error.what() << "\n";
        cout << "LL(1) Parsing Table construction failed.\n";
    }
}
