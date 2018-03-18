#include "cpp-lib/util.h"

#include "boost/algorithm/string.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

////////////////////////////////////////////////////////////////////////
// Output
////////////////////////////////////////////////////////////////////////
std::string person(const std::string& s) {
  return "\\person{" + s + "}";
}

std::string initial_instruction(const std::string& s) {
  return "\\initialinstruction{" + s + "}";
}

std::string intermediate_instruction(const std::string& s) {
  return "\\intermediateinstruction{" + s + "}";
}

bool is_instruction(const std::string& line) {
  assert (line.size() >= 1);
  if ('(' == line.at(0)) {
    if (')' != line.back()) {
      std::cerr << line << std::endl;
    }
    assert(')' == line.back());
    return true;
  } else {
    return false;
  }
}

bool allcaps(const std::string& line) {
  return cpl::util::utf8_toupper(line) == line;
}



std::string flush_block(
    const std::vector<std::string>& block,
    const std::vector<char>& types) {
  assert(1 <= block.size());
  assert(block.size() == types.size());
  assert('P' == types.at(0));

  const unsigned N = block.size();

  std::ostringstream oss;
  oss << person(block.at(0)) << '\n';
  for (unsigned i = 1; i < N; ++i) {
    const auto& line = block.at(i);
    const char t     = types.at(i);
    if ('V' == t) {
      oss << line;
      // Linebreak if continued verse
      if (i + 1 < N && 'V' == types.at(i + 1)) {
        oss << " \\\\";
      }
      oss << '\n';
    } else if ('I' == t) {
      if (1 == i) {
        oss << initial_instruction(line) << '\n';
      } else {
        oss << intermediate_instruction(line) << '\n';
      }
    } else {
      assert(!"Unknown type");
    }
  }

  return oss.str();
}





// Generates blocks for one language
std::vector<std::string> convert(std::istream& is) {
  std::string line;

  std::vector<std::string> ret;

  // All trimmed lines in current block
  std::vector<std::string> block;
  // and their types
  // P .... person 
  // I .... instruction 
  // V .... verse
  std::vector<char>        types;
  
  while (std::getline(is, line)) {
    boost::trim(line);
    if (line.empty()) { continue; }

    if (allcaps(line)) {
      // New person is speaking : flush
      if (!block.empty()) {
        ret.push_back(flush_block(block, types));
        block.clear();
        types.clear();
      }
      block.push_back(line);
      types.push_back('P');
    } else if (is_instruction(line)) {
      block.push_back(line);
      types.push_back('I');
    } else {
      block.push_back(line);
      types.push_back('V');
    }
  }
  return ret;
}

std::string empty() {
  return person("UNKNOWN CHARACTER") + '\n';
}

void output(
    std::ostream& os,
    const std::vector<std::vector<std::string>>& blocks) {
  assert(2 == blocks.size());

  const unsigned N = std::max(blocks.at(0).size(), blocks.at(1).size());

  for (unsigned i = 0; i < N; ++i) {
    const auto b0 = i < blocks.at(0).size() ? blocks.at(0).at(i) : empty();
    const auto b1 = i < blocks.at(1).size() ? blocks.at(1).at(i) : empty();
    os << "\\begin{paracol}{2}\n"
       << b0
       << "\n\\switchcolumn\n\n"
       << b1
       << "\\end{paracol}\n\n";
  }
}


int main() {
  try {
  std::ifstream l1("german.txt");
  std::ifstream l2("english.txt");

  std::ofstream out("generated.tex");

  const std::vector<std::vector<std::string>> blocks(
      {convert(l1), convert(l2)});

  output(out, blocks);

  } catch (std::exception const& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}
