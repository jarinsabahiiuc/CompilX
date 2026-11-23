#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <cctype>
#include <map>
#include <queue>
#include <set>

#include "regex_to_nfa.h"
#include "language_recognition.h"

using namespace std;

struct Fragment {
    int start;
    int end;
};

bool isOperator(char symbol) {
    return symbol == '|' || symbol == '.' ||
           symbol == '*' || symbol == '+';
}

int precedence(char symbol) {
    if (symbol == '*' || symbol == '+') {
        return 3;
    }

    if (symbol == '.') {
        return 2;
    }

    if (symbol == '|') {
        return 1;
    }

    return 0;
}

string convertToPostfix(const string& expression) {
    string postfix;
    stack<char> operators;

    bool expectOperand = true;

    for (char symbol : expression) {

        if (isspace(static_cast<unsigned char>(symbol))) {
            throw invalid_argument(
                "Spaces are not allowed in regular expressions."
            );
        }

        if (symbol == '(') {
            if (!expectOperand) {
                throw invalid_argument(
                    "Missing concatenation operator before '('."
                );
            }

            operators.push(symbol);
        }

        else if (symbol == ')') {
            if (expectOperand) {
                throw invalid_argument(
                    "Empty group or missing operand before ')'."
                );
            }

            while (!operators.empty() && operators.top() != '(') {
                postfix += operators.top();
                operators.pop();
            }

            if (operators.empty()) {
                throw invalid_argument("Mismatched parentheses.");
            }

            operators.pop();
            expectOperand = false;
        }

        else if (symbol == '*' || symbol == '+') {
            if (expectOperand) {
                throw invalid_argument(
                    "Unary operator must follow an operand."
                );
            }

            postfix += symbol;
        }

        else if (symbol == '.' || symbol == '|') {
            if (expectOperand) {
                throw invalid_argument(
                    "Binary operator is missing a left operand."
                );
            }

            while (!operators.empty() &&
                   operators.top() != '(' &&
                   precedence(operators.top()) >= precedence(symbol)) {
                postfix += operators.top();
                operators.pop();
            }

            operators.push(symbol);
            expectOperand = true;
        }

        else {
            if (!expectOperand) {
                throw invalid_argument(
                    "Missing concatenation operator between operands."
                );
            }

            postfix += symbol;
            expectOperand = false;
        }
    }

    if (expectOperand) {
        throw invalid_argument(
            "Expression is empty or ends with an operator."
        );
    }

    while (!operators.empty()) {
        if (operators.top() == '(') {
            throw invalid_argument("Mismatched parentheses.");
        }

        postfix += operators.top();
        operators.pop();
    }

    return postfix;
}
string extractAlphabet(const string& expression) {
    string alphabet;

    for (char symbol : expression) {
        if (!isOperator(symbol) && symbol != '(' && symbol != ')') {
            if (alphabet.find(symbol) == string::npos) {
                alphabet += symbol;
            }
        }
    }

    return alphabet;
}

int calculateStateCount(const string& postfix) {
    int stateCount = 0;
    int fragments = 0;

    for (char symbol : postfix) {
        if (symbol == '*' || symbol == '+') {
            if (fragments < 1) {
                throw invalid_argument("Invalid regular expression.");
            }

            stateCount += 2;
        }
        else if (symbol == '.' || symbol == '|') {
            if (fragments < 2) {
                throw invalid_argument("Invalid regular expression.");
            }

            if (symbol == '|') {
                stateCount += 2;
            }

            fragments--;
        }
        else {
            stateCount += 2;
            fragments++;
        }
    }

    if (fragments != 1 || stateCount <= 0) {
        throw invalid_argument("Invalid regular expression.");
    }

    if (stateCount > 400) {
        throw invalid_argument("Regular expression is too large.");
    }

    return stateCount;
}

void buildThompsonNFA(
    const string& postfix,
    NFA& nfa
) {
    stack<Fragment> fragments;
    int nextState = 0;

    for (char symbol : postfix) {

        if (symbol != '|' && symbol != '.' &&
            symbol != '*' && symbol != '+') {

            int start = nextState++;
            int end = nextState++;

            nfa.addTransition(start, symbol, end);

            fragments.push({ start, end });
        }

        else if (symbol == '.') {
            Fragment second = fragments.top();
            fragments.pop();

            Fragment first = fragments.top();
            fragments.pop();

            nfa.addEpsilonTransition(first.end, second.start);

            fragments.push({ first.start, second.end });
        }

        else if (symbol == '|') {
            Fragment second = fragments.top();
            fragments.pop();

            Fragment first = fragments.top();
            fragments.pop();

            int start = nextState++;
            int end = nextState++;

            nfa.addEpsilonTransition(start, first.start);
            nfa.addEpsilonTransition(start, second.start);

            nfa.addEpsilonTransition(first.end, end);
            nfa.addEpsilonTransition(second.end, end);

            fragments.push({ start, end });
        }

        else if (symbol == '*') {
            Fragment original = fragments.top();
            fragments.pop();

            int start = nextState++;
            int end = nextState++;

            nfa.addEpsilonTransition(start, original.start);
            nfa.addEpsilonTransition(start, end);

            nfa.addEpsilonTransition(original.end, original.start);
            nfa.addEpsilonTransition(original.end, end);

            fragments.push({ start, end });
        }

        else if (symbol == '+') {
            Fragment original = fragments.top();
            fragments.pop();

            int start = nextState++;
            int end = nextState++;

            nfa.addEpsilonTransition(start, original.start);

            nfa.addEpsilonTransition(original.end, original.start);
            nfa.addEpsilonTransition(original.end, end);

            fragments.push({ start, end });
        }
    }

    Fragment result = fragments.top();

    nfa.setStartState(result.start);
    nfa.addFinalState(result.end);
}

DFA displayEquivalentDFA(const NFA& nfa) {

    const string& alphabet = nfa.getAlphabet();

    const auto& transitions = nfa.getTransitions();
    const auto& epsilonTransitions = nfa.getEpsilonTransitions();
    const auto& finalStates = nfa.getFinalStates();

    // Calculate epsilon closure of a set of NFA states.
    auto epsilonClosure = [&](const vector<int>& initialStates) {

        set<int> closure(
            initialStates.begin(),
            initialStates.end()
        );

        vector<int> pending(
            initialStates.begin(),
            initialStates.end()
        );

        while (!pending.empty()) {

            int current = pending.back();
            pending.pop_back();

            for (int next : epsilonTransitions[current]) {

                if (closure.insert(next).second) {
                    pending.push_back(next);
                }
            }
        }

        return vector<int>(closure.begin(), closure.end());
    };

    // Each DFA state represents a unique set of NFA states.
    map<vector<int>, int> stateIds;
    vector<vector<int>> dfaStates;
    vector<vector<int>> dfaTransitions;

    queue<int> pending;

    vector<int> initialState =
        epsilonClosure({ nfa.getStartState() });

    stateIds[initialState] = 0;
    dfaStates.push_back(initialState);
    pending.push(0);

    // Generate all reachable DFA states.
    while (!pending.empty()) {

        int currentId = pending.front();
        pending.pop();

        vector<int> currentNfaStates = dfaStates[currentId];

        dfaTransitions.resize(dfaStates.size());

        dfaTransitions[currentId].resize(
            alphabet.length(),
            -1
        );

        for (size_t symbolIndex = 0;
             symbolIndex < alphabet.length();
             symbolIndex++) {


            vector<int> reachableStates;

            // Move operation.
            for (int state : currentNfaStates) {

                for (int next :
                     transitions[state][symbolIndex]) {

                    reachableStates.push_back(next);
                }
            }

            // Apply epsilon closure.
            vector<int> nextDfaState =
                epsilonClosure(reachableStates);

            // Check whether this state already exists.
            auto found = stateIds.find(nextDfaState);

            int nextId;

            if (found == stateIds.end()) {

                nextId = static_cast<int>(dfaStates.size());

                stateIds[nextDfaState] = nextId;
                dfaStates.push_back(nextDfaState);

                dfaTransitions.resize(dfaStates.size());

                pending.push(nextId);
            }
            else {
                nextId = found->second;
            }

            dfaTransitions[currentId][symbolIndex] = nextId;
        }
    }

    // Create the DFA using all discovered states.
    DFA dfa(
        static_cast<int>(dfaStates.size()),
        alphabet
    );

    dfa.setStartState(0);

    for (size_t i = 0; i < dfaStates.size(); i++) {

        bool isFinal = false;

        for (int state : dfaStates[i]) {

            if (find(finalStates.begin(), finalStates.end(), state)
                != finalStates.end()) {

                isFinal = true;
                break;
            }
        }

        if (isFinal) {
            dfa.addFinalState(static_cast<int>(i));
        }

        for (size_t j = 0; j < alphabet.length(); j++) {

            dfa.setTransition(
                static_cast<int>(i),
                alphabet[j],
                dfaTransitions[i][j]
            );
        }
    }

    cout << "\nSubset Construction completed successfully.\n";

    cout << "\nDFA State Mapping:\n";

    for (size_t i = 0; i < dfaStates.size(); i++) {

        cout << "D" << i << " = {";

        for (size_t j = 0; j < dfaStates[i].size(); j++) {

            if (j > 0) {
                cout << ", ";
            }

            cout << "q" << dfaStates[i][j];
        }

        cout << "}\n";
    }

    dfa.displayTransitionTable();

    return dfa;
}

void regexToNfaModule() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "\n============================================\n";
    cout << "       REGULAR EXPRESSION TO NFA            \n";
    cout << "============================================\n";

    cout << "Supported operators:\n";
    cout << "  |  Union\n";
    cout << "  .  Concatenation\n";
    cout << "  *  Kleene Star\n";
    cout << "  +  One or More\n";
    cout << "  () Grouping\n\n";

    cout << "Example: (a|b)*.a\n";
    cout << "Enter regular expression: ";

    string expression;
    getline(cin, expression);

    try {
        if (expression.empty() || expression.length() > 200) {
            throw invalid_argument(
                "Expression must contain 1 to 200 characters."
            );
        }

        string postfix = convertToPostfix(expression);
        string alphabet = extractAlphabet(expression);

        if (alphabet.empty()) {
            throw invalid_argument("Expression has no alphabet symbols.");
        }

        int stateCount = calculateStateCount(postfix);

        cout << "\nInfix Expression: " << expression << "\n";
        cout << "Postfix Expression: " << postfix << "\n";
        cout << "Alphabet: " << alphabet << "\n";

        NFA nfa(stateCount, alphabet);

        buildThompsonNFA(postfix, nfa);

        cout << "\nThompson's Construction completed successfully.\n";

        nfa.displayTransitionTable();

        DFA dfa = displayEquivalentDFA(nfa);

        cout << "\nEnter strings to test.";
        cout << "\nType END on a separate line to finish.\n\n";

        string input;

        while (getline(cin, input)) {
            if (input == "END") {
                break;
            }

            bool nfaAccepted = nfa.accepts(input);
            bool dfaAccepted = dfa.accepts(input);

            cout << "\nInput: '" << input << "'\n";

            cout << "NFA: "
                 << (nfaAccepted ? "ACCEPTED" : "REJECTED")
                 << "\n";

            cout << "DFA: "
                 << (dfaAccepted ? "ACCEPTED" : "REJECTED")
                 << "\n";

            if (nfaAccepted == dfaAccepted) {
                cout << "Comparison: MATCH\n";
            }
            else {
                cout << "Comparison: MISMATCH - ERROR!\n";
            }
        }
    }
    catch (const exception& error) {
        cout << "\nError: " << error.what() << "\n";
    }
}






