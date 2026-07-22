#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

#include "orbitalforge/app/scenario.hpp"

namespace orbitalforge::app {

struct BenchmarkResult {
  std::size_t body_count;
  double elapsed_milliseconds;
  double milliseconds_per_step;
};

[[nodiscard]] std::vector<std::size_t>
parse_benchmark_body_counts(std::string_view text);

[[nodiscard]] BenchmarkResult run_benchmark(const Scenario &scenario,
                                            std::size_t body_count);

} // namespace orbitalforge::app
