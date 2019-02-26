#include<vector>
#include<algorithm>
#include<chrono>
#include<climits>
#include <unordered_map>
#include "apta.h"
using namespace std;

INITIALIZE_EASYLOGGINGPP

void walkit(int redNode, int blueNode, int &nofChanges) {
    if (apta.nodes[blueNode]->label != LABEL_NONE) {
        if (apta.nodes[redNode]->label != LABEL_NONE) {
            if (apta.nodes[redNode]->label != apta.nodes[blueNode]->label) {
                throw 0; // returns to caller of `try_merge()`
            }
        } else {
            nofChanges += set_label(redNode, apta.nodes[blueNode]->label); // copy label
        }
    }
    for (int i = 0; i < sofAlphabet; ++i) {
        if (apta.nodes[blueNode]->children[i] != NO_CHILD) {
            if (apta.nodes[redNode]->children[i] != NO_CHILD) {
                walkit(apta.nodes[redNode]->children[i], apta.nodes[blueNode]->children[i], nofChanges); // recurse
            } else {
                nofChanges += set_child(redNode, i, apta.nodes[blueNode]->children[i]); // splice branch
            }
        }
    }
}

bool try_merge(int redNode, int blueNode, int &nofChanges) {
    nofChanges += set_father_point(redNode, blueNode); // creates loops
    try {
        walkit(redNode, blueNode, nofChanges);
    } catch (int e) {
        return false;
    }
    return true;
}

unordered_map<string, int> pickBlueNodeCache; 

int pickBlueNode() {
    vector<int> redNodes;
    vector<int> blueNodes;
    string currentDfaHash = "";
    for (int i = 0; i < apta.nodes.size(); ++i) {
        if (apta.nodes[i]->color == COLOR_RED) {
            redNodes.push_back(i);
            currentDfaHash += string("#") + to_string(i);
            for (int j = 0; j < apta.nodes[i]->children.size(); ++j) {
                int child = apta.nodes[i]->children[j];
                currentDfaHash += string("|") + to_string(child);
                if (
                    child != NO_CHILD
                    && apta.nodes[child]->color == COLOR_NONE
                    && apta.nodes[child]->isMerged == false
                    && find(blueNodes.begin(), blueNodes.end(), child) == blueNodes.end()
                ) {
                    blueNodes.push_back(child);
                }
            }
        }
    }
    if (blueNodes.size() == 0) {
        return -1;
    }
    /*unordered_map<std::string, int>::const_iterator got = pickBlueNodeCache.find(currentDfaHash);
    if (got != pickBlueNodeCache.end()) {
        LOG(DEBUG) << "The blue node has been read from the cache";
        return got->second;
    }*/
    int bestNofPossibleMerges = INT_MAX;
    int bestBlueNode = -1;
    for (int i = 0; i < blueNodes.size(); ++i) {
        int nofPossibleMerges = 0;
        for (int j = 0; j < redNodes.size(); ++j) {
            int nofChanges = 0;
            if (try_merge(redNodes[j], blueNodes[i], nofChanges)) {
                nofPossibleMerges++;
            }
            undo_changes(nofChanges);
        }
        LOG(DEBUG) << "The number of possible merge for node #" << blueNodes[i] << ": " << nofPossibleMerges;
        if (nofPossibleMerges < bestNofPossibleMerges) {
            bestBlueNode = blueNodes[i];
            bestNofPossibleMerges = nofPossibleMerges;
        }
    }
    //pickBlueNodeCache[currentDfaHash] = bestBlueNode; // saves the best blue node for the current DFA
    return bestBlueNode;
}

int max_red = 1;

void exh_search() {
    if (getNumberOfRedNodes() > max_red) {
        return;
    }
    LOG(DEBUG) << "The limit of red nodes = " << max_red;
    LOG(DEBUG) << "Looking for the best possible blue node...";
    int blueNode = pickBlueNode();
    LOG(DEBUG) << "The blue node has been picked: " << blueNode;
    if (blueNode != -1) {
        for (int i = 0; i < apta.nodes.size(); ++i) {
            if (apta.nodes[i]->color == COLOR_RED) {
                int nofChanges = 0;
                if (try_merge(i, blueNode, nofChanges)) {
                    LOG(DEBUG) << "Trying merging the blue node = " << blueNode << " and the red node = " << i << ": success";
                    nofChanges += set_as_merged(blueNode);
                    exh_search();
                } else {
                    LOG(DEBUG) << "Trying merging the blue node = " << blueNode << " and the red node = " << i << ": failure";
                }
                undo_changes(nofChanges);
            }
        }
        LOG(DEBUG) << "The blue node = " << blueNode << " has been transformed into a red node";
        set_color(blueNode, COLOR_RED);
        exh_search();
        undo_changes(1); // reverts the last red node transformation
    } else {
        throw 1;
    }
}

int main(int argc, char* argv[])
{
    START_EASYLOGGINGPP(argc, argv);
    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format, "[%datetime] %msg");
    // the algorithm starts here
    string filepath = getCmdOption(argv, argv + argc, "--file");
    max_red = atoi(getCmdOption(argv, argv + argc, "--max"));
    if (max_red < 1) {
        max_red = 1;
    }
    build_APTA_from_file(filepath);
    LOG(INFO) << "[ exbar ]";
    //---
    clock_t time = clock();
    apta.nodes[0]->color = COLOR_RED;
    int iteration = 0;
    while (true) {
        try {
            LOG(INFO) << "The begin of the iteration = " << ++iteration << ", the limit of red nodes = " << max_red;
            exh_search();
            LOG(INFO) << "The end of the iteration = " << iteration;
            //getchar(); // test
            max_red++;
        } catch (int e) {
            LOG(INFO) << "The solution has been found";
            break;
        }
    }
    time = clock() - time;
    int ms = time / CLOCKS_PER_SEC * 1000;
    //---
    LOG(INFO) << "[ RESULT ]";
    LOG(INFO) << "The measured time in ms = " << ms;
    print_dfa();
    //---
    LOG(INFO) << "Saving the generated DFA to files";
    std::ofstream out1("dfa.txt");
    out1 << get_dfa(false);
    out1.close();
    std::ofstream out2("dfa.yaml");
    out2 << get_yaml_dfa();
    out2.close();
    //---
    if (cmdOptionExists(argv, argv+argc, "--verify")) {
        verify_APTA_from_file(filepath);
    }
    return 0;
}