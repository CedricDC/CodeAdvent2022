#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {
enum class Part { FIRST = 0, SECOND };

using MonkeyId = unsigned int;

template <typename ItemType = std::size_t>
struct ItemThrow {
  ItemType item;
  MonkeyId target;
};

// Special "item" which keeps values as seen by each monkey
// This is pretty ugly as it mixes monkey internal and problem global information
struct MultiItem {
 public:
  struct Pair {
    Pair(std::size_t item, std::size_t mod) : item_value{item}, mod_value{mod} {}
    std::size_t item_value;
    std::size_t mod_value;
  };

  MultiItem(std::size_t start_value) { pairs_.emplace_back(start_value, 1); }

  void initialize(const std::vector<std::size_t>& mod_values) {
    if (pairs_.size() > 1) {
      std::cout << "Already initialized" << std::endl;
      return;
    }

    std::size_t start_value = pairs_.front().item_value;

    pairs_.clear();
    pairs_.reserve(mod_values.size());
    for (const std::size_t mod : mod_values) {
      pairs_.emplace_back(start_value, mod);
    }
  }

  std::vector<Pair>::iterator begin() { return pairs_.begin(); }
  std::vector<Pair>::iterator end() { return pairs_.end(); }
  std::size_t getValue(const MonkeyId id) const { return pairs_[id].item_value; }

 private:
  std::vector<Pair> pairs_;
};

// an operation consists of the full monkey inspection
template <typename ItemType = std::size_t>
using Operation = std::function<ItemThrow<ItemType>(ItemType)>;

template <typename ItemType = std::size_t>
class Monkey {
 public:
  using Item = ItemType;

  Monkey(MonkeyId id) : id_{id} {
    operation_ = [id = id_](ItemType val) -> ItemThrow<ItemType> { return {val, id}; };
  }

  void pushItem(ItemType item) { items_.push_back(item); }

  void setOperation(Operation<ItemType> op) { operation_ = op; }

  void setModValue(unsigned int value) { mod_value_ = value; }

  ItemThrow<ItemType> inspectNext() {
    ++num_inspections_;
    ItemType item = items_.front();
    items_.pop_front();
    return operation_(item);
  }

  bool hasItem() const { return !items_.empty(); }

  const std::deque<ItemType>& items() const { return items_; }
  std::deque<ItemType>& items() { return items_; }

  std::size_t numInspections() const { return num_inspections_; }

  MonkeyId id() const { return id_; }

  unsigned int getModValue() const { return mod_value_; }

 private:
  MonkeyId id_ = 0;
  std::deque<ItemType> items_;
  Operation<ItemType> operation_;

  // for part 2
  unsigned int mod_value_ = 1;

  std::size_t num_inspections_ = 0;
};

/*
 * Assumptions:
 * - Ids increase incrementally
 * - No division operations
 */
template <typename ItemType>
bool parseMonkey(Part part, std::ifstream& ifile, std::vector<Monkey<ItemType>>& monkeys);

// return product of two highest inspection numbers among all monkeys
template <typename MonkeyType>
std::size_t computeMonkeyBusiness(const std::vector<MonkeyType>& monkeys);

template <typename MonkeyType>
void printItems(const std::vector<MonkeyType>& monkeys);

template <>
void printItems(const std::vector<Monkey<MultiItem>>& monkeys);

template <typename MonkeyType>
void printInspections(const std::vector<MonkeyType>& monkeys);

// find least common multiple of a set of values
std::size_t getLcm(const std::vector<std::size_t>& values);

}  // namespace

/*
 * Notes for part 2:
 * Two ways I can think of solving this (neither very elegant) are
 * 1. Take all the module values of all monkeys, take the product and for each item, apply module to
 * it This is easier to implement as it does not require changing the item type and accessing, but
 * is not scalable and the values are still very big. For our input, this happens to be ok.
 * 2. Each item consists of a map representing the value as seen from each monkey.
 */
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

  auto t0 = std::chrono::steady_clock::now();
  std::string filename = argv[1];
  std::ifstream ifile(argv[1]);

  if (!ifile.good()) {
    std::cout << "Could not find " << filename << std::endl;
    return 1;
  }

  switch (part) {
    case Part::FIRST: {
      using ItemType = std::size_t;
      std::vector<Monkey<ItemType>> monkeys;

      while (parseMonkey(part, ifile, monkeys)) {
      }

      // run simulation
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
      std::cout << "Monkey business level: " << computeMonkeyBusiness(monkeys) << std::endl;

    } break;
    case Part::SECOND: {
      static constexpr int num_rounds = 10000;
      static constexpr int version = 1;

      // Version 1 : big modulo value
      if (version == 1) {
        std::cout << "Using big modulo version" << std::endl;

        using ItemType = std::size_t;
        std::vector<Monkey<ItemType>> monkeys;

        while (parseMonkey(part, ifile, monkeys)) {
        }

        std::vector<std::size_t> mod_values;
        mod_values.reserve(monkeys.size());
        for (const auto& m : monkeys) mod_values.push_back(m.getModValue());

        const std::size_t lcm = getLcm(mod_values);

        if (lcm > std::sqrt(std::numeric_limits<std::size_t>::max())) {
          throw std::runtime_error("LCM exceeds max manageable value");
        }

        // run simulation
        for (int i = 0; i < num_rounds; ++i) {
          for (auto& monkey : monkeys) {
            while (monkey.hasItem()) {
              auto item_throw = monkey.inspectNext();

              // limit size to manageable value
              item_throw.item = item_throw.item % lcm;

              monkeys[item_throw.target].pushItem(item_throw.item);
            }
          }
        }

        // find two most active monkeys
        std::cout << "Monkey business level: " << computeMonkeyBusiness(monkeys) << std::endl;

      } else if (version == 2) {
        std::cout << "Using individual modulo version" << std::endl;

        // Version 2 : multi item type
        using ItemType = MultiItem;
        std::vector<Monkey<ItemType>> monkeys;

        while (parseMonkey(part, ifile, monkeys)) {
        }

        std::vector<std::size_t> mod_values;
        mod_values.reserve(monkeys.size());
        for (const auto& m : monkeys) mod_values.push_back(m.getModValue());

        // initialize multi item to have value for each monkey
        for (auto& m : monkeys) {
          for (auto& item : m.items()) {
            item.initialize(mod_values);
          }
        }

        // run simulation
        for (int i = 0; i < num_rounds; ++i) {
          for (auto& monkey : monkeys) {
            while (monkey.hasItem()) {
              auto item_throw = monkey.inspectNext();
              monkeys[item_throw.target].pushItem(item_throw.item);
            }
          }
        }

        // find two most active monkeys
        std::cout << "Monkey business level: " << computeMonkeyBusiness(monkeys) << std::endl;
      }
    } break;
  }
  auto t1 = std::chrono::steady_clock::now();
  std::cout << "Computation time: " << 1e-3 * (t1 - t0).count() << " [us]" << std::endl;

  return 0;
}

namespace {

template <typename ItemType>
bool parseMonkey(Part part, std::ifstream& ifile, std::vector<Monkey<ItemType>>& monkeys) {
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
        ItemType item(token_int);
        monkey.pushItem(item);
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

    Operation<ItemType> operation;

    if constexpr (std::is_same_v<ItemType, std::size_t>) {
      if (part == Part::FIRST) {
        switch (op_type) {
          case '*': {
            operation = [op_value, mod_value, true_target, false_target](ItemType item) {
              item *= op_value;
              item /= 3;
              if (item % mod_value == 0) {
                return ItemThrow<ItemType>{item, true_target};
              } else {
                return ItemThrow<ItemType>{item, false_target};
              }
            };
          } break;
          case '+': {
            operation = [op_value, mod_value, true_target, false_target](ItemType item) {
              item += op_value;
              item /= 3;
              if (item % mod_value == 0) {
                return ItemThrow<ItemType>{item, true_target};
              } else {
                return ItemThrow<ItemType>{item, false_target};
              }
            };
          } break;
          case 's': {
            operation = [op_value, mod_value, true_target, false_target](ItemType item) {
              item *= item;
              item /= 3;
              if (item % mod_value == 0) {
                return ItemThrow<ItemType>{item, true_target};
              } else {
                return ItemThrow<ItemType>{item, false_target};
              }
            };
          } break;
          default:
            throw std::runtime_error("Operation not supported: " + std::to_string(op_type));
        }
      } else {  // Part::SECOND
        switch (op_type) {
          case '*': {
            operation = [op_value, mod_value, true_target, false_target](ItemType item) {
              item *= op_value;
              if (item % mod_value == 0) {
                return ItemThrow<ItemType>{item, true_target};
              } else {
                return ItemThrow<ItemType>{item, false_target};
              }
            };
          } break;
          case '+': {
            operation = [op_value, mod_value, true_target, false_target](ItemType item) {
              item += op_value;
              if (item % mod_value == 0) {
                return ItemThrow<ItemType>{item, true_target};
              } else {
                return ItemThrow<ItemType>{item, false_target};
              }
            };
          } break;
          case 's': {
            operation = [op_value, mod_value, true_target, false_target](ItemType item) {
              item *= item;
              if (item % mod_value == 0) {
                return ItemThrow<ItemType>{item, true_target};
              } else {
                return ItemThrow<ItemType>{item, false_target};
              }
            };
          } break;
          default:
            throw std::runtime_error("Operation not supported: " + std::to_string(op_type));
        }

        monkey.setModValue(mod_value);
      }
    } else if constexpr (std::is_same_v<ItemType, MultiItem>) {
      switch (op_type) {
        case '*': {
          operation = [id = monkey.id(), op_value, mod_value, true_target,
                       false_target](ItemType item) {
            for (auto& [item_val, mod_val] : item) {
              item_val = (item_val * op_value) % mod_val;
            }
            if (item.getValue(id) == 0) {
              return ItemThrow<ItemType>{item, true_target};
            } else {
              return ItemThrow<ItemType>{item, false_target};
            }
          };
        } break;
        case '+': {
          operation = [id = monkey.id(), op_value, mod_value, true_target,
                       false_target](ItemType item) {
            for (auto& [item_val, mod_val] : item) {
              item_val = (item_val + op_value) % mod_val;
            }
            if (item.getValue(id) == 0) {
              return ItemThrow<ItemType>{item, true_target};
            } else {
              return ItemThrow<ItemType>{item, false_target};
            }
          };
        } break;
        case 's': {
          operation = [id = monkey.id(), op_value, mod_value, true_target,
                       false_target](ItemType item) {
            for (auto& [item_val, mod_val] : item) {
              item_val = (item_val * item_val) % mod_val;
            }
            if (item.getValue(id) == 0) {
              return ItemThrow<ItemType>{item, true_target};
            } else {
              return ItemThrow<ItemType>{item, false_target};
            }
          };
        } break;
        default:
          throw std::runtime_error("Operation not supported: " + std::to_string(op_type));
      }

      monkey.setModValue(mod_value);
    }

    monkey.setOperation(operation);
  }

  // Note: This will not trigger yet at eof as we want to process the last monkey too
  return !ifile.fail();
}

template <typename MonkeyType>
std::size_t computeMonkeyBusiness(const std::vector<MonkeyType>& monkeys) {
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

  return most_inspections[0] * most_inspections[1];
}

template <typename MonkeyType>
void printItems(const std::vector<MonkeyType>& monkeys) {
  for (const auto& monkey : monkeys) {
    std::cout << monkey.id() << ":";
    for (const auto item : monkey.items()) {
      std::cout << " " << item;
    }
    std::cout << std::endl;
  }
}

template <>
void printItems(const std::vector<Monkey<MultiItem>>& monkeys) {
  for (const auto& monkey : monkeys) {
    std::cout << monkey.id() << ":";
    for (const auto item : monkey.items()) {
      std::cout << " " << item.getValue(monkey.id());
    }
    std::cout << std::endl;
  }
}

template <typename MonkeyType>
void printInspections(const std::vector<MonkeyType>& monkeys) {
  for (const auto& monkey : monkeys) {
    std::cout << monkey.id() << ": " << monkey.numInspections() << std::endl;
  }
}

std::size_t getLcm(const std::vector<std::size_t>& values) {
  // First, get list of prime numbers up to sqrt(max value)
  // For simplicity, we also have elements 0 and 1 (which are not considered)
  std::size_t max_value = *std::max_element(values.cbegin(), values.cend());
  std::vector<bool> prime_number_candidates(1 + std::sqrt(max_value), true);
  prime_number_candidates[0] = false;
  prime_number_candidates[1] = false;

  std::vector<std::size_t> primes;
  for (std::size_t i = 2; i < prime_number_candidates.size(); ++i) {
    if (prime_number_candidates[i]) {  // found prime!
      primes.push_back(i);

      // all multiples of that prime are not prime
      for (std::size_t j = i; j < prime_number_candidates.size(); j += i) {
        prime_number_candidates[j] = false;
      }
    }
  }

  // now, perform prime decomposition and find least common multiple
  std::map<std::size_t, std::size_t> factors;

  for (std::size_t value : values) {
    for (const auto p : primes) {
      std::size_t counter = 0;
      while (value % p == 0) {
        ++counter;
        value /= p;
      }

      if (counter > 0) {
        // add multiple if bigger than biggers so far
        auto it = factors.find(p);
        if (it == factors.cend()) {
          factors[p] = counter;
        } else {
          factors[p] = std::max(counter, it->second);
        }
      }

      // early exit
      if (p > value) break;
    }

    if (value > 1) {  // remainder must be prime
      auto it = factors.find(value);
      if (factors.find(value) == factors.cend()) {
        factors[value] = 1;
      }
    }
  }

  std::size_t lcm = 1;
  for (const auto& factor_pair : factors) {
    lcm *= std::pow(factor_pair.first, factor_pair.second);
  }

  return lcm;
}

}  // namespace
