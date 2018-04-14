#include "cpp-lib/assert.h"
#include "cpp-lib/util.h"

#include "boost/algorithm/string.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

struct parsing_context {
  long the_line_number = 0;
  std::string the_file;
  std::ifstream is;

  void open(const std::string& fn) {
    is.close();
    is.open(fn);
    the_file = fn;
    the_line_number = 0;
  }
};

////////////////////////////////////////////////////////////////////////
// Output
////////////////////////////////////////////////////////////////////////
std::string person(const std::string& s) {
  return "\\person{" + s + "}";
}

std::string stage_direction(const std::string& s) {
  return "\\stagedirection{" + s + "}";
}

std::string stage_direction_sep() {
  return "\\stagedirectionsep";
}

bool is_instruction(const std::string& line) {
  always_assert (line.size() >= 1);
  if ('(' == line.at(0)) {
    if (')' != line.back()) {
      std::cerr << line << std::endl;
    }
    always_assert(')' == line.back());
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
  always_assert(1 <= block.size());
  always_assert(block.size() == types.size());
  cpl::util::verify('P' == types.at(0), "Block doesn't start with person");

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
      if (i + 1 < N && 'I' == types.at(i + 1)) {
        oss << stage_direction_sep() << '\n';
      }
    } else if ('I' == t) {
      // Stage direction.
      // Add vspace iff followed by verse or another stage direction.
      oss << stage_direction(line) << '\n';
      if (i + 1 < N) {
        oss << stage_direction_sep() << '\n';
      }
    } else {
      always_assert(!"Unknown type");
    }
  }

  return oss.str();
}





// Generates blocks for one language
std::vector<std::string> convert(parsing_context& ctx) {
  std::string line;

  std::vector<std::string> ret;

  // All trimmed lines in current block
  std::vector<std::string> block;
  // and their types
  // P .... person 
  // I .... stage direction
  // V .... verse
  std::vector<char>        types;

  while (std::getline(ctx.is, line)) {
    ++ctx.the_line_number;
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
  if (!block.empty()) {
    ret.push_back(flush_block(block, types));
  }
  return ret;
}

std::string empty() {
  return person("UNKNOWN CHARACTER") + '\n';
}

void output(
    std::ostream& os,
    const std::vector<std::vector<std::string>>& blocks) {
  always_assert(2 == blocks.size());

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
  parsing_context ctx;

  try {
  ctx.open("german.txt");
  auto b1 = convert(ctx);

  ctx.open("english.txt");
  auto b2 = convert(ctx);

  std::ofstream out("generated.tex");
  output(out, {b1, b2});

  } catch (std::exception const& e) {
    std::cerr << ctx.the_file << ":" << ctx.the_line_number << ": " 
              << e.what() << std::endl;
    return 1;
  }
}
