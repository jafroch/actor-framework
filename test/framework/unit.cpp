#include "framework/unit.hpp"

#include <thread>

char const* unit::color::reset        = "\033[0m";
char const* unit::color::black        = "\033[30m";
char const* unit::color::red          = "\033[31m";
char const* unit::color::green        = "\033[32m";
char const* unit::color::yellow       = "\033[33m";
char const* unit::color::blue         = "\033[34m";
char const* unit::color::magenta      = "\033[35m";
char const* unit::color::cyan         = "\033[36m";
char const* unit::color::white        = "\033[37m";
char const* unit::color::bold_black   = "\033[1m\033[30m";
char const* unit::color::bold_red     = "\033[1m\033[31m";
char const* unit::color::bold_green   = "\033[1m\033[32m";
char const* unit::color::bold_yellow  = "\033[1m\033[33m";
char const* unit::color::bold_blue    = "\033[1m\033[34m";
char const* unit::color::bold_magenta = "\033[1m\033[35m";
char const* unit::color::bold_cyan    = "\033[1m\033[36m";
char const* unit::color::bold_white   = "\033[1m\033[37m";
char const* unit::detail::suite       = nullptr;

namespace unit {

test::test(std::string name)
  : name_{std::move(name)} {
}

size_t test::__expected_failures() const {
  return expected_failures_;
}

void test::__pass(std::string msg) {
  trace_.emplace_back(true, std::move(msg));
}

void test::__fail(std::string msg, bool expected) {
  trace_.emplace_back(false, std::move(msg));
  if (expected)
    ++expected_failures_;
}

const std::vector<std::pair<bool, std::string>>& test::__trace() const {
  return trace_;
}

const std::string& test::__name() const {
  return name_;
}


void engine::add(char const* name, std::unique_ptr<test> t) {
  auto& suite = instance().suites_[std::string{name ? name : ""}];
  for (auto& x : suite)
    if (x->__name() == t->__name()) {
      std::cout << "duplicate test name: " << t->__name() << '\n';
      std::abort();
    }
  suite.emplace_back(std::move(t));
}

namespace {

std::string render(std::chrono::microseconds t) {
  return t.count() > 1000000
    ? (std::to_string(t.count() / 1000000) + '.'
      + std::to_string((t.count() % 1000000) / 10000) + " s")
    : t.count() > 1000
      ? (std::to_string(t.count() / 1000) + " ms")
      : (std::to_string(t.count()) + " us");
}

char const* check_file = "<none>";
size_t check_line = 0;

} // namespace <anonymous>

char const* engine::last_check_file() {
  return check_file;
}

void engine::last_check_file(char const* file) {
  check_file = file;
}

size_t engine::last_check_line() {
  return check_line;
}

void engine::last_check_line(size_t line) {
  check_line = line;
}

bool engine::run(bool colorize,
                 const std::string& log_file,
                 int verbosity_console,
                 int verbosity_file,
                 int max_runtime,
                 const std::regex& suites,
                 const std::regex& not_suites,
                 const std::regex& tests,
                 const std::regex& not_tests) {
  if (! colorize) {
    color::reset        = "";
    color::black        = "";
    color::red          = "";
    color::green        = "";
    color::yellow       = "";
    color::blue         = "";
    color::magenta      = "";
    color::cyan         = "";
    color::white        = "";
    color::bold_black   = "";
    color::bold_red     = "";
    color::bold_green   = "";
    color::bold_yellow  = "";
    color::bold_blue    = "";
    color::bold_magenta = "";
    color::bold_cyan    = "";
    color::bold_white   = "";
  }

  if (! logger::init(verbosity_console, verbosity_file, log_file))
    return false;
  auto& log = logger::instance();

  std::chrono::microseconds runtime{0};
  size_t total_suites = 0;
  size_t total_tests = 0;
  size_t total_good = 0;
  size_t total_bad = 0;
  size_t total_bad_expected = 0;
  auto bar = '+' + std::string(70, '-') + '+';

  for (auto& p : instance().suites_) {
    if (! std::regex_search(p.first, suites) ||
        std::regex_search(p.first, not_suites))
      continue;
    auto suite_name = p.first.empty() ? "<unnamed>" : p.first;
    auto pad = std::string((bar.size() - suite_name.size()) / 2, ' ');
    bool displayed_header = false;
    size_t tests_ran = 0;
    for (auto& t : p.second) {
      if (! std::regex_search(t->__name(), tests)
          || std::regex_search(t->__name(), not_tests))
        continue;
      ++tests_ran;
      if (! displayed_header) {
        log.verbose()
          << color::yellow << bar << '\n' << pad << suite_name << '\n' << bar
          << color::reset << "\n\n";
        displayed_header = true;
      }
      log.verbose()
          << color::yellow << "- " << color::reset << t->__name() << '\n';
      auto failed_require = false;
      auto start = std::chrono::steady_clock::now();
      try {
        auto test_name = t->__name();
        std::thread{[test_name, max_runtime] {
          std::this_thread::sleep_for(std::chrono::seconds(max_runtime));
          std::cerr
            << "WATCHDOG: test " << test_name << " did not complete within "
            << max_runtime << " seconds" << std::endl;
          ::abort();
        }}.detach();
        t->__run();
      } catch (const require_error& e) {
        failed_require = true;
      }
      auto stop = std::chrono::steady_clock::now();
      auto elapsed =
        std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
      runtime += elapsed;
      size_t good = 0;
      size_t bad = 0;
      for (auto& trace : t->__trace())
        if (trace.first) {
          ++good;
          log.massive() << "  " << trace.second << '\n';
        } else {
          ++bad;
          log.error() << "  " << trace.second << '\n';
        }
      if (failed_require)
        log.error()
          << color::red << "     REQUIRED" << color::reset << '\n'
          << "     " << color::blue << last_check_file() << color::yellow
          << ":" << color::cyan << last_check_line() << color::reset
          << detail::fill(last_check_line()) << "had last successful check"
          << '\n';
      total_good += good;
      total_bad += bad;
      total_bad_expected += t->__expected_failures();
      log.verbose()
          << color::yellow << "  -> " << color::cyan << good + bad
          << color::reset << " check" << (good + bad > 1 ? "s " : " ")
          << "took " << color::cyan << render(elapsed) << color::reset;
      if (bad > 0)
        log.verbose()
          << " (" << color::green << good << color::reset << '/'
          << color::red << bad << color::reset << ")" << '\n';
      else
        log.verbose() << '\n';
      if (failed_require)
        break;
      ++total_tests;
    }
    // We only counts suites which have executed one or more tests.
    if (tests_ran > 0)
      ++total_suites;
    if (displayed_header)
      log.verbose() << '\n';
  }

  auto percent_good =
    unsigned(100000 * total_good / double(total_good + total_bad)) / 1000.0;

  auto title = std::string{"summary"};
  auto pad = std::string((bar.size() - title.size()) / 2, ' ');
  auto indent = std::string(24, ' ');

  log.info()
    << color::cyan << bar << '\n' << pad << title << '\n' << bar
    << color::reset << "\n\n"
    << indent << "suites:  " << color::yellow << total_suites << color::reset
    << '\n' << indent << "tests:   " << color::yellow << total_tests
    << color::reset << '\n' << indent << "checks:  " << color::yellow
    << total_good + total_bad << color::reset;

  if (total_bad > 0) {
    log.info()
    << " (" << color::green << total_good << color::reset << '/'
    << color::red << total_bad << color::reset << ")";
    if (total_bad_expected)
      log.info()
        << ' ' << color::cyan << total_bad_expected << color::reset
        << " failures expected";
  }

  log.info()
    << '\n' << indent << "time:    " << color::yellow << render(runtime)
    << '\n' << color::reset << indent << "success: "
    << (percent_good == 100.0 ? color::green : color::yellow)
    << percent_good << "%" << color::reset << "\n\n"
    << color::cyan << bar << color::reset << '\n';

  return total_bad == total_bad_expected;
}

engine& engine::instance() {
  static engine e;
  return e;
}

logger::message::message(logger& l, level lvl)
  : logger_{l},
    level_{lvl} {
}

bool logger::init(int lvl_cons, int lvl_file, const std::string& logfile) {
  instance().level_console_ = static_cast<level>(lvl_cons);
  instance().level_file_ = static_cast<level>(lvl_file);
  if (! logfile.empty()) {
    instance().file_.open(logfile, std::ofstream::out | std::ofstream::app);
    return !! instance().file_;
  }
  return true;
}

logger& logger::instance() {
  static logger l;
  return l;
}

logger::message logger::error() {
  return message{*this, level::error};
}

logger::message logger::info() {
  return message{*this, level::info};
}

logger::message logger::verbose() {
  return message{*this, level::verbose};
}

logger::message logger::massive() {
  return message{*this, level::massive};
}

logger::logger()
  : console_{std::cerr} {
}

} // namespace unit
