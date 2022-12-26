#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

// TODO: Maybe this would be a challenge application for coroutines?
// Could traverse the tree, co_await a next directory command etc
namespace {
enum class Part { FIRST = 0, SECOND };

struct Node {
  Node(Node* parent);
  Node(Node* parent, const std::string& name);

  std::string name;

  // Total size of files in current directory
  std::size_t local_size = 0;

  // owning pointers to child nodes (directories)
  std::unordered_map<std::string, std::unique_ptr<Node>> children;

  // non-owning pointer to parent node
  Node* parent = nullptr;

  // enter (and if needed create) new directory
  Node* visitChild(const std::string& name);

  // return total size of tree below this node
  std::size_t getTotalSize() const;

  // for part one
  std::size_t getTotalSmallDirs(std::size_t max_size,
                                std::size_t& selected_total);
};

// helpers to make input parsing more readable
enum class Command { INVALID, LS, CD, CD_ROOT };

struct CliCommand {
  Command command = Command::INVALID;
  std::string name;
};

// we assume the input is valid
void parse(std::ifstream& ifile, CliCommand& cli);

void print(const Node& node);

void print(const Node& node, int level);
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

  Node root(nullptr, "root");
  Node* current_node = &root;
  switch (part) {
    case Part::FIRST: {
      CliCommand cli;
      while (parse(ifile, cli), cli.command != Command::INVALID) {
        switch (cli.command) {
          case Command::CD_ROOT:
            std::cout << "CD to root" << std::endl;
            current_node = &root;
            break;
          case Command::CD:
            std::cout << "CD to " << cli.name << std::endl;
            current_node = current_node->visitChild(cli.name);
            break;
          case Command::LS:
            // read current directory
            std::cout << "going to ls" << std::endl;
            std::string element;
            int next_char = ifile.peek();
            std::cout << "next char: " << next_char << " " << char(next_char)
                      << std::endl;
            while (ifile.peek() != '$' && ifile.good()) {
              ifile >> element;
              if (element.front() != 'd') {  // directory, don't care
                // can we have multiple ls of the same
                // directory? if yes, we need to keep track of
                // which file is how big... assuming not needed
                int size = std::stoi(element);
                current_node->local_size += std::stoi(element);
                std::cout << "Adding size of " << size << std::endl;
              }
              ifile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            break;
        }
      }

      std::size_t max_size = 100000;
      std::size_t selected_sizes = 0;
      root.getTotalSmallDirs(max_size, selected_sizes);

      std::cout << "Total directory sizes with max size " << max_size << ": "
                << selected_sizes << std::endl;
    } break;
    case Part::SECOND: {
      // TODO
    } break;
  }

  return 0;
}

namespace {
Node::Node(Node* parent) : parent(parent) {}

Node::Node(Node* parent, const std::string& name)
    : parent(parent), name(name) {}

Node* Node::visitChild(const std::string& name) {
  if (name == "..") {
    if (parent != nullptr) {
      std::cout << "Moving up to " << parent->name << std::endl;
    } else {
      std::cout << "Parent was nullptr!" << std::endl;
    }
    return parent;
  } else {
    std::cout << "Moving down to " << name << std::endl;
    auto it =
        children.try_emplace(name, std::make_unique<Node>(this, name)).first;
    return it->second.get();
  }
}

std::size_t Node::getTotalSize() const {
  std::size_t total_size = local_size;
  for (const auto& child : children) {
    total_size += child.second->getTotalSize();
  }
  std::cout << "Total size of " << name << " is " << total_size << std::endl;
  return total_size;
}

std::size_t Node::getTotalSmallDirs(std::size_t max_size,
                                    std::size_t& selected_total) {
  std::size_t dir_size = local_size;
  for (const auto& child : children) {
    dir_size += child.second->getTotalSmallDirs(max_size, selected_total);
  }

  if (dir_size <= max_size) {
    selected_total += dir_size;
  }

  return dir_size;
}

void parse(std::ifstream& ifile, CliCommand& cli) {
  cli.command = Command::INVALID;
  char c = static_cast<char>(ifile.peek());
  if (ifile.get() == '$') {
    ifile.ignore(1);
    c = ifile.get();
    if (c == 'c') {  // cd
      ifile.ignore(2);
      ifile >> cli.name;

      if (cli.name.front() == '/') {
        cli.command = Command::CD_ROOT;
      } else {
        cli.command = Command::CD;
      }
    } else if (c == 'l') {  // ls
      cli.command = Command::LS;
    }
  } else if (c == std::char_traits<char>::eof()) {
    std::cout << "Reached EOF" << std::endl;
  } else {
    std::cout << "Unexpected input: " << static_cast<int>(c) << std::endl;
  }

  // read until end of line
  ifile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void print(const Node& node) {
  std::cout << "root: " << node.children.size() << std::endl;

  int level = 0;
  for (const auto& c : node.children) {
    std::cout << "|- " << c.first << std::endl;
    print(*c.second, level + 1);
  }
}

void print(const Node& node, int level) {
  std::string head = std::string(2 * level, ' ') + "|- ";
  for (const auto& c : node.children) {
    std::cout << head << c.first << " " << c.second->children.size()
              << std::endl;
    print(*c.second, level + 1);
  }
}
}  // namespace