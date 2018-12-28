#include <vector>
#include <string>
using namespace std;

vector<int> split(string input) {
    string buffer = "";
    vector<int> output;
    for (int i = 0; i < input.length(); ++i) {
        if (input.at(i) != ' ') {
            buffer += input.at(i);
        } else if (input.at(i) == ' ' && buffer != "") {
            output.push_back(stoi(buffer));
            buffer = "";
        }
    }
    if (buffer != "") {
        output.push_back(stoi(buffer));
    }
    return output;
}

string print_vector(vector<int> input) {
    string output = "";
    for (int i = 0; i < input.size(); ++i) {
        output += (i == 0 ? "" : " ") + to_string(input[i]);
    }
    return output;
}