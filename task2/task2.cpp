#include <iostream>
#include <pthread.h>
#include <map>
#include <mutex>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>

using namespace std;

map<string, int> global_counts; // global map to store the final results
//the mutex
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

struct thread_data {
    //vector containing lines of the file
    const vector<string> *data;
    //the start index
    int start;
    //the end segement
    int end;
};

//function to clean the words from integers and make them lower case
string clean(string& word) {
    string result = "";
    for (char &c: word) {
        if (isalpha(c)) {
            result += tolower(c);
        }
    }
    return result;
}

//the main function that counts the words
void *count(void *arg) {
    thread_data *d = (thread_data *) arg;
    //a local map per thread
    map<string, int> local_count;

    // Process only the assigned slice of the vector
    for (int i = d->start; i < d->end; i++) {
        stringstream ss((*d->data)[i]);
        string word;
        while (ss >> word) {
            //call the cleaning function
            word = clean(word);
            local_count[word]++;
        }
    }
    //critical segment
    pthread_mutex_lock(&mtx);
    for (auto const &[word, count]: local_count) {
        //modify the global result by what each thread found
        global_counts[word] += count;
    }
    pthread_mutex_unlock(&mtx);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        fstream file(argv[1]);
        if (!file.is_open()) {
            //checking if the file could not be open
            cout << "Error: Could not open file " << argv[1] << endl;
            return 1;
        }

        string dummy;
        vector<string> data;
        while (getline(file, dummy)) {
            data.push_back(dummy);
        }
        file.close();

        if (data.size() == 0) {
            cout << "File is empty." << endl;
            return 0;
        }
        //accept thread count from user or default to 4 threads
        int num_threads = (argc > 2) ? atoi(argv[2]) : 4;
        pthread_t th[num_threads];
        thread_data t_args[num_threads];
        int chunk = data.size() / num_threads;
        auto start = chrono::high_resolution_clock::now();
        for (int i = 0; i < num_threads; i++) {
            //store the address of the vector in the struct
            t_args[i].data = &data;
            //the start index
            t_args[i].start = i * chunk;
            //if there are enough remaining lines then take them or else take till the end of the vector
            //ensures that if there are more lines the last thread will take the rest
            t_args[i].end = (i == num_threads - 1) ? data.size() : (i + 1) * chunk;
            pthread_create(&th[i], NULL, count, &t_args[i]);
        }

        for (int i = 0; i < num_threads; i++) {
            pthread_join(th[i], NULL);
        }

        // Print final results
        for (auto const& [word, count] : global_counts) {
            cout << word << ": " << count << "\n";
        }
        auto stop = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
        cout << "Execution time: " << duration.count() << " milliseconds\n";
    }
    else {
        cout << "Error: provide a file name!" <<'\n';
        return 1;
    }
    return 0;
}
