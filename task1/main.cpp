#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h> // for path max
#include <dirent.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sstream>

using namespace std;

int main () {
    string command;

    while (1) {
        char cwd [PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        cout << cwd << " $ " ;

        getline(cin, command);
        if (command.empty()) continue;
        else if (command == "exit") {
            break;
        }

        stringstream ss(command);
        string word;
        vector<string> words;
        
        while (ss >> word) {
            //skiping the spaces 
            words.push_back(word);
        }
        cout << words[0] <<"\n";

    }

   return 0;
}