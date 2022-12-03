#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace {
    enum class Part {
        FIRST = 0,
        SECOND
    };

    void addNewTotal(std::vector<std::size_t>& max_calories, std::size_t new_total) {
        // if current total higher than lowest kept total, replace
        if (max_calories[0] < new_total) {

            max_calories[0]  = new_total;

            // reorder so that highest is last
            std::size_t max_idx = max_calories.size()-1;
            for (std::size_t idx = 0; idx < max_idx; ++idx) {
                if (max_calories[idx + 1] < max_calories[idx]) {
                    std::swap(max_calories[idx+1], max_calories[idx]);
                } else {
                    return;
                }
            }
        }
    }
}


int main(int argc, char** argv) {

    std::size_t num_max = 1; // number of max elements to keep
    if (argc < 2) {
        std::cout << "Please provide input file" << std::endl;
        return 1;
    }

    if (argc > 2) {
        unsigned int part = std::atol(argv[2]);
        if (part == 0) {
            num_max = 1;
        } else if (part == 1) {
            num_max = 3;
        } else {
            std::cout << "Invalid part number: " << part << std::endl;
            return 1;
        }
    }

    std::string filename = argv[1];
    std::ifstream ifile(argv[1]);

    if (!ifile.good()) {
        std::cout << "Could not find " << filename << std::endl;
        return 1;
    }

    std::vector<std::size_t> max_calories(num_max, 0);
    std::size_t current_calories = 0;
    std::string line;
    while (std::getline(ifile, line)) {
        if (line.empty()) { // new elf
            addNewTotal(max_calories, current_calories);
            current_calories = 0;
        } else {
            current_calories += std::stoi(line);
        }
    }

    // if last line is not empty
    if (current_calories > 0) {
        addNewTotal(max_calories, current_calories);
    }

    std::cout << "Sum of top calories: " << std::accumulate(max_calories.cbegin(), max_calories.cend(), std::size_t{0}) << std::endl;

    return 0;
}