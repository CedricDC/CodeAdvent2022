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

unsigned int getScorePart1(char c1, char c2) {
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

unsigned int getScorePart2(char c1, char c2) {
    static constexpr unsigned int SCORE = 3;

    // X : lose, Y : draw, Z : win
    //
    // We just need to shift by 2 + desired outcome to get the desired hand,
    // e.g. 'A', 'X' -> 'A' + 2 + 0; 'B', 'X' -> ('B' + 2 + 0) % 3
    //      'A', 'Y' -> 'A' + 2 + 1; 'B', 'Y' -> ('B' + 2 + 1) % 3

    // Since 'A' % 3 == 65 % 3 == 2 --> shift all by 2 (we want 0)
    // Since 'X' % 3 == 88 % 3 == 1 --> shift all by 1 more
    // The two shifts sum up to 3, which is 0 when mod 3
    //
    // The win score is also easier now, since win_score = (c2 - 'X') * 3
    //
    // Together, we have: hand_score + win_score
    // std::cout << "hand score: " << char('A' + (c1 + c2 + 2) % 3) << ": " << 1 + (c1 + c2 + 2) % 3 << ", win score: " << (c2 - 'X') * SCORE << std::endl;
    return (1 + (c1 + c2 + 2) % 3) + (c2 - 'X') * SCORE; 
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

    std::size_t total_score = 0;
    switch (part) {
        case Part::FIRST:
        {
            char c1;
            char c2;
            while (ifile >> c1, ifile >> c2, ifile.good()) {
                total_score += getScorePart1(c1, c2);
            }
        }
        break;
        case Part::SECOND:
        {
            char c1;
            char c2;
            while (ifile >> c1, ifile >> c2, ifile.good()) {
                total_score += getScorePart2(c1, c2);
            }
        }
        break;
    }
 
    std::cout << "total score: " << total_score << std::endl;

    return 0;
}