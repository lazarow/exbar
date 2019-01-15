#include<vector>
#include<algorithm>
#include<chrono>
#include "apta.h"
using namespace std;

INITIALIZE_EASYLOGGINGPP

int nofMergeTries = 0; // the number of merging tries
int nofSearchingCalls = 0; // the number of `exh_search` calls

void walkit(int redNode, int blueNode, int &nofChanges)
{
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
    nofMergeTries++;   
    nofChanges += set_father_point(redNode, blueNode); // creates loops
    try {
        walkit(redNode, blueNode, nofChanges);
    } catch (int e) {
        LOG(DEBUG) << "[-] The red node and blue node has not been merged: red node #" << redNode << ", blue node #" << blueNode;
        return false;
    }
    LOG(DEBUG) << "[+] The red node and blue node has been merged: red node #" << redNode << ", blue node #" << blueNode;
    return true;
}

int maxRed = 1;

void exh_search() {
    if (nofRedNodes <= maxRed) {
        nofSearchingCalls++;
        if (nofBlueNodes == 0) {
            throw 1;
        } else {
            int blueNode = pickBlueNode(0);
            LOG(DEBUG) << "The blue node has been picked: node #" << blueNode;
            if (blueNode != -1) {
                for (int j = 0; j < redNodes.size(); ++j) {
                    int nofChanges = 0;
                    nofChanges += set_color(blueNode, COLOR_MERGED, true);
                    if (try_merge(redNodes[j], blueNode, nofChanges)) {
                        LOG(DEBUG) << "Recolor the red node in order to color new blue nodes";
                        nofChanges += set_color(redNodes[j], COLOR_RED, true);
                        getchar();
                        LOG(DEBUG) << "Recurse call after the merge";
                        exh_search();
                    }
                    LOG(DEBUG) << "Reverting changes";
                    undo_changes(nofChanges);
                }
                LOG(DEBUG) << "No merge possible for the picked blue node";
                set_color(blueNode, COLOR_RED, true);
                getchar();
                exh_search();
            }
        }
    }
}

int main(int argc, char* argv[])
{
    START_EASYLOGGINGPP(argc, argv);
    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format, "[%datetime] %msg");
    // the algorithm starts here
    string filepath = getCmdOption(argv, argv + argc, "--file");
    build_APTA_from_file(filepath);
    LOG(INFO) << "[ exbar ]";
    //---
    clock_t time = clock();
    set_color(0, COLOR_RED, true);
    while (true) {
        try {
            exh_search();
            maxRed++;
        } catch (int e) {
            LOG(INFO) << "The solution has been found";
            break;
        }
    }
    time = clock() - time;
    int ms = time / CLOCKS_PER_SEC * 1000;
    //---
    LOG(INFO) << "[ RESULT ]";
    LOG(INFO) << "The number of merge tries = " << nofMergeTries;
    LOG(INFO) << "The number of searching calls = " << nofSearchingCalls;
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