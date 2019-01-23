//
//  main.cpp
//  Minimization
//
//  Created by Yegor Lindberg on 18/12/2018.
//  Copyright © 2018 Yegor Lindberg. All rights reserved.
//

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

using namespace std;

enum class AutomataType {
    Moore = 1,
    Mealy = 2,
};

struct Automata {
    vector<vector<int>> states;
    vector<vector<int>> outputs;
    int countX;
    int countY;
    AutomataType type;
};

template<typename T>
T GetParameter(istream& stream) {
    T result;
    return (stream >> result) ? result : 0;
}

void MinimizeAutomata(istream& streamIn, ostream& ostream, const string& dotFileName);

int main() {
    string inFileName = "inputMili.txt";
    string outFileName = "output.txt";
    ifstream streamIn(inFileName);
    ofstream streamOut(outFileName);
    MinimizeAutomata(streamIn, streamOut, "visualization.dot");
    return 0;
}



Automata GetMooreAutomata(istream& stream) {
    Automata automata;
    automata.type = AutomataType::Moore;
    automata.countX = GetParameter<int>(stream);
    automata.countY = GetParameter<int>(stream);
    auto countQ = GetParameter<int>(stream);
    
    automata.states = vector<vector<int>>(countQ, vector<int>());
    automata.outputs = vector<vector<int>>(countQ, vector<int>());
    
    for (int index = 0; index < countQ; ++index)
    {
        automata.outputs[index].push_back({ GetParameter<int>(stream) });
    }
    
    for (int index = 0; index < automata.countX; ++index)
    {
        for (int j = 0; j < countQ; ++j)
        {
            automata.states[j].push_back(GetParameter<int>(stream));
        }
    }
    
    return automata;
}

Automata GetMealyAutomata(istream& stream) {
    Automata automata;
    automata.type = AutomataType::Mealy;
    automata.countX = GetParameter<int>(stream);
    automata.countY = GetParameter<int>(stream);
    auto countQ = GetParameter<int>(stream);
    
    automata.states = vector<vector<int>>(countQ, vector<int>());
    automata.outputs = vector<vector<int>>(countQ, vector<int>());
    
    for (int indexOfX = 0; indexOfX < automata.countX; ++indexOfX) {
        for (int index = 0; index < countQ; ++index) {
            auto state = GetParameter<int>(stream);
            automata.states[index].push_back(state);
            
            auto output = GetParameter<int>(stream);
            automata.outputs[index].push_back(output);
        }
    }
    return automata;
}

Automata GetAutomata(int type, istream& stream) {
    switch (static_cast<AutomataType>(type))
    {
        case AutomataType::Moore:
            return GetMooreAutomata(stream);
        case AutomataType::Mealy:
            return GetMealyAutomata(stream);
        default:
            throw new invalid_argument("Invalid machine type");
    }
}

vector<int> GetClasses(const vector<vector<int>>& states, const vector<int>& classes) {
    int k = 0;
    vector<int> result = vector<int>(states.size(), -1);
    for (size_t index = 0; index < result.size(); ++index) {
        if (result[index] != -1) { continue; }
        for (size_t j = index; j < result.size(); ++j) {
            if (result[j] == -1 && (states[index] == states[j]) && (classes[index] == classes[j])) {
                result[j] = k;
            }
        }
        ++k;
    }
    return result;
}

void TransformByNewClasses(Automata& newAutomata, vector<int>& classes) {
    for (size_t i = 0; i < newAutomata.states.size(); ++i) {
        for (size_t j = 0; j < newAutomata.states[i].size(); ++j) {
            newAutomata.states[i][j] = classes[newAutomata.states[i][j]];
        }
    }
}

Automata Minimizate(const Automata& automata) {
    auto newAutomata = automata;
    auto prevClasses = vector<int>(automata.states.size(), -2);
    auto classes = GetClasses(newAutomata.outputs, prevClasses);
    
    while (prevClasses != classes) {
        newAutomata = automata;
        TransformByNewClasses(newAutomata, classes);
        prevClasses = classes;
        
        classes = GetClasses(newAutomata.states, classes);
    }
    
    vector<vector<int>> states;
    vector<vector<int>> outputs;
    vector<int> visited;
    for (size_t index = 0; index < classes.size(); ++index) {
        if (find(visited.begin(), visited.end(), classes[index]) == visited.end()) {
            visited.push_back(classes[index]);
            states.push_back(newAutomata.states[index]);
            outputs.push_back(newAutomata.outputs[index]);
        }
    }
    newAutomata.states = states;
    newAutomata.outputs = outputs;
    return newAutomata;
}

void PrintMooreAutomata(ostream& ostream, const Automata& automata) {
    ostream << static_cast<int>(automata.type) << endl;
    ostream << automata.countX << endl;
    ostream << automata.countY << endl;
    ostream << automata.states.size() << endl;
    
    for (size_t j = 0; j < automata.states.size(); ++j) {
        ostream << automata.outputs[j][0] << " ";
    }
    ostream << endl;
    
    for (int index = 0; index < automata.countX; ++index) {
        for (size_t j = 0; j < automata.states.size(); ++j) {
            ostream << automata.states[j][index] << " ";
        }
        ostream << endl;
    }
}

void PrintMealyAutomata(ostream& ostream, const Automata& automata) {
    ostream << static_cast<int>(automata.type) << endl;
    ostream << automata.countX << endl;
    ostream << automata.countY << endl;
    ostream << automata.states.size() << endl;
    
    for (int index = 0; index < automata.countX; ++index) {
        for (size_t j = 0; j < automata.states.size(); ++j) {
            ostream << automata.states[j][index] << " ";
            ostream << automata.outputs[j][index] << " ";
        }
        ostream << endl;
    }
}

void PrintAutomataInDotFile(const string& fileName, const Automata& automata) {
    ofstream ofs(fileName);
    
    ofs << "digraph G {" << endl;
    ofs << "node [shape = circle];" << endl;
    
    for (size_t index = 0; index < automata.states.size(); ++index) {
        for (size_t jindex = 0; jindex < automata.states[index].size(); ++jindex) {
            auto value = (jindex < automata.outputs[index].size()) ? automata.outputs[index][jindex] : automata.outputs[index][0];
            ofs << index << " -> " << automata.states[index][jindex]
            << " [ label = \"" << jindex + 1 << "/" << value << "\" ];" << endl;
        }
    }
    ofs << "}" << endl;
}

void PrintAutomata(ostream& ostream, const string& dotFileName, const Automata& automata) {
    switch (automata.type) {
        case AutomataType::Moore:
            PrintMooreAutomata(ostream, automata);
            PrintAutomataInDotFile(dotFileName, automata);
            break;
        case AutomataType::Mealy:
            PrintMealyAutomata(ostream, automata);
            PrintAutomataInDotFile(dotFileName, automata);
            break;
        default:
            throw new invalid_argument("Invalid machine type");
    }
}

void MinimizeAutomata(istream& streamIn, ostream& ostream, const string& dotFileName) {
    string typeFSAutomata;
    streamIn >> typeFSAutomata;
    int type = std::stoi(typeFSAutomata, nullptr, 10);
    
    auto automata = GetAutomata(type, streamIn);
    auto minimizedAutomata = Minimizate(automata);
    PrintAutomata(ostream, dotFileName, minimizedAutomata);
}
