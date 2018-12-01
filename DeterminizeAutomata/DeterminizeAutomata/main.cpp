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

vector<int> describesArrayOfFinalStates(istream& inputFile, int finalStateCount) {
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

StateTable describesOriginalTable(istream& inputFile, int stateCount, int inputSignalCount) {
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

void printingOriginalTable(StateTable& originalTable, int stateCount, int inputSignalCount) {
    for (size_t row = 0; row < stateCount; row++) {
        for (size_t column = 0; column < inputSignalCount; column++) {
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

void printingExtendedTable(ExtendedStateTable& extendedTabel) {
    cout << extendedTabel[0].size() << " - extendedTabel[0].size();\n" << extendedTabel.size() << " - extendedTabel.size();\n";
    for (size_t row = 0; row < extendedTabel[0].size(); row++) {
        for (size_t column = 0; column < extendedTabel.size(); column++) {
            for (auto element = extendedTabel[column][row].second.begin(); element != extendedTabel[column][row].second.end(); element++) {
                cout << *element << "";
            }
            cout << " ";
        }
        cout << endl;
    }
}

ExtendedStateTable definesAllTransitions(ExtendedStateTable& fullTableOfTransitions, queue<set<int>>& queueStates, set<set<int>>& visited, set<set<int>>& repeatsCheckerForQueue) {
    size_t inputSignalsCount = fullTableOfTransitions.size();
    set<int> visitState;
    set<int> listOfStates;
    set<int> mergeStates;
    while (!queueStates.empty()) {
        cout << "queue not empty.\n";
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
            if ((visited.find(listOfStates) != visited.end()) && (listOfStates.find(NO_TRANSITIONS) != listOfStates.end())) {
                queueStates.push(listOfStates);
                repeatsCheckerForQueue.insert(listOfStates);
            }
            listOfStates.clear();
        }
    }
    return fullTableOfTransitions;
}

ExtendedStateTable determineOriginalTable(StateTable& origTable, queue<set<int>>& queueStates, set<set<int>>& visited) {
    cout << origTable[0].size() << " - origTable[0].size();\n" << origTable.size() << " - origTable.size();\n";
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
        for (size_t column = 0; column < tableColumns; column++) {
            auto repeatRecorded = repeatsCheckerForQueue.find(origTable[column][row]);
            auto repeatVisited = visited.find(origTable[column][row]);
            if ((*origTable[column][row].begin() != NO_TRANSITIONS)  && (repeatVisited == visited.end()) && (repeatRecorded == repeatsCheckerForQueue.end())){
                queueStates.push(origTable[column][row]);
            }
            tableOfTransitions[column][row] = make_pair(visitState, origTable[column][row]);
        }
    }
    return definesAllTransitions(tableOfTransitions, queueStates, visited, repeatsCheckerForQueue);
}

void makeDeterminization(istream& inputFile, ostream& outputFile) {
    int inputSignalCount = 0; // число входных сигналов
    int stateCount = 0; // число состояний
    int finalStateCount = 0; // число финальных состояний
    inputFile >> inputSignalCount >> stateCount >> finalStateCount;
    
    queue<set<int>> queueStates;
    set<set<int>> visited;
    
    vector<int> finalStates = describesArrayOfFinalStates(inputFile, finalStateCount);
    StateTable originalTable = describesOriginalTable(inputFile, stateCount, inputSignalCount);
    printingOriginalTable(originalTable, stateCount, inputSignalCount);
    cout << "-----------\n";
    ExtendedStateTable fullTableOfTransitions = determineOriginalTable(originalTable, queueStates, visited);
    printingExtendedTable(fullTableOfTransitions);
}


int main() {
    
    ifstream inputFile("input.txt");
    ofstream outputFile("output.txt");
    
    if ((!inputFile.is_open()) || (!outputFile.is_open())) {
        cerr << "Files aren't opened.\n";
        return 1;
    }
    
    makeDeterminization(inputFile, outputFile);
    
    inputFile.close();
    outputFile.close();
    
    return 0;
}
