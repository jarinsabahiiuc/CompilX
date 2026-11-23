#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "comment_remover.h"
#include "lexical_analyzer.h"
#include "language_recognition.h"
#include "regex_to_nfa.h"
#include "grammar_transformation.h"
#include "left_recursion.h"
#include "first_calculation.h"
#include "follow_calculation.h"
#include "ll1_parsing_table.h"
#include "predictive_parsing.h"
#include "quadruple_generation.h"
#include "code_optimization.h"
#include "three_address_code.h"

using namespace std;

void displayMenu() {
    cout << "\n============================================\n";
    cout << "              COMPILX TOOLKIT               \n";
    cout << "============================================\n";
    cout << "1. Comment Removal\n";
    cout << "2. Lexical Analyzer\n";
    cout << "3. Formal Language & Automata\n";
    cout << "4. Regular Expression to NFA/DFA\n";
    cout << "5. Left Factoring\n";
    cout << "6. Left Recursion Elimination\n";
    cout << "7. FIRST Calculation\n";
    cout << "8. FOLLOW Calculation\n";
    cout << "9. LL(1) Parsing Table\n";
    cout << "10. Predictive Parsing\n";
    cout << "11. Three Address Code\n";
    cout << "12. Quadruple Generation\n";
    cout << "13. Code Optimization\n";
    cout << "0. Exit\n";
    cout << "============================================\n";
    cout << "Enter your choice: ";
}

void commentRemovalModule() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string source;
    string line;

    cout << "\nEnter your source code.";
    cout << "\nType END on a separate line to finish.\n\n";

    while (getline(cin, line)) {
        if (line == "END") {
            break;
        }

        source += line + "\n";
    }

    string result = removeComments(source);

    cout << "\n========== CLEANED SOURCE CODE ==========\n";
    cout << result;
    cout << "=========================================\n";
}

void lexicalAnalyzerModule() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string source;
    string line;

    cout << "\nEnter your source code.";
    cout << "\nType END on a separate line to finish.\n\n";

    while (getline(cin, line)) {
        if (line == "END") {
            break;
        }

        source += line + "\n";
    }

    vector<Token> tokens = analyzeLexically(source);

    cout << "\n========== LEXICAL ANALYSIS RESULT ==========\n";
    cout << "Lexeme\t\tToken Type\n";
    cout << "---------------------------------------------\n";

    for (const Token& token : tokens) {
        cout << token.lexeme << "\t\t" << token.type << "\n";
    }

    cout << "=============================================\n";
    cout << "Total tokens: " << tokens.size() << "\n";
}

int main() {
    int choice;

    do {
        displayMenu();

        if (!(cin >> choice)) {
            cout << "\nInvalid input. Please enter a number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
            case 1:
                commentRemovalModule();
                break;

            case 2:
                lexicalAnalyzerModule();
                break;

            case 3:
                languageRecognitionModule();
                break;

            case 4:
                regexToNfaModule();
                break;

            case 5:
                leftFactoringModule();
                break;

            case 6:
                leftRecursionModule();
                break;

            case 7:
                firstCalculationModule();
                break;

            case 8:
                followCalculationModule();
                break;

            case 9:
                ll1ParsingTableModule();
                break;

            case 10:
                predictiveParsingModule();
                break;

            case 11:
                threeAddressCodeModule();
                break;

            case 12:
                quadrupleGenerationModule();
                break;

            case 13:
                codeOptimizationModule();
                break;

            case 0:
                cout << "\nThank you for using CompilX!\n";
                break;

            default:
                cout << "\nThis module has not been implemented yet.\n";
        }

    } while (choice != 0);

    return 0;
}











