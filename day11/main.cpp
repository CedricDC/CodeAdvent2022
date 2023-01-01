#include <cctype>
#include <cstdint>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {
enum class Part { FIRST = 0, SECOND };

using Item = unsigned int;
using MonkeyId = unsigned int;

struct ItemThrow {
  Item item;
  MonkeyId target;
};

// an operation consists of the full monkey inspection
using Operation = std::function<ItemThrow(Item)>;

class Monkey {
 public:
  Monkey(MonkeyId id) : id_{id} {
    operation_ = [id = id_](Item val) -> ItemThrow { return {val, id}; };
  }

  void pushItem(Item item) { items_.push_back(item); }

  void setOperation(Operation op) { operation_ = op; }

  ItemThrow inspectNext() {
    ++num_inspections_;
    Item item = items_.front();
    items_.pop_front();
    return operation_(item);
  }

  bool hasItem() const { return !items_.empty(); }

  const std::deque<Item>& items() const { return items_; }

  std::size_t numInspections() const { return num_inspections_; }

  MonkeyId id() const { return id_; }

 private:
  MonkeyId id_ = 0;
  std::deque<Item> items_;
  Operation operation_;

  std::size_t num_inspections_ = 0;
};

/*
 * Assumptions:
 * - Ids increase incrementally
 * - No division operations
 * - All values of operation are below 100 (i.e. value in old = old * value)
 */
bool parseMonkey(std::ifstream& ifile, std::vector<Monkey>& monkeys);

void printItems(const std::vector<Monkey>& monkeys);

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

  std::vector<Monkey> monkeys;

  while (parseMonkey(ifile, monkeys)) {
  }

  switch (part) {
    case Part::FIRST: {
      static constexpr int num_rounds = 20;
      for (int i = 0; i < num_rounds; ++i) {
        for (auto& monkey : monkeys) {
          while (monkey.hasItem()) {
            auto item_throw = monkey.inspectNext();
            monkeys[item_throw.target].pushItem(item_throw.item);
          }
        }
      }

      // find two most active monkeys
      std::vector<std::size_t> most_inspections{0, 0};
      for (const auto& monkey : monkeys) {
        std::size_t activity = monkey.numInspections();
        if (activity > most_inspections[0]) {
          most_inspections[0] = activity;
        }

        if (most_inspections[0] > most_inspections[1]) {
          std::swap(most_inspections[0], most_inspections[1]);
        }
      }
      std::cout << "Monkey business level: " << most_inspections[0] * most_inspections[1]
                << std::endl;

    } break;
    case Part::SECOND: {
      // TODO
    } break;
  }

  return 0;
}

namespace {

bool parseMonkey(std::ifstream& ifile, std::vector<Monkey>& monkeys) {
  // we expect incremental ids, if false we need the monkeys in a map
  static std::size_t expected_id = 0;

  std::string line;
  std::string token_str;
  unsigned int token_int = 0;

  ifile.ignore(std::string_view("Monkey ").size());

  // if parsing the above failed, we have reached eof (or an error)
  if (ifile.fail()) return false;

  ifile >> token_int;
  ifile.ignore(2);  // remove : and \n

  auto& monkey = monkeys.emplace_back(token_int);
  if (expected_id++ != token_int) {
    throw std::runtime_error("Expected incremental monkey ids, assumption violated");
  }

  auto get_string_after = [&ifile](const char* text) {
    std::string line;
    std::getline(ifile, line);
    int skip_size = std::string_view(text).size();
    return line.substr(skip_size);
  };

  // parse starting items
  {
    std::string item_string = get_string_after(" Starting items: ");
    if (!item_string.empty()) {  // else, no starting items
      std::stringstream iss(item_string);

      while (iss >> token_int) {
        monkey.pushItem(token_int);
        if (iss.peek() == ',') iss.ignore(1);  // ignore comma
      }
    }
  }

  // parse operation
  {
    char op_type;
    int op_value, mod_value;
    MonkeyId true_target, false_target;

    {
      std::istringstream iss(get_string_after("  Operation: new = old "));
      op_type = iss.get();
      iss.ignore(1);  // remove space

      // corner case : instead of value, we can have string "old"
      //               We will encode this as operation 's' (squared)
      if (std::isdigit(static_cast<unsigned char>(iss.peek()))) {
        iss >> op_value;
      } else {
        op_type = 's';
      }
    }

    {
      std::istringstream iss(get_string_after("  Test: divisible by "));
      iss >> mod_value;
    }

    {
      std::istringstream iss(get_string_after("   If true: throw to monkey "));
      iss >> true_target;
    }

    {
      std::istringstream iss(get_string_after("   If false: throw to monkey "));
      iss >> false_target;
    }

    Operation operation;
    switch (op_type) {
      case '*': {
        operation = [op_value, mod_value, true_target, false_target](Item item) {
          item *= op_value;
          item /= 3;
          if (item % mod_value == 0) {
            return ItemThrow{item, true_target};
          } else {
            return ItemThrow{item, false_target};
          }
        };
      } break;
      case '+': {
        operation = [op_value, mod_value, true_target, false_target](Item item) {
          item += op_value;
          item /= 3;
          if (item % mod_value == 0) {
            return ItemThrow{item, true_target};
          } else {
            return ItemThrow{item, false_target};
          }
        };
      } break;
      case 's': {
        operation = [op_value, mod_value, true_target, false_target](Item item) {
          item *= item;
          item /= 3;
          if (item % mod_value == 0) {
            return ItemThrow{item, true_target};
          } else {
            return ItemThrow{item, false_target};
          }
        };
      } break;
      default:
        throw std::runtime_error("Operation not supported: " + std::to_string(op_type));
    }

    monkey.setOperation(operation);
  }

  // Note: This will not trigger yet at eof as we want to process the last monkey too
  return !ifile.fail();
}

void printItems(const std::vector<Monkey>& monkeys) {
  for (const auto& monkey : monkeys) {
    std::cout << monkey.id() << ":";
    for (const auto item : monkey.items()) {
      std::cout << " " << item;
    }
    std::cout << std::endl;
  }
}

}  // namespace
