#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <iomanip>

using namespace std;
//needed for interacting with the os files
using namespace std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <directory_path> <bin_width>\n";
        return 1;
    }

    string dir_path = argv[1];
    //ensure minimum of 0
    size_t bin_width;

    try {
        //using unsigned long long to prevent overflow
        bin_width = stoull(argv[2]);
        if (bin_width == 0) {
            cerr << "Error: Bin width must be greater than 0.\n";
            return 1;
        }
    } catch (const exception& e) {
        cerr << "Error: Invalid bin width provided.\n";
        return 1;
    }

    //checking if the path exists
    if (!exists(dir_path) || !is_directory(dir_path)) {
        cerr << "Error: The specified path is not a valid directory.\n";
        return 1;
    }

    map<size_t, size_t> histogram;

    try {
        //skipping the files that are hidden or require higher privileges
        auto options = directory_options::skip_permission_denied;

        //searches the directory using DFS (pre-order traversal)
        for (const auto& entry : recursive_directory_iterator(dir_path, options)) {
            //check if it is not a directory
            if (is_regular_file(entry.status())) {
                error_code ec;
                uintmax_t f_size = file_size(entry, ec);

                //if no error code
                if (!ec) {
                    size_t bin_index = f_size / bin_width;
                    histogram[bin_index]++;
                }
            }
        }
    } catch (const filesystem_error& e) {
        cerr << "Filesystem error encountered: " << e.what() << "\n";
        return 1;
    }

    string output_filename = "histogram_data.csv";
    ofstream outfile(output_filename);

    if (outfile.is_open()) {
        outfile << "BinStart,FileCount\n";
    } else {
        cerr << "Warning: Could not open " << output_filename << " for writing.\n";
    }

    //output
    cout << "\nFile Size Histogram (Bin Width: " << bin_width << " bytes)\n";
    cout << string(45, '-') << "\n";
    cout << left << setw(30) << "Size Range (Bytes)" << "File Count\n";
    cout << string(45, '-') << "\n";

    if (histogram.empty()) {
        cout << "No regular files found in the specified directory.\n";
    } else {
        for (const auto& [bin_index, count] : histogram) {
            size_t range_start = bin_index * bin_width;
            size_t range_end = range_start + bin_width - 1;

            string range_str = to_string(range_start) + " - " + to_string(range_end);
            cout << left << setw(30) << range_str << count << "\n";

            if (outfile.is_open()) {
                outfile << range_start << "," << count << "\n";
            }
        }
    }

    if (outfile.is_open()) {
        outfile.close();
        cout << "\nData successfully written to " << output_filename << "\n";
    }

    return 0;
}