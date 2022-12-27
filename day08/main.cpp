#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <unordered_set>
#include <vector>

namespace {
enum class Part { FIRST = 0, SECOND };

class Grid {
 public:
  static constexpr uint8_t MIN_TREE_HEIGHT = 0;
  static constexpr uint8_t MAX_TREE_HEIGHT = 9;

  // push one row of input file to expand grid
  void pushRow(const std::string& row);

  // print current grid (in values)
  void print() const;

  // print visibility map
  void printVisibility(
      const std::vector<std::unordered_set<std::size_t>>& candidates,
      int row_idx = -1) const;

  // counter number of trees which are visible from at least one direction
  std::size_t countVisible() const;

 private:
  std::size_t rows_ = 0;
  std::size_t cols_ = 0;
  std::vector<uint8_t> grid_;
};

}  // namespace

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

  // Read input
  std::string line;
  Grid grid;
  while (std::getline(ifile, line)) {
    grid.pushRow(line);
  }

  switch (part) {
    case Part::FIRST: {
      std::size_t visible_counter = grid.countVisible();
      std::cout << "Number of visible trees: " << visible_counter << std::endl;
    } break;
    case Part::SECOND: {
      // TODO
    } break;
  }

  return 0;
}

namespace {
void Grid::pushRow(const std::string& row) {
  // although not explicitely mentioned, it looks like the grid is square
  // pre-allocate memory
  if (grid_.empty()) {
    cols_ = row.size();
    grid_.reserve(cols_ * cols_);
  }

  for (char c : row) {
    grid_.push_back(c - '0');
  }
  ++rows_;
}

void Grid::print() const {
  const uint8_t* data = grid_.data();
  for (std::size_t row_idx = 0; row_idx < rows_; ++row_idx) {
    for (std::size_t col_idx = 0; col_idx < cols_; ++col_idx) {
      std::cout << static_cast<int>(*(data++));
    }
    std::cout << std::endl;
  }
}

void Grid::printVisibility(
    const std::vector<std::unordered_set<std::size_t>>& candidates,
    int row_idx /*= -1*/) const {
  if (row_idx < 0) {
    std::cout << "Forest: " << std::endl;
    for (std::size_t row_idx = 0; row_idx < rows_; ++row_idx) {
      for (std::size_t col_idx = 0; col_idx < cols_; ++col_idx) {
        if (candidates[col_idx].count(row_idx) > 0) {
          std::cout << "*";
        } else {
          std::cout << ".";
        }
      }
      std::cout << std::endl;
    }
  } else {
    std::cout << "Forest row: " << row_idx << std::endl;
    for (std::size_t col_idx = 0; col_idx < cols_; ++col_idx) {
      if (candidates[col_idx].count(row_idx) > 0) {
        std::cout << "*";
      } else {
        std::cout << ".";
      }
    }
    std::cout << std::endl;
  }
}

std::size_t Grid::countVisible() const {
  // unclear whether it's more efficient to find visible or hidden trees
  // for small forests, but we know that there may be at max 20 trees visible
  // per row / column, whereas the number of hidden trees can scale with the
  // size of the forest

  // for each column, have set of (potentially) visible trees
  std::vector<std::unordered_set<std::size_t>> visible_candidates(cols_);

  // top and bottom row are visible
  for (auto& col : visible_candidates) {
    col.insert(0);
    col.insert(rows_ - 1);
  }

  // first and last column are visible
  {
    auto& first_col = visible_candidates.front();
    for (std::size_t i = 0; i < rows_; ++i) {
      first_col.insert(i);
    }
    visible_candidates.back() = visible_candidates.front();
  }

  for (std::size_t row_idx = 1; row_idx < rows_ - 1; ++row_idx) {
    // Could do this with a stack, but this would potentially
    // result in many nested loops. I think performing left/right
    // separately is actually faster.

    // view from left
    const uint8_t* data = grid_.data() + (cols_ * row_idx);
    uint8_t max_left = *data++;
    for (std::size_t col_idx = 1; col_idx < cols_; ++col_idx) {
      uint8_t tree_height = *data++;

      // if (row_idx == 1) {
      //   std::cout << (int)tree_height << ", " << (int)max_left <<
      //   std::endl; std::cin.get();
      // }
      if (tree_height > max_left) {
        visible_candidates[col_idx].insert(row_idx);
        max_left = tree_height;
        if (max_left == MAX_TREE_HEIGHT) break;
      }
    }

    // view from right
    data = grid_.data() + (cols_ * row_idx) + (cols_ - 1);
    uint8_t max_right = *data--;  // going backwards!!
    for (int col_idx = cols_ - 2; col_idx >= 0; --col_idx, --data) {
      uint8_t tree_height = *data;
      if (tree_height > max_right) {
        visible_candidates[col_idx].insert(row_idx);
        max_right = tree_height;
        if (max_right == max_left) break;
      }
    }
  }

  for (std::size_t col_idx = 1; col_idx < cols_ - 1; ++col_idx) {
    auto& col_candidates = visible_candidates[col_idx];

    // view from top
    const uint8_t* data = grid_.data() + col_idx;
    uint8_t max_top = *data;
    data += rows_;
    for (std::size_t row_idx = 1; row_idx < rows_ - 1;
         ++row_idx, data += rows_) {
      uint8_t tree_height = *data;
      if (tree_height > max_top) {
        col_candidates.insert(row_idx);
        max_top = tree_height;
        if (max_top == MAX_TREE_HEIGHT) break;
      }
    }

    // view from bottom
    data = grid_.data() + (rows_ - 1) * cols_ + col_idx;
    uint8_t max_bottom = *data;
    data -= rows_;
    for (int row_idx = rows_ - 2; row_idx >= 0; --row_idx, data -= rows_) {
      uint8_t tree_height = *data;
      if (tree_height > max_bottom) {
        col_candidates.insert(row_idx);
        max_bottom = tree_height;
        if (max_bottom == max_top) break;
      }
    }
  }

  printVisibility(visible_candidates);

  std::size_t visible_count = std::accumulate(
      visible_candidates.cbegin(), visible_candidates.cend(), std::size_t{0},
      [](const std::size_t current_count,
         const std::unordered_set<std::size_t>& candidates) {
        return current_count + candidates.size();
      });

  return visible_count;
}

}  // namespace