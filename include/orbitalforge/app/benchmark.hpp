#pragma once

#include "orbitalforge/app/scenario.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

namespace orbitalforge::app {

struct BenchmarkConfig {
  std::size_t warmup_count;
  std::size_t repetition_count;
  std::size_t body_count;
  std::size_t step_count;
  simulation::IntegratorKind integrator_kind;
};

struct BenchmarkSample {
  std::size_t body_count;
  std::size_t step_count;
  simulation::IntegratorKind integrator_kind;
  double elapsed_milliseconds;
  double milliseconds_per_step;
  std::uint64_t checksum;
};

struct BenchmarkResult {
  std::size_t body_count;
  double elapsed_milliseconds;
  double milliseconds_per_step;
};

struct BenchmarkBatch {
  std::vector<BenchmarkSample> samples;
  std::uint64_t checksum;
};

[[nodiscard]] std::vector<std::size_t>
parse_benchmark_body_counts(std::string_view text);

[[nodiscard]] BenchmarkResult run_benchmark(const Scenario &scenario,
                                            std::size_t body_count);

void validate_benchmark_config(const BenchmarkConfig &config);

[[nodiscard]] BenchmarkSample
run_benchmark_sample(const Scenario &scenario, const BenchmarkConfig &config);

template <typename Operation>
[[nodiscard]] BenchmarkBatch
collect_benchmark_samples(std::size_t warmup_count,
                          std::size_t repetition_count, Operation operation) {
  std::uint64_t checksum = 0;

  for (std::size_t index = 0; index < warmup_count; index++) {
    const BenchmarkSample sample = operation();
    checksum ^= sample.checksum;
  }

  std::vector<BenchmarkSample> samples;
  samples.reserve(repetition_count);

  for (std::size_t index = 0; index < repetition_count; ++index) {
    BenchmarkSample sample = operation();
    checksum ^= sample.checksum;
    samples.push_back(std::move(sample));
  }

  return BenchmarkBatch{
      .samples = std::move(samples),
      .checksum = checksum,
  };
}

[[nodiscard]] BenchmarkBatch run_benchmark_batch(const Scenario &scenario,
                                                 const BenchmarkConfig &config);

} // namespace orbitalforge::app
