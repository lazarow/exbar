#include<vector>
#include<algorithm>
#include<chrono>
#include<climits>
#include "apta.h"
using namespace std;

INITIALIZE_EASYLOGGINGPP

int nofSearchingCalls = 0; // the number of `exh_search` calls

std::ofstream graphmlOut("test.graphml");

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
    nofChanges += set_father_point(redNode, blueNode); // creates loops
    try {
        walkit(redNode, blueNode, nofChanges);
    } catch (int e) {
        //LOG(DEBUG) << "[-] The red node and blue node has not been merged: red node #" << redNode << ", blue node #" << blueNode;
        return false;
    }
    //LOG(DEBUG) << "[+] The red node and blue node has been merged: red node #" << redNode << ", blue node #" << blueNode;
    return true;
}

int pickBlueNode() {
    vector<int> redNodes;
    vector<int> blueNodes;
    for (int i = 0; i < apta.nodes.size(); ++i) {
        if (apta.nodes[i]->color == COLOR_RED) {
            redNodes.push_back(i);
            for (int j = 0; j < apta.nodes[i]->children.size(); ++j) {
                int child = apta.nodes[i]->children[j];
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
    return bestBlueNode;
}

int maxRed = 1;
int bestSolutionNofRedNodes = INT_MAX;
string solutionDfa = "";

void exh_search() {
    if (getNumberOfRedNodes() <= maxRed) {
        nofSearchingCalls++;
        LOG(DEBUG) << "Looking for the best possible blue node";
        //getchar();
        int blueNode = pickBlueNode();
        LOG(DEBUG) << "The blue node has been picked: node #" << blueNode;
        //getchar();
        if (blueNode != -1) {
            for (int i = 0; i < apta.nodes.size(); ++i) { // try all red nodes
                if (apta.nodes[i]->color == COLOR_RED) {
                    int nofChanges = 0;
                    if (try_merge(i, blueNode, nofChanges)) {
                        //getchar();
                        nofChanges += set_as_merged(blueNode);
                        LOG(DEBUG) << "Recurse call after the merge";
                        exh_search();
                    }
                    LOG(DEBUG) << "Reverting changes";
                    undo_changes(nofChanges);
                }
            }
            LOG(DEBUG) << "No merge possible for the picked blue node";
            apta.nodes[blueNode]->color = COLOR_RED;
            //getchar();
            exh_search();
        } else { // no more blue nodes
            throw 1;
            // save the solution
            /*if (getNumberOfRedNodes() < bestSolutionNofRedNodes) {
                LOG(INFO) << "The solution has been found";
                getchar();
                bestSolutionNofRedNodes = getNumberOfRedNodes();
                solutionDfa = get_dfa(false);
            }*/
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
    graphmlOut << get_graphml_header();
    //---
    clock_t time = clock();
    apta.nodes[0]->color = COLOR_RED;
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
    LOG(INFO) << "The number of searching calls = " << nofSearchingCalls;
    LOG(INFO) << "The measured time in ms = " << ms;
    print_dfa();
    //---
    LOG(INFO) << "Saving the generated DFA to files";
    graphmlOut << get_graphml_dfa();
    graphmlOut << get_graphml_footer();
    graphmlOut.close();
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