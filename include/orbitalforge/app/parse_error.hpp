#pragma once

#include <cstddef>
#include <string>

namespace orbitalforge::app {

struct ParseError {
  std::size_t line_number{};
  std::string message;
};

} // namespace orbitalforge::app