#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
enum class Part { FIRST = 0, SECOND };

struct MoveInstruction {
  int from = 0;
  int to = 0;
  int num_crates = 0;
};

// Note: Ids start at 1
class Graph {
 public:
  struct Node {
    Node(char c) : name{c} {}

    std::unique_ptr<Node> next = nullptr;
    char name;
  };

  Graph(std::size_t num_nodes);

  void pushNode(const int root_id, const char c);

  void moveIndividual(const MoveInstruction& instruction);

  void moveGrouped(const MoveInstruction& instruction);

  void print() const;

  void printRoots() const;

  bool empty() const;

 private:
  // top of crates
  std::vector<std::unique_ptr<Node>> roots_;

  // non-owning pointers to bottom node of each stack of crates
  // only used for construction of graph, not maintained later
  std::vector<Node*> ends_;
};

Graph readGraph(std::ifstream& ifile);

bool parseInstruction(std::ifstream& ifile, MoveInstruction& instruction);

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

  Graph graph = readGraph(ifile);

  if (graph.empty()) {
    std::cout << "Graph was empty" << std::endl;
    return 1;
  }

  graph.print();

  switch (part) {
    case Part::FIRST: {
      MoveInstruction instruction;
      while (parseInstruction(ifile, instruction)) {
        graph.moveIndividual(instruction);
      }
      graph.printRoots();
    } break;
    case Part::SECOND: {
      MoveInstruction instruction;
      while (parseInstruction(ifile, instruction)) {
        graph.moveGrouped(instruction);
      }
      graph.printRoots();
    } break;
  }

  return 0;
}

namespace {

Graph::Graph(std::size_t num_nodes) {
  roots_.resize(num_nodes + 1);
  ends_.resize(num_nodes + 1);
}

void Graph::pushNode(const int root_id, const char c) {
  Node*& root_end = ends_[root_id];

  if (root_end == nullptr) {
    // first node of that stack of crates
    roots_[root_id] = std::make_unique<Node>(c);
    root_end = roots_[root_id].get();
  } else {
    root_end->next = std::make_unique<Node>(c);
    root_end = root_end->next.get();
  }
}

void Graph::moveIndividual(const MoveInstruction& instruction) {
  // move one crate at a time
  for (int i = 0; i < instruction.num_crates; ++i) {
    if (roots_[instruction.from] == nullptr) {
      throw std::runtime_error("Stack empty");
    }

    Node* crate = roots_[instruction.from].get();

    // place crate on top of target stack
    // original target stack root is now pointing to new origin root
    crate->next.swap(roots_[instruction.to]);

    // origin root picks up temporary value from target root
    // and provides correct target root (which used to be origin root)
    roots_[instruction.from].swap(roots_[instruction.to]);

    // The last step could maybe be optimized by swapping the meaning of
    // "to" and "from" in each loop, but unclear which one is really faster
  }
}

void Graph::moveGrouped(const MoveInstruction& instruction) {
  Node* last_to_move = roots_[instruction.from].get();

  // find last crate to be moved
  for (int i = 0; i < instruction.num_crates - 1; ++i) {
    if (last_to_move->next != nullptr) {
      last_to_move = last_to_move->next.get();
    } else {
      throw std::runtime_error("Unexpectedly reached bottom of stack");
    }
  }

  // place last node on top of target stack
  // original target stack root is now pointing to new origin root
  last_to_move->next.swap(roots_[instruction.to]);

  // origin root picks up temporary value from target root
  // and provides correct target root (which used to be origin root)
  roots_[instruction.from].swap(roots_[instruction.to]);
}

void Graph::print() const {
  for (int i = 0; i < roots_.size(); ++i) {
    if (roots_[i] == nullptr) {
      std::cout << i << ": empty" << std::endl;
    } else {
      std::cout << i << ": ";
      Node* node = roots_[i].get();
      while (node != nullptr) {
        std::cout << char(node->name);
        node = node->next.get();
      }
      std::cout << std::endl;
    }
  }
}

void Graph::printRoots() const {
  for (const auto& up : roots_) {
    std::cout << static_cast<char>(up == nullptr ? '*' : up->name);
  }
  std::cout << std::endl;
}

bool Graph::empty() const { return (roots_.size() == 0); }

Graph readGraph(std::ifstream& ifile) {
  // initial lines have whitespace if no elements are present
  // characters are at every fourth plane
  std::string line;
  bool reached_actions = false;

  if (!std::getline(ifile, line)) {
    std::cout << "File stream was empty" << std::endl;
    return Graph{0};
  }

  // format is [X] [X]     [X] ... [X]
  std::size_t num_stacks = (line.size() + 1) / 4;
  Graph graph(num_stacks);

  do {
    char* c = &line[1];
    for (int stack_idx = 1; stack_idx <= num_stacks; ++stack_idx, c += 4) {
      if (*c != ' ') {
        if (std::isupper(static_cast<unsigned char>(*c))) {
          graph.pushNode(stack_idx, *c);
        } else if (std::isdigit(static_cast<unsigned char>(*c))) {
          // reached final lines with column digits
          reached_actions = true;
          break;
        } else {
          std::cout << "Unexpected input: " << char(*c) << std::endl;
        }
      }
    }
  } while (!reached_actions && std::getline(ifile, line));

  // skip empty line after stack description
  ifile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  return graph;
}

bool parseInstruction(std::ifstream& ifile, MoveInstruction& instruction) {
  // format of line is always
  // move X from A to B
  // where X, A, B are positive integers
  ifile.ignore(5);  // skip "move"
  ifile >> instruction.num_crates;
  ifile.ignore(6);  // skip " from "
  ifile >> instruction.from;
  ifile.ignore(4);  // skip " to "
  ifile >> instruction.to;

  // last line does not contain eol and would fail
  bool success = !ifile.fail();

  ifile.ignore(1);  // skip eol

  return success;
}

}  // namespace
