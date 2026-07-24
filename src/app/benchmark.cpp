#include "orbitalforge/app/benchmark.hpp"

#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <optional>
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

[[nodiscard]] SimulationConfig
make_simulation_config(const Scenario &scenario) {
  return SimulationConfig{
      .gravitational_constant = scenario.gravitational_constant,
      .softening = scenario.softening,
      .time_step = scenario.time_step,
      .step_count = scenario.step_count,
      .output_interval = scenario.output_interval,
      .integrator = scenario.integrator,
  };
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

BenchmarkResult run_benchmark(const Scenario &scenario,
                              std::size_t body_count) {
  const SystemState initial_state = make_benchmark_system(scenario, body_count);
  SimulationConfig config = make_simulation_config(scenario);
  config.output_interval = config.step_count == 0 ? 1 : config.step_count;

  const SimulationRunner runner;
  const auto start_time = std::chrono::steady_clock::now();
  const auto result = runner.run(initial_state, config);
  const auto end_time = std::chrono::steady_clock::now();
  const std::chrono::duration<double, std::milli> elapsed_time =
      end_time - start_time;
  const double milliseconds_per_step =
      config.step_count == 0
          ? 0.0
          : elapsed_time.count() / static_cast<double>(config.step_count);

  return BenchmarkResult{
      .body_count = result.final_system_state.bodies.size(),
      .elapsed_milliseconds = elapsed_time.count(),
      .milliseconds_per_step = milliseconds_per_step,
  };
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

} // namespace orbitalforge::app
