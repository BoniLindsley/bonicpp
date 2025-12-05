#pragma once

// External dependencies.
#include <argparse/argparse.hpp>

namespace bonicpp::argparse {

class State {
public:
  auto add_arguments(::argparse::ArgumentParser& program) -> void;
  auto process_arguments(const ::argparse::ArgumentParser& program) -> void;

private:
  int help_count = 0;
};

} // namespace bonicpp::argparse
