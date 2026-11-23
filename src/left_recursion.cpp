#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <cctype>
#include <stdexcept>
#include <algorithm>

#include "left_recursion.h"

using namespace std;

struct GrammarRule {
    string nonTerminal;
    vector<vector<string>> alternatives;
};

string lrTrim(const string& text) {
    size_t start = text.find_first_not_of(" \t\r\n");

    if (start == string::npos) {
        return "";
    }

    size_t end = text.find_last_not_of(" \t\r\n");

    return text.substr(start, end - start + 1);
}

vector<string> lrTokenize(const string& alternative) {

    string value = lrTrim(alternative);

    if (value == "epsilon") {
        return {};
    }

    if (value.empty()) {
        throw invalid_argument("Empty production alternative found.");
    }

    vector<string> symbols;

    for (size_t i = 0; i < value.length(); i++) {

        if (isspace(static_cast<unsigned char>(value[i]))) {
            throw invalid_argument(
                "Spaces are not allowed inside production alternatives."
            );
        }

        string symbol(1, value[i]);

        // Uppercase letters represent non-terminals.
        // Apostrophes are included in generated non-terminal names.
        if (isupper(static_cast<unsigned char>(value[i]))) {

            while (
                i + 1 < value.length() &&
                value[i + 1] == '\''
            ) {
                symbol += "'";
                i++;
            }
        }

        symbols.push_back(symbol);
    }

    return symbols;
}

vector<vector<string>> lrSplitAlternatives(const string& expression) {

    vector<vector<string>> alternatives;
    string current;

    for (char symbol : expression) {

        if (symbol == '|') {
            alternatives.push_back(lrTokenize(current));
            current.clear();
        }
        else {
            current += symbol;
        }
    }

    alternatives.push_back(lrTokenize(current));

    return alternatives;
}

void lrDisplayProduction(const GrammarRule& rule) {

    cout << rule.nonTerminal << " -> ";

    for (size_t i = 0; i < rule.alternatives.size(); i++) {

        if (i > 0) {
            cout << " | ";
        }

        const vector<string>& alternative =
            rule.alternatives[i];

        if (alternative.empty()) {
            cout << "epsilon";
        }
        else {
            for (const string& symbol : alternative) {
                cout << symbol;
            }
        }
    }

    cout << "\n";
}

vector<string> lrConcatenate(
    const vector<string>& first,
    const vector<string>& second
) {
    vector<string> result = first;

    result.insert(
        result.end(),
        second.begin(),
        second.end()
    );

    return result;
}

void lrEliminateImmediate(GrammarRule& rule) {

    vector<vector<string>> recursive;
    vector<vector<string>> nonRecursive;

    for (const auto& alternative : rule.alternatives) {

        if (
            !alternative.empty() &&
            alternative[0] == rule.nonTerminal
        ) {
            vector<string> suffix(
                alternative.begin() + 1,
                alternative.end()
            );

            if (!suffix.empty()) {
                recursive.push_back(suffix);
            }
        }
        else {
            nonRecursive.push_back(alternative);
        }
    }

    if (recursive.empty()) {
        return;
    }

    if (nonRecursive.empty()) {
        throw runtime_error(
            "Cannot eliminate left recursion in " +
            rule.nonTerminal +
            ": no non-recursive alternative exists."
        );
    }

    string newNonTerminal = rule.nonTerminal + "'";

    vector<vector<string>> newMain;
    vector<vector<string>> newRecursive;

    // A -> beta A'
    for (const auto& beta : nonRecursive) {

        vector<string> transformed =
            beta;

        transformed.push_back(newNonTerminal);

        newMain.push_back(transformed);
    }

    // A' -> alpha A' | epsilon
    for (const auto& alpha : recursive) {

        vector<string> transformed =
            alpha;

        transformed.push_back(newNonTerminal);

        newRecursive.push_back(transformed);
    }

    newRecursive.push_back({});

    rule.alternatives = newMain;

    // Generated rules are returned through the caller.
    // Store them in a temporary global-free result below.
    // The caller creates the corresponding GrammarRule.
    static vector<GrammarRule> generatedRules;

    generatedRules.push_back({
        newNonTerminal,
        newRecursive
    });
}

void leftRecursionModule() {

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "\n============================================\n";
    cout << "          LEFT RECURSION ELIMINATION        \n";
    cout << "============================================\n";

    cout << "\nEnter grammar productions one per line.\n";
    cout << "Example:\n";
    cout << "A -> Ba | c\n";
    cout << "B -> Ab | d\n";
    cout << "Type END on a separate line when finished.\n\n";

    vector<GrammarRule> grammar;
    string line;

    try {

        while (true) {

            cout << "Production: ";

            if (!getline(cin, line)) {
                return;
            }

            line = lrTrim(line);

            if (line == "END") {
                break;
            }

            if (line.empty()) {
                continue;
            }

            size_t arrow = line.find("->");

            if (arrow == string::npos) {
                throw invalid_argument(
                    "Invalid production. Use the format A -> a | b."
                );
            }

            string lhs = lrTrim(line.substr(0, arrow));
            string rhs = lrTrim(line.substr(arrow + 2));

            if (
                lhs.length() != 1 ||
                !isupper(static_cast<unsigned char>(lhs[0]))
            ) {
                throw invalid_argument(
                    "Each original non-terminal must be one uppercase letter."
                );
            }

            for (const auto& existing : grammar) {

                if (existing.nonTerminal == lhs) {
                    throw invalid_argument(
                        "Duplicate production for " + lhs + "."
                    );
                }
            }

            if (rhs.empty()) {
                throw invalid_argument(
                    "Production RHS cannot be empty. Use epsilon instead."
                );
            }

            grammar.push_back({
                lhs,
                lrSplitAlternatives(rhs)
            });

            if (grammar.size() > 26) {
                throw invalid_argument(
                    "Maximum of 26 original non-terminals supported."
                );
            }
        }

        if (grammar.empty()) {
            throw invalid_argument("No grammar productions entered.");
        }

        cout << "\nOriginal Grammar:\n";
        cout << "============================================\n";

        for (const auto& rule : grammar) {
            lrDisplayProduction(rule);
        }

        cout << "============================================\n";

        vector<GrammarRule> generatedRules;

        // Process non-terminals in their input order.
        for (size_t i = 0; i < grammar.size(); i++) {

            // Substitute productions of all earlier non-terminals.
            for (size_t j = 0; j < i; j++) {

                vector<vector<string>> updatedAlternatives;

                for (const auto& alternative :
                     grammar[i].alternatives) {

                    if (
                        !alternative.empty() &&
                        alternative[0] == grammar[j].nonTerminal
                    ) {

                        vector<string> suffix(
                            alternative.begin() + 1,
                            alternative.end()
                        );

                        for (const auto& earlierAlternative :
                             grammar[j].alternatives) {

                            updatedAlternatives.push_back(
                                lrConcatenate(
                                    earlierAlternative,
                                    suffix
                                )
                            );
                        }
                    }
                    else {
                        updatedAlternatives.push_back(alternative);
                    }

                    if (updatedAlternatives.size() > 5000) {
                        throw runtime_error(
                            "Grammar expansion exceeded the 5000-alternative limit."
                        );
                    }
                }

                grammar[i].alternatives =
                    updatedAlternatives;
            }

            // Eliminate immediate left recursion after substitution.
            vector<vector<string>> recursive;
            vector<vector<string>> nonRecursive;

            for (const auto& alternative :
                 grammar[i].alternatives) {

                if (
                    !alternative.empty() &&
                    alternative[0] == grammar[i].nonTerminal
                ) {

                    vector<string> suffix(
                        alternative.begin() + 1,
                        alternative.end()
                    );

                    if (!suffix.empty()) {
                        recursive.push_back(suffix);
                    }
                }
                else {
                    nonRecursive.push_back(alternative);
                }
            }

            if (!recursive.empty()) {

                if (nonRecursive.empty()) {
                    throw runtime_error(
                        "Cannot eliminate left recursion in " +
                        grammar[i].nonTerminal +
                        ": no non-recursive alternative exists."
                    );
                }

                string newNonTerminal =
                    grammar[i].nonTerminal + "'";

                vector<vector<string>> newMain;
                vector<vector<string>> newRecursive;

                for (const auto& beta : nonRecursive) {

                    vector<string> transformed = beta;
                    transformed.push_back(newNonTerminal);

                    newMain.push_back(transformed);
                }

                for (const auto& alpha : recursive) {

                    vector<string> transformed = alpha;
                    transformed.push_back(newNonTerminal);

                    newRecursive.push_back(transformed);
                }

                newRecursive.push_back({});

                grammar[i].alternatives = newMain;

                generatedRules.push_back({
                    newNonTerminal,
                    newRecursive
                });
            }
        }

        cout << "\nLeft Recursion Elimination Result:\n";
        cout << "============================================\n";

        for (const auto& rule : grammar) {
            lrDisplayProduction(rule);
        }

        for (const auto& rule : generatedRules) {
            lrDisplayProduction(rule);
        }

        cout << "============================================\n";
        cout << "\nLeft Recursion Elimination completed successfully.\n";
        cout << "Processed " << grammar.size()
             << " original non-terminal(s).\n";

    }
    catch (const exception& error) {
        cout << "\nError: " << error.what() << "\n";
    }
}
