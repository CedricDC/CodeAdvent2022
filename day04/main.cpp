#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

namespace {
enum class Part { FIRST = 0, SECOND };

struct Assignment {
  bool parse(std::ifstream& ifile) {
    // format is <value>-<value>,<value>-<value>
    // assuming values fit in uint16_t
    if (ifile >> elf_0.first) {
      ifile.ignore();
      ifile >> elf_0.second;
      ifile.ignore();
      ifile >> elf_1.first;
      ifile.ignore();
      ifile >> elf_1.second;
      ifile.ignore();

      return true;
    } else {
      return false;
    }
  }

  void print() {
    std::cout << elf_0.first << " --> " << elf_0.second << " | " << elf_1.first << " --> "
              << elf_1.second << std::endl;
  }

  std::pair<uint16_t, uint16_t> elf_0;
  std::pair<uint16_t, uint16_t> elf_1;
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

  Assignment assignment;
  switch (part) {
    case Part::FIRST: {
      std::size_t contained_counter = 0;

      while (assignment.parse(ifile)) {
        if ((assignment.elf_0.first <= assignment.elf_1.first) &&
            (assignment.elf_0.second >= assignment.elf_1.second)) {
          ++contained_counter;
        } else if ((assignment.elf_1.first <= assignment.elf_0.first) &&
                   (assignment.elf_1.second >= assignment.elf_0.second)) {
          ++contained_counter;
        }
      }

      std::cout << "Number of assigned pairs containing each other: " << contained_counter
                << std::endl;
    } break;
    case Part::SECOND: {
      std::size_t overlap_counter = 0;

      while (assignment.parse(ifile)) {
        if ((assignment.elf_0.first <= assignment.elf_1.first) &&
            (assignment.elf_0.second >= assignment.elf_1.first)) {
          ++overlap_counter;
        } else if ((assignment.elf_1.first <= assignment.elf_0.first) &&
                   (assignment.elf_1.second >= assignment.elf_0.first)) {
          ++overlap_counter;
        }
      }

      std::cout << "Number of overlapping pairs: " << overlap_counter << std::endl;
    } break;
  }

  return 0;
}
