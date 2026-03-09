#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "to use enter the source code followed by the input file name"<< endl;
        return 1;
    }
    string filename = argv[1];
    ifstream infile(filename);
    if (!infile) {
        cerr << "Error: Could not open input.txt\n";
        return 1;
    }

    int n;
    if (!(infile >> n)) {
        cerr << "Error: Could not read the number of test cases.\n";
        return 1;
    }

    for (int t = 0; t < n; ++t) {
        cout << "\n--- Test Case " << (t + 1) << " ---\n";

        int num_processes, num_resources;
        if (!(infile >> num_processes >> num_resources)) {
            cerr << "Error: File is empty or incorrectly formatted.\n";
            break;
        }

        // Initialize vectors with dynamic sizes
        vector<int> E(num_resources); //existing recourses in the system
        vector<vector<int>> C(num_processes, vector<int>(num_resources)); //the currently held recourses by each process
        vector<vector<int>> R(num_processes, vector<int>(num_resources)); //the requested recourses needed by each process

        // Read Existence Vector (E)
        for (int i = 0; i < num_resources; ++i) {
            infile >> E[i];
        }

        // Read Current Allocation Matrix (C)
        for (int i = 0; i < num_processes; ++i) {
            for (int j = 0; j < num_resources; ++j) {
                infile >> C[i][j];
            }
        }

        // Read Request Matrix (R)
        for (int i = 0; i < num_processes; ++i) {
            for (int j = 0; j < num_resources; ++j) {
                infile >> R[i][j];
            }
        }

        vector<int> A = E; //the available vector
        for (int i = 0; i < num_processes; ++i) {
            for (int j = 0; j < num_resources; ++j) {
                //subtract from the total available
                A[j] -= C[i][j];
            }
        }

        //finish vector shows if the process is finished
        vector<bool> Finish(num_processes, false);
        for (int i = 0; i < num_processes; ++i) {
            bool has_allocation = false;
            for (int j = 0; j < num_resources; ++j) {
                if (C[i][j] > 0) {
                    has_allocation = true;
                    break;
                }
            }
            if (!has_allocation) {
                //set to ture if doesnt hold resources
                //the process will just be starved if there are no resources available
                Finish[i] = true;
            }
        }

        //a flag to show if there was a completed process to repeat the check
        bool process_finished;
        do {
            process_finished = false;
            for (int i = 0; i < num_processes; ++i) {
                if (!Finish[i]) {
                    bool can_allocate = true;

                    // Check if Request <= Available
                    cout << "Attempting to allocate for process: " << i <<'\n';
                    for (int j = 0; j < num_resources; ++j) {
                        if (R[i][j] > A[j]) {
                            can_allocate = false; //made false if on resource cant be allocated
                            cout << "Failed to allocate for resource: " << j << '\n';
                            cout << "Required: "  << R[i][j] << " available: " << A[j] <<'\n';
                            break;
                        }
                    }

                    //as if the process is completed and the held recourses are released
                    if (can_allocate) {
                        cout << "-------------------------------------------------------\n";
                        cout << "Successfully allocated and completed for process: " << i << '\n';
                        cout << "-------------------------------------------------------\n";
                        for (int j = 0; j < num_resources; ++j) {
                            //add the released to the available
                            A[j] += C[i][j];
                            cout << "New available for resource " << j << " is " << A[j] << '\n';
                        }
                        Finish[i] = true;
                        process_finished = true;
                    }
                }
            }
        } while (process_finished); //loop will keep repeating if a new process is finished

        bool is_deadlock = false;
        vector<int> deadlocked_processes;

        for (int i = 0; i < num_processes; ++i) {
            if (!Finish[i]) {
                is_deadlock = true;
                //adding deadlocked processes
                deadlocked_processes.push_back(i);
            }
        }

        if (is_deadlock) {
            cout << "Deadlock detected in the system.\n";
            cout << "Deadlocked processes: ";
            for (int p : deadlocked_processes) {
                cout << "P" << p << " ";
            }
            cout << "\n";
        } else {
            cout << "No deadlock detected in the system.\n";
        }
    }

    infile.close();
    return 0;
}