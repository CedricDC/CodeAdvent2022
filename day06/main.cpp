#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

namespace {
enum class Part { FIRST = 0, SECOND };

// For high values of SIZE, having a set with a insert lookup probably more efficient
template <std::size_t SIZE>
struct UniqueBuffer {
  void push(char c) {
    if (buffer_[idx_] != c) {
      if (contains(c)) {
        ++duplicate_counter_;
      }

      char old_char = std::exchange(buffer_[idx_], c);

      if (contains(old_char)) {
        --duplicate_counter_;
      }
    }

    idx_ = (idx_ + 1) % BUFFER_SIZE_;
  }

  bool isUnique() const { return (duplicate_counter_ == 0); }

  bool contains(char c) {
    for (const char other : buffer_) {
      if (c == other) return true;
    }

    return false;
  }

 private:
  static constexpr std::size_t BUFFER_SIZE_ = SIZE;
  std::size_t idx_ = 0;
  std::array<char, SIZE> buffer_ = {};  // zero-initialized

  // for each duplicate character, increase by one
  // Note : N identical characters == N-1 duplicates
  std::size_t duplicate_counter_ = BUFFER_SIZE_ - 1;
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

  switch (part) {
    case Part::FIRST: {
      static constexpr std::size_t MSG_LEN = 4;
      std::size_t pos = 0;
      UniqueBuffer<MSG_LEN> buffer;

      bool start_found = false;
      char c;
      while (!start_found && ifile.get(c)) {
        ++pos;
        buffer.push(c);
        if (buffer.isUnique() && pos >= MSG_LEN) {
          start_found = true;
        }
      }

      if (start_found) {
        std::cout << "Start sequence ended at character " << pos << std::endl;
      } else {
        std::cout << "No start sequence found" << std::endl;
      }
    } break;
    case Part::SECOND: {
      static constexpr std::size_t MSG_LEN = 14;
      std::size_t pos = 0;
      UniqueBuffer<MSG_LEN> buffer;

      bool start_found = false;
      char c;
      while (!start_found && ifile.get(c)) {
        ++pos;
        buffer.push(c);
        if (buffer.isUnique() && pos >= MSG_LEN) {
          start_found = true;
        }
      }

      if (start_found) {
        std::cout << "Start packet sequence ended at character " << pos << std::endl;
      } else {
        std::cout << "No packet start found" << std::endl;
      }
    } break;
  }

  return 0;
}
