#include "orbitalforge/app/benchmark.hpp"

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string_view>
#include <utility>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/simulation_runner.hpp"

namespace orbitalforge::app {

namespace {

using math::Vec3;
using physics::Body;
using physics::SystemState;
using simulation::SimulationConfig;
using simulation::SimulationRunner;

[[nodiscard]] std::optional<std::size_t> parse_size(std::string_view text) {
  std::size_t value{};
  const char *begin = text.data();
  const char *end = begin + text.size();
  const auto [pointer, error] = std::from_chars(begin, end, value);

  if (error != std::errc{} || pointer != end || value == 0) {
    return std::nullopt;
  }

  return value;
}

[[nodiscard]] SystemState make_benchmark_system(const Scenario &scenario,
                                                std::size_t body_count) {
  const std::vector<Body> &source_bodies = scenario.initial_state.bodies;

  if (source_bodies.empty()) {
    throw std::invalid_argument{"benchmark scenario contains no bodies"};
  }

  SystemState system;
  system.bodies.reserve(body_count);

  for (std::size_t index = 0; index < body_count; ++index) {
    Body body = source_bodies[index % source_bodies.size()];
    const std::size_t replica = index / source_bodies.size();
    body.name = "Body_" + std::to_string(index);
    body.position += Vec3{0.0, 0.0, static_cast<double>(replica)};
    system.bodies.push_back(std::move(body));
  }

  return system;
}

[[nodiscard]] std::uint64_t
calculate_checksum(const physics::SystemState &system) {
  std::uint64_t checksum = 0;

  for (const physics::Body &body : system.bodies) {
    const auto position_x =
        static_cast<std::uint64_t>(std::abs(body.position.x()) * 1000000.0);

    const auto position_y =
        static_cast<std::uint64_t>(std::abs(body.position.y()) * 1000000.0);

    const auto position_z =
        static_cast<std::uint64_t>(std::abs(body.position.z()) * 1000000.0);

    checksum ^= position_x + 0x9e3779b97f4a7c15ULL + (checksum << 6U) +
                (checksum >> 2U);

    checksum ^= position_y + 0x9e3779b97f4a7c15ULL + (checksum << 6U) +
                (checksum >> 2U);

    checksum ^= position_z + 0x9e3779b97f4a7c15ULL + (checksum << 6U) +
                (checksum >> 2U);
  }

  return checksum;
}

} // namespace

std::vector<std::size_t> parse_benchmark_body_counts(std::string_view text) {
  std::vector<std::size_t> counts;
  std::size_t start = 0;

  while (start < text.size()) {
    const std::size_t comma = text.find(',', start);
    const std::size_t end =
        comma == std::string_view::npos ? text.size() : comma;
    const std::string_view token = text.substr(start, end - start);
    const std::optional<std::size_t> value = parse_size(token);

    if (!value) {
      throw std::invalid_argument{"invalid benchmark size: " +
                                  std::string{token}};
    }

    counts.push_back(*value);

    if (comma == std::string_view::npos) {
      break;
    }

    start = comma + 1;
  }

  if (counts.empty()) {
    throw std::invalid_argument{"benchmark size list must not be empty"};
  }

  return counts;
}

void validate_benchmark_config(const BenchmarkConfig &config) {
  if (config.warmup_count == 0) {
    throw std::invalid_argument{
        "benchmark warmup count must be greater than zero"};
  }

  if (config.repetition_count == 0) {
    throw std::invalid_argument{
        "benchmark repetition count must be greater than zero"};
  }

  if (config.body_count == 0) {
    throw std::invalid_argument{
        "benchmark body count must be greater than zero"};
  }

  if (config.step_count == 0) {
    throw std::invalid_argument{
        "benchmark step count must be greater than zero"};
  }

  const int integrator_value = static_cast<int>(config.integrator_kind);

  if (integrator_value < 0 || integrator_value > 4) {
    throw std::invalid_argument{"benchmark integrator is invalid"};
  }
}

BenchmarkSample run_benchmark_sample(const Scenario &scenario,
                                     const BenchmarkConfig &config) {

  validate_benchmark_config(config);

  const SystemState initial_state =
      make_benchmark_system(scenario, config.body_count);

  SimulationConfig simulation_config{
      .gravitational_constant = scenario.gravitational_constant,
      .softening = scenario.softening,
      .time_step = scenario.time_step,
      .step_count = config.step_count,
      .output_interval = config.step_count,
      .integrator = config.integrator_kind,
  };

  const SimulationRunner runner;

  const auto start_time = std::chrono::steady_clock::now();

  const auto simulation_result = runner.run(initial_state, simulation_config);

  const auto end_time = std::chrono::steady_clock::now();

  const std::chrono::duration<double, std::milli> elapsed_time =
      end_time - start_time;

  const double elapsed_milliseconds = elapsed_time.count();

  const double milliseconds_per_step =
      elapsed_milliseconds / static_cast<double>(config.step_count);

  const std::uint64_t checksum =
      calculate_checksum(simulation_result.final_system_state);

  return BenchmarkSample{
      .body_count = config.body_count,
      .step_count = config.step_count,
      .integrator_kind = config.integrator_kind,
      .elapsed_milliseconds = elapsed_milliseconds,
      .milliseconds_per_step = milliseconds_per_step,
      .checksum = checksum,
  };
}

BenchmarkBatch run_benchmark_batch(const Scenario &scenario,
                                   const BenchmarkConfig &config) {
  validate_benchmark_config(config);

  const auto operation = [&scenario, &config]() {
    return run_benchmark_sample(scenario, config);
  };

  return collect_benchmark_samples(config.warmup_count, config.repetition_count,
                                   operation);
}

statistics::SampleStatistics
summarize_benchmark_batch(const BenchmarkBatch &batch) {
  if (batch.samples.empty()) {
    throw std::invalid_argument{
        "cannot summarize benchmark batch without samples"};
  }

  std::vector<double> elapsed_samples;
  elapsed_samples.reserve(batch.samples.size());

  std::transform(batch.samples.begin(), batch.samples.end(),
                 std::back_inserter(elapsed_samples),
                 [](const BenchmarkSample &sample) {
                   return sample.elapsed_milliseconds;
                 });

  return statistics::summarize_samples(elapsed_samples);
}

std::vector<IntegratorBenchmarkResult>
benchmark_all_integrators(const Scenario &scenario, std::size_t body_count,
                          std::size_t warmup_count,
                          std::size_t repetition_count) {
  constexpr std::array integrators{
      simulation::IntegratorKind::explicit_euler,
      simulation::IntegratorKind::semi_implicit_euler,
      simulation::IntegratorKind::velocity_verlet,
      simulation::IntegratorKind::leapfrog,
      simulation::IntegratorKind::runge_kutta_4,
  };

  std::vector<IntegratorBenchmarkResult> results;
  results.reserve(integrators.size());

  for (const simulation::IntegratorKind integrator_kind : integrators) {
    const BenchmarkConfig config{
        .warmup_count = warmup_count,
        .repetition_count = repetition_count,
        .body_count = body_count,
        .step_count = scenario.step_count,
        .integrator_kind = integrator_kind,
    };

    const BenchmarkBatch batch = run_benchmark_batch(scenario, config);

    results.push_back(IntegratorBenchmarkResult{
        .integrator_kind = integrator_kind,
        .body_count = body_count,
        .step_count = scenario.step_count,
        .timing = summarize_benchmark_batch(batch),
        .checksum = batch.checksum,
    });
  }

  return results;
}

void sort_integrator_results(std::vector<IntegratorBenchmarkResult> &results) {
  std::sort(results.begin(), results.end(),
            [](const IntegratorBenchmarkResult &left,
               const IntegratorBenchmarkResult &right) {
              if (left.timing.mean != right.timing.mean) {
                return left.timing.mean < right.timing.mean;
              }

              return static_cast<int>(left.integrator_kind) <
                     static_cast<int>(right.integrator_kind);
            });
}

void stable_sort_integrator_results(
    std::vector<IntegratorBenchmarkResult> &results) {
  std::stable_sort(
      results.begin(), results.end(),
      [](const IntegratorBenchmarkResult &left,
         const IntegratorBenchmarkResult &right) {
        return left.timing.mean < right.timing.mean;
      });
}

void partial_sort_integrator_results(
    std::vector<IntegratorBenchmarkResult> &results, std::size_t count) {
  const std::size_t middle = std::min(count, results.size());
  std::partial_sort(
      results.begin(), results.begin() + static_cast<std::ptrdiff_t>(middle),
      results.end(), [](const IntegratorBenchmarkResult &left,
                        const IntegratorBenchmarkResult &right) {
        return left.timing.mean < right.timing.mean;
      });
}

void nth_element_integrator_results(
    std::vector<IntegratorBenchmarkResult> &results, std::size_t index) {
  if (index >= results.size()) {
    throw std::out_of_range{"nth element index is outside the results"};
  }

  std::nth_element(
      results.begin(), results.begin() + static_cast<std::ptrdiff_t>(index),
      results.end(), [](const IntegratorBenchmarkResult &left,
                        const IntegratorBenchmarkResult &right) {
        return left.timing.mean < right.timing.mean;
      });
}

std::pair<std::size_t, std::size_t> find_minmax_integrator_results(
    const std::vector<IntegratorBenchmarkResult> &results) {
  if (results.empty()) {
    throw std::invalid_argument{"cannot find min and max of empty results"};
  }

  const auto [minimum, maximum] = std::minmax_element(
      results.begin(), results.end(),
      [](const IntegratorBenchmarkResult &left,
         const IntegratorBenchmarkResult &right) {
        return left.timing.mean < right.timing.mean;
      });

  return {
      static_cast<std::size_t>(std::distance(results.begin(), minimum)),
      static_cast<std::size_t>(std::distance(results.begin(), maximum)),
  };
}

std::optional<std::size_t> lower_bound_integrator_results(
    const std::vector<IntegratorBenchmarkResult> &results, double mean) {
  const auto position = std::lower_bound(
      results.begin(), results.end(), mean,
      [](const IntegratorBenchmarkResult &result, double value) {
        return result.timing.mean < value;
      });

  if (position == results.end()) {
    return std::nullopt;
  }

  return static_cast<std::size_t>(std::distance(results.begin(), position));
}

std::size_t partition_integrator_results(
    std::vector<IntegratorBenchmarkResult> &results, double maximum_mean) {
  const auto boundary = std::partition(
      results.begin(), results.end(),
      [maximum_mean](const IntegratorBenchmarkResult &result) {
        return result.timing.mean <= maximum_mean;
      });

  return static_cast<std::size_t>(std::distance(results.begin(), boundary));
}

BenchmarkSortKind parse_benchmark_sort_kind(std::string_view text) {
  if (text == "sort") return BenchmarkSortKind::sort;
  if (text == "stable-sort") return BenchmarkSortKind::stable_sort;
  if (text == "partial-sort") return BenchmarkSortKind::partial_sort;
  if (text == "nth-element") return BenchmarkSortKind::nth_element;
  if (text == "partition") return BenchmarkSortKind::partition;
  throw std::invalid_argument{"unknown benchmark sort kind"};
}

void apply_benchmark_sort(std::vector<IntegratorBenchmarkResult> &results,
                          BenchmarkSortKind kind) {
  switch (kind) {
  case BenchmarkSortKind::sort:
    sort_integrator_results(results);
    break;
  case BenchmarkSortKind::stable_sort:
    stable_sort_integrator_results(results);
    break;
  case BenchmarkSortKind::partial_sort:
    partial_sort_integrator_results(results, results.size());
    break;
  case BenchmarkSortKind::nth_element:
    nth_element_integrator_results(results, results.size() / 2);
    break;
  case BenchmarkSortKind::partition:
    static_cast<void>(
        partition_integrator_results(results, results.front().timing.mean));
    break;
  }
}

} // namespace orbitalforge::app
