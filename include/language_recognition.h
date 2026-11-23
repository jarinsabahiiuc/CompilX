#ifndef LANGUAGE_RECOGNITION_H
#define LANGUAGE_RECOGNITION_H

#include <string>
#include <vector>

class DFA {
private:
    int stateCount;
    std::string alphabet;
    int startState;
    std::vector<int> finalStates;
    std::vector<std::vector<int>> transitions;

public:
    DFA(int states, const std::string& symbols);

    void setStartState(int state);
    void addFinalState(int state);
    void setTransition(int state, char symbol, int nextState);

    bool accepts(const std::string& input) const;
    void displayTransitionTable() const;
};

class NFA {
private:
    int stateCount;
    std::string alphabet;
    int startState;
    std::vector<int> finalStates;

    std::vector<std::vector<std::vector<int>>> transitions;
    std::vector<std::vector<int>> epsilonTransitions;

    std::vector<int> epsilonClosure(const std::vector<int>& states) const;

public:
    NFA(int states, const std::string& symbols);

    void setStartState(int state);
    void addFinalState(int state);
    void addTransition(int state, char symbol, int nextState);
    void addEpsilonTransition(int state, int nextState);

    bool accepts(const std::string& input) const;
    void displayTransitionTable() const;

    int getStateCount() const;
    const std::string& getAlphabet() const;
    int getStartState() const;
    const std::vector<int>& getFinalStates() const;
    const std::vector<std::vector<std::vector<int>>>& getTransitions() const;
    const std::vector<std::vector<int>>& getEpsilonTransitions() const;
};

void languageRecognitionModule();
void dfaRecognitionModule();
void nfaRecognitionModule();

#endif

