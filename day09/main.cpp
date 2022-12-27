#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

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

    friend bool operator==(const Position& a, const Position& b);

    friend Position operator+(const Position& a, const Position& b);

    Position& operator+=(const Position& other);

    Position& operator-=(const Position& other);

    Position abs() const;

    void print() const;
  };

  using PositionDiff = Position;

  // This is a vector of the relative position of knots.
  // The front points to the head, the last element connects to the tail.
  std::vector<PositionDiff> knot_diffs;
  Position tail;

  Map(std::size_t knots);

  void moveHead(char direction);

  // print relative picture of field
  void printState() const;

 private:
  void followAction_(PositionDiff& diff, Position& next);
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
      Map map(2);
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
      Map map(10);
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
  }

  return 0;
}

namespace {

bool operator==(const Map::Position& a, const Map::Position& b) {
  return (a.row == b.row) && (a.col == b.col);
}

Map::Position operator+(const Map::Position& a, const Map::Position& b) {
  return {a.row + b.row, a.col + b.col};
}

Map::Position& Map::Position::operator+=(const Position& other) {
  row += other.row;
  col += other.col;
  return *this;
}

Map::Position& Map::Position::operator-=(const Position& other) {
  row -= other.row;
  col -= other.col;
  return *this;
}

Map::Position Map::Position::abs() const {
  return {std::abs(row), std::abs(col)};
}

void Map::Position::print() const {
  std::cout << row << ", " << col << std::endl;
}

Map::Map(std::size_t knots) : knot_diffs(knots - 1), tail{} {}

void Map::moveHead(char direction) {
  switch (direction) {
    case 'R':
      ++knot_diffs.front().col;
      break;
    case 'L':
      --knot_diffs.front().col;
      break;
    case 'U':
      --knot_diffs.front().row;
      break;
    case 'D':
      ++knot_diffs.front().row;
      break;
  }

  // See if and how the tail follows the head.
  // We can use relative and absolute vectors
  // almost interchangeably
  for (std::size_t i = 1; i < knot_diffs.size(); ++i) {
    followAction_(knot_diffs[i - 1], knot_diffs[i]);
  }

  // This takes care of the last, absolute motion
  followAction_(knot_diffs.back(), tail);
}

// print relative picture of field
void Map::printState() const {
  std::pair<int, int> row_range(tail.row, tail.row);
  std::pair<int, int> col_range(tail.col, tail.col);

  std::vector<Position> positions;
  positions.reserve(knot_diffs.size() + 1);
  positions.push_back(tail);
  for (auto rit = knot_diffs.crbegin(); rit != knot_diffs.crend(); ++rit) {
    positions.push_back(positions.back() + *rit);

    if (positions.back().row < row_range.first) {
      row_range.first = positions.back().row;
    } else if (positions.back().row > row_range.second) {
      row_range.second = positions.back().row;
    }
    if (positions.back().col < col_range.first) {
      col_range.first = positions.back().col;
    } else if (positions.back().col > col_range.second) {
      col_range.second = positions.back().col;
    }
  }

  std::size_t num_rows = 1 + row_range.second - row_range.first;
  std::size_t num_cols = 1 + col_range.second - col_range.first;
  for (auto& pos : positions) {
    pos.row -= row_range.first;
    pos.col -= col_range.first;
  }

  std::vector<char> field(num_rows * num_cols, '.');
  const auto& local_head = positions.back();
  const auto& local_tail = positions.front();
  field[local_tail.col + num_cols * local_tail.row] = 'T';
  for (int i = 1; i < positions.size(); ++i) {
    field[positions[i].col + num_cols * positions[i].row] =
        '0' + (positions.size() - 1 - i);
  }
  field[local_head.col + num_cols * local_head.row] = 'H';

  std::size_t counter = 0;
  for (const auto& c : field) {
    std::cout << char(c);
    if (++counter % num_cols == 0) {
      std::cout << std::endl;
    }
  }

  std::cout << std::endl
            << "-------------------------" << std::endl
            << std::endl;
}

void Map::followAction_(PositionDiff& diff, Position& next) {
  // In part 2, we additionally have the possible case where
  // "leading knots" (i.e. the "head") can move diagonally as well
  auto elem_abs = diff.abs();

  if (elem_abs.row > 1) {
    diff.row /= 2;  // we know it's max 2
    if (diff.col == 0) {
      next.row += diff.row;
    } else if (elem_abs.col < 2) {
      next += diff;
      diff.col = 0;
    } else {
      diff.col /= 2;
      next += diff;
    }
  } else if (elem_abs.col > 1) {
    diff.col /= 2;  // we know it's max 2
    if (diff.row == 0) {
      next.col += diff.col;
    } else if (elem_abs.row < 2) {
      next += diff;
      diff.row = 0;
    } else {
      diff.row /= 2;
      next += diff;
    }
  }
}
}  // namespace
