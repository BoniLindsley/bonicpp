#pragma once

// External dependencies.
#include <argparse/argparse.hpp>

namespace bonicpp::watchdog {

class State {
public:
  auto add_arguments(::argparse::ArgumentParser& program) -> void;
  auto process_arguments(const ::argparse::ArgumentParser& program)
      -> void;
  auto run() -> int;

private:
  std::string directory;
};

} // namespace bonicpp::watchdog
