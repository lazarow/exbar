#include<vector>
#include <algorithm>
#include "apta.h"
using namespace std;

INITIALIZE_EASYLOGGINGPP

bool walkit(int redNode, int blueNode) {
    LOG(DEBUG) << "... ... ...walkit, red node = " << redNode << ", blue node = " << blueNode;
    if (apta.nodes[blueNode]->label != LABEL_NONE) {
        if (apta.nodes[redNode]->label != LABEL_NONE) {
            if (apta.nodes[redNode]->label != apta.nodes[blueNode]->label) {
                LOG(DEBUG) << "... ... ... ...inconsistent nodes";
                return false; // returns to caller of `try_merge()`
            }
        } else {
            LOG(DEBUG) << "... ... ... ...copying label = " << apta.nodes[blueNode]->label;
            set_label(redNode, apta.nodes[blueNode]->label); // copy label
        }
    }
    for (int i = 0; i < sofAlphabet; ++i) {
        if (apta.nodes[blueNode]->children[i] != NO_CHILD) {
            if (apta.nodes[redNode]->children[i] != NO_CHILD) {
                // recurse
                LOG(DEBUG) << "... ... ... ...determinization, letter = " << i << ", going into recursion";
                if (walkit(apta.nodes[redNode]->children[i], apta.nodes[blueNode]->children[i]) == false) {
                    return false;
                }
            } else {
                LOG(DEBUG) << "... ... ... ...splice branch, letter = " << i << ", child node = " << apta.nodes[blueNode]->children[i];
                set_child(redNode, i, apta.nodes[blueNode]->children[i]); // splice branch
            }
        }
    }
    return true;
}

bool try_merge(int redNode, int blueNode) {
    LOG(DEBUG) << "... ...try merging, red node = " << redNode << ", blue nodes = " << blueNode;     
    set_father_point(redNode, blueNode);
    walkit(redNode, blueNode);
    return false;
}

int maxRed = 1;

bool exh_search(vector<int> redNodes) {
    if (redNodes.size() <= maxRed) {
        vector<int> blueNodes;
        for (int i = 0; i < redNodes.size(); ++i) {
            for (int j = 0; j < sofAlphabet; ++j) {
                int child = apta.nodes[redNodes[i]]->children[j];
                if (child != NO_CHILD && find(redNodes.begin(), redNodes.end(), child) == redNodes.end()) {
                    blueNodes.push_back(child);
                }
            }
        }
        LOG(INFO) << "...max red = " << maxRed << ", red nodes = " << print_vector(redNodes) << ", blue nodes = " << print_vector(blueNodes); 
        if (blueNodes.size() == 0) {
            return true;
        } else {
            for (int i = 0; i < blueNodes.size(); ++i) {
                for (int j = 0; j < redNodes.size(); ++j) {
                    if (try_merge(redNodes[j], blueNodes[i])) {
                        if (exh_search(redNodes)) {
                            return true;
                        }
                    }
                    LOG(DEBUG) << "... ...reverting all changes";
                    undo_changes();
                }
                //vector<int> extendedRedNodes(redNodes);
                redNodes.push_back(blueNodes[i]);
                if (exh_search(redNodes)) {
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
    build_APTA_from_file("./samples/sample3.txt");
    LOG(INFO) << "Starting EXBAR";
    vector<int> redNodes;
    redNodes.push_back(0);
    while (exh_search(redNodes) == false) {
        maxRed++;
    }
    print_dfa();
    return 0;
}