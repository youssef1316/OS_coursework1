#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <limits.h>
#include <dirent.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sstream>
#include <filesystem>

using namespace std;

int main()
{
    string command;

    while (1)
    {
        // the current working directory
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        // simulating an actual terminal
        cout << cwd << " $ ";

        getline(cin, command);
        // continue the loop if the input was empty
        if (command.empty())
            continue;
        if (command == "quit")
            break;

        // create a stream of the command to be able to split it into words
        stringstream ss(command);
        string word;
        vector<string> words;

        // loop to split the command by spaces
        while (ss >> word)
        {
            // skipping the spaces
            words.push_back(word);
        }

        if (words[0] == "cd")
        {
            if (words.size() > 1)
            {
                char *path = words[1].data();
                if (chdir(path) != 0)
                {
                    cout << "Error the directory doesnt exit\n";
                }
                else
                {
                    getcwd(cwd, sizeof(cwd));
                    setenv("PWD", cwd, 1);
                }
            }
            else
                cout << "Error: no path entered\n";
        }

        else if (words[0] == "dir")
        {
            // point to the current directory
            DIR *dir = opendir(".");
            struct dirent *entry;
            // read a file -> if valid -> print its name
            while ((entry = readdir(dir)) != nullptr)
            {
                cout << entry->d_name << ' ';
            }
            // the .. is the previous directory and . is the current
            cout << "\n";
            // close the folder
            closedir(dir);
        }

        else if (words[0] == "environ")
        {
            if (words.size() > 1) {
                char *variable = words[1].data();
                char *result = getenv(variable);
                if (result != nullptr)
                    cout << variable << " : " << result << '\n';
                else
                    cout << "Error enter a valid command (environ USER)\n";
            }
            else
                cout << "Error enter a valid command (environ USER)\n";
        }

        else if (words[0] == "set")
        {
            if (words.size() > 1)
            {
                char *variable = words[1].data();
                if (words.size() > 2)
                {
                    char *value = words[2].data();
                    setenv(variable, value, 1);
                    cout << variable << " = " << value << "\n";
                }
                else
                    cout << "Error missing command must be set variable value\n";
            }
            else
                cout << "Error missing command must be set variable value\n";
        }

        else if (words[0] == "echo")
        {
            for (int i = 1; i < words.size(); i++) {
                cout << words[i] << " " ;
            }
            cout << "\n";
        }

        else if (words[0] == "help")
        {
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
           }
           else if (words[1] == "cd")
                cout <<"The cd functions changes the current directory.\n"
                << "to use it, enter cd followed by a space and the directory name you need to move to\n";
           else if (words[1] == "dir")
                cout <<"The dir function shows the current items in the directory\n"
                << "to use it, enter dir\n";
           else if (words[1] == "environ")
                cout << "The environ function shows the environment variables \n"
                << "to use it, enter environ followed by a space and then the variable you want to see\n";
           else if (words[1] == "set")
                cout <<"The set function allows the user to change an environment variable and create a new variable if does not exisit\n"
                << "to use it, enter set followed by a space and then the variable\n";
           else if (words[1] == "echo")
                cout <<"The echo function prints the string entered after it\n"
                << "to use it, enter echo followed by the string you want to print\n";
           else if (words[1] == "pause")
                cout << "The pause function pauses the shell until the enter key is pressed\n"
                << "to use it, enter pause and to resume the shell press enter\n";
        }
        else if (words[0] == "pause") {
            cout << "Shell is paused press enter to resume";
            string buffer;
            getline(cin, buffer);
        }
        else {
            pid_t pid = fork();
            if (pid < 0)
                cout << "Fork Failed\n";
            else if (pid == 0) {
                vector<char*> args;
                for (auto &s : words) args.push_back(s.data());
                args.push_back(nullptr);
                execvp(args[0], args.data());
                perror("Error in executing command\n");
                exit(1);
            }
            else {
                int status;
                //the waiting process
                //the status to know how the process got terminated and can be null if we dont care about the process
                //options to show how to wait either freeze or not
                waitpid(pid, &status, 0);
            }
        }
    }

    return 0;
}