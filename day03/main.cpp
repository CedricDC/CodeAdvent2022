#include <bitset>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

namespace {
    enum class Part {
        FIRST = 0,
        SECOND
    };

    int getPriority(const char c) {
        // A (65) -> Z (90) : 27-52
        // a (97) -> z (122) : 1-26
        if (c < 'a') {
            return 27 + (c - 'A');
        } else {
            return 1 + (c - 'a');
        }
    }
}

int main(int argc, char** argv) {

    if (argc < 2) {
        std::cout << "Please provide input file" << std::endl;
        return 1;
    }

    Part part = Part::FIRST;
    if (argc > 2) {
        unsigned int part_value = std::atol(argv[2]);
        if (part_value == 0) {
            part = Part::FIRST;
        } else if (part_value == 1) {
            part = Part::SECOND;
        } else {
            std::cout << "Invalid part number: " << part_value << std::endl;
            return 1;
        }
    }

    std::string filename = argv[1];
    std::ifstream ifile(argv[1]);

    if (!ifile.good()) {
        std::cout << "Could not find " << filename << std::endl;
        return 1;
    }

    std::uint64_t sum_priorities = 0;
    switch (part) {
        case Part::FIRST:
        {
            // A-Z : 65-90
            // a-z : 97-122
            // Map A -> 0, then flip bits, e.g. flipping bit 0 means we have 'A'
            std::string line;
            while (std::getline(ifile, line)) {
                std::uint64_t compartment_1 = 0;

                const std::size_t items_per_bag = line.size() / 2;
                const char* c = &line[0];
                for (std::size_t i = 0; i < items_per_bag; ++i, ++c) {
                    compartment_1 |= (std::uint64_t(1) << (*c - 'A'));
                    // std::cout << "Adding " << char(*c) << "(" << (*c - 'A') << "), current compartment: " << std::bitset<64>(compartment_1).to_string() << std::endl;
                }

                // check if particular bits are set in compartment 1
                for (std::size_t i = 0; i < items_per_bag; ++i, ++c) {
                    // std::cout << "Checking: " << char(*c) << std::endl;
                    if ((compartment_1 & (std::uint64_t(1) << (*c - 'A'))) > 0) {
                        // current item was already in compartment 2
                        sum_priorities += getPriority(*c);
                        // std::cout << "Common letter: " << *c << ", score = " << getPriority(*c) << std::endl;
                        break;
                    }
                }
            }
        }
        break;
        case Part::SECOND:
        {
            // TODO
        }
        break;
    }
 
    std::cout << "total score: " << sum_priorities << std::endl;

    return 0;
}