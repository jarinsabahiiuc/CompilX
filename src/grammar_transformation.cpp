#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include "grammar_transformation.h"

using namespace std;

string trim(const string& text) {
    size_t start = text.find_first_not_of(" \t\r\n");

    if (start == string::npos) {
        return "";
    }

    size_t end = text.find_last_not_of(" \t\r\n");

    return text.substr(start, end - start + 1);
}

vector<string> splitAlternatives(const string& expression) {
    vector<string> alternatives;
    string current;

    for (char symbol : expression) {
        if (symbol == '|') {
            string alternative = trim(current);

            if (alternative.empty()) {
                throw invalid_argument(
                    "Empty production alternative found."
                );
            }

            alternatives.push_back(alternative);
            current.clear();
        }
        else {
            current += symbol;
        }
    }

    string alternative = trim(current);

    if (alternative.empty()) {
        throw invalid_argument(
            "Empty production alternative found."
        );
    }

    alternatives.push_back(alternative);

    for (const string& item : alternatives) {
        for (char symbol : item) {
            if (isspace(static_cast<unsigned char>(symbol))) {
                throw invalid_argument(
                    "Use single-character grammar symbols without "
                    "spaces inside an alternative."
                );
            }
        }
    }

    return alternatives;
}

void displayProduction(
    const string& nonTerminal,
    const vector<string>& alternatives
) {
    cout << nonTerminal << " -> ";

    for (size_t i = 0; i < alternatives.size(); i++) {
        if (i > 0) {
            cout << " | ";
        }

        cout << (alternatives[i].empty() ? "epsilon" : alternatives[i]);
    }

    cout << "\n";
}

void factorProduction(
    const string& nonTerminal,
    vector<string> alternatives,
    vector<pair<string, vector<string>>>& result,
    int& generatedCount
) {
    while (true) {
        string bestPrefix;

        // Find the longest common prefix among any two alternatives.
        for (size_t i = 0; i < alternatives.size(); i++) {
            for (size_t j = i + 1; j < alternatives.size(); j++) {

                size_t length = 0;

                while (
                    length < alternatives[i].length() &&
                    length < alternatives[j].length() &&
                    alternatives[i][length] == alternatives[j][length]
                ) {
                    length++;
                }

                if (length > bestPrefix.length()) {
                    bestPrefix = alternatives[i].substr(0, length);
                }
            }
        }

        // No common prefix remains.
        if (bestPrefix.empty()) {
            break;
        }

        generatedCount++;

        string newNonTerminal =
            nonTerminal + string(generatedCount, '\'');

        vector<string> remainingAlternatives;
        vector<string> newAlternatives;

        bool inserted = false;

        // Group all alternatives that share the selected prefix.
        for (const string& alternative : alternatives) {

            if (
                alternative.compare(
                    0,
                    bestPrefix.length(),
                    bestPrefix
                ) == 0
            ) {
                newAlternatives.push_back(
                    alternative.substr(bestPrefix.length())
                );

                if (!inserted) {
                    remainingAlternatives.push_back(
                        bestPrefix + newNonTerminal
                    );

                    inserted = true;
                }
            }
            else {
                remainingAlternatives.push_back(alternative);
            }
        }

        alternatives = remainingAlternatives;

        // Process the newly generated production separately.
        result.push_back({
            newNonTerminal,
            newAlternatives
        });
    }

    result.insert(
        result.begin(),
        { nonTerminal, alternatives }
    );
}

void leftFactoringModule() {

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "\n============================================\n";
    cout << "                LEFT FACTORING              \n";
    cout << "============================================\n";

    cout << "\nEnter the non-terminal (example: A): ";

    string nonTerminal;
    getline(cin, nonTerminal);

    nonTerminal = trim(nonTerminal);

    if (
        nonTerminal.length() != 1 ||
        !isupper(static_cast<unsigned char>(nonTerminal[0]))
    ) {
        cout << "\nError: Enter one uppercase non-terminal.\n";
        return;
    }

    cout << "\nEnter production alternatives.\n";
    cout << "Example: ab | ac | ad\n";
    cout << "Production RHS: ";

    string expression;
    getline(cin, expression);

    try {
        vector<string> alternatives =
            splitAlternatives(expression);

        cout << "\nOriginal Production:\n";
        displayProduction(nonTerminal, alternatives);

        vector<pair<string, vector<string>>> result;

        int generatedCount = 0;

        factorProduction(
            nonTerminal,
            alternatives,
            result,
            generatedCount
        );

        cout << "\nLeft Factoring Result:\n";
        cout << "============================================\n";

        for (const auto& production : result) {
            displayProduction(
                production.first,
                production.second
            );
        }

        cout << "============================================\n";
        cout << "\nLeft Factoring completed successfully.\n";
    }
    catch (const exception& error) {
        cout << "\nError: " << error.what() << "\n";
    }
}
