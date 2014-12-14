#include "framework/unit.hpp"

int main(int argc, char* argv[])
{
  // TODO: parse command line.
  bool colorize = true;
  std::string log_file;
  int verbosity_console = 3;
  int verbosity_file = 3;
  std::regex suites{".*"};
  std::regex not_suites;
  std::regex tests{".*"};
  std::regex not_tests;

  auto result = unit::engine::run(colorize, log_file, verbosity_console,
                                  verbosity_file, suites, not_suites, tests,
                                  not_tests);
  return result ? 0 : 1;
}
