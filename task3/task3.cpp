#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <bitset>

using namespace std;

struct Frame {
    int page; //page ID
    unsigned int counter; //the 8 bit int showing the usage
    bool referenceBit; //is the page referenced in the recent read
    bool valid; //true if the slot isnt empty and false if empty

    Frame() : page(-1), counter(0), referenceBit(false), valid(false) {}
    //valid is false by default because the page should be empty
};

// check if the page is in the frame
bool findPage(vector<Frame>& frames, int page, int& hits) {
    //loop over all the frames
    for (auto& frame : frames) {
        //if page not empty and the id is equal (found the page)
        if (frame.valid && frame.page == page) {
            frame.referenceBit = true;
            hits++;
            return true;
        }
    }
    return false;
}

//if the page isnt in the frame
void handlePageFault(vector<Frame>& frames, int page, int& faults) {
    faults++;
    int targetIndex = -1;

    // 1. Search for an empty frame
    for (int i = 0; i < (int)frames.size(); i++) {
        if (!frames[i].valid) {
            targetIndex = i;
            break;
        }
    }

    // 2. If no empty frame, find the one with the minimum counter (the oldest accessed frame)
    if (targetIndex == -1) {
        targetIndex = 0;
        for (int i = 1; i < frames.size(); i++) {
            if (frames[i].counter < frames[targetIndex].counter) {
                targetIndex = i;
            }
        }
    }

    // Update the selected frame
    frames[targetIndex].page = page;
    frames[targetIndex].valid = true;
    frames[targetIndex].referenceBit = true;
    frames[targetIndex].counter = 0;
}

//increase aging logic
void performAging(vector<Frame>& frames) {
    for (auto& frame : frames) {
        if (frame.valid) {
            frame.counter >>= 1; //right shift the counter
            if (frame.referenceBit) {
                //set the leftmost bit to 1
                //or is used to keep the rest of the count as is
                frame.counter |= (1 << 7);
            }
            //set to false
            frame.referenceBit = false;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "to use enter the source code followed by the number of frames and the the input file"<< endl;
        return 1;
    }

    int numFrames = stoi(argv[1]);
    string filename = argv[2];

    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return 1;
    }

    //initiate the frames according to input
    vector<Frame> frames(numFrames);
    int page;
    int hits = 0;
    int faults = 0;

    //loop over the file
    while (file >> page) {
        cout << "**current memory**\n";
        for (int i = 0; i < frames.size(); i++) {

            if (frames[i].valid)
                cout << frames[i].page <<" Age: " << bitset<8>(frames[i].counter) << endl;
        }
        cout <<"------------------\n";
        cout << "New frame id: " << page << endl;
        cout <<"------------------\n";
        if (!findPage(frames, page, hits)) {
            handlePageFault(frames, page, faults);
        }
        performAging(frames);
    }

    cout << "Successful hits: " << hits << endl;
    cout << "Faults: " << faults << endl;

    return 0;
}