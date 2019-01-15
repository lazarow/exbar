#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <tuple>
#include <fstream>
#include <string>
#include "libraries/common.h"
#include "libraries/easylogging/easylogging++.h"
using namespace std;

const int LABEL_ACCEPTED = 1;
const int LABEL_REJECTED = -1;
const int LABEL_NONE = 0;

const int NO_CHILD = -1;

int sofAlphabet; // the size of the alphabet

struct Node {
    int index;
    int label;
    vector<int> children;
    Node(int _index, int _label) {
        index = _index; label = _label;
        for (int i = 0; i < sofAlphabet; ++i) {
            children.push_back(NO_CHILD);
        }
    }
};

struct APTA {
    vector<Node*> nodes;
} apta; // the global APTA tree variable

/**
 * The input file should be in the following format:
 * 5 2 {the number of examples, the size of the alphabet}
 * 1 1 1 {example|counter-example, the word length, word}
 * 1 4 1 0 1 1
 * 1 2 0 0
 * 0 3 1 0 0
 * 0 1 0
 * The alphabet need to encoded with numbers.
 * 
 * The APTA tree constains nodes (as a vector). A node contains the index (corresponding to the nodes' vector), the label and children, which is a vector of indices.
 * The number of children is fixed and equals to the size of the alphabet.
 */
void build_APTA_from_file(string filepath) {
    LOG(INFO) << "Starting reading from the filepath: " << filepath;
    // reads data from an input file
    ifstream in(filepath); string line; getline(in, line); vector<int> data = split(line);
    sofAlphabet = data[1];
    int nofExamples = data[0];
    LOG(INFO) << "The size of the alphabet = " << sofAlphabet;
    // creates the root node
    Node* root = new Node(0, LABEL_NONE);
    apta.nodes.push_back(root);
    for (int i = 0; i < nofExamples; ++i) {
        if (i % 1000 == 0) {
            LOG(INFO) << "...parsing examples = " << i << " / " << nofExamples;
        }
        getline(in, line); data = split(line);
        int isAccepted = data[0];
        int sofExample = data[1]; // the size of example
        int current = 0;
        for (int j = 0; j < sofExample; ++j) {
            if (apta.nodes[current]->children[data[2 + j]] == NO_CHILD) {
                Node* child = new Node(apta.nodes.size(), LABEL_NONE);
                apta.nodes[current]->children[data[2 + j]] = child->index;
                apta.nodes.push_back(child);
            }
            current = apta.nodes[current]->children[data[2 + j]];
        }
        apta.nodes[current]->label = isAccepted ? LABEL_ACCEPTED : LABEL_REJECTED;
    }
    in.close();
    LOG(INFO) << "...parsing examples = " << nofExamples << " / " << nofExamples;
    LOG(INFO) << "The APTA tree is built";
}

void verify_APTA_from_file(string filepath) {
    LOG(INFO) << "Starting verification from the filepath: " << filepath;
    // reads data from an input file
    ifstream in(filepath); string line; getline(in, line); vector<int> data = split(line);
    sofAlphabet = data[1];
    int nofExamples = data[0];
    LOG(INFO) << "The size of the alphabet = " << sofAlphabet;
    for (int i = 0; i < nofExamples; ++i) {
        getline(in, line); data = split(line);
        int isAccepted = data[0];
        int sofExample = data[1];
        int current = 0;
        for (int j = 0; j < sofExample; ++j) {
            current = apta.nodes[current]->children[data[2 + j]];
        }
        bool isCorrect = (isAccepted == 1 && apta.nodes[current]->label == LABEL_ACCEPTED)
            || (isAccepted == 0 && apta.nodes[current]->label == LABEL_REJECTED);
        if (isCorrect) {
            LOG(INFO) << "...example #" << i << " is correct";
        } else {
            LOG(ERROR) << "...example #" << i << " is incorrect!, DFA says this is " << apta.nodes[current]->label;
        }
    }
}

/**
 * The format:
 * {the number of states}
 * {the size of the alphabet}
 * {accepting states, separated by space, preceded with the number of accepting states}
 * {rejecting states, separated by space, preceded with the number of rejecting states}
 * {the state 0}
 * {the state 1}
 * ...
 * 
 * Note that 0 is always the start state.
 */
string get_dfa(bool log) {
    int map1[apta.nodes.size()]; // the states map
    int visited[apta.nodes.size()]; // the states map
    for (int i = 0; i < apta.nodes.size(); ++i) {
        visited[i] = -1;
    }
    int map2[apta.nodes.size()];
    vector<int> acceptedStates;
    vector<int> rejectedStates;
    int stateIndex = 0;
    queue<int> nodes;
    nodes.push(0); // add the root node
    visited[0] = 1;
    while (nodes.empty() == false) {
        int node = nodes.front(); nodes.pop();
        map1[node] = stateIndex;
        if (apta.nodes[node]->label == LABEL_ACCEPTED) {
            acceptedStates.push_back(stateIndex);
        } else if (apta.nodes[node]->label == LABEL_REJECTED) {
            rejectedStates.push_back(stateIndex);
        }
        map2[stateIndex++] = node;
        for (int i = 0; i < sofAlphabet; ++i) {
            if (apta.nodes[node]->children[i] != NO_CHILD && visited[apta.nodes[node]->children[i]] == -1) {
                nodes.push(apta.nodes[node]->children[i]);
                visited[apta.nodes[node]->children[i]] = 1;
            }
        }
    }
    string output = to_string(stateIndex) + "\n" + to_string(sofAlphabet);
    output += "\n" + to_string(acceptedStates.size());
    for (int i = 0; i < acceptedStates.size(); ++i) {
        output += " " + to_string(acceptedStates[i]);
    }
    output += "\n" + to_string(rejectedStates.size());
    for (int i = 0; i < rejectedStates.size(); ++i) {
        output += " " + to_string(rejectedStates[i]);
    }
    for (int i = 0; i < stateIndex; ++i) {
        output += "\n";
        for (int j = 0; j < sofAlphabet; ++j) {
            output += j == 0 ? "" : " ";
            if (apta.nodes[map2[i]]->children[j] == NO_CHILD) {
                output += "-1";
            } else {
                output += to_string(map1[apta.nodes[map2[i]]->children[j]]);
            }
        }
    }
    if (log) {
        LOG(INFO) << "The number of states before EXBAR = " << apta.nodes.size();
        LOG(INFO) << "The number of states after EXBAR = " << stateIndex;
    }
    return output;
}

string get_yaml_dfa() {
    int map1[apta.nodes.size()]; // the states map
    int visited[apta.nodes.size()]; // the states map
    for (int i = 0; i < apta.nodes.size(); ++i) {
        visited[i] = -1;
    }
    int map2[apta.nodes.size()];
    vector<int> acceptedStates;
    vector<int> rejectedStates;
    int stateIndex = 0;
    queue<int> nodes;
    nodes.push(0); // add the root node
    visited[0] = 1;
    while (nodes.empty() == false) {
        int node = nodes.front(); nodes.pop();
        map1[node] = stateIndex;
        if (apta.nodes[node]->label == LABEL_ACCEPTED) {
            acceptedStates.push_back(stateIndex);
        } else if (apta.nodes[node]->label == LABEL_REJECTED) {
            rejectedStates.push_back(stateIndex);
        }
        map2[stateIndex++] = node;
        for (int i = 0; i < sofAlphabet; ++i) {
            if (apta.nodes[node]->children[i] != NO_CHILD && visited[apta.nodes[node]->children[i]] == -1) {
                nodes.push(apta.nodes[node]->children[i]);
                visited[apta.nodes[node]->children[i]] = 1;
            }
        }
    }
    string output = "number of states: " + to_string(stateIndex) + "\nsize of alphabet: " + to_string(sofAlphabet) + "\n";
    output += "accepting states: [";
    for (int i = 0; i < acceptedStates.size(); ++i) {
        output += (i == 0 ? "" : ", ") + to_string(acceptedStates[i]);
    }
    output += "]\nrejecting states: [";
    for (int i = 0; i < rejectedStates.size(); ++i) {
        output += (i == 0 ? "" : ", ") + to_string(rejectedStates[i]);
    }
    output += "]\ninitial state: 0\ntransitions:";
    for (int i = 0; i < stateIndex; ++i) {
        output += "\n- [" + to_string(i);
        for (int j = 0; j < sofAlphabet; ++j) {
            if (apta.nodes[map2[i]]->children[j] == NO_CHILD) {
                output += ", -1";
            } else {
                output += ", " + to_string(map1[apta.nodes[map2[i]]->children[j]]);
            }
        }
        output += "]";
    }
    return output;
}

void print_dfa() {
    string dfa = get_dfa(true);
    cout << "The current DFA:\n" << dfa << endl;
}

// The changes stack handles all changes that can be reverted
stack<tuple<int, string, int, int>> changes;

int set_label(int nodeIndex, int label) {
    changes.push(make_tuple(nodeIndex, "label", apta.nodes[nodeIndex]->label, -1));
    apta.nodes[nodeIndex]->label = label;
    return 1;
}

int set_child(int nodeIndex, int letter, int child) {
    changes.push(make_tuple(nodeIndex, "child", apta.nodes[nodeIndex]->children[letter], letter));
    apta.nodes[nodeIndex]->children[letter] = child;
    return 1;
}

int set_father_point(int redIndex, int blueIndex) {
    int nofChanges = 0;
    for (int i = 0; i < apta.nodes.size(); ++i) {
        for (int j = 0; j < sofAlphabet; ++j) {
            if (apta.nodes[i]->children[j] == blueIndex) {
                set_child(i, j, redIndex);
                nofChanges++;
            }
        }
    }
    return nofChanges;
}

void undo_changes(int nofChanges) {
    while (nofChanges > 0) {
        tuple<int, string, int, int> change = changes.top(); changes.pop();
        if (get<1>(change) == "label") {
            apta.nodes[get<0>(change)]->label = get<2>(change);
        } else if (get<1>(change) == "child") {
            apta.nodes[get<0>(change)]->children[get<3>(change)] = get<2>(change);
        }
        nofChanges--;
    }
}
