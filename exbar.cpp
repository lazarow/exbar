#include<vector>
#include<algorithm>
#include<chrono>
#include "apta.h"
using namespace std;

INITIALIZE_EASYLOGGINGPP

int nofMergeTries = 0;
int nofSearchingCalls = 0;

bool walkit(int redNode, int blueNode, int &nofChanges) {
    LOG(DEBUG) << "... ... ...walkit, red node = " << redNode << ", blue node = " << blueNode;
    if (apta.nodes[blueNode]->label != LABEL_NONE) {
        if (apta.nodes[redNode]->label != LABEL_NONE) {
            if (apta.nodes[redNode]->label != apta.nodes[blueNode]->label) {
                LOG(DEBUG) << "... ... ... ...inconsistent nodes";
                return false; // returns to caller of `try_merge()`
            }
        } else {
            LOG(DEBUG) << "... ... ... ...copying label = " << apta.nodes[blueNode]->label;
            nofChanges += set_label(redNode, apta.nodes[blueNode]->label); // copy label
        }
    }
    for (int i = 0; i < sofAlphabet; ++i) {
        if (apta.nodes[blueNode]->children[i] != NO_CHILD) {
            if (apta.nodes[redNode]->children[i] != NO_CHILD) {
                // recurse
                LOG(DEBUG) << "... ... ... ...determinization, letter = " << i << ", going into recursion";
                if (walkit(apta.nodes[redNode]->children[i], apta.nodes[blueNode]->children[i], nofChanges) == false) {
                    return false;
                }
            } else {
                LOG(DEBUG) << "... ... ... ...splice branch, letter = " << i << ", child node = " << apta.nodes[blueNode]->children[i];
                nofChanges += set_child(redNode, i, apta.nodes[blueNode]->children[i]); // splice branch
            }
        }
    }
    LOG(DEBUG) << "... ... ... ...the merge is completed";
    return true;
}

bool try_merge(int redNode, int blueNode, int &nofChanges) {
    nofMergeTries++;
    LOG(DEBUG) << "... ...try merging, red node = " << redNode << ", blue nodes = " << blueNode;     
    nofChanges += set_father_point(redNode, blueNode); // creates loops
    return walkit(redNode, blueNode, nofChanges);
}

int maxRed = 1;

bool exh_search(vector<int> redNodes) {
    if (redNodes.size() <= maxRed) {
        nofSearchingCalls++;
        vector<int> blueNodes;
        for (int i = 0; i < redNodes.size(); ++i) {
            for (int j = 0; j < sofAlphabet; ++j) {
                int child = apta.nodes[redNodes[i]]->children[j];
                if (child != NO_CHILD && find(redNodes.begin(), redNodes.end(), child) == redNodes.end()) {
                    blueNodes.push_back(child);
                }
            }
        }
        if (nofSearchingCalls % 1000 == 0) {
            LOG(INFO) << "...max red = " << maxRed << ", red nodes = " << print_vector(redNodes) << ", blue nodes = " << print_vector(blueNodes); 
        }
        LOG(DEBUG) << "...[debug] max red = " << maxRed << ", red nodes = " << print_vector(redNodes) << ", blue nodes = " << print_vector(blueNodes); 
        if (blueNodes.size() == 0) {
            return true;
        } else {
            for (int i = 0; i < blueNodes.size(); ++i) {
                for (int j = 0; j < redNodes.size(); ++j) {
                    int nofChanges = 0;
                    if (try_merge(redNodes[j], blueNodes[i], nofChanges)) {
                        if (exh_search(redNodes)) {
                            return true;
                        }
                    }
                    LOG(DEBUG) << "... ...reverting all changes";
                    undo_changes(nofChanges);
                }
                vector<int> extendedRedNodes(redNodes);
                extendedRedNodes.push_back(blueNodes[i]);
                if (exh_search(extendedRedNodes)) {
                    return true;
                }
            }
        }
    }
    return false;
}

int main(int argc, char* argv[])
{
    START_EASYLOGGINGPP(argc, argv);
    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format, "[%datetime] %msg");
    // the algorithm starts here
    string filepath = getCmdOption(argv, argv + argc, "--file");
    build_APTA_from_file(filepath);
    LOG(INFO) << "Starting EXBAR";
    clock_t time = clock();
    vector<int> redNodes;
    redNodes.push_back(0);
    while (exh_search(redNodes) == false) {
        maxRed++;
    }
    time = clock() - time;
    int ms = time / CLOCKS_PER_SEC * 1000;
    LOG(INFO) << "The number of merge tries = " << nofMergeTries;
    LOG(INFO) << "The number of searching calls = " << nofSearchingCalls;
    LOG(INFO) << "The measured time in ms = " << ms;
    print_dfa();
    LOG(INFO) << "Saving the minified DFA";
    std::ofstream out1("dfa.txt");
    out1 << get_dfa(false);
    out1.close();
    std::ofstream out2("dfa.yaml");
    out2 << get_yaml_dfa();
    out2.close();
    if (cmdOptionExists(argv, argv+argc, "--verify")) {
        verify_APTA_from_file(filepath);
    }
    return 0;
}