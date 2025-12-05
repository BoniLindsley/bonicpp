#pragma once

// External dependencies.
#include <argparse/argparse.hpp>

namespace bonicpp::logging {

class State {
public:
  auto add_arguments(::argparse::ArgumentParser& program) -> void;
  auto process_arguments(const ::argparse::ArgumentParser& program) -> void;

private:
  int verbose_count = 0;
};

} // namespace bonicpp::logging
