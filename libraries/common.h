#include <vector>
#include <string>
#include <fstream>

using namespace std;

vector<int> split(string input)
{
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
    if (buffer != "" && buffer != "\r") {
        output.push_back(stoi(buffer));
    }
    return output;
}

string printVector(vector<int> input)
{
    string output = "";
    for (int i = 0; i < input.size(); ++i) {
        output += (i == 0 ? "" : " ") + to_string(input[i]);
    }
    return output;
}

char* getCmdOption(char ** begin, char ** end, const string & option)
{
    char ** itr = find(begin, end, option);
    if (itr != end && ++itr != end) {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const string& option)
{
    return find(begin, end, option) != end;
}

vector<int> getdata(ifstream in)
{
    string line;
    getline(in, line);
    return split(line);
}

class InputParser{
    public:
        InputParser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(string(argv[i]));
        }
        /// @author iain
        const string& getCmdOption(const string &option) const{
            vector<string>::const_iterator itr;
            itr =  find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            static const string empty_string("");
            return empty_string;
        }
        /// @author iain
        bool cmdOptionExists(const string &option) const{
            return find(this->tokens.begin(), this->tokens.end(), option)
                   != this->tokens.end();
        }
    private:
        vector <string> tokens;
};