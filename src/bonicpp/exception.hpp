#pragma once

// Standard libraries.
#include <exception>

namespace bonicpp::exception {

class Exception : public std::exception {};

class SystemExit : public Exception {
public:
  SystemExit(int code_) : code(code_) {}
  [[nodiscard]] auto get_code() const -> int { return code; }
private:
  int code = 0;
};

} // namespace bonicpp::exception
