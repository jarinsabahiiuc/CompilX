#include "language_recognition.h"

#include <iostream>
#include <algorithm>
#include <iomanip>
#include <stdexcept>
#include <limits>
#include <set>

using namespace std;

DFA::DFA(int states, const string& symbols) {
    if (states <= 0) {
        throw invalid_argument("DFA must have at least one state.");
    }

    if (symbols.empty()) {
        throw invalid_argument("Alphabet cannot be empty.");
    }

    set<char> uniqueSymbols(symbols.begin(), symbols.end());

    if (uniqueSymbols.size() != symbols.size()) {
        throw invalid_argument("Alphabet symbols must be unique.");
    }

    stateCount = states;
    alphabet = symbols;
    startState = 0;

    transitions.assign(
        stateCount,
        vector<int>(alphabet.length(), -1)
    );
}

void DFA::setStartState(int state) {
    if (state < 0 || state >= stateCount) {
        throw out_of_range("Invalid start state.");
    }

    startState = state;
}

void DFA::addFinalState(int state) {
    if (state < 0 || state >= stateCount) {
        throw out_of_range("Invalid final state.");
    }

    if (find(finalStates.begin(), finalStates.end(), state)
        == finalStates.end()) {
        finalStates.push_back(state);
    }
}

void DFA::setTransition(int state, char symbol, int nextState) {
    if (state < 0 || state >= stateCount ||
        nextState < 0 || nextState >= stateCount) {
        throw out_of_range("Invalid transition state.");
    }

    size_t position = alphabet.find(symbol);

    if (position == string::npos) {
        throw invalid_argument("Symbol is not in the alphabet.");
    }

    transitions[state][position] = nextState;
}

bool DFA::accepts(const string& input) const {
    int currentState = startState;

    for (char symbol : input) {
        size_t position = alphabet.find(symbol);

        if (position == string::npos) {
            return false;
        }

        int nextState = transitions[currentState][position];

        if (nextState == -1) {
            return false;
        }

        currentState = nextState;
    }

    return find(
        finalStates.begin(),
        finalStates.end(),
        currentState
    ) != finalStates.end();
}

void DFA::displayTransitionTable() const {
    cout << "\n========== DFA TRANSITION TABLE ==========\n";

    cout << left << setw(12) << "State";

    for (char symbol : alphabet) {
        cout << setw(12) << symbol;
    }

    cout << "\n";
    cout << "------------------------------------------\n";

    for (int state = 0; state < stateCount; state++) {

        string stateLabel = "q" + to_string(state);

        if (state == startState) {
            stateLabel += " ->";
        }

        if (find(finalStates.begin(), finalStates.end(), state)
            != finalStates.end()) {
            stateLabel += " *";
        }

        cout << left << setw(12) << stateLabel;

        for (size_t j = 0; j < alphabet.length(); j++) {

            if (transitions[state][j] == -1) {
                cout << setw(12) << "-";
            }
            else {
                string next = "q" + to_string(transitions[state][j]);
                cout << setw(12) << next;
            }
        }

        cout << "\n";
    }

    cout << "==========================================\n";
}

void dfaRecognitionModule() {
    int stateCount;
    string alphabet;

    cout << "\n============================================\n";
    cout << "       FORMAL LANGUAGE & AUTOMATA            \n";
    cout << "============================================\n";

    cout << "\nEnter number of states: ";

    if (!(cin >> stateCount) || stateCount <= 0 || stateCount > 100) {
        cout << "Invalid number of states.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cout << "Enter alphabet symbols without spaces (e.g. 01 or ab): ";
    cin >> alphabet;

    if (alphabet.empty() || alphabet.length() > 26) {
        cout << "Invalid alphabet.\n";
        return;
    }

    try {
        DFA dfa(stateCount, alphabet);

        int startState;

        cout << "Enter initial state (0 to " << stateCount - 1 << "): ";

        if (!(cin >> startState)) {
            throw invalid_argument("Invalid initial state input.");
        }

        dfa.setStartState(startState);

        int finalCount;

        cout << "Enter number of final states: ";

        if (!(cin >> finalCount) || finalCount < 0 ||
            finalCount > stateCount) {
            throw invalid_argument("Invalid number of final states.");
        }

        cout << "Enter final state numbers: ";

        for (int i = 0; i < finalCount; i++) {
            int finalState;

            if (!(cin >> finalState)) {
                throw invalid_argument("Invalid final state input.");
            }

            dfa.addFinalState(finalState);
        }

        cout << "\nEnter transitions for each state and symbol.\n";
        cout << "Use state numbers (0 to " << stateCount - 1 << ").\n\n";

        for (int state = 0; state < stateCount; state++) {

            for (char symbol : alphabet) {

                int nextState;

                cout << "Transition from q" << state
                     << " on '" << symbol << "' -> ";

                if (!(cin >> nextState)) {
                    throw invalid_argument("Invalid transition input.");
                }

                dfa.setTransition(state, symbol, nextState);
            }
        }

        dfa.displayTransitionTable();

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "\nEnter strings to test.";
        cout << "\nType END on a separate line to finish.\n\n";

        string input;

        while (getline(cin, input)) {

            if (input == "END") {
                break;
            }

            if (dfa.accepts(input)) {
                cout << "'" << input << "' : ACCEPTED\n";
            }
            else {
                cout << "'" << input << "' : REJECTED\n";
            }
        }
    }
    catch (const exception& error) {
        cout << "\nError: " << error.what() << "\n";
    }
}

// ============================================
// NFA IMPLEMENTATION
// ============================================

NFA::NFA(int states, const string& symbols) {
    if (states <= 0) {
        throw invalid_argument("NFA must have at least one state.");
    }

    if (symbols.empty()) {
        throw invalid_argument("Alphabet cannot be empty.");
    }

    set<char> uniqueSymbols(symbols.begin(), symbols.end());

    if (uniqueSymbols.size() != symbols.size()) {
        throw invalid_argument("Alphabet symbols must be unique.");
    }

    stateCount = states;
    alphabet = symbols;
    startState = 0;

    transitions.resize(stateCount);
    epsilonTransitions.resize(stateCount);

    for (int i = 0; i < stateCount; i++) {
        transitions[i].resize(alphabet.length());
    }
}

void NFA::setStartState(int state) {
    if (state < 0 || state >= stateCount) {
        throw out_of_range("Invalid NFA start state.");
    }

    startState = state;
}

void NFA::addFinalState(int state) {
    if (state < 0 || state >= stateCount) {
        throw out_of_range("Invalid NFA final state.");
    }

    if (find(finalStates.begin(), finalStates.end(), state)
        == finalStates.end()) {
        finalStates.push_back(state);
    }
}

void NFA::addTransition(int state, char symbol, int nextState) {
    if (state < 0 || state >= stateCount ||
        nextState < 0 || nextState >= stateCount) {
        throw out_of_range("Invalid NFA transition state.");
    }

    size_t position = alphabet.find(symbol);

    if (position == string::npos) {
        throw invalid_argument("Symbol is not in the NFA alphabet.");
    }

    vector<int>& destinations = transitions[state][position];

    if (find(destinations.begin(), destinations.end(), nextState)
        == destinations.end()) {
        destinations.push_back(nextState);
    }
}

void NFA::addEpsilonTransition(int state, int nextState) {
    if (state < 0 || state >= stateCount ||
        nextState < 0 || nextState >= stateCount) {
        throw out_of_range("Invalid epsilon transition state.");
    }

    vector<int>& destinations = epsilonTransitions[state];

    if (find(destinations.begin(), destinations.end(), nextState)
        == destinations.end()) {
        destinations.push_back(nextState);
    }
}

std::vector<int> NFA::epsilonClosure(
    const std::vector<int>& states) const {

    vector<int> closure = states;
    vector<int> pending = states;

    while (!pending.empty()) {
        int currentState = pending.back();
        pending.pop_back();

        for (int nextState : epsilonTransitions[currentState]) {
            if (find(closure.begin(), closure.end(), nextState)
                == closure.end()) {
                closure.push_back(nextState);
                pending.push_back(nextState);
            }
        }
    }

    return closure;
}
bool NFA::accepts(const string& input) const {
    vector<int> currentStates = epsilonClosure({ startState });

    for (char symbol : input) {
        size_t position = alphabet.find(symbol);

        if (position == string::npos) {
            return false;
        }

        vector<int> nextStates;

        for (int state : currentStates) {
            const vector<int>& destinations =
                transitions[state][position];

            for (int nextState : destinations) {
                if (find(nextStates.begin(), nextStates.end(), nextState)
                    == nextStates.end()) {
                    nextStates.push_back(nextState);
                }
            }
        }

        currentStates = epsilonClosure(nextStates);

        if (currentStates.empty()) {
            return false;
        }
    }

    for (int state : currentStates) {
        if (find(finalStates.begin(), finalStates.end(), state)
            != finalStates.end()) {
            return true;
        }
    }

    return false;
}

int NFA::getStateCount() const {
    return stateCount;
}

const string& NFA::getAlphabet() const {
    return alphabet;
}

int NFA::getStartState() const {
    return startState;
}

const vector<int>& NFA::getFinalStates() const {
    return finalStates;
}

const vector<vector<vector<int>>>& NFA::getTransitions() const {
    return transitions;
}

const vector<vector<int>>& NFA::getEpsilonTransitions() const {
    return epsilonTransitions;
}

void NFA::displayTransitionTable() const {
    cout << "\n========== NFA TRANSITION TABLE ==========\n";

    cout << left << setw(12) << "State";

    for (char symbol : alphabet) {
        cout << setw(16) << symbol;
    }

    cout << setw(16) << "epsilon";
    cout << "\n";
    cout << "----------------------------------------------------------\n";

    for (int state = 0; state < stateCount; state++) {
        string stateLabel = "q" + to_string(state);

        if (state == startState) {
            stateLabel += " ->";
        }

        if (find(finalStates.begin(), finalStates.end(), state)
            != finalStates.end()) {
            stateLabel += " *";
        }

        cout << left << setw(12) << stateLabel;

        for (size_t j = 0; j < alphabet.length(); j++) {
            const vector<int>& destinations = transitions[state][j];

            if (destinations.empty()) {
                cout << setw(16) << "-";
            }
            else {
                string result;

                for (size_t k = 0; k < destinations.size(); k++) {
                    if (k > 0) {
                        result += ",";
                    }

                    result += "q" + to_string(destinations[k]);
                }

                cout << setw(16) << result;
            }
        }

        const vector<int>& epsilonDestinations =
            epsilonTransitions[state];

        if (epsilonDestinations.empty()) {
            cout << setw(16) << "-";
        }
        else {
            string result;

            for (size_t k = 0; k < epsilonDestinations.size(); k++) {
                if (k > 0) {
                    result += ",";
                }

                result += "q" + to_string(epsilonDestinations[k]);
            }

            cout << setw(16) << result;
        }

        cout << "\n";
    }

    cout << "==========================================\n";
}


void nfaRecognitionModule() {
    int stateCount;
    string alphabet;

    cout << "\n========== NFA CONFIGURATION ==========\n";

    cout << "Enter number of states: ";

    if (!(cin >> stateCount) || stateCount <= 0 || stateCount > 100) {
        cout << "Invalid number of states.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cout << "Enter alphabet symbols without spaces (e.g. 01 or ab): ";
    cin >> alphabet;

    if (alphabet.empty() || alphabet.length() > 26) {
        cout << "Invalid alphabet.\n";
        return;
    }

    try {
        NFA nfa(stateCount, alphabet);

        int startState;

        cout << "Enter initial state (0 to " << stateCount - 1 << "): ";

        if (!(cin >> startState)) {
            throw invalid_argument("Invalid initial state input.");
        }

        nfa.setStartState(startState);

        int finalCount;

        cout << "Enter number of final states: ";

        if (!(cin >> finalCount) || finalCount < 0 ||
            finalCount > stateCount) {
            throw invalid_argument("Invalid number of final states.");
        }

        cout << "Enter final state numbers: ";

        for (int i = 0; i < finalCount; i++) {
            int finalState;

            if (!(cin >> finalState)) {
                throw invalid_argument("Invalid final state input.");
            }

            nfa.addFinalState(finalState);
        }

        cout << "\nEnter NFA transitions.\n";
        cout << "Multiple destinations are allowed for each symbol.\n\n";

        for (int state = 0; state < stateCount; state++) {
            for (char symbol : alphabet) {
                int destinationCount;

                cout << "Number of destinations from q" << state
                     << " on '" << symbol << "': ";

                if (!(cin >> destinationCount) ||
                    destinationCount < 0 || destinationCount > stateCount) {
                    throw invalid_argument("Invalid destination count.");
                }

                for (int j = 0; j < destinationCount; j++) {
                    int nextState;

                    cout << "  Destination " << j + 1 << ": ";

                    if (!(cin >> nextState)) {
                        throw invalid_argument("Invalid destination state.");
                    }

                    nfa.addTransition(state, symbol, nextState);
                }
            }
        }

        cout << "\nEnter epsilon transitions (without consuming input).\n";
        cout << "Multiple destinations are allowed.\n\n";

        for (int state = 0; state < stateCount; state++) {
            int destinationCount;

            cout << "Number of epsilon destinations from q"
                 << state << ": ";

            if (!(cin >> destinationCount) ||
                destinationCount < 0 || destinationCount > stateCount) {
                throw invalid_argument("Invalid epsilon destination count.");
            }

            for (int j = 0; j < destinationCount; j++) {
                int nextState;

                cout << "  Epsilon destination " << j + 1 << ": ";

                if (!(cin >> nextState)) {
                    throw invalid_argument("Invalid epsilon destination state.");
                }

                nfa.addEpsilonTransition(state, nextState);
            }
        }

        nfa.displayTransitionTable();

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "\nEnter strings to test.";
        cout << "\nType END on a separate line to finish.\n\n";

        string input;

        while (getline(cin, input)) {
            if (input == "END") {
                break;
            }

            if (nfa.accepts(input)) {
                cout << "'" << input << "' : ACCEPTED\n";
            }
            else {
                cout << "'" << input << "' : REJECTED\n";
            }
        }
    }
    catch (const exception& error) {
        cout << "\nError: " << error.what() << "\n";

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}



// ============================================
// FORMAL LANGUAGE & AUTOMATA MENU
// ============================================

void languageRecognitionModule() {
    int choice;

    do {
        cout << "\n============================================\n";
        cout << "       FORMAL LANGUAGE & AUTOMATA            \n";
        cout << "============================================\n";
        cout << "1. Deterministic Finite Automaton (DFA)\n";
        cout << "2. Nondeterministic Finite Automaton (NFA)\n";
        cout << "0. Back to Main Menu\n";
        cout << "============================================\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice)) {
            cout << "\nInvalid input. Please enter a number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
            case 1:
                dfaRecognitionModule();
                break;

            case 2:
                nfaRecognitionModule();
                break;

            case 0:
                cout << "\nReturning to CompilX Main Menu...\n";
                break;

            default:
                cout << "\nInvalid choice. Try again.\n";
        }

    } while (choice != 0);
}






