#include <fstream>
#include <iostream>
#include <string>

namespace {
    enum Part {
        FIRST = 0,
        SECOND
    };
}

int main(int argc, char** argv) {

    if (argc < 2) {
        std::cout << "Please provide input file" << std::endl;
        return 1;
    }

    Part part = Part::FIRST;
    if (argc > 2) {
        int value = std::stoul(argv[2]);
        if (value > static_cast<unsigned int>(Part::SECOND)) {
            std::cout << "Invalid problem version: " << value << std::endl;
            return 1;
        }
    }

    std::string filename = argv[1];
    std::ifstream ifile(argv[1]);

    if (!ifile.good()) {
        std::cout << "Could not find " << filename << std::endl;
        return 1;
    }

    if (part == Part::FIRST) {
        std::size_t max_calories = 0;
        std::size_t current_calories = 0;
        std::string line;
        while (std::getline(ifile, line)) {
            if (line.empty()) { // new elf
                if (max_calories < current_calories) {
                    max_calories = current_calories;
                }
                current_calories = 0;
            } else {
                current_calories += std::stoi(line);
            }
        }

        // if last line is not empty
        if (current_calories > 0) {
            if (max_calories < current_calories) {
                max_calories = current_calories;
            }
        }

        std::cout << "max calories: " << max_calories << std::endl;
    }

    std::cout << "Done" << std::endl;

    return 0;
}