#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <string_view>
#include <utility>
#include <vector>

#include "orbitalforge/app/scenario.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"
#include "orbitalforge/statistics/sample_statistics.hpp"

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

struct BenchmarkBatch {
  std::vector<BenchmarkSample> samples;
  std::uint64_t checksum;
};

struct IntegratorBenchmarkResult {
  simulation::IntegratorKind integrator_kind;
  std::size_t body_count;
  std::size_t step_count;
  statistics::SampleStatistics timing;
  std::uint64_t checksum;
};

enum class BenchmarkSortKind {
  sort,
  stable_sort,
  partial_sort,
  nth_element,
  partition,
};

[[nodiscard]] std::vector<IntegratorBenchmarkResult>
benchmark_all_integrators(const Scenario &scenario, std::size_t body_count,
                          std::size_t warmup_count,
                          std::size_t repetition_count);

[[nodiscard]] std::vector<std::size_t>
parse_benchmark_body_counts(std::string_view text);

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
[[nodiscard]] statistics::SampleStatistics
summarize_benchmark_batch(const BenchmarkBatch &batch);

void sort_integrator_results(std::vector<IntegratorBenchmarkResult> &results);

void stable_sort_integrator_results(
    std::vector<IntegratorBenchmarkResult> &results);

void partial_sort_integrator_results(
    std::vector<IntegratorBenchmarkResult> &results, std::size_t count);

void nth_element_integrator_results(
    std::vector<IntegratorBenchmarkResult> &results, std::size_t index);

[[nodiscard]] std::pair<std::size_t, std::size_t>
find_minmax_integrator_results(
    const std::vector<IntegratorBenchmarkResult> &results);

[[nodiscard]] std::optional<std::size_t> lower_bound_integrator_results(
    const std::vector<IntegratorBenchmarkResult> &results, double mean);

[[nodiscard]] std::size_t partition_integrator_results(
    std::vector<IntegratorBenchmarkResult> &results, double maximum_mean);

[[nodiscard]] BenchmarkSortKind parse_benchmark_sort_kind(std::string_view text);

void apply_benchmark_sort(std::vector<IntegratorBenchmarkResult> &results,
                          BenchmarkSortKind kind);

} // namespace orbitalforge::app
