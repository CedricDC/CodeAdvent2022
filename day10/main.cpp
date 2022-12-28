#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {
enum class Part { FIRST = 0, SECOND };

struct Instruction {
  enum class Type { NOOP, ADD };

  Type type;
  int value;
  int cycles;
};

bool readInstruction(std::ifstream& ifile, Instruction& instruction);

struct Memory {
  static constexpr int REG_INIT = 1;
  // adding 1 to the "delay" because the instruction is only applied in the next
  // cycle
  Memory(int max_delay);

  // go to next clock cycle
  void stepCycle();

  void apply(const Instruction& instruction);

  int value() const;

 private:
  const int max_cycles_;
  std::vector<int> register_value_;
  int idx_ = 0;
  int next_idx_ = 1;
};

struct CRT {
  void drawPixel(int value);

 private:
  int counter_ = 0;
  static constexpr int num_cols_ = 40;
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
      Memory memory(2);  // max cycles is the addition with 2
      Instruction instruction;
      std::size_t cycle_counter = 1;  // during first cycle, value is 1
      int total_signal_strength = 0;
      while (readInstruction(ifile, instruction)) {
        memory.apply(instruction);

        for (int i = 0; i < instruction.cycles; ++i) {
          memory.stepCycle();
          if (++cycle_counter % 40 == 20) {
            std::cout << "Reached cycle " << cycle_counter << ", value is "
                      << memory.value() << std::endl;
            total_signal_strength += cycle_counter * memory.value();
          }
        }
      }

      std::cout << "Total signal strength: " << total_signal_strength
                << std::endl;
    } break;
    case Part::SECOND: {
      Memory memory(2);  // max cycles is the addition with 2
      CRT crt;
      Instruction instruction;
      crt.drawPixel(memory.value());  // draw initial value
      while (readInstruction(ifile, instruction)) {
        memory.apply(instruction);

        for (int i = 0; i < instruction.cycles; ++i) {
          memory.stepCycle();
          crt.drawPixel(memory.value());
        }
      }
    } break;
  }

  return 0;
}

namespace {

bool readInstruction(std::ifstream& ifile, Instruction& instruction) {
  std::string cmd;
  ifile >> cmd;

  if (cmd.front() == 'a') {
    instruction.type = Instruction::Type::ADD;
    ifile >> instruction.value;
    instruction.cycles = 2;
  } else {
    instruction.type = Instruction::Type::NOOP;
    instruction.cycles = 1;
  }

  return !ifile.fail();
}

Memory::Memory(int max_delay)
    : max_cycles_(max_delay + 1), register_value_(max_delay + 1, REG_INIT) {}

// go to next clock cycle
void Memory::stepCycle() {
  idx_ = next_idx_;
  next_idx_ = (next_idx_ + 1) % max_cycles_;
}

void Memory::apply(const Instruction& instruction) {
  switch (instruction.type) {
    case Instruction::Type::NOOP:
      register_value_[next_idx_] = register_value_[idx_];
      break;
    case Instruction::Type::ADD: {
      int current_value = register_value_[idx_];
      register_value_[next_idx_] = current_value;
      register_value_[(next_idx_ + 1) % max_cycles_] =
          current_value + instruction.value;
      break;
    }
  }
}

int Memory::value() const { return register_value_[idx_]; }

void CRT::drawPixel(int value) {
  if (std::abs(value - counter_) <= 1) {
    // within range of sprite
    std::cout << '#';
  } else {
    std::cout << '.';
  }

  if (++counter_ % num_cols_ == 0) {
    std::cout << std::endl;
    counter_ = 0;
  }
}

}  // namespace