#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <utility>

namespace {
enum class Part { FIRST = 0, SECOND };

struct Operation {
  char direction;
  int repetitions;
};

bool parseLine(std::ifstream& ifile, Operation& operation) {
  ifile >> operation.direction;
  ifile >> operation.repetitions;

  // ifile.good() would already trigger at EOF which would
  // miss the last line as it does not contain a '\n' character
  return !ifile.fail();
}

struct Map {
  struct Position {
    int row = 0;
    int col = 0;

    friend bool operator==(const Position& a, const Position& b) {
      return (a.row == b.row) && (a.col == b.col);
    }

    Position& operator+=(const Position& other) {
      row += other.row;
      col += other.col;
      return *this;
    }

    Position& operator-=(const Position& other) {
      row -= other.row;
      col -= other.col;
      return *this;
    }

    void print() const { std::cout << row << ", " << col << std::endl; }
  };

  using PositionDiff = Position;

  PositionDiff tail_to_head;
  Position tail;

  void moveHead(char direction) {
    switch (direction) {
      case 'R':
        ++tail_to_head.col;
        break;
      case 'L':
        --tail_to_head.col;
        break;
      case 'U':
        --tail_to_head.row;
        break;
      case 'D':
        ++tail_to_head.row;
        break;
    }

    // See if and how the tail follows the head.
    // This could be merged with the switch statement for optimization
    if (std::abs(tail_to_head.row) > 1) {
      tail_to_head.row /= 2;  // we know it's max 2
      if (tail_to_head.col == 0) {
        tail.row += tail_to_head.row;
      } else {  // moving diagonally
        tail += tail_to_head;
        tail_to_head.col = 0;
      }
    } else if (std::abs(tail_to_head.col) > 1) {
      tail_to_head.col /= 2;  // we know it's max 2
      if (tail_to_head.row == 0) {
        tail.col += tail_to_head.col;
      } else {  // moving diagonally
        tail += tail_to_head;
        tail_to_head.row = 0;
      }
    }
  }
};

}  // namespace

// Specialization of std::hash so we can use 'Position' in std::unordered_set
namespace std {
template <>
struct hash<typename Map::Position> {
  std::size_t operator()(const Map::Position& pos) const {
    return hash<int>{}(pos.row) ^ hash<int>{}(pos.col);
  }
};
}  // namespace std

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

  switch (part) {
    case Part::FIRST: {
      Map map;
      Operation operation;
      std::unordered_set<typename Map::Position> visited;
      while (parseLine(ifile, operation)) {
        for (int i = 0; i < operation.repetitions; ++i) {
          map.moveHead(operation.direction);
          visited.insert(map.tail);
        }
      }

      std::cout << "Number of visited locations: " << visited.size()
                << std::endl;
    } break;
    case Part::SECOND: {
      // TODO
    } break;
  }

  return 0;
}
