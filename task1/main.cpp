#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>
#include <dirent.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>

using namespace std;

//to access all environment variables
extern char **environ;

//to create a copy of the current input/output mode
void saveIO(int &saved_in, int &saved_out) {
    saved_in = dup(STDIN_FILENO);
    saved_out = dup(STDOUT_FILENO);
}

//switch back to default input mode
void restoreIO(int saved_in, int saved_out) {
    if (saved_in != -1) {
        dup2(saved_in, STDIN_FILENO);
        close(saved_in);
    }
    if (saved_out != -1) {
        dup2(saved_out, STDOUT_FILENO);
        close(saved_out);
    }
}

//change the mode
bool applyRedirection(const string &inputFile, const string &outputFile, bool appendMode) {
    if (!inputFile.empty()) {
        int fd = open(inputFile.c_str(), O_RDONLY);
        if (fd < 0) {
            perror("Input file error");
            return false;
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if (!outputFile.empty()) {
        int flags = O_WRONLY | O_CREAT;
        flags |= (appendMode ? O_APPEND : O_TRUNC);

        // 0644 gives read/write to owner, read to others
        int fd = open(outputFile.c_str(), flags, 0644);
        if (fd < 0) {
            perror("Output file error");
            return false;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    return true;
}


int main(int argc, char *argv[]) {
    string command;
    //the default input mode is cin
    istream *input_source = &cin;
    ifstream file_stream;
    int saved_stdin = -1;
    int saved_stdout = -1;
    //if there is a batch file provided switch to file
    if (argc > 1) {
        file_stream.open(argv[1]);
        if (!file_stream.is_open()) {
            cout << "Error: Could not open file " << argv[1] << endl;
            return 1;
        }
        input_source = &file_stream;
    }
    while (1) {
        // the current working directory
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        if (input_source == &cin) {
            // simulating an actual terminal
            cout << cwd << " $ ";
        }
        //if there is no more input left to read break the loop (for batch files)
        //it also freezes the loop until input in the normal mode
        if (!getline(*input_source, command)) {
            break;
        }
        // continue the loop if the input was empty
        if (command.empty())
            continue;
        if (command == "quit")
            break;

        // create a stream of the command to be able to split it into words
        stringstream ss(command);
        string word;
        vector<string> words;
        bool backgroundRunning = false;

        // loop to split the command by spaces
        while (ss >> word) {
            // skipping the spaces
            words.push_back(word);
        }
        //check if there is & at the end
        if (words.back() == "&") {
            backgroundRunning = true;
            words.pop_back();
        }

        string inputFile;
        string outputFile;
        bool appendMode = false;

        //loop to check for I/O redirection symbols
        for (int i = 0; i < words.size(); i++) {
            if (words[i] == "<") {
                if (i + 1 < words.size()) {
                    inputFile = words[i + 1];
                    // Remove "<" and "file"
                    words.erase(words.begin() + i, words.begin() + i + 2);
                    //update the index since elements are removed
                    i--;
                } else {
                    cout << "Error: No input file specified.\n";
                }
            } else if (words[i] == ">") {
                if (i + 1 < words.size()) {
                    outputFile = words[i + 1];
                    appendMode = false;
                    words.erase(words.begin() + i, words.begin() + i + 2);
                    i--;
                } else {
                    cout << "Error: No output file specified.\n";
                }
            } else if (words[i] == ">>") {
                if (i + 1 < words.size()) {
                    outputFile = words[i + 1];
                    appendMode = true;
                    words.erase(words.begin() + i, words.begin() + i + 2);
                    i--;
                } else {
                    cerr << "Error: No output file specified.\n";
                }
            }
        }
        //checking if the user did not enter a valid command
        if (words.empty()) continue;

        if (words[0] == "cd") {
            if (words.size() > 1) {
                char *path = words[1].data(); //to make it C style
                if (chdir(path) != 0) {
                    cout << "Error the directory does not exist\n";
                } else {
                    getcwd(cwd, sizeof(cwd));
                    setenv("PWD", cwd, 1);
                }
            } else
                cout << cwd << "\n";
        } else if (words[0] == "dir") {
            int saved_in = -1, saved_out = -1;
            saveIO(saved_in, saved_out); //save the current mode
            //check if there will be mood change
            if (applyRedirection(inputFile, outputFile, appendMode)) {
                //check if there is a directory name entered
                const char *path = (words.size() > 1) ? words[1].c_str() : ".";
                DIR *dir = opendir(path);
                if (!dir) {
                    //no directory with the entered name
                    perror("dir");
                } else {
                    //a pointer to the data inside the current directory
                    dirent *entry;
                    while ((entry = readdir(dir)) != nullptr) {
                        cout << entry->d_name << " ";
                    }
                    cout << endl;
                    closedir(dir);
                }
            }
            //switch the mode back to normal
            restoreIO(saved_in, saved_out);
        } else if (words[0] == "environ") {
            int saved_in = -1, saved_out = -1;
            saveIO(saved_in, saved_out);

            if (applyRedirection(inputFile, outputFile, appendMode)) {
                //printing all the environment variables
                for (char **env = environ; *env != 0; env++) {
                    cout << *env << endl;
                }
            }
            restoreIO(saved_in, saved_out);
        } else if (words[0] == "set") {
            if (words.size() < 3) {
                cerr << "Error: usage is 'set VARIABLE VALUE'\n";
            } else {
                setenv(words[1].c_str(), words[2].c_str(), 1);
            }
        } else if (words[0] == "echo") {
            int saved_in = -1, saved_out = -1;
            saveIO(saved_in, saved_out);

            if (applyRedirection(inputFile, outputFile, appendMode)) {
                for (size_t i = 1; i < words.size(); i++) {
                    cout << words[i] << (i == words.size() - 1 ? "" : " ");
                }
                cout << "\n";
            }
            restoreIO(saved_in, saved_out);
        } else if (words[0] == "help") {
            int saved_in = -1, saved_out = -1;
            saveIO(saved_in, saved_out);

            if (applyRedirection(inputFile, outputFile, appendMode)) {
                if (words.size() == 1) {
                    cout << "This is a terminal simulation system that has support to few commands including:\n"
                            << "cd\n"
                            << "dir\n"
                            << "environ\n"
                            << "set\n"
                            << "echo\n"
                            << "help\n"
                            << "pause\n"
                            << "to know more about a command enter help and the command \n";
                } else if (words[1] == "cd")
                    cout << "The cd functions changes the current directory.\n"
                            << "to use it, enter cd followed by a space and the directory name you need to move to\n";
                else if (words[1] == "dir")
                    cout << "The dir function shows the current items in the directory\n"
                            << "to use it, enter dir\n";
                else if (words[1] == "environ")
                    cout << "The environ function shows the environment variables \n"
                            << "to use it, enter environ followed by a space and then the variable you want to see\n";
                else if (words[1] == "set")
                    cout <<
                            "The set function allows the user to change an environment variable and create a new variable if does not exisit\n"
                            << "to use it, enter set followed by a space and then the variable\n";
                else if (words[1] == "echo")
                    cout << "The echo function prints the string entered after it\n"
                            << "to use it, enter echo followed by the string you want to print\n";
                else if (words[1] == "pause")
                    cout << "The pause function pauses the shell until the enter key is pressed\n"
                            << "to use it, enter pause and to resume the shell press enter\n";
                cout.flush();
                restoreIO(saved_in, saved_out);
            }
        } else if (words[0] == "pause") {
            cout << "Shell is paused press enter to resume";
            string buffer;
            getline(cin, buffer);
        } else {
            pid_t pid = fork();
            if (pid < 0)
                cout << "Fork Failed\n";
            else if (pid == 0) {
                vector<char *> args;
                for (auto &s: words) args.push_back(s.data());
                args.push_back(nullptr);
                //check if there is an input file
                if (!applyRedirection(inputFile, outputFile, appendMode)) {
                    exit(1);
                }
                execvp(args[0], args.data());
                perror("Error in executing command\n");
                exit(1);
            } else {
                if (backgroundRunning) {
                    cout << "The process is running in the background, pid: " << pid << "\n";
                } else {
                    int status;
                    //the waiting process
                    //the status to know how the process got terminated and can be null if we dont care about the process
                    //options to show how to wait either freeze or not
                    waitpid(pid, &status, 0);
                }
            }
        }
    }

    return 0;
}