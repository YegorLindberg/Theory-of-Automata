///
///  main.cpp
///  conversionToDeterministicAutomat
///
//  Created by Yegor Lindberg on 25/11/2018.
///  Copyright © 2018 Yegor Lindberg. All rights reserved.
///
#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;
using StateTable = vector<vector<set<int>>>;
using ExtendedStateTable = vector<vector<pair<set<int>, set<int>>>>;
static const int NO_TRANSITIONS = -1;

vector<int> readFinalStates(istream& inputFile, int finalStateCount) {
    vector<int> finalStates(finalStateCount);
    for (size_t i = 0; i < finalStateCount; i++) {
        inputFile >> finalStates[i];
    }
    { //переводим каретку на новую строку
        string extra = "";
        getline(inputFile, extra);
    }
    return finalStates;
}

StateTable readOriginalTable(istream& inputFile, int stateCount, int inputSignalCount) {
    StateTable table(inputSignalCount, vector<set<int>>(stateCount));
    vector<string> lineTable(stateCount);
    for (size_t i = 0; i < stateCount; i++) {
        getline(inputFile, lineTable[i]);
        stringstream strstream(lineTable[i]);
        bool even = false;
        int value = 0;
        int prevValue = 0;
        int currState = 0;
        while(strstream >> value) {
            if (even) {
                currState = value;
                table[currState][i].insert(prevValue);
            }
            prevValue = value;
            even == true ? even = false : even = true;
        }
    }
    return table;
}

void print(StateTable& originalTable) {
    for (size_t row = 0; row < originalTable[0].size(); row++) {
        for (size_t column = 0; column < originalTable.size(); column++) {
            if (originalTable[column][row].empty()) {
                originalTable[column][row].insert(NO_TRANSITIONS);
            }
            for (auto element = originalTable[column][row].begin(); element != originalTable[column][row].end(); element++) {
                cout << *element << "";
            }
            cout << " ";
        }
        cout << endl;
    }
}

void print(ExtendedStateTable& extendedTabel) {
    cout << extendedTabel[0].size() << " - extendedTabel[0].size();\n" << extendedTabel.size() << " - extendedTabel.size();\n";
    for (size_t row = 0; row < extendedTabel[0].size(); row++) {
        for (size_t column = 0; column < extendedTabel.size(); column++) {
            for (auto firstElement = extendedTabel[column][row].first.begin(); firstElement != extendedTabel[column][row].first.end(); firstElement++) {
                cout << *firstElement << "";
            }
            cout << " ";
            for (auto secElement = extendedTabel[column][row].second.begin(); secElement != extendedTabel[column][row].second.end(); secElement++) {
                cout << *secElement << "";
            }
            cout << " ";
        }
        cout << endl;
    }
}

ExtendedStateTable defineAllTransitions(ExtendedStateTable& fullTableOfTransitions, queue<set<int>>& queueStates, set<set<int>>& visited, set<set<int>>& repeatsCheckerForQueue) {
    size_t inputSignalsCount = fullTableOfTransitions.size();
    set<int> visitState;
    set<int> listOfStates;
    set<int> mergeStates;
    while (!queueStates.empty()) {
        visitState = queueStates.front();
        queueStates.pop();
        for (int column = 0; column < inputSignalsCount; column++) {
            for (auto element = visitState.begin(); element != visitState.end(); element++) {
                    merge(listOfStates.begin(), listOfStates.end(),
                          fullTableOfTransitions[column][*element].second.begin(), fullTableOfTransitions[column][*element].second.end(), inserter(mergeStates, mergeStates.begin()));
                    listOfStates = mergeStates;
                    mergeStates.clear();
            }
            if ((listOfStates.find(NO_TRANSITIONS) != listOfStates.end()) && (listOfStates.size() > 1)) {
                listOfStates.erase(NO_TRANSITIONS);
            }
            fullTableOfTransitions[column].push_back(make_pair(visitState, listOfStates));
            visited.insert(visitState);
            if ((visited.find(listOfStates) == visited.end()) && (listOfStates.find(NO_TRANSITIONS) == listOfStates.end()) && (repeatsCheckerForQueue.find(listOfStates) == repeatsCheckerForQueue.end())) {
                queueStates.push(listOfStates);
                repeatsCheckerForQueue.insert(listOfStates);
            }
            listOfStates.clear();
        }
    }
    return fullTableOfTransitions;
}

ExtendedStateTable determineOriginalTable(StateTable& origTable, queue<set<int>>& queueStates, set<set<int>>& visited) {
    size_t tableRows = origTable[0].size();
    size_t tableColumns = origTable.size();
    ExtendedStateTable tableOfTransitions(tableColumns, vector<pair<set<int>, set<int>>>(tableRows));
    set<int> visitState;
    set<set<int>> repeatsCheckerForQueue;
    //помечаем все имеющиеся состояния, как посещенные, поскольку мы туда зайдем следующим циклом
    for (int iterator = 0; iterator < tableRows; iterator++) {
        visitState.insert(iterator);
        visited.insert(visitState);
        repeatsCheckerForQueue.insert(visitState);
        visitState.clear();
    }
    //запись из оригинальной таблицы в новую и добавление в очередь новых состояний
    for (int row = 0; row < tableRows; row++) {
        visitState.insert(row);
        for (size_t column = 0; column < tableColumns; column++) {
            auto repeatRecorded = repeatsCheckerForQueue.find(origTable[column][row]);
            auto repeatVisited = visited.find(origTable[column][row]);
            if ((*origTable[column][row].begin() != NO_TRANSITIONS)  && (repeatVisited == visited.end()) && (repeatRecorded == repeatsCheckerForQueue.end())){
                queueStates.push(origTable[column][row]);
            }
            tableOfTransitions[column][row] = make_pair(visitState, origTable[column][row]);
        }
        visitState.clear();
    }
    return defineAllTransitions(tableOfTransitions, queueStates, visited, repeatsCheckerForQueue);
}

int getIndex(ExtendedStateTable& fullTable, set<int> toFind) {
    for (int row = 0; row < fullTable[0].size(); row++) {
        if (fullTable[0][row].first == toFind) {
            return row;
        }
    }
    return 0;
}

void printFinalStates(ostream& outputFile, vector<set<int>>& newFinalStates) {
    for (int fstate = 0; fstate < newFinalStates.size(); fstate++) {
        for (auto iter = newFinalStates[fstate].begin(); iter != newFinalStates[fstate].end(); iter++) {
            outputFile << *iter;
        }
        outputFile << " ";
    }
    outputFile << endl;
}

void printOutput(ostream& outputFile, ExtendedStateTable& endTable, vector<set<int>>& newFinalStates, int inputSignalCount) {
    outputFile << inputSignalCount << endl;
    outputFile << endTable[0].size() << endl;
    outputFile << newFinalStates.size() << endl;
    printFinalStates(outputFile, newFinalStates);
    for (int column = 0; column < endTable.size(); column++) {
        for (int row = 0; row < endTable[0].size(); row++) {
            for (auto iter = endTable[column][row].second.begin(); iter != endTable[column][row].second.end(); iter++) {
                outputFile << *iter;
            }
            outputFile << " ";
        }
        outputFile << endl;
    }
    outputFile << endl;
}

void printFinalStatesToDot(ostream& ofs, vector<set<int>>& newFinalStates) {
    for (int fstate = 0; fstate < newFinalStates.size(); fstate++) {
        ofs << "S_";
        for (auto iter = newFinalStates[fstate].begin(); iter != newFinalStates[fstate].end(); iter++) {
            ofs << *iter;
            auto checkEnd = iter;
            if (++checkEnd != newFinalStates[fstate].end()) { ofs << "_"; }
        }
        ofs << " ";
    }
}

void printTableInDotFile(ExtendedStateTable& endTable, vector<set<int>>& newFinalStates, string fileName) {
    ofstream ofs(fileName);
    
    stringstream stream;
    printFinalStatesToDot(stream, newFinalStates);
    ofs << "digraph G {" << endl;
    ofs << "node [shape = doublecircle]; " << stream.str() << ";" << endl;
    ofs << "node [shape = circle];" << endl;
    
    for (int column = 0; column < endTable.size(); column++) {
        for (int row = 0; row < endTable[0].size(); row++) {
            if (*endTable[column][row].second.begin() == -1) { continue; }
            ofs << "S_";
            for (auto iter = endTable[column][row].first.begin(); iter != endTable[column][row].first.end(); iter++) {
                ofs << *iter;
                auto checkEnd = iter;
                if (++checkEnd != endTable[column][row].first.end()) {
                    ofs << "_";
                }
            }
            ofs << " -> " << "S_";
            for (auto iter = endTable[column][row].second.begin(); iter != endTable[column][row].second.end(); iter++) {
                ofs << *iter;
                auto checkEnd = iter;
                if (++checkEnd != endTable[column][row].second.end()) {
                    ofs << "_";
                }
            }
            ofs << " [ label = \"" << column << "\" ];" << endl;
        }
    }
    ofs << "}" << endl;
}

ExtendedStateTable deleteUnnecessaryStates(ExtendedStateTable& fullTable, vector<int>& finalStates, vector<set<int>>& newFinalStates) {
    size_t countColumn = fullTable.size();
    ExtendedStateTable transitionsTable(countColumn);
    queue<set<int>> queueStates;
    set<set<int>> passed;
    set<int> currentState;
    currentState.insert(0);
    queueStates.push(currentState);
    passed.insert(currentState);
    currentState.clear();
    while (!queueStates.empty()) {
        currentState = queueStates.front();
        queueStates.pop();
        int index = getIndex(fullTable, currentState);
        for (int column = 0; column < countColumn; column++) {
            transitionsTable[column].push_back(fullTable[column][index]);
            for (int i = 0; i < finalStates.size(); i++) {
                if ((fullTable[column][index].second.find(finalStates[i]) != fullTable[column][index].second.end()) && (passed.find(fullTable[column][index].second) == passed.end())) {
                    newFinalStates.push_back(fullTable[column][index].second);
                    break;
                }
            }
            if ((passed.find(fullTable[column][index].second) == passed.end()) && (*fullTable[column][index].second.begin() != NO_TRANSITIONS)) {
                queueStates.push(fullTable[column][index].second);
                passed.insert(fullTable[column][index].second);
            }
        }
        currentState.clear();
    }
    return transitionsTable;
}

void makeDeterminization(istream& inputFile, ostream& outputFile, string dotFileName) {
    int inputSignalCount = 0; // число входных сигналов
    int stateCount = 0; // число состояний
    int finalStateCount = 0; // число финальных состояний
    inputFile >> inputSignalCount >> stateCount >> finalStateCount;
    
    queue<set<int>> queueStates;
    set<set<int>> visited;
    
    vector<int> finalStates = readFinalStates(inputFile, finalStateCount);
    StateTable originalTable = readOriginalTable(inputFile, stateCount, inputSignalCount);
    print(originalTable);
    cout << "-----------\n";
    ExtendedStateTable fullTableOfTransitions = determineOriginalTable(originalTable, queueStates, visited);
    print(fullTableOfTransitions);
    vector<set<int>> newFinalStates;
    ExtendedStateTable endTable = deleteUnnecessaryStates(fullTableOfTransitions, finalStates, newFinalStates);
    print(endTable);
    printOutput(outputFile, endTable, newFinalStates, inputSignalCount);
    printTableInDotFile(endTable, newFinalStates, dotFileName);
}


int main() {
    ifstream inputFile("6/in.txt");
    ofstream outputFile("output.txt");
    
    if ((!inputFile.is_open()) || (!outputFile.is_open())) {
        cerr << "Files aren't opened.\n";
        return 1;
    }
    
    makeDeterminization(inputFile, outputFile, "vizualizatioin.dot");
    
    inputFile.close();
    outputFile.close();
    return 0;
}
