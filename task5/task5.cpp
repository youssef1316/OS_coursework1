#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <fstream>

using namespace std;

struct process {
    int id;
    int arrival_time;
    int burst_time;
    int turn_around;
    int wait;
    int completion_time;
    int remaining_time;

};

void fcfs (vector<process> p, int n, ofstream& out) {
    //sort the process according to the arrival time
    sort(p.begin(), p.end(), [](const process& a, const process& b) {
        return a.arrival_time < b.arrival_time;
    });
    int current_time = 0, total_wait = 0, total_turn_around = 0;
    //main loop
    for (int i = 0; i < n; i ++) {
        //check if the cpu is idle initially
        if (current_time < p[i].arrival_time) current_time = p[i].arrival_time;
        p[i].completion_time = current_time + p[i].burst_time;
        p[i].turn_around = p[i].completion_time - p[i].arrival_time;
        p[i].wait = p[i].turn_around - p[i].burst_time;
        total_wait += p[i].wait;
        total_turn_around += p[i].turn_around;

        current_time = p[i].completion_time;
    }
    out << (float)total_wait / n << " " << (float)total_turn_around / n << " ";
}

void sjf(vector<process>p, int n, ofstream& out) {
    //sorting according to the arrival time
    sort(p.begin(), p.end(), [](const process& a, const process& b) {
        return a.arrival_time < b.arrival_time;
    });
    int current_time = 0, total_wait = 0, total_turn_around = 0, completed_count = 0;
    //main loop executing until all process are completed
    while (completed_count < n) {
        int shortest = -1;
        int min_burst = 1e9;

        //finding the shortest job
        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= current_time && p[i].remaining_time > 0) {
                if (p[i].burst_time < min_burst) {
                    min_burst = p[i].burst_time;
                    shortest = i;
                }
                else if (p[i].burst_time == min_burst) {
                    if (p[i].arrival_time < p[shortest].arrival_time) {
                        shortest = i;
                    }
                }
            }
        }
        if (shortest != -1) {
            //complete the selected process
            p[shortest].completion_time = current_time + p[shortest].burst_time;
            p[shortest].turn_around = p[shortest].completion_time - p[shortest].arrival_time;
            p[shortest].wait = p[shortest].turn_around - p[shortest].burst_time;
            p[shortest].remaining_time = 0;
            total_wait += p[shortest].wait;
            total_turn_around += p[shortest].turn_around;
            current_time = p[shortest].completion_time;
            completed_count++;
        }
        else {
            //idle cpu
            int next_arrival = 1e9;
            for (int i = 0; i < n; i++) {
                //jump to the next available arrival time
                if (p[i].remaining_time > 0 && p[i].arrival_time < next_arrival) {
                    next_arrival = p[i].arrival_time;
                }
            }
            current_time = next_arrival;
        }
    }
    out << (float)total_wait / n << " " << (float)total_turn_around / n << " ";
}

void round_robin (vector<process>p, int n, int q, ofstream& out) {
    sort(p.begin(), p.end(), [](const process& a, const process& b) {
        return a.arrival_time < b.arrival_time;
    });

    int current_time = 0, total_wait = 0, total_turn_around = 0, completed_count = 0;
    queue<int> ready_queue;
    int idx = 0;

    while (completed_count < n) {
        if (ready_queue.empty()) {
            //cpu is idle jump to the next arrival time
            if (idx < n) {
                current_time = p[idx].arrival_time;
                ready_queue.push(idx);
                idx++;
            }
            continue;
        }

        int i = ready_queue.front();
        ready_queue.pop();

        //keeping track of the current time
        int execute_time = min(q, p[i].remaining_time); //assign the min to avoid idle time
        current_time += execute_time;
        p[i].remaining_time -= execute_time;

        //check if new processes arrived to be pushed in the queue
        while (idx < n && p[idx].arrival_time <= current_time) {
            ready_queue.push(idx);
            idx++;
        }

        if (p[i].remaining_time > 0) {
            //if process isnt completed push to the back of the array
            ready_queue.push(i);
        } else {
            // Process completed
            p[i].completion_time = current_time;
            p[i].turn_around = p[i].completion_time - p[i].arrival_time;
            p[i].wait = p[i].turn_around - p[i].burst_time;

            total_wait += p[i].wait;
            total_turn_around += p[i].turn_around;
            completed_count++;
        }
    }

    out << (float)total_wait / n << " " << (float)total_turn_around / n << " ";
}

int main () {
    ifstream in("input.txt");
    ofstream out("output.txt");

    if (!in.is_open() || !out.is_open()) {
        cerr << "Error opening files!" << '\n';
        return 1;
    }

    int t;
    if (!(in >> t)) return 0;
    while (t--) {
        int n;
        in >> n;
        vector <process> p(n);
        for (int i = 0; i < n; i++) {
            p[i].id = i;
            in >> p[i].arrival_time;
            in >> p[i].burst_time;
            p[i].remaining_time = p[i].burst_time;
        }
        fcfs(p,n, out);
        sjf(p,n, out);
        int q;
        in >> q;
        round_robin(p, n, q, out);
        out << '\n';
    }
    in.close();
    out.close();
    cout << "Program completed\n";
    return 0;
}