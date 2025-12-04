// Corresponding header.
#include "argparse.hpp"

// Internal headers.
#include "exception.hpp"

// External dependencies.
#include <argparse/argparse.hpp>

namespace bonicpp::argparse {

auto State::add_arguments(::argparse::ArgumentParser& program) -> void {
  program.add_argument("-h", "--help")
      .action([&](const auto&) { ++help_count; })
      .default_value(false)
      .help("Shows help message and exits.")
      .implicit_value(true)
      .nargs(0);
}

auto State::process_arguments(const ::argparse::ArgumentParser& program)
    -> void {
  if (help_count > 0) {
    std::cerr << program;
    throw exception::SystemExit(0);
  }
}

} // namespace bonicpp::argparse
