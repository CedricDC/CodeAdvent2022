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
}

unsigned int getScore(char c1, char c2) {
    static constexpr unsigned int WIN_SCORE = 6;
    static constexpr unsigned int DRAW_SCORE = 3;

    // map X,Y,Z also to A,B,C
    c2 = c2 - ('X' - 'A');

    // score A -> 1, B -> 2, C -> 3
    unsigned int selection_score = 1 + (c2 - 'A');

    // early exit in case of draw
    if (c1 == c2) {
        return selection_score + DRAW_SCORE;
    }

    // determine win by taking (wrapped) difference
    int res = (3 + c2 - c1) % 3;
    if (res == 1) {
        // we were one above their result : win!
        return selection_score + WIN_SCORE;
    }

    return selection_score;
}

int main(int argc, char** argv) {

    if (argc < 2) {
        std::cout << "Please provide input file" << std::endl;
        return 1;
    }

    if (argc > 2) {
        unsigned int part = std::atol(argv[2]);
        if (part == 0) {
        } else if (part == 1) {
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

    std::size_t total_score = 0;
    char c1;
    char c2;
    while (ifile >> c1, ifile >> c2, ifile.good()) {
        // std::cout << c1 << ", " << c2 << ", score: " << getScore(c1, c2) << std::endl;
        total_score += getScore(c1, c2);
    }

    std::cout << "total score: " << total_score << std::endl;

    return 0;
}