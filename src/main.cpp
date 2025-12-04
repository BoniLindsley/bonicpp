// Internal headers.
#include "argparse.hpp"
#include "exception.hpp"
#include "logging.hpp"

// External dependencies.
#include <argparse/argparse.hpp>

namespace {

auto run() -> int { return 0; }

} // namespace

auto main(int argc, char** argv) -> int {
  try {
    auto program_name = "bonicpp";
    auto program_version = "0.0.1";
    argparse::ArgumentParser program(
        program_name, program_version,
        argparse::default_arguments::none);

    auto argparse_state = bonicpp::argparse::State{};
    argparse_state.add_arguments(program);
    auto logging_state = bonicpp::logging::State{};
    logging_state.add_arguments(program);

    try {
      program.parse_args(argc, argv);
    } catch (const std::exception& error) {
      std::cerr << error.what() << "\n";
      std::cerr << program;
      return 1;
    }

    argparse_state.process_arguments(program);
    logging_state.process_arguments(program);

    return run();
  } catch (const bonicpp::exception::SystemExit& error) {
    return error.get_code();
  }
}
